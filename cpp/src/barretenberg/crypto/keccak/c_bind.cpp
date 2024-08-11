#include "keccak.hpp"
#include <barretenberg/common/log.hpp>
#include <barretenberg/common/streams.hpp>
#include <barretenberg/common/wasm_export.hpp>
#include <barretenberg/numeric/uint256/uint256.hpp>
#include <cstring>
#include <vector>

// Operates over input memory that's assumed to be 32 byte slots.
WASM_EXPORT void blackbox_keccak1600(uint256_t const* in, const size_t length, uint256_t* r)
{
    std::vector<uint64_t> message;
    message.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        message.emplace_back(in[i]);
    }
    ethash_keccakf1600(message.data());
    // auto output = ethash_keccak256(message.data(), length);
    for (size_t i = 0; i < length; ++i) {
        r[i] = message[i];
    }
}