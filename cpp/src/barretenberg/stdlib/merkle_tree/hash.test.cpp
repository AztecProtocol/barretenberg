#include "hash.hpp"
#include <gtest/gtest.h>
#include "barretenberg/stdlib/hash/pedersen/pedersen.hpp"
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
