#include "c_bind.hpp"
#include "aes128.hpp"
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/streams.hpp"
#include "barretenberg/numeric/uint256/uint256.hpp"

WASM_EXPORT void aes_encrypt_buffer_cbc(
    uint8_t const* in, uint8_t const* iv, uint8_t const* key, uint32_t const* length, uint8_t** r)
{
    auto len = ntohl(*length);
    bb::crypto::aes128_encrypt_buffer_cbc((uint8_t*)in, (uint8_t*)iv, key, len);
    std::vector<uint8_t> result(in, in + len);
    *r = to_heap_buffer(result);
}

WASM_EXPORT void aes_decrypt_buffer_cbc(
    uint8_t const* in, uint8_t const* iv, uint8_t const* key, uint32_t const* length, uint8_t** r)
{
    auto len = ntohl(*length);
    bb::crypto::aes128_decrypt_buffer_cbc((uint8_t*)in, (uint8_t*)iv, key, len);
    std::vector<uint8_t> result(in, in + len);
    *r = to_heap_buffer(result);
}

WASM_EXPORT void blackbox_aes_encrypt(uint256_t const* in,
                                      uint256_t const* iv,
                                      uint256_t const* key,
                                      uint64_t const length,
                                      uint256_t* r,
                                      uint256_t* r_size)
{
    auto padded_length = (length + 15) & ~15UL;
    std::vector<uint8_t> in_vec;
    in_vec.reserve(padded_length);
    for (size_t i = 0; i < length; ++i) {
        in_vec.emplace_back(in[i]);
    }
    in_vec.resize(padded_length, (uint8_t)(padded_length - length));
    std::array<uint8_t, 16> iv_arr;
    std::array<uint8_t, 16> key_arr;
    for (size_t i = 0; i < 16; ++i) {
        iv_arr[i] = static_cast<uint8_t>(iv[i].data[0]);
        key_arr[i] = static_cast<uint8_t>(key[i].data[0]);
    }
    bb::crypto::aes128_encrypt_buffer_cbc(in_vec.data(), iv_arr.data(), key_arr.data(), padded_length);

    for (size_t i = 0; i < padded_length; ++i) {
        r[i] = in_vec[i];
    }
    *r_size = padded_length;
}
