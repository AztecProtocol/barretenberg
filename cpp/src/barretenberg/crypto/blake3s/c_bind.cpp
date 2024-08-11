#include "barretenberg/common/wasm_export.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "blake3s.hpp"

WASM_EXPORT void blake3s_to_field(uint8_t const* data, size_t length, uint8_t* r)
{
    std::vector<uint8_t> inputv(data, data + length);
    std::vector<uint8_t> output = blake3::blake3s(inputv);
    auto result = bb::fr::serialize_from_buffer(output.data());
    bb::fr::serialize_to_buffer(result, r);
}

// Operates over input memory that's assumed to be 32 byte slots.
WASM_EXPORT void blackbox_blake3(uint8_t* in, const size_t length, uint8_t* r)
{
    std::vector<uint8_t> message;
    message.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        message.emplace_back(in[i * 32]);
    }
    const auto output = blake3::blake3s(message);
    memset(r, 0, length * 32);
    for (size_t i = 0; i < 32; ++i) {
        r[i * 32] = output[i];
    }
}