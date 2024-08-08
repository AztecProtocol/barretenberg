#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "blake2s.hpp"
#include <barretenberg/common/wasm_export.hpp>
#include <cstring>

using namespace bb;

WASM_EXPORT void blake2s(uint8_t const* data, out_buf32 out)
{
    std::vector<uint8_t> inputv;
    read(data, inputv);
    auto output = bb::crypto::blake2s(inputv);
    std::copy(output.begin(), output.end(), out);
}

WASM_EXPORT void blake2s_to_field(uint8_t const* data, size_t length, uint8_t* r)
{
    std::vector<uint8_t> inputv(data, data + length);
    auto output = bb::crypto::blake2s(inputv);
    auto result = bb::fr::serialize_from_buffer(output.data());
    bb::fr::serialize_to_buffer(result, r);
}

// Underscore to not conflict with old cbind. Remove the above when right.
WASM_EXPORT void blake2s_to_field_(uint8_t const* data, fr::out_buf r)
{
    std::vector<uint8_t> inputv;
    read(data, inputv);
    auto output = bb::crypto::blake2s(inputv);
    auto result = bb::fr::serialize_from_buffer(output.data());
    bb::fr::serialize_to_buffer(result, r);
}

// Operates over input memory that's assumed to be 32 byte slots.
WASM_EXPORT void blackbox_blake2s(uint8_t* in, const size_t length, uint8_t* r)
{
    std::vector<uint8_t> message;
    message.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        message.emplace_back(in[i * 32]);
    }
    const auto output = crypto::blake2s(message);
    memset(r, 0, length * 32);
    for (size_t i = 0; i < 32; ++i) {
        r[i * 32] = output[i];
    }
}