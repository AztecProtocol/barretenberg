
#include "acir_proofs.hpp"
#include "barretenberg/plonk/proof_system/proving_key/serialize.hpp"
#include "barretenberg/dsl/acir_format/acir_format.hpp"
#include "barretenberg/stdlib/types/types.hpp"
#include "barretenberg/srs/reference_string/pippenger_reference_string.hpp"
#include "barretenberg/plonk/proof_system/verification_key/sol_gen.hpp"

using namespace proof_system::plonk::stdlib::types;

namespace acir_proofs {

size_t get_solidity_verifier(uint8_t const* g2x, uint8_t const* vk_buf, uint8_t** output_buf)
{
    auto crs = std::make_shared<VerifierMemReferenceString>(g2x);
    proof_system::plonk::verification_key_data vk_data;
    read(vk_buf, vk_data);
    auto verification_key = std::make_shared<proof_system::plonk::verification_key>(std::move(vk_data), crs);

    std::ostringstream stream;
    // TODO(blaine): Should we just use "VerificationKey" generically?
    output_vk_sol(stream, verification_key, "UltraVerificationKey");

    auto content_str = stream.str();
    auto raw_buf = (uint8_t*)malloc(content_str.size());
    memcpy(raw_buf, (void*)content_str.data(), content_str.size());
    *output_buf = raw_buf;

    return content_str.size();
}

uint32_t get_exact_circuit_size(uint8_t const* constraint_system_buf)
{
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
    auto crs_factory = std::make_unique<proof_system::ReferenceStringFactory>();
    auto composer = create_circuit(constraint_system, std::move(crs_factory));

    auto num_gates = composer.get_num_gates();
    return static_cast<uint32_t>(num_gates);
}

uint32_t get_total_circuit_size(uint8_t const* constraint_system_buf)
{
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
    auto crs_factory = std::make_unique<proof_system::ReferenceStringFactory>();
    auto composer = create_circuit(constraint_system, std::move(crs_factory));

    return static_cast<uint32_t>(composer.get_total_circuit_size());
}

size_t init_proving_key(uint8_t const* constraint_system_buf, uint8_t const** pk_buf)
{
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);

    // constraint_system.recursion_constraints[0].

    // We know that we don't actually need any CRS to create a proving key, so just feed in a nothing.
    // Hacky, but, right now it needs *something*.
    auto crs_factory = std::make_unique<ReferenceStringFactory>();
    auto composer = create_circuit(constraint_system, std::move(crs_factory));
    auto proving_key = composer.compute_proving_key();

    auto buffer = to_buffer(*proving_key);
    auto raw_buf = (uint8_t*)malloc(buffer.size());
    memcpy(raw_buf, (void*)buffer.data(), buffer.size());
    *pk_buf = raw_buf;

    return buffer.size();
}

size_t init_verification_key(void* pippenger, uint8_t const* g2x, uint8_t const* pk_buf, uint8_t const** vk_buf)
{
    std::shared_ptr<ProverReferenceString> crs;
    plonk::proving_key_data pk_data;
    read(pk_buf, pk_data);
    auto proving_key = std::make_shared<plonk::proving_key>(std::move(pk_data), crs);

    auto crs_factory = std::make_unique<PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    proving_key->reference_string = crs_factory->get_prover_crs(proving_key->circuit_size);

    Composer composer(proving_key, nullptr);
    auto verification_key =
        plonk::stdlib::types::Composer::compute_verification_key_base(proving_key, crs_factory->get_verifier_crs());

    // The composer_type has not yet been set. We need to set the composer_type for when we later read in and
    // construct the verification key so that we have the correct polynomial manifest
    verification_key->composer_type = proof_system::ComposerType::PLOOKUP;

    auto buffer = to_buffer(*verification_key);
    auto raw_buf = (uint8_t*)malloc(buffer.size());
    memcpy(raw_buf, (void*)buffer.data(), buffer.size());
    *vk_buf = raw_buf;

    return buffer.size();
}

/**
 * @brief Takes in a proof buffer and converts into a vector of field elements.
 *        The Recursion opcode requires the proof serialized as a vector of witnesses.
 *        Use this method to get the witness values!
 *
 * @param proof_data_buf
 * @param serialized_proof_data_buf
 */
