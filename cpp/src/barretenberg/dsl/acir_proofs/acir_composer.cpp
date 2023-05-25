#include "acir_composer.hpp"
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/throw_or_abort.hpp"
#include "barretenberg/plonk/proof_system/proving_key/serialize.hpp"
#include "barretenberg/dsl/acir_format/acir_format.hpp"
#include "barretenberg/dsl/types.hpp"
#include "barretenberg/srs/reference_string/pippenger_reference_string.hpp"
#include "barretenberg/plonk/proof_system/verification_key/sol_gen.hpp"
#include "barretenberg/srs/reference_string/reference_string.hpp"
#include "barretenberg/stdlib/recursion/verifier/verifier.hpp"

namespace acir_proofs {

using namespace proof_system::plonk;

AcirComposer::AcirComposer(std::shared_ptr<proof_system::ReferenceStringFactory> const& crs_factory)
    : crs_factory_(crs_factory)
    , composer_(0, 0, 0)
{}

void AcirComposer::init_proving_key(acir_format::acir_format& constraint_system, size_t size_hint)
{
    composer_ = create_circuit(constraint_system, crs_factory_, size_hint);

    // We are done with the constraint system at this point, and we need the memory slab back.
    // constraint_system = acir_format::acir_format();
    constraint_system.constraints.clear();
    constraint_system.constraints.shrink_to_fit();

    exact_circuit_size_ = composer_.get_num_gates();
    total_circuit_size_ = composer_.get_total_circuit_size();
    std::cout << "total_circuit_size_: " << total_circuit_size_ << std::endl;
    // Exact or total fed in here?
    circuit_subgroup_size_ = composer_.get_circuit_subgroup_size(total_circuit_size_);
    std::cout << "circuit_subgroup_size_: " << circuit_subgroup_size_ << std::endl;

    proving_key_ = composer_.compute_proving_key();

    std::cout << "proving key circuit_size: " << proving_key_.get()->circuit_size << std::endl;
}

std::vector<uint8_t> AcirComposer::create_proof(acir_format::acir_format& constraint_system,
                                                acir_format::WitnessVector& witness,
                                                bool is_recursive)
{
    composer_ = acir_format::Composer(proving_key_, verification_key_, circuit_subgroup_size_);
    // You can't produce the verification key unless you manually set the crs. Which seems like a bug.
    composer_.crs_factory_ = crs_factory_;
    for (size_t i = 0; i < witness.size(); i++) {
        std::cout << "witness: " << witness[i] << std::endl;
    }

    create_circuit_with_witness(composer_, constraint_system, witness);

    // We are done with the constraint system at this point, and we need the memory slab back.
    constraint_system.constraints.clear();
    constraint_system.constraints.shrink_to_fit();
    witness.clear();
    witness.shrink_to_fit();

    std::cout << "num_gates: " << composer_.num_gates << std::endl;
    std::cout << "circuit_finalised: " << composer_.circuit_finalised << std::endl;

    std::vector<uint8_t> proof;
    if (is_recursive) {
        auto prover = composer_.create_prover();
        proof = prover.construct_proof().proof_data;
    } else {
        auto prover = composer_.create_ultra_with_keccak_prover();
        proof = prover.construct_proof().proof_data;
    }
    return proof;
}

void AcirComposer::init_verification_key()
{
    verification_key_ = composer_.compute_verification_key();
}

bool AcirComposer::verify_proof(std::vector<uint8_t> const& proof, bool is_recursive)
{
    bool verified = false;
    if (is_recursive) {
        auto verifier = composer_.create_verifier();
        verified = verifier.verify_proof({ proof });
    } else {
        auto verifier = composer_.create_ultra_with_keccak_verifier();
        verified = verifier.verify_proof({ proof });
    }
    return verified;
}

std::string AcirComposer::get_solidity_verifier()
{
    std::ostringstream stream;
    output_vk_sol(stream, verification_key_, "UltraVerificationKey");
    return stream.str();
}

std::vector<barretenberg::fr> AcirComposer::verify_recursive_proof(
    std::vector<barretenberg::fr> const& proof,
    std::vector<barretenberg::fr> const& verification_key,
    uint32_t num_public_inputs,
    std::vector<barretenberg::fr> input_aggregation_object)
{
    const size_t NUM_AGGREGATION_ELEMENTS = acir_format::RecursionConstraint::NUM_AGGREGATION_ELEMENTS;

    bool inner_aggregation_all_zero = true;
    for (size_t i = 0; i < input_aggregation_object.size(); i++) {
        inner_aggregation_all_zero &= (input_aggregation_object[i].is_zero());
    }

    acir_format::aggregation_state_ct previous_aggregation;
    if (!inner_aggregation_all_zero) {
        std::array<acir_format::bn254::fq_ct, NUM_AGGREGATION_ELEMENTS> aggregation_elements;
        for (size_t i = 0; i < NUM_AGGREGATION_ELEMENTS; ++i) {
            aggregation_elements[i] = acir_format::bn254::fq_ct(
                acir_format::field_ct(input_aggregation_object[NUM_AGGREGATION_ELEMENTS * i]),
                acir_format::field_ct(input_aggregation_object[NUM_AGGREGATION_ELEMENTS * i + 1]),
                acir_format::field_ct(input_aggregation_object[NUM_AGGREGATION_ELEMENTS * i + 2]),
                acir_format::field_ct(input_aggregation_object[NUM_AGGREGATION_ELEMENTS * i + 3]));
            aggregation_elements[i].assert_is_in_field();
        }
        // If we have a previous aggregation object, assign it to `previous_aggregation` so that it is included
        // in stdlib::recursion::verify_proof
        previous_aggregation.P0 = acir_format::bn254::g1_ct(aggregation_elements[0], aggregation_elements[1]);
        previous_aggregation.P1 = acir_format::bn254::g1_ct(aggregation_elements[2], aggregation_elements[3]);
        previous_aggregation.has_data = true;
    } else {
        previous_aggregation.has_data = false;
    }

    composer_ = acir_format::Composer();

    std::vector<acir_format::field_ct> proof_fields(proof.size());
    std::vector<acir_format::field_ct> key_fields(verification_key.size());
    for (size_t i = 0; i < proof.size(); i++) {
        // TODO(maxim): The stdlib pairing primitives fetch the context from the elements being used in the pairing
        // computation When attempting to simulate recursive verification without a full circuit where the context of
        // these elements are not set we will get a seg fault. Using `witness_ct` here provides a workaround where these
        // elements will have their context set. We should enable the native verifier to return an aggregation state so
        // that we can avoid creating an unnecessary circuit and this workaround
        proof_fields[i] = acir_format::field_ct(acir_format::witness_ct(&composer_, proof[i]));
    }
    for (size_t i = 0; i < verification_key.size(); i++) {
        key_fields[i] = acir_format::field_ct(verification_key[i]);
    }

    transcript::Manifest manifest = acir_format::Composer::create_unrolled_manifest(num_public_inputs);

    std::array<uint32_t, acir_format::RecursionConstraint::AGGREGATION_OBJECT_SIZE> nested_aggregation_object = {};
    for (size_t i = 6; i < 22; ++i) {
        nested_aggregation_object[i - 6] = uint32_t(key_fields[i].get_value());
    }
    bool nested_aggregation_indices_all_zero = true;
    for (const auto& idx : nested_aggregation_object) {
        nested_aggregation_indices_all_zero &= (idx == 0);
    }
    const bool inner_proof_contains_recursive_proof = !nested_aggregation_indices_all_zero;

    std::shared_ptr<acir_format::verification_key_ct> vkey = acir_format::verification_key_ct::from_field_elements(
        &composer_, key_fields, inner_proof_contains_recursive_proof, nested_aggregation_object);

    acir_format::Transcript_ct transcript(&composer_, manifest, proof_fields, num_public_inputs);

    acir_format::aggregation_state_ct result =
        proof_system::plonk::stdlib::recursion::verify_proof_<acir_format::bn254, acir_format::noir_recursive_settings>(
            &composer_, vkey, transcript, previous_aggregation);

    std::vector<barretenberg::fr> output_aggregation_object;
    for (size_t i = 0; i < 4; ++i) {
        auto P0_x = result.P0.x.binary_basis_limbs[i].element.get_value();
        auto P0_y = result.P0.y.binary_basis_limbs[i].element.get_value();
        auto P1_x = result.P1.x.binary_basis_limbs[i].element.get_value();
        auto P1_y = result.P1.y.binary_basis_limbs[i].element.get_value();
        output_aggregation_object.emplace_back(P0_x);
        output_aggregation_object.emplace_back(P0_y);
        output_aggregation_object.emplace_back(P1_x);
        output_aggregation_object.emplace_back(P1_y);
    }

    return output_aggregation_object;
}

std::vector<barretenberg::fr> AcirComposer::serialize_proof_into_fields(std::vector<uint8_t> const& proof,
                                                                        size_t num_inner_public_inputs)
{
    transcript::StandardTranscript transcript(proof,
                                              acir_format::Composer::create_manifest(num_inner_public_inputs),
                                              transcript::HashType::PlookupPedersenBlake3s,
                                              16);

    std::vector<barretenberg::fr> output = acir_format::export_transcript_in_recursion_format(transcript);
    return output;
}

// The composer should already have a verification key initialized
std::vector<barretenberg::fr> AcirComposer::serialize_verification_key_into_fields()
{
    std::vector<barretenberg::fr> output = acir_format::export_key_in_recursion_format(verification_key_);
    return output;
}

} // namespace acir_proofs
