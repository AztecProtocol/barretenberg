#include "c_bind.hpp"
#include "ultra_proofs.hpp"
#include <cstdint>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

// Get the exact circuit size for the constraint system.
WASM_EXPORT uint32_t ultra_get_exact_circuit_size(uint8_t const* constraint_system_buf)
{
    return ultra_proofs::ultra_get_exact_circuit_size(constraint_system_buf);
}

WASM_EXPORT size_t ultra_init_proving_key(uint8_t const* constraint_system_buf, uint8_t const** pk_buf)
{
    return ultra_proofs::ultra_init_proving_key(constraint_system_buf, pk_buf);
}

WASM_EXPORT size_t ultra_init_verification_key(void* pippenger,
                                               uint8_t const* g2x,
                                               uint8_t const* pk_buf,
                                               uint8_t const** vk_buf)
{
    return ultra_proofs::ultra_init_verification_key(pippenger, g2x, pk_buf, vk_buf);
}

WASM_EXPORT size_t ultra_new_proof(void* pippenger,
                                   uint8_t const* g2x,
                                   uint8_t const* pk_buf,
                                   uint8_t const* constraint_system_buf,
                                   uint8_t const* witness_buf,
                                   uint8_t** proof_data_buf)
{
    return ultra_proofs::ultra_new_proof(pippenger, g2x, pk_buf, constraint_system_buf, witness_buf, proof_data_buf);
}

WASM_EXPORT bool ultra_verify_proof(
    uint8_t const* g2x, uint8_t const* vk_buf, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length)
{
    return ultra_proofs::ultra_verify_proof(g2x, vk_buf, constraint_system_buf, proof, length);
}
}