size_t serialize_proof_into_field_elements(uint8_t const* proof_data_buf,
                                           uint8_t** serialized_proof_data_buf,
                                           size_t proof_data_length)
{
    plonk::proof proof = { std::vector<uint8_t>(proof_data_buf, &proof_data_buf[proof_data_length]) };

    transcript::StandardTranscript transcript(
        proof.proof_data, Composer::create_manifest(1), transcript::HashType::PlookupPedersenBlake3s, 16);

    std::vector<barretenberg::fr> output = transcript.export_transcript_in_recursion_format();

    // NOTE: this output buffer will always have a fixed size! Maybe just precompute?
    const size_t output_size_bytes = output.size() * sizeof(barretenberg::fr);
    auto raw_buf = (uint8_t*)malloc(output_size_bytes);
    // NOTE: currently we dump the fr values into memory in Mongtomery form
    // This is so we don't have to re-convert into Montgomery form when we read these back in.
    // I think this makes it easier to handle the data in barretenberg-sys / aztec-backend.
    // If this is not the case, the commented out serialize code will convert out of Montgomery form before writing to
    // the buffer
    // for (size_t i = 0; i < output.size(); ++i) {
    //     barretenberg::fr::serialize_to_buffer(output[i], &raw_buf[i * 32]);
    // }
    // for (size_t i = 0; i < output.size(); ++i) {
    //     barretenberg::fr::serialize_to_buffer(output[i], &raw_buf[i * 32]);
    // }
    memcpy(raw_buf, (void*)output.data(), output_size_bytes);
    *serialized_proof_data_buf = raw_buf;

    return output_size_bytes;
}

/**
 * @brief Takes in a verification key buffer and converts into a vector of field elements.
 *        The Recursion opcode requires the vk serialized as a vector of witnesses.
 *        Use this method to get the witness values!
 *
 * @param vk_buf
 * @param serialized_vk_buf
 */
size_t serialize_verification_key_into_field_elements(uint8_t const* g2x,
                                                      uint8_t const* vk_buf,
                                                      uint8_t** serialized_vk_buf,
                                                      uint8_t** serialized_vk_hash_buf)
{
    auto crs = std::make_shared<VerifierMemReferenceString>(g2x);
    plonk::verification_key_data vk_data;
    read(vk_buf, vk_data);
    auto vkey = std::make_shared<proof_system::plonk::verification_key>(std::move(vk_data), crs);

    std::vector<barretenberg::fr> output = vkey->export_key_in_recursion_format();

    // NOTE: this output buffer will always have a fixed size! Maybe just precompute?
    // Cut off 32 bytes as last element is the verification key hash which is not part of the key :o
    const size_t output_size_bytes = output.size() * sizeof(barretenberg::fr) - 32;
    auto raw_buf = (uint8_t*)malloc(output_size_bytes);
    // NOTE: currently we dump the fr values into memory in Mongtomery form
    // This is so we don't have to re-convert into Montgomery form when we read these back in.
    // I think this makes it easier to handle the data in barretenberg-sys / aztec-backend.
    // If this is not the case, the commented out serialize code will convert out of Montgomery form before writing to
    // the buffer
    // for (size_t i = 0; i < output.size(); ++i) {
    //     barretenberg::fr::serialize_to_buffer(output[i], &raw_buf[i * 32]);
    // }

    // copy all but the vkey hash into raw_buf
    memcpy(raw_buf, (void*)output.data(), output_size_bytes);
    *serialized_vk_buf = raw_buf;

    // copy the vkey hash into vk_hash_raw_buf
    auto vk_hash_raw_buf = (uint8_t*)malloc(32);
    memcpy(vk_hash_raw_buf, (void*)&output[output.size() - 1], 32);
    *serialized_vk_hash_buf = vk_hash_raw_buf;

    return output_size_bytes;
}

size_t new_proof(void* pippenger,
                 uint8_t const* g2x,
                 uint8_t const* pk_buf,
                 uint8_t const* constraint_system_buf,
                 uint8_t const* witness_buf,
                 uint8_t** proof_data_buf)
{
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);

    std::shared_ptr<ProverReferenceString> crs;
    plonk::proving_key_data pk_data;
    read(pk_buf, pk_data);
    auto proving_key = std::make_shared<plonk::proving_key>(std::move(pk_data), crs);

    auto witness = from_buffer<std::vector<fr>>(witness_buf);

    auto crs_factory = std::make_unique<PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    proving_key->reference_string = crs_factory->get_prover_crs(proving_key->circuit_size);

    auto env_crs = std::make_unique<proof_system::EnvReferenceStringFactory>();

    auto composer = acir_format::create_circuit_with_witness(constraint_system, witness);

    auto prover = composer.create_prover();
    auto verifier = composer.create_verifier();
    auto heapProver = new stdlib::types::Prover(std::move(prover));
    auto& proof_data = heapProver->construct_proof().proof_data;
    *proof_data_buf = proof_data.data();

    return proof_data.size();
}

bool verify_proof(
    uint8_t const* g2x, uint8_t const* vk_buf, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length)
{
    bool verified = false;

#ifndef __wasm__
    try {
#endif
        auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
        auto crs = std::make_shared<VerifierMemReferenceString>(g2x);
        plonk::verification_key_data vk_data;
        read(vk_buf, vk_data);
        auto verification_key = std::make_shared<proof_system::plonk::verification_key>(std::move(vk_data), crs);

        Composer composer(nullptr, verification_key);
        create_circuit(composer, constraint_system);
        plonk::proof pp = { std::vector<uint8_t>(proof, proof + length) };

        auto verifier = composer.create_verifier();

        verified = verifier.verify_proof(pp);
#ifndef __wasm__
    } catch (const std::exception& e) {
        verified = false;
        info(e.what());
    }
#endif
    return verified;
}

} // namespace acir_proofs
