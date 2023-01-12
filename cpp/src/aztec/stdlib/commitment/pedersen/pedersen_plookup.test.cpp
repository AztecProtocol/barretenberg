#include "pedersen.hpp"
#include "pedersen_plookup.hpp"
#include <crypto/pedersen_commitment/pedersen.hpp>
#include <crypto/pedersen_commitment/pedersen_lookup.hpp>
#include <crypto/pedersen_hash/pedersen_lookup.hpp>
#include <ecc/curves/grumpkin/grumpkin.hpp>
#include <numeric/random/engine.hpp>
#include <common/test.hpp>
#include <stdlib/primitives/curves/bn254.hpp>

namespace test_stdlib_pedersen {
using namespace barretenberg;
using namespace plonk;
namespace {
auto& engine = numeric::random::get_debug_engine();
}

namespace plookup_pedersen_tests {
typedef stdlib::field_t<waffle::UltraComposer> field_ct;
typedef stdlib::witness_t<waffle::UltraComposer> witness_ct;
TEST(stdlib_pedersen, test_pedersen_plookup)
{
    waffle::UltraComposer composer = waffle::UltraComposer();

    fr left_in = fr::random_element();
    fr right_in = fr::random_element();

    field_ct left = witness_ct(&composer, left_in);
    field_ct right = witness_ct(&composer, right_in);

    field_ct result = stdlib::pedersen_plookup_commitment<waffle::UltraComposer>::compress(left, right);

    fr expected = crypto::pedersen_hash::lookup::hash_pair(left_in, right_in);

    EXPECT_EQ(result.get_value(), expected);

    auto prover = composer.create_prover();

    printf("composer gates = %zu\n", composer.get_num_gates());
    auto verifier = composer.create_verifier();

    waffle::plonk_proof proof = prover.construct_proof();

    bool proof_result = verifier.verify_proof(proof);
    EXPECT_EQ(proof_result, true);
}

TEST(stdlib_pedersen, test_compress_many_plookup)
{
    waffle::UltraComposer composer = waffle::UltraComposer();

    std::vector<fr> input_values{
        fr::random_element(), fr::random_element(), fr::random_element(),
        fr::random_element(), fr::random_element(), fr::random_element(),
    };
    std::vector<field_ct> inputs;
    for (const auto& input : input_values) {
        inputs.emplace_back(witness_ct(&composer, input));
    }

    const size_t hash_idx = 20;

    field_ct result = stdlib::pedersen_plookup_commitment<waffle::UltraComposer>::compress(inputs, hash_idx);

    auto expected = crypto::pedersen_commitment::lookup::compress_native(input_values, hash_idx);

    EXPECT_EQ(result.get_value(), expected);

    auto prover = composer.create_prover();

    printf("composer gates = %zu\n", composer.get_num_gates());
    auto verifier = composer.create_verifier();

    waffle::plonk_proof proof = prover.construct_proof();

    bool proof_result = verifier.verify_proof(proof);
    EXPECT_EQ(proof_result, true);
}

TEST(stdlib_pedersen, test_merkle_damgard_compress_plookup)
{
    waffle::UltraComposer composer = waffle::UltraComposer();

    std::vector<fr> input_values{
        fr::random_element(), fr::random_element(), fr::random_element(),
        fr::random_element(), fr::random_element(), fr::random_element(),
    };
    std::vector<field_ct> inputs;
    for (const auto& input : input_values) {
        inputs.emplace_back(witness_ct(&composer, input));
    }
    field_ct iv = witness_ct(&composer, fr(10));

    field_ct result = stdlib::pedersen_plookup_commitment<waffle::UltraComposer>::merkle_damgard_compress(inputs, iv).x;

    auto expected = crypto::pedersen_commitment::lookup::merkle_damgard_compress(input_values, 10);

    EXPECT_EQ(result.get_value(), expected.normalize().x);

    auto prover = composer.create_prover();

    printf("composer gates = %zu\n", composer.get_num_gates());
    auto verifier = composer.create_verifier();

    waffle::plonk_proof proof = prover.construct_proof();

    bool proof_result = verifier.verify_proof(proof);
    EXPECT_EQ(proof_result, true);
}
} // namespace plookup_pedersen_tests
} // namespace test_stdlib_pedersen
