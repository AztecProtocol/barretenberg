#include <cstdint>
#include <stddef.h>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void standard_example__init_proving_key();

WASM_EXPORT void composer__init_circuit_def(uint8_t const* constraint_system_buf);

WASM_EXPORT void standard_example__init_verification_key(void* pippenger_ptr, uint8_t const* g2x);

WASM_EXPORT void* standard_example__new_prover(uint8_t const* witness_buf);

WASM_EXPORT void standard_example__delete_prover(void* prover);

WASM_EXPORT bool standard_example__verify_proof(uint8_t* proof, uint32_t length);

WASM_EXPORT uint32_t standard_example__get_exact_circuit_size(uint8_t const* constraint_system_buf);

// Backwards compatibility
WASM_EXPORT uint32_t composer__get_circuit_size(uint8_t const* constraint_system_buf);
WASM_EXPORT uint32_t composer__smart_contract(void* pippenger,
                                              uint8_t const* g2x,
                                              uint8_t const* constraint_system_buf,
                                              uint8_t** output_buf);
WASM_EXPORT size_t composer__new_proof(void* pippenger,
                                       uint8_t const* g2x,
                                       uint8_t const* constraint_system_buf,
                                       uint8_t const* witness_buf,
                                       uint8_t** proof_data_buf);
WASM_EXPORT bool composer__verify_proof(
    void* pippenger, uint8_t const* g2x, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length);

// Construct composer using prover and verifier key buffers
WASM_EXPORT size_t c_init_proving_key(uint8_t const* constraint_system_buf, uint8_t const** pk_buf);
WASM_EXPORT size_t c_init_verification_key(void* pippenger,
                                           uint8_t const* g2x,
                                           uint8_t const* pk_buf,
                                           uint8_t const** vk_buf);
WASM_EXPORT size_t c_new_proof(void* pippenger,
                               uint8_t const* g2x,
                               uint8_t const* pk_buf,
                               uint8_t const* constraint_system_buf,
                               uint8_t const* witness_buf,
                               uint8_t** proof_data_buf);
WASM_EXPORT bool c_verify_proof(
    uint8_t const* g2x, uint8_t const* vk_buf, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length);
}
