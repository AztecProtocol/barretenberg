#include "barretenberg/common/streams.hpp"
#include "barretenberg/common/wasm_export.hpp"
#include "sha256.hpp"
#include <string.h>

using namespace bb;

WASM_EXPORT void sha256__hash(uint8_t* in, const size_t length, uint8_t* r)
{
    std::vector<uint8_t> message;
    message.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        message.emplace_back(in[i]);
    }
    const auto output = crypto::sha256(message);
    for (size_t i = 0; i < 32; ++i) {
        r[i] = output[i];
    }
}

// Operates over input memory that's assumed to be 32 byte slots.
WASM_EXPORT void blackbox_sha256(uint8_t* in, const size_t length, uint8_t* r)
{
    std::vector<uint8_t> message;
    message.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        message.emplace_back(in[i * 32]);
    }
    const auto output = crypto::sha256(message);
    memset(r, 0, length * 32);
    for (size_t i = 0; i < 32; ++i) {
        r[i * 32] = output[i];
    }
}
