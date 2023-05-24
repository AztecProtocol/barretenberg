#include <cstdint>
#include <cstddef>

namespace acir_proofs {

size_t get_solidity_verifier(uint8_t const* g2x, uint8_t const* vk_buf, uint8_t** output_buf);
uint32_t get_exact_circuit_size(uint8_t const* constraint_system_buf);
uint32_t get_total_circuit_size(uint8_t const* constraint_system_buf);
size_t init_proving_key(uint8_t const* constraint_system_buf, uint8_t const** pk_buf);
size_t init_verification_key(void* pippenger, uint8_t const* g2x, uint8_t const* pk_buf, uint8_t const** vk_buf);
size_t new_proof(void* pippenger,
                 uint8_t const* g2x,
                 uint8_t const* pk_buf,
                 uint8_t const* constraint_system_buf,
                 uint8_t const* witness_buf,
                 uint8_t** proof_data_buf,
                 bool is_recursive);
bool verify_proof(uint8_t const* g2x,
                  uint8_t const* vk_buf,
                  uint8_t const* constraint_system_buf,
                  uint8_t* proof,
                  uint32_t length,
                  bool is_recursive);

// Recursion specific methods
size_t verify_recursive_proof(uint8_t const* proof_buf,
                              uint32_t proof_length,
                              uint8_t const* vk_buf,
                              uint32_t vk_length,
                              uint32_t num_public_inputs,
                              uint8_t const* input_aggregation_obj_buf,
                              uint8_t** output_aggregation_obj_buf);
size_t serialize_verification_key_into_field_elements(uint8_t const* g2x,
                                                      uint8_t const* vk_buf,
                                                      uint8_t** serialized_vk_buf,
                                                      uint8_t** serialized_vk_hash_buf);
size_t serialize_proof_into_field_elements(uint8_t const* proof_data_buf,
                                           uint8_t** serialized_proof_data_buf,
                                           size_t proof_data_length,
                                           size_t num_inner_public_inputs);

} // namespace acir_proofs