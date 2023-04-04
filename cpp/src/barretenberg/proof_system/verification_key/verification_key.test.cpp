#include "barretenberg/common/test.hpp"
#include "barretenberg/common/streams.hpp"
#include "barretenberg/numeric/random/engine.hpp"
#include "verification_key.hpp"

namespace {
auto& engine = numeric::random::get_debug_engine();
}

using namespace barretenberg;
using namespace bonk;

/**
 * @brief generated a random vk data for use in tests
 *
 * @return verification_key_data randomly generated
 */
verification_key_data rand_vk_data()
{
    verification_key_data key_data;
    key_data.composer_type = static_cast<uint32_t>(plonk::ComposerType::STANDARD);
    key_data.circuit_size = 1024; // not random - must be power of 2
    key_data.num_public_inputs = engine.get_random_uint32();
    key_data.commitments["test1"] = g1::element::random_element();
    key_data.commitments["test2"] = g1::element::random_element();
    key_data.commitments["foo1"] = g1::element::random_element();
    key_data.commitments["foo2"] = g1::element::random_element();
    return key_data;
}

/**
 * @brief expect that two vk data compressions are equal for a few different hash indices
 *
 * @param key0_data
 * @param key1_data
 */
void expect_compressions_eq(verification_key_data key0_data, verification_key_data key1_data)
{
    // 0 hash index
    EXPECT_EQ(key0_data.compress_native(0), key1_data.compress_native(0));
    // nonzero hash index
    EXPECT_EQ(key0_data.compress_native(15), key1_data.compress_native(15));
}

/**
 * @brief expect that two vk data compressions are not-equal for a few different hash indices
 *
 * @param key0_data
 * @param key1_data
 */
void expect_compressions_ne(verification_key_data key0_data, verification_key_data key1_data)
{
    EXPECT_NE(key0_data.compress_native(0), key1_data.compress_native(0));
    EXPECT_NE(key0_data.compress_native(15), key1_data.compress_native(15));
    // ne hash indeces still lead to ne compressions
    EXPECT_NE(key0_data.compress_native(0), key1_data.compress_native(15));
    EXPECT_NE(key0_data.compress_native(14), key1_data.compress_native(15));
}

TEST(verification_key, buffer_serialization)
{
    verification_key_data key_data = rand_vk_data();

    auto buf = to_buffer(key_data);
    auto result = from_buffer<verification_key_data>(buf);

    EXPECT_EQ(key_data, result);
}

TEST(verification_key, stream_serialization)
{
    verification_key_data key_data = rand_vk_data();

    std::stringstream s;
    write(s, key_data);

    verification_key_data result;
    read(static_cast<std::istream&>(s), result);

    EXPECT_EQ(key_data, result);
}

TEST(verification_key, basic_compression_equality)
{
    verification_key_data key0_data = rand_vk_data();
    verification_key_data key1_data = key0_data; // copy
    expect_compressions_eq(key0_data, key1_data);
}

TEST(verification_key, compression_inequality_index_mismatch)
{
    verification_key_data key0_data = rand_vk_data();
    verification_key_data key1_data = key0_data; // copy
    // inquality on hash index mismatch
    EXPECT_NE(key0_data.compress_native(0), key1_data.compress_native(15));
    EXPECT_NE(key0_data.compress_native(14), key1_data.compress_native(15));
}

TEST(verification_key, compression_inequality_composer_type)
{
    verification_key_data key0_data = rand_vk_data();
    verification_key_data key1_data = key0_data; // copy
    key0_data.composer_type = static_cast<uint32_t>(plonk::ComposerType::PLOOKUP);
    expect_compressions_ne(key0_data, key1_data);
}

TEST(verification_key, compression_inequality_different_circuit_size)
{
    verification_key_data key0_data = rand_vk_data();
    verification_key_data key1_data = key0_data;
    key0_data.circuit_size = 4096;
    expect_compressions_ne(key0_data, key1_data);
}

TEST(verification_key, compression_inequality_different_num_public_inputs)
{
    verification_key_data key0_data = rand_vk_data();
    verification_key_data key1_data = key0_data;
    key0_data.num_public_inputs = 42;
    expect_compressions_ne(key0_data, key1_data);
}

TEST(verification_key, compression_inequality_different_commitments)
{
    verification_key_data key0_data = rand_vk_data();
    verification_key_data key1_data = key0_data;
    key0_data.commitments["test1"] = g1::element::random_element();
    expect_compressions_ne(key0_data, key1_data);
}

TEST(verification_key, compression_inequality_different_num_commitments)
{
    verification_key_data key0_data = rand_vk_data();
    verification_key_data key1_data = key0_data;
    key0_data.commitments["new"] = g1::element::random_element();
    expect_compressions_ne(key0_data, key1_data);
}

TEST(verification_key, compression_equality_different_contains_recursive_proof)
{
    verification_key_data key0_data = rand_vk_data();
    verification_key_data key1_data = key0_data;
    key0_data.contains_recursive_proof = false;
    key1_data.contains_recursive_proof = true;
    expect_compressions_eq(key0_data, key1_data);
}

TEST(verification_key, compression_equality_different_recursive_proof_public_input_indices)
{
    verification_key_data key0_data = rand_vk_data();
    verification_key_data key1_data = key0_data;
    key1_data.recursive_proof_public_input_indices.push_back(42);
    expect_compressions_eq(key0_data, key1_data);
}
