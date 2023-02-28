#include <stdlib/types/types.hpp>
#include <dsl/acir_format/acir_format.hpp>

namespace turbo_proofs {

void init_circuit(acir_format::acir_format cs);

void init_proving_key(std::unique_ptr<bonk::ReferenceStringFactory>&& crs_factory);

void init_verification_key(std::unique_ptr<bonk::ReferenceStringFactory>&& crs_factory);

void build_circuit(plonk::stdlib::types::Composer& composer);

TurboProver new_prover(std::vector<fr> witness);

bool verify_proof(plonk::proof const& proof);
// Ideally we want the c_bind file to call a C++ method with C++ arguments
// However, we are getting duplicate definition problems when c_bind imports
// standard_format
// The fix is to just have the c_bind call other c functions
// and define the C++ method, we want to call in the standard_example.cpp file
void c_init_circuit_def(uint8_t const* constraint_system_buf);
uint32_t c_get_circuit_size(uint8_t const* constraint_system_buf);
uint32_t c_get_exact_circuit_size(uint8_t const* constraint_system_buf);
size_t c_composer__new_proof(void* pippenger,
                             uint8_t const* g2x,
                             uint8_t const* constraint_system_buf,
                             uint8_t const* witness_buf,
                             uint8_t** proof_data_buf);
bool c_composer__verify_proof(
    void* pippenger, uint8_t const* g2x, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length);
uint32_t c_composer__smart_contract(void* pippenger,
                                    uint8_t const* g2x,
                                    uint8_t const* constraint_system_buf,
                                    uint8_t** output_buf);

size_t c_init_proving_key(uint8_t const* constraint_system_buf, uint8_t const** pk_buf);
size_t c_init_verification_key(void* pippenger, uint8_t const* g2x, uint8_t const* pk_buf, uint8_t const** vk_buf);
size_t c_new_proof(void* pippenger,
                   uint8_t const* g2x,
                   uint8_t const* pk_buf,
                   uint8_t const* constraint_system_buf,
                   uint8_t const* witness_buf,
                   uint8_t** proof_data_buf);
bool c_verify_proof(
    uint8_t const* g2x, uint8_t const* vk_buf, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length);

} // namespace turbo_proofs
