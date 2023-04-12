#pragma once
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

using namespace barretenberg;

WASM_EXPORT void pedersen_init();

WASM_EXPORT void pedersen_compress_fields(fr::in_buf left, fr::in_buf right, fr::out_buf result);
WASM_EXPORT void pedersen_plookup_compress_fields(fr::in_buf left, fr::in_buf right, fr::out_buf result);

WASM_EXPORT void pedersen_compress(fr::vec_in_buf inputs_buffer, fr::out_buf output);
WASM_EXPORT void pedersen_plookup_compress(fr::vec_in_buf inputs_buffer, fr::out_buf output);

WASM_EXPORT void pedersen_compress_with_hash_index(fr::vec_in_buf inputs_buffer,
                                                   uint32_t const* hash_index,
                                                   fr::out_buf output);

WASM_EXPORT void pedersen_commit(fr::vec_in_buf inputs_buffer, fr::out_buf output);
WASM_EXPORT void pedersen_plookup_commit(fr::vec_in_buf inputs_buffer, fr::out_buf output);

WASM_EXPORT void pedersen_buffer_to_field(uint8_t const* data, fr::out_buf r);
}