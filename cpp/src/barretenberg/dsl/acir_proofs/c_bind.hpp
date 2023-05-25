#include <cstdint>
#include <cstddef>
#include <barretenberg/common/wasm_export.hpp>
#include <barretenberg/common/serialize.hpp>
#include <barretenberg/ecc/curves/bn254/fr.hpp>

WASM_EXPORT void acir_new_acir_composer(in_ptr pippenger, uint8_t const* g2x, out_ptr out);

WASM_EXPORT void acir_delete_acir_composer(in_ptr acir_composer_ptr);

WASM_EXPORT void acir_init_proving_key(in_ptr acir_composer_ptr,
                                       uint8_t const* constraint_system_buf,
                                       uint32_t const* size_hint);

/**
 * It would have been nice to just hold onto the constraint_system in the acir_composer, but we can't waste the
 * memory. Being able to reuse the underlying Composer would help as well. But, given the situation, we just have
 * to pass it in everytime.
 */
WASM_EXPORT void acir_create_proof(in_ptr acir_composer_ptr,
                                   uint8_t const* constraint_system_buf,
                                   uint8_t const* witness_buf,
                                   uint8_t** out);

WASM_EXPORT void acir_init_verification_key(in_ptr acir_composer_ptr);

WASM_EXPORT void acir_verify_proof(in_ptr acir_composer_ptr, uint8_t const* proof_buf, bool* result);

WASM_EXPORT void acir_get_solidity_verifier(in_ptr acir_composer_ptr, out_str_buf out);

WASM_EXPORT void acir_get_exact_circuit_size(in_ptr acir_composer_ptr, uint32_t* out);

WASM_EXPORT void acir_get_total_circuit_size(in_ptr acir_composer_ptr, uint32_t* out);

WASM_EXPORT void acir_verify_recursive_proof(in_ptr acir_composer_ptr,
                                             uint8_t const* proof_buf,
                                             uint32_t const* num_inner_public_inputs,
                                             uint8_t** out);

WASM_EXPORT void acir_serialize_proof_into_fields(in_ptr acir_composer_ptr,
                                                  uint8_t const* proof_buf,
                                                  uint32_t const* num_inner_public_inputs,
                                                  uint8_t** out);

WASM_EXPORT void acir_serialize_verification_key_into_fields(in_ptr acir_composer_ptr,
                                                             uint8_t** out_vkey,
                                                             uint8_t** out_key_hash);