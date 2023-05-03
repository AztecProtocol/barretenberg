
#include "acir_proofs.hpp"
#include "barretenberg/plonk/proof_system/proving_key/serialize.hpp"
#include "barretenberg/dsl/acir_format/acir_format.hpp"
#include "barretenberg/dsl/types.hpp"
#include "barretenberg/srs/reference_string/pippenger_reference_string.hpp"
#include "barretenberg/plonk/proof_system/verification_key/sol_gen.hpp"
#include "barretenberg/stdlib/recursion/verifier/verifier.hpp"

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

    acir_format::Composer composer(proving_key, nullptr);
    auto verification_key =
        acir_format::Composer::compute_verification_key_base(proving_key, crs_factory->get_verifier_crs());

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
        proof.proof_data, acir_format::Composer::create_manifest(1), transcript::HashType::PlookupPedersenBlake3s, 16);

    std::vector<barretenberg::fr> output = transcript.export_transcript_in_recursion_format();

    // NOTE: this output buffer will always have a fixed size! Maybe just precompute?
    const size_t output_size_bytes = output.size() * sizeof(barretenberg::fr);
    auto raw_buf = (uint8_t*)malloc(output_size_bytes);

    // The serialization code below will convert out of Montgomery form before writing to the buffer
    for (size_t i = 0; i < output.size(); ++i) {
        barretenberg::fr::serialize_to_buffer(output[i], &raw_buf[i * 32]);
    }
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
    auto vk_hash_raw_buf = (uint8_t*)malloc(32);

    // The serialization code below will convert out of Montgomery form before writing to the buffer
    for (size_t i = 0; i < output.size() - 1; ++i) {
        barretenberg::fr::serialize_to_buffer(output[i], &raw_buf[i * 32]);
    }
    barretenberg::fr::serialize_to_buffer(output[output.size() - 1], vk_hash_raw_buf);

    // copy the vkey into serialized_vk_buf
    *serialized_vk_buf = raw_buf;

    // copy the vkey hash into serialized_vk_hash_buf
    *serialized_vk_hash_buf = vk_hash_raw_buf;

    return output_size_bytes;
}

size_t new_proof(void* pippenger,
                 uint8_t const* g2x,
                 uint8_t const* pk_buf,
                 uint8_t const* constraint_system_buf,
                 uint8_t const* witness_buf,
                 uint8_t** proof_data_buf,
                 bool is_recursive)
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

    acir_format::Composer composer(proving_key, nullptr);

    create_circuit_with_witness(composer, constraint_system, witness);

    // Either need a context flag for recursive proofs or a new_recursive_proof method that uses regular UltraProver
    if (is_recursive) {
        auto prover = composer.create_prover();
        auto heapProver = new acir_format::RecursiveProver(std::move(prover));
        auto& proof_data = heapProver->construct_proof().proof_data;
        *proof_data_buf = proof_data.data();
        return proof_data.size();
    } else {
        auto prover = composer.create_ultra_with_keccak_prover();
        auto heapProver = new acir_format::Prover(std::move(prover));
        auto& proof_data = heapProver->construct_proof().proof_data;
        *proof_data_buf = proof_data.data();
        return proof_data.size();
    }
}

bool verify_proof(uint8_t const* g2x,
                  uint8_t const* vk_buf,
                  uint8_t const* constraint_system_buf,
                  uint8_t* proof,
                  uint32_t length,
                  bool is_recursive)
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

        acir_format::Composer composer(nullptr, verification_key);
        create_circuit(composer, constraint_system);
        plonk::proof pp = { std::vector<uint8_t>(proof, proof + length) };

        // For inner circuit use recursive prover and verifier, then for outer circuit use the normal prover and
        // verifier.
        // Either need a context flag for recursive verify or a new_recursive_verify_proof method that uses regular
        // UltraVerifier
        if (is_recursive) {
            auto verifier = composer.create_verifier();
            verified = verifier.verify_proof(pp);
        } else {
            auto verifier = composer.create_ultra_with_keccak_verifier();
            verified = verifier.verify_proof(pp);
        }

#ifndef __wasm__
    } catch (const std::exception& e) {
        verified = false;
        info(e.what());
    }
#endif
    return verified;
}

