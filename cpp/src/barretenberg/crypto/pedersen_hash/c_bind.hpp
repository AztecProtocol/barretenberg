// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#pragma once

#include "barretenberg/common/wasm_export.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"

extern "C" {

using namespace bb;

WASM_EXPORT void pedersen_hash(fr::vec_in_buf inputs_buffer, uint32_t const* hash_index, fr::out_buf output);
WASM_EXPORT void pedersen_hashes(fr::vec_in_buf inputs_buffer, uint32_t const* hash_index, fr::out_buf output);
WASM_EXPORT void pedersen_hash_buffer(uint8_t const* input_buffer, uint32_t const* hash_index, fr::out_buf output);
}