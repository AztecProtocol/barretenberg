#include "c_bind.hpp"
#include "acir_composer.hpp"
#include <cstdint>
#include <memory>
#include "barretenberg/common/net.hpp"
#include "barretenberg/dsl/acir_format/acir_format.hpp"
#include "barretenberg/srs/reference_string/pippenger_reference_string.hpp"

WASM_EXPORT void acir_new_acir_composer(in_ptr pippenger, uint8_t const* g2x, out_ptr out)
{
    auto crs_factory = std::make_shared<proof_system::PippengerReferenceStringFactory>(
        reinterpret_cast<barretenberg::scalar_multiplication::Pippenger*>(*pippenger), g2x);
    *out = new acir_proofs::AcirComposer(crs_factory);
}

WASM_EXPORT void acir_delete_acir_composer(in_ptr acir_composer_ptr)
{
    delete reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
}

WASM_EXPORT void acir_init_proving_key(in_ptr acir_composer_ptr, uint8_t const* constraint_system_buf)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    auto constraint_system_vec = from_buffer<std::vector<uint8_t>>(constraint_system_buf);
    info("length: ", constraint_system_vec.size());
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_vec);
    info("here");
    acir_composer->init_proving_key(std::move(constraint_system));
}

WASM_EXPORT void acir_create_proof(in_ptr acir_composer_ptr, barretenberg::fr::vec_in_buf witness_buf)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    auto witness = from_buffer<std::vector<fr>>(witness_buf);
    acir_composer->create_proof(witness);
}

WASM_EXPORT void acir_init_verification_key(in_ptr acir_composer_ptr)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    acir_composer->init_verification_key();
}

WASM_EXPORT void acir_verify_proof(in_ptr acir_composer_ptr, uint8_t const* proof_buf, bool* result)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    auto proof = from_buffer<std::vector<uint8_t>>(proof_buf);
    *result = acir_composer->verify_proof(proof);
}

WASM_EXPORT void acir_get_solidity_verifier(in_ptr acir_composer_ptr, out_str_buf out)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    *out = to_heap_buffer(acir_composer->get_solidity_verifier());
}

WASM_EXPORT void acir_get_exact_circuit_size(in_ptr acir_composer_ptr, uint32_t* out)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    *out = htonl(acir_composer->get_exact_circuit_size());
}

WASM_EXPORT void acir_get_total_circuit_size(in_ptr acir_composer_ptr, uint32_t* out)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    *out = htonl(acir_composer->get_total_circuit_size());
}