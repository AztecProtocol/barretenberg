#include <cstdint>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void ultra_example__init_proving_key();

WASM_EXPORT void ultra_example__init_verification_key(void* pippenger_ptr, uint8_t const* g2x);

WASM_EXPORT void* ultra_example__new_prover();

WASM_EXPORT void ultra_example__delete_prover(void* prover);

WASM_EXPORT bool ultra_example__verify_proof(uint8_t* proof, uint32_t length);

}