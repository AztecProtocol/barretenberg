#include "blake2s.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"

#define WASM_EXPORT __attribute__((visibility("default")))

using namespace barretenberg;

extern "C" {

WASM_EXPORT void blake2s(uint8_t const* data, out_buf32 out)
{
    std::vector<uint8_t> inputv;
    read(data, inputv);
    auto output = blake2::blake2s(inputv);

    write(out, output);
}

WASM_EXPORT void blake2s_to_field(uint8_t const* data, fr::out_buf r)
{
    std::vector<uint8_t> inputv;
    read(data, inputv);
    auto output = blake2::blake2s(inputv);
    auto result = barretenberg::fr::serialize_from_buffer(output.data());
    barretenberg::fr::serialize_to_buffer(result, r);
}
}