/**
 * @brief Enables simulation of recursion in the ACVM. In order to be able to simulate execute of ACIR,
 *        a DSL needs to be able to insert into a circuit's witness the correct output aggregation state of a recursive
 * verification. The resulting aggergation state is what will be compared against in the recursion constraint.
 *
 * @param proof_buf - The proof to be verified
 * @param proof_length - The length of the proof to enable reading the proof from buffer
 * @param vk_buf - The verification key of the circuit being verified
 * @param vk_length - The length of the verification eky to enable reading the proof from buffer
 * @param public_inputs_buf - The public inputs of the proof being verified. To be included when construct the input
 * aggregation state
 * @param input_aggregation_obj_buf - The fields represnting the two G1 points that must be fed into a pairing.
 *                                    This will be empty if this is the first proof to be recursively.
 * @param output_aggregation_obj_buf - Two G1 points that the top-level verifier needs to run a pairing upon to complete
 * verification
 */
size_t verify_recursive_proof(uint8_t const* proof_buf,
                              uint32_t proof_length,
                              uint8_t const* vk_buf,
                              uint32_t vk_length,
                              uint8_t const* public_inputs_buf,
                              uint8_t const* input_aggregation_obj_buf,
                              uint8_t** output_aggregation_obj_buf)
{
    // TODO: not doing anything with public_inputs_buf right now because we only have one layer of recursion
    // and the previous aggregation state will be empty. When arbitrary depth recursion is available we will have to
    // construct the correct input aggregation_state_ct
    (void)public_inputs_buf;
    (void)input_aggregation_obj_buf;

    acir_format::aggregation_state_ct previous_aggregation;
    previous_aggregation.has_data = false;

    std::vector<acir_format::field_ct> proof_fields(proof_length / 32);
    std::vector<acir_format::field_ct> key_fields(vk_length / 32);
    for (size_t i = 0; i < proof_length / 32; i++) {
        proof_fields[i] = acir_format::field_ct(barretenberg::fr::serialize_from_buffer(&proof_buf[i * 32]));
    }
    for (size_t i = 0; i < vk_length / 32; i++) {
        key_fields[i] = acir_format::field_ct(barretenberg::fr::serialize_from_buffer(&vk_buf[i * 32]));
    }

    acir_format::Composer composer;

    transcript::Manifest manifest = acir_format::Composer::create_unrolled_manifest(1);
    // We currently only support RecursionConstraint where inner_proof_contains_recursive_proof = false.
    // We would either need a separate ACIR opcode where inner_proof_contains_recursive_proof = true,
    // or we need non-witness data to be provided as metadata in the ACIR opcode
    std::shared_ptr<acir_format::verification_key_ct> vkey =
        acir_format::verification_key_ct::template from_field_pt_vector<false>(&composer, key_fields);
    vkey->program_width = acir_format::noir_recursive_settings::program_width;
    acir_format::Transcript_ct transcript(&composer, manifest, proof_fields, 1);
    acir_format::aggregation_state_ct result =
        proof_system::plonk::stdlib::recursion::verify_proof_<acir_format::bn254, acir_format::noir_recursive_settings>(
            &composer, vkey, transcript, previous_aggregation);

    // just writing the output aggregation G1 elements, and no public inputs, proof witnesses, or any other data
    const size_t output_size_bytes = 16 * sizeof(barretenberg::fr);
    auto raw_buf = (uint8_t*)malloc(output_size_bytes);

    for (size_t i = 0; i < 4; ++i) {
        auto field = result.P0.x.binary_basis_limbs[i].element.get_value();
        barretenberg::fr::serialize_to_buffer(field, &raw_buf[i * 32]);
    }

    for (size_t i = 4; i < 8; ++i) {
        auto field = result.P0.y.binary_basis_limbs[i % 4].element.get_value();
        barretenberg::fr::serialize_to_buffer(field, &raw_buf[i * 32]);
    }

    for (size_t i = 8; i < 12; ++i) {
        auto field = result.P1.x.binary_basis_limbs[i % 4].element.get_value();
        barretenberg::fr::serialize_to_buffer(field, &raw_buf[i * 32]);
    }

    for (size_t i = 12; i < 16; ++i) {
        auto field = result.P1.y.binary_basis_limbs[i % 4].element.get_value();
        barretenberg::fr::serialize_to_buffer(field, &raw_buf[i * 32]);
    }

    *output_aggregation_obj_buf = raw_buf;

    return output_size_bytes;
}

} // namespace acir_proofs
