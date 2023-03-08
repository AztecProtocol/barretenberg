#include "hash.hpp"
#include <gtest/gtest.h>
#include "barretenberg/stdlib/hash/pedersen/pedersen.hpp"
#include "barretenberg/stdlib/types/types.hpp"

using namespace barretenberg;
using namespace plonk::stdlib::types;

TEST(stdlib_merkle_tree_hash, compress_native_vs_circuit)
{
    fr x = uint256_t(0x5ec473eb273a8011, 0x50160109385471ca, 0x2f3095267e02607d, 0x02586f4a39e69b86);
    Composer composer = Composer();
    witness_ct y = witness_ct(&composer, x);
    field_ct z = plonk::stdlib::pedersen<Composer>::compress(y, y);
    auto zz = crypto::pedersen::compress_native({ x, x });
    EXPECT_EQ(z.get_value(), zz);
}

TEST(stdlib_merkle_tree_hash, hash_value_native_vs_circuit)
{
    std::vector<uint8_t> x = std::vector<uint8_t>(64, '\1');
    Composer composer = Composer();
    byte_array_ct y(&composer, x);
    field_ct z = merkle_tree::hash_value(y);
    fr zz = merkle_tree::hash_value_native(x);
    EXPECT_EQ(z.get_value(), zz);
}
