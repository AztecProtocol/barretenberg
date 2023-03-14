#include <cstdint>
#include <cstddef>

namespace ultra_proofs {

uint32_t ultra_get_exact_circuit_size(uint8_t const* constraint_system_buf);
size_t ultra_init_proving_key(uint8_t const* constraint_system_buf, uint8_t const** pk_buf);
size_t ultra_init_verification_key(void* pippenger, uint8_t const* g2x, uint8_t const* pk_buf, uint8_t const** vk_buf);
size_t ultra_new_proof(void* pippenger,
                       uint8_t const* g2x,
                       uint8_t const* pk_buf,
                       uint8_t const* constraint_system_buf,
                       uint8_t const* witness_buf,
                       uint8_t** proof_data_buf);
bool ultra_verify_proof(
    uint8_t const* g2x, uint8_t const* vk_buf, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length);

} // namespace ultra_proofs
