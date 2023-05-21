#include "../bool/bool.hpp"
#include "barretenberg/numeric/uint256/uint256.hpp"
#include "barretenberg/proof_system/types/composer_type.hpp"
#include "logic.hpp"
#include <gtest/gtest.h>
#include "barretenberg/honk/composer/standard_honk_composer.hpp"
#include "barretenberg/plonk/composer/standard_composer.hpp"
#include "barretenberg/plonk/composer/ultra_composer.hpp"
#include "barretenberg/plonk/composer/turbo_composer.hpp"
#include "barretenberg/numeric/random/engine.hpp"

#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

#define STDLIB_TYPE_ALIASES                                                                                            \
    using Composer = TypeParam;                                                                                        \
    using witness_ct = stdlib::witness_t<Composer>;                                                                    \
    using field_ct = stdlib::field_t<Composer>;                                                                        \
    using bool_ct = stdlib::bool_t<Composer>;                                                                          \
    using public_witness_ct = stdlib::public_witness_t<Composer>;

namespace test_stdlib_logic {

namespace {
auto& engine = numeric::random::get_debug_engine();
}

template <class T> void ignore_unused(T&) {} // use to ignore unused variables in lambdas

using namespace barretenberg;
using namespace proof_system::plonk;

template <class Composer> class LogicTest : public testing::Test {};

using ComposerTypes = ::testing::Types<honk::StandardHonkComposer,
                                       plonk::StandardPlonkComposer,
                                       plonk::TurboPlonkComposer,
                                       plonk::UltraPlonkComposer>;

TYPED_TEST_SUITE(LogicTest, ComposerTypes);

TYPED_TEST(LogicTest, TestCorrectLogic)
{
    STDLIB_TYPE_ALIASES

    auto run_test = [](size_t num_bits, Composer& composer) {
        uint256_t mask = (uint256_t(1) << num_bits) - 1;

        uint256_t a = engine.get_random_uint256() & mask;
        uint256_t b = engine.get_random_uint256() & mask;

        uint256_t and_expected = a & b;
        uint256_t xor_expected = a ^ b;

        field_ct x = witness_ct(&composer, a);
        field_ct y = witness_ct(&composer, b);

        field_ct x_const(&composer, a);
        field_ct y_const(&composer, b);

        field_ct and_result = stdlib::logic<Composer>::create_logic_constraint(x, y, num_bits, false);
        field_ct xor_result = stdlib::logic<Composer>::create_logic_constraint(x, y, num_bits, true);

        field_ct and_result_left_constant =
            stdlib::logic<Composer>::create_logic_constraint(x_const, y, num_bits, false);
        field_ct xor_result_left_constant =
            stdlib::logic<Composer>::create_logic_constraint(x_const, y, num_bits, true);

        field_ct and_result_right_constant =
            stdlib::logic<Composer>::create_logic_constraint(x, y_const, num_bits, false);
        field_ct xor_result_right_constant =
            stdlib::logic<Composer>::create_logic_constraint(x, y_const, num_bits, true);

        field_ct and_result_both_constant =
            stdlib::logic<Composer>::create_logic_constraint(x_const, y_const, num_bits, false);
        field_ct xor_result_both_constant =
            stdlib::logic<Composer>::create_logic_constraint(x_const, y_const, num_bits, true);

        EXPECT_EQ(uint256_t(and_result.get_value()), and_expected);
        EXPECT_EQ(uint256_t(and_result_left_constant.get_value()), and_expected);
        EXPECT_EQ(uint256_t(and_result_right_constant.get_value()), and_expected);
        EXPECT_EQ(uint256_t(and_result_both_constant.get_value()), and_expected);

        EXPECT_EQ(uint256_t(xor_result.get_value()), xor_expected);
        EXPECT_EQ(uint256_t(xor_result_left_constant.get_value()), xor_expected);
        EXPECT_EQ(uint256_t(xor_result_right_constant.get_value()), xor_expected);
        EXPECT_EQ(uint256_t(xor_result_both_constant.get_value()), xor_expected);
    };

    auto composer = Composer();
    for (size_t i = 8; i < 248; i += 8) {
        run_test(i, composer);
    }
    auto prover = composer.create_prover();
    plonk::proof proof = prover.construct_proof();
    auto verifier = composer.create_verifier();
    bool result = verifier.verify_proof(proof);
    EXPECT_EQ(result, true);
}

// Tests the constraints will fail if the operands are larger than expected even though the result contains the correct
// number of bits when using the UltraPlonkComposer This is because the range constraints on the right and left operand
// are not being satisfied.
TYPED_TEST(LogicTest, LargeOperands)
{
    STDLIB_TYPE_ALIASES
    auto composer = Composer();

    uint256_t mask = (uint256_t(1) << 48) - 1;
    uint256_t a = engine.get_random_uint256() & mask;
    uint256_t b = engine.get_random_uint256() & mask;

    uint256_t expected_mask = (uint256_t(1) << 40) - 1;
    uint256_t and_expected = (a & b) & expected_mask;
    uint256_t xor_expected = (a ^ b) & expected_mask;

    field_ct x = witness_ct(&composer, a);
    field_ct y = witness_ct(&composer, b);

    field_ct xor_result = stdlib::logic<Composer>::create_logic_constraint(x, y, 40, true);
    field_ct and_result = stdlib::logic<Composer>::create_logic_constraint(x, y, 40, false);
    EXPECT_EQ(uint256_t(and_result.get_value()), and_expected);
    EXPECT_EQ(uint256_t(xor_result.get_value()), xor_expected);

    auto prover = composer.create_prover();
    plonk::proof proof = prover.construct_proof();
    auto verifier = composer.create_verifier();
    bool result = verifier.verify_proof(proof);
    EXPECT_EQ(result, true);
}

// Ensures that malicious witnesses which produce the same result are detected. This potential security issue cannot
// happen if the composer doesn't support lookup gates because constraints will be created for each bit of the left and
// right operand.
TYPED_TEST(LogicTest, DifferentWitnessSameResult)
{

    STDLIB_TYPE_ALIASES
    auto composer = Composer();
    if (Composer::type == ComposerType::PLOOKUP) {
        uint256_t a = 3758096391;
        uint256_t b = 2147483649;
        field_ct x = witness_ct(&composer, uint256_t(a));
        field_ct y = witness_ct(&composer, uint256_t(b));

        uint256_t xor_expected = a ^ b;
        const std::function<std::pair<uint256_t, uint256_t>(uint256_t, uint256_t, size_t)>& get_bad_chunk =
            [](uint256_t left, uint256_t right, size_t chunk_size) {
                (void)left;
                (void)right;
                (void)chunk_size;
                auto left_chunk = uint256_t(2684354565);
                auto right_chunk = uint256_t(3221225475);
                return std::make_pair(left_chunk, right_chunk);
            };

        field_ct xor_result = stdlib::logic<Composer>::create_logic_constraint(x, y, 32, true, get_bad_chunk);
        EXPECT_EQ(uint256_t(xor_result.get_value()), xor_expected);

        auto prover = composer.create_prover();
        plonk::proof proof = prover.construct_proof();
        auto verifier = composer.create_verifier();
        bool result = verifier.verify_proof(proof);
        EXPECT_EQ(result, false);
    }
}

} // namespace test_stdlib_logic