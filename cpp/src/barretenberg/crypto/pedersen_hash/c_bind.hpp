#pragma once
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/timer.hpp"
#include "barretenberg/common/mem.hpp"
#include "barretenberg/common/streams.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#define WASM_EXPORT __attribute__((visibility("default")))

using namespace barretenberg;

extern "C" {

WASM_EXPORT void pedersen_hash__init();

WASM_EXPORT void pedersen__hash_pair(fr::in_buf left, fr::in_buf right, fr::out_buf result);

WASM_EXPORT void pedersen__hash_multiple(fr::vec_in_buf inputs_buffer, fr::out_buf output);

WASM_EXPORT void pedersen__hash_multiple_with_hash_index(fr::vec_in_buf inputs_buffer,
                                                         uint32_t const* hash_index,
                                                         fr::out_buf output);

/**
 * Given a buffer containing 32 byte pedersen leaves, return a new buffer containing the leaves and all pairs of
 * nodes that define a merkle tree.
 * e.g.
 * input:  [1][2][3][4]
 * output: [1][2][3][4][compress(1,2)][compress(3,4)][compress(5,6)]
 */
WASM_EXPORT void pedersen__hash_to_tree(fr::vec_in_buf data, fr::vec_out_buf out);
}