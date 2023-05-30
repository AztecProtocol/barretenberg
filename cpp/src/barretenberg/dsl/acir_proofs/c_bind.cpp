#include "c_bind.hpp"
#include "acir_composer.hpp"
#include <cstdint>
#include <memory>
#include "barretenberg/common/net.hpp"
#include "barretenberg/common/mem.hpp"
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/slab_allocator.hpp"
#include "barretenberg/dsl/acir_format/acir_format.hpp"
#include "barretenberg/srs/global_crs.hpp"

WASM_EXPORT void acir_new_acir_composer(out_ptr out)
{
    *out = new acir_proofs::AcirComposer(barretenberg::srs::get_crs_factory());
}

WASM_EXPORT void acir_delete_acir_composer(in_ptr acir_composer_ptr)
{
    delete reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
}

WASM_EXPORT void acir_create_circuit(in_ptr acir_composer_ptr,
                                     uint8_t const* constraint_system_buf,
                                     uint32_t const* size_hint)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);

    // The binder would normally free the the constraint_system_buf, but we need the memory now.
    free_mem_slab_raw((void*)constraint_system_buf);

    acir_composer->create_circuit(constraint_system, ntohl(*size_hint));
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
                                   bool const* is_recursive,
                                   uint8_t** out)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
    auto witness = from_buffer<std::vector<fr, ContainerSlabAllocator<fr>>>(witness_buf);

    // The binder would normally free the the constraint_system_buf, but we need the memory now.
    free_mem_slab_raw((void*)constraint_system_buf);
    free_mem_slab_raw((void*)witness_buf);

    auto proof_data = acir_composer->create_proof(constraint_system, witness, *is_recursive);
    *out = to_heap_buffer(proof_data);
}

WASM_EXPORT void acir_init_verification_key(in_ptr acir_composer_ptr)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    acir_composer->init_verification_key();
}

WASM_EXPORT void acir_get_verification_key(in_ptr acir_composer_ptr, uint8_t** out)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    auto vk = acir_composer->init_verification_key();
    // to_buffer gives a vector<uint8_t>.
    // to_heap_buffer serializes that into the heap (i.e. length prefixes).
    // if you just did: to_heap_buffer(*vk) you get the un-prefixed buffer. no good.
    *out = to_heap_buffer(to_buffer(*vk));
}

WASM_EXPORT void acir_verify_proof(in_ptr acir_composer_ptr,
                                   uint8_t const* proof_buf,
                                   bool const* is_recursive,
                                   bool* result)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    auto proof = from_buffer<std::vector<uint8_t>>(proof_buf);
    *result = acir_composer->verify_proof(proof, *is_recursive);
}

WASM_EXPORT void acir_get_solidity_verifier(in_ptr acir_composer_ptr, out_str_buf out)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    auto str = acir_composer->get_solidity_verifier();
    *out = to_heap_buffer(str);
}

WASM_EXPORT void acir_get_exact_circuit_size(in_ptr acir_composer_ptr, uint32_t* out)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    *out = htonl((uint32_t)acir_composer->get_exact_circuit_size());
}

WASM_EXPORT void acir_get_total_circuit_size(in_ptr acir_composer_ptr, uint32_t* out)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    *out = htonl((uint32_t)acir_composer->get_total_circuit_size());
}

WASM_EXPORT void acir_serialize_proof_into_fields(in_ptr acir_composer_ptr,
                                                  uint8_t const* proof_buf,
                                                  uint32_t const* num_inner_public_inputs,
                                                  uint8_t** out)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);
    auto proof = from_buffer<std::vector<uint8_t>>(proof_buf);

    auto proof_as_fields = acir_composer->serialize_proof_into_fields(proof, ntohl(*num_inner_public_inputs));

    // NOTE: this output buffer will always have a fixed size! Maybe just precompute?
    std::vector<uint8_t> proof_fields_data;
    // The serialization code below will convert out of Montgomery form before writing to the buffer
    for (size_t i = 0; i < proof_as_fields.size(); ++i) {
        auto proof_field = proof_as_fields[i].to_buffer();
        copy(proof_field.begin(), proof_field.end(), std::back_inserter(proof_fields_data));
    }
    *out = to_heap_buffer(proof_fields_data);
}

WASM_EXPORT void acir_serialize_verification_key_into_fields(in_ptr acir_composer_ptr,
                                                             uint8_t** out_vkey,
                                                             uint8_t** out_key_hash)
{
    auto acir_composer = reinterpret_cast<acir_proofs::AcirComposer*>(*acir_composer_ptr);

    auto vkey_as_fields = acir_composer->serialize_verification_key_into_fields();

    // NOTE: this output buffer will always have a fixed size! Maybe just precompute?
    // Cut off 32 bytes as last element is the verification key hash which is not part of the key :o
    std::vector<uint8_t> vk_fields_data;
    // The serialization code below will convert out of Montgomery form before writing to the buffer
    for (size_t i = 0; i < vkey_as_fields.size() - 1; ++i) {
        auto vk_field = vkey_as_fields[i].to_buffer();
        copy(vk_field.begin(), vk_field.end(), std::back_inserter(vk_fields_data));
    }

    std::vector<uint8_t> vk_hash_data;
    auto vk_hash_field = vkey_as_fields[vkey_as_fields.size() - 1].to_buffer();
    copy(vk_hash_field.begin(), vk_hash_field.end(), std::back_inserter(vk_hash_data));

    // copy the vkey into serialized_vk_buf
    *out_vkey = to_heap_buffer(vk_fields_data);

    // copy the vkey hash into serialized_vk_hash_buf
    *out_key_hash = to_heap_buffer(vk_hash_data);
}
