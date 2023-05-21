#include <cstdint>
#include <cstddef>
#include <barretenberg/common/wasm_export.hpp>
#include <barretenberg/common/serialize.hpp>
#include <barretenberg/ecc/curves/bn254/fr.hpp>

WASM_EXPORT void acir_new_acir_composer(in_ptr pippenger, uint8_t const* g2x, out_ptr out);

WASM_EXPORT void acir_delete_acir_composer(in_ptr acir_composer_ptr);

WASM_EXPORT void acir_init_proving_key(in_ptr acir_composer_ptr, uint8_t const* constraint_system_buf);

WASM_EXPORT void acir_create_proof(in_ptr acir_composer_ptr, barretenberg::fr::vec_in_buf witness_buf);

WASM_EXPORT void acir_init_verification_key(in_ptr acir_composer_ptr);

WASM_EXPORT void acir_verify_proof(in_ptr acir_composer_ptr, uint8_t const* proof_buf, bool* result);

WASM_EXPORT void acir_get_solidity_verifier(in_ptr acir_composer_ptr, out_str_buf out);

WASM_EXPORT void acir_get_exact_circuit_size(in_ptr acir_composer_ptr, uint32_t* out);

WASM_EXPORT void acir_get_total_circuit_size(in_ptr acir_composer_ptr, uint32_t* out);