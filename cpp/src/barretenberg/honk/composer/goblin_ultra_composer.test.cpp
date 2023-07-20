#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "barretenberg/common/log.hpp"
#include "barretenberg/honk/composer/ultra_composer.hpp"
#include "barretenberg/honk/proof_system/prover.hpp"
#include "barretenberg/honk/proof_system/ultra_prover.hpp"
#include "barretenberg/honk/sumcheck/relations/permutation_relation.hpp"
#include "barretenberg/honk/sumcheck/relations/relation_parameters.hpp"
#include "barretenberg/honk/sumcheck/sumcheck_round.hpp"
#include "barretenberg/honk/utils/grand_product_delta.hpp"
#include "barretenberg/numeric/uint256/uint256.hpp"
#include "barretenberg/proof_system/circuit_builder/ultra_circuit_builder.hpp"
#include "barretenberg/proof_system/plookup_tables/types.hpp"

using namespace proof_system::honk;

namespace test_ultra_honk_composer {

namespace {
auto& engine = numeric::random::get_debug_engine();
}

class GoblinUltraHonkComposerTests : public ::testing::Test {
  protected:
    static void SetUpTestSuite() { barretenberg::srs::init_crs_factory("../srs_db/ignition"); }
};

/**
 * @brief Test simple circuit with public inputs
 *
 */
TEST_F(GoblinUltraHonkComposerTests, Basic)
{
    auto builder = UltraCircuitBuilder();
    size_t num_gates = 10;

    // Add some arbitrary arithmetic gates that utilize public inputs
    for (size_t i = 0; i < num_gates; ++i) {
        fr a = fr::random_element();
        uint32_t a_idx = builder.add_public_variable(a);

        fr b = fr::random_element();
        fr c = fr::random_element();
        fr d = a + b + c;
        uint32_t b_idx = builder.add_variable(b);
        uint32_t c_idx = builder.add_variable(c);
        uint32_t d_idx = builder.add_variable(d);

        builder.create_big_add_gate({ a_idx, b_idx, c_idx, d_idx, fr(1), fr(1), fr(1), fr(-1), fr(0) });
    }

    auto composer = GoblinUltraComposer();
}

// WORKTODO: flesh this out!
TEST_F(GoblinUltraHonkComposerTests, BasicExecutionTraceOrdering)
{
    auto builder = UltraCircuitBuilder();

    size_t num_ecc_ops = 3;
    size_t num_gates_per_op = 2;
    size_t num_ecc_op_gates = num_gates_per_op * num_ecc_ops;
    size_t num_conventional_gates = 10;
    size_t num_public_inputs = 5;
    size_t num_zero_rows = honk::flavor::GoblinUltra::has_zero_row ? 1 : 0;

    // Add some ecc op gates
    for (size_t i = 0; i < num_ecc_ops; ++i) {
        builder.queue_ecc_add_accum(g1::affine_one);
    }

    // Add some public inputs
    for (size_t i = 0; i < num_public_inputs; ++i) {
        builder.add_public_variable(fr::random_element());
    }

    // Add some conventional gates
    for (size_t i = 0; i < num_conventional_gates; ++i) {
        fr a = fr::random_element();
        fr b = fr::random_element();
        fr c = fr::random_element();
        fr d = a + b + c;
        uint32_t a_idx = builder.add_variable(a);
        uint32_t b_idx = builder.add_variable(b);
        uint32_t c_idx = builder.add_variable(c);
        uint32_t d_idx = builder.add_variable(d);

        builder.create_big_add_gate({ a_idx, b_idx, c_idx, d_idx, fr(1), fr(1), fr(1), fr(-1), fr(0) });
    }

    auto composer = GoblinUltraComposer();
    auto prover = composer.create_prover(builder);
    auto circuit_size = prover.key->circuit_size;

    // Check that the ecc op selector is 1 on the block of ecc op gates and 0 elsewhere
    auto q_ecc_op = prover.key->q_ecc_op_queue;
    for (size_t i = 0; i < circuit_size; ++i) {
        auto val = q_ecc_op[i];
        auto anti_val = q_ecc_op[i] * (-1) + 1; // also needed in realtion
        if (i >= num_zero_rows && i < num_zero_rows + num_ecc_op_gates) {
            EXPECT_EQ(val, 1);
            EXPECT_EQ(anti_val, 0);
        } else {
            EXPECT_EQ(val, 0);
            EXPECT_EQ(anti_val, 1);
        }
    }
}

} // namespace test_ultra_honk_composer
