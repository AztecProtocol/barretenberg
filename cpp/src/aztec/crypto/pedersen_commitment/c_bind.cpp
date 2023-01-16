#include "pedersen.hpp"
#include <common/serialize.hpp>
#include <common/timer.hpp>
#include <common/mem.hpp>
#include <common/streams.hpp>
#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void pedersen__init()
{
    crypto::generators::init_generator_data();
}

WASM_EXPORT void pedersen__compress(uint8_t const* inputs_buffer, uint8_t* output)
{
    std::vector<grumpkin::fq> to_compress;
    read(inputs_buffer, to_compress);
    auto r = crypto::pedersen_commitment::compress_native(to_compress);
    barretenberg::fr::serialize_to_buffer(r, output);
}

WASM_EXPORT void pedersen__compress_with_hash_index(uint8_t const* inputs_buffer, uint8_t* output, uint32_t hash_index)
{
    std::vector<grumpkin::fq> to_compress;
    read(inputs_buffer, to_compress);
    auto r = crypto::pedersen_commitment::compress_native(to_compress, hash_index);
    barretenberg::fr::serialize_to_buffer(r, output);
}

WASM_EXPORT void pedersen__buffer_to_field(uint8_t const* data, size_t length, uint8_t* r)
{
    std::vector<uint8_t> to_compress(data, data + length);
    auto output = crypto::pedersen_commitment::compress_native(to_compress);
    write(r, output);
}
}