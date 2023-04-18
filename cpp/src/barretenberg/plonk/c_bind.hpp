#include <cstdint>
#include <barretenberg/common/serialize.hpp>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void plonk_prover_process_queue(in_ptr prover);

WASM_EXPORT void plonk_prover_get_circuit_size(in_ptr prover, uint32_t* out);

WASM_EXPORT void plonk_prover_export_proof(in_ptr prover, uint8_t** proof_data_buf);
}
