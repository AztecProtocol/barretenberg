#include "c_bind.hpp"
#include "acir_composer.hpp"
#include <cstdint>
#include <memory>
#include "barretenberg/common/net.hpp"
#include "barretenberg/common/mem.hpp"
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/slab_allocator.hpp"
#include "barretenberg/dsl/acir_format/acir_format.hpp"
#include "barretenberg/srs/reference_string/pippenger_reference_string.hpp"

WASM_EXPORT void acir_new_acir_composer(in_ptr pippenger, uint8_t const* g2x_buf, out_ptr out)
{
    auto g2x = from_buffer<std::vector<uint8_t>>(g2x_buf);
    auto crs_factory = std::make_shared<proof_system::PippengerReferenceStringFactory>(
        reinterpret_cast<barretenberg::scalar_multiplication::Pippenger*>(*pippenger), g2x.data());
    *out = new acir_proofs::AcirComposer(crs_factory);
}

WASM_EXPORT void acir_delete_acir_composer(in_ptr acir_composer_ptr)
{
    delete reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
}

WASM_EXPORT void acir_init_proving_key(in_ptr acir_composer_ptr,
                                       uint8_t const* constraint_system_buf,
                                       uint32_t const* size_hint)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);

    // The binder would normally free the the constraint_system_buf, but we need the memory now.
    free_mem_slab_raw((void*)constraint_system_buf);

    acir_composer->init_proving_key(constraint_system, ntohl(*size_hint));
}

WASM_EXPORT void acir_create_proof(in_ptr acir_composer_ptr,
                                   uint8_t const* constraint_system_buf,
                                   uint8_t const* witness_buf,
                                   uint8_t** out)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
    auto witness = from_buffer<std::vector<fr, ContainerSlabAllocator<fr>>>(witness_buf);

    // The binder would normally free the the constraint_system_buf, but we need the memory now.
    free_mem_slab_raw((void*)constraint_system_buf);
    free_mem_slab_raw((void*)witness_buf);

    auto proof_data = acir_composer->create_proof(constraint_system, witness);
    *out = to_heap_buffer(proof_data);
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
    *out = htonl((uint32_t)acir_composer->get_exact_circuit_size());
}

WASM_EXPORT void acir_get_total_circuit_size(in_ptr acir_composer_ptr, uint32_t* out)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    *out = htonl(acir_composer->get_total_circuit_size());
}