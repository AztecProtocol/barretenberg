#include <cstdint>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void plonk_prover_process_queue(void const* prover);

WASM_EXPORT void plonk_prover_get_circuit_size(void const* prover, uint32_t* out);

WASM_EXPORT void plonk_prover_export_proof(void const* prover, uint8_t** proof_data_buf);
}
