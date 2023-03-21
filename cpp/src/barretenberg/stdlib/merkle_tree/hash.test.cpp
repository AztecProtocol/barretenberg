#include "hash.hpp"
#include <gtest/gtest.h>
#include "barretenberg/stdlib/hash/pedersen/pedersen.hpp"
#include "barretenberg/stdlib/merkle_tree/membership.hpp"
#include "barretenberg/stdlib/types/types.hpp"

using namespace barretenberg;
using namespace proof_system::plonk::stdlib::types;

TEST(stdlib_merkle_tree_hash, compress_native_vs_circuit)
{
    fr x = uint256_t(0x5ec473eb273a8011, 0x50160109385471ca, 0x2f3095267e02607d, 0x02586f4a39e69b86);
    Composer composer = Composer();
    witness_ct y = witness_ct(&composer, x);
    field_ct z = plonk::stdlib::pedersen_hash<Composer>::hash_multiple({ y, y });
    auto zz = crypto::pedersen_hash::hash_multiple({ x, x }); // uses fixed-base multiplication gate
    if constexpr (Composer::type == ComposerType::PLOOKUP) {
        zz = crypto::pedersen_hash::lookup::hash_multiple({ x, x }); // uses lookup tables
    }

    EXPECT_EQ(z.get_value(), zz);
}

TEST(stdlib_merkle_tree_hash, compute_tree_root_native_vs_circuit)
{
    Composer composer = Composer();
    std::vector<fr> inputs;
    std::vector<field_ct> inputs_ct;
    for (size_t i = 0; i < 16; i++) {
        auto input = fr::random_element();
        auto input_ct = witness_ct(&composer, input);
        inputs.push_back(input);
        inputs_ct.push_back(input_ct);
    }

    field_ct z = plonk::stdlib::merkle_tree::compute_tree_root<Composer>(inputs_ct);
    auto zz = plonk::stdlib::merkle_tree::compute_tree_root_native(inputs);

    EXPECT_EQ(z.get_value(), zz);
}
