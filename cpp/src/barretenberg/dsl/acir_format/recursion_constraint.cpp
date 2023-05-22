#include "recursion_constraint.hpp"
#include "barretenberg/plonk/proof_system/verification_key/verification_key.hpp"
#include "barretenberg/stdlib/recursion/aggregation_state/aggregation_state.hpp"
#include "barretenberg/stdlib/recursion/verifier/verifier.hpp"
#include "barretenberg/transcript/transcript_wrappers.hpp"

namespace acir_format {

using namespace proof_system::plonk;

void generate_dummy_proof() {}
/**
 * @brief Add constraints required to recursively verify an UltraPlonk proof
 *
 * @param composer
 * @param input
 * @tparam has_valid_witness_assignment. Do we have witnesses or are we just generating keys?
 * @tparam inner_proof_contains_recursive_proof. Do we expect the inner proof to also have performed recursive
 * verification? We need to know this at circuit-compile time.
 *
 * @note We currently only support RecursionConstraint where inner_proof_contains_recursive_proof = false.
 *       We would either need a separate ACIR opcode where inner_proof_contains_recursive_proof = true,
 *       or we need non-witness data to be provided as metadata in the ACIR opcode
 */
template <bool has_valid_witness_assignment>
void create_recursion_constraints(Composer& composer, const RecursionConstraint& input)
{
    const auto& nested_aggregation_indices = input.nested_aggregation_object;
    bool nested_aggregation_indices_all_zero = true;
    for (const auto& idx : nested_aggregation_indices) {
        nested_aggregation_indices_all_zero &= (idx == 0);
    }
    const bool inner_proof_contains_recursive_proof = !nested_aggregation_indices_all_zero;

    // If we do not have a witness, we must ensure that our dummy witness will not trigger
    // on-curve errors and inverting-zero errors
    {
        // get a fake key/proof that satisfies on-curve + inversion-zero checks
        const std::vector<fr> dummy_key = export_dummy_key_in_recursion_format(PolynomialManifest(Composer::type),
                                                                               inner_proof_contains_recursive_proof);
        const auto manifest = Composer::create_unrolled_manifest(input.public_inputs.size());
        const std::vector<barretenberg::fr> dummy_proof =
            transcript::StandardTranscript::export_dummy_transcript_in_recursion_format(
                manifest, inner_proof_contains_recursive_proof);
        for (size_t i = 0; i < input.proof.size(); ++i) {
            const auto proof_field_idx = input.proof[i];
            // if we do NOT have a witness assignment (i.e. are just building the proving/verification keys),
            // we add our dummy proof values as Composer variables.
            // if we DO have a valid witness assignment, we use the real witness assignment
            barretenberg::fr dummy_field =
                has_valid_witness_assignment ? composer.get_variable(proof_field_idx) : dummy_proof[i];
            // Create a copy constraint between our dummy field and the witness index provided by RecursionConstraint.
            // This will make the RecursionConstraint idx equal to `dummy_field`.
            // In the case of a valid witness assignment, this does nothing (as dummy_field = real value)
            // In the case of no valid witness assignment, this makes sure that the RecursionConstraint witness indices
            // will not trigger basic errors (check inputs are on-curve, check we are not inverting 0)
            composer.assert_equal(composer.add_variable(dummy_field), proof_field_idx);
        }
        for (size_t i = 0; i < input.key.size(); ++i) {
            const auto key_field_idx = input.key[i];
            barretenberg::fr dummy_field =
                has_valid_witness_assignment ? composer.get_variable(key_field_idx) : dummy_key[i];
            composer.assert_equal(composer.add_variable(dummy_field), key_field_idx);
        }
    }

    // Construct an in-circuit representation of the verification key.
    // For now, the v-key is a circuit constant and is fixed for the circuit.
    // (We may need a separate recursion opcode for this to vary, or add more config witnesses to this opcode)
    const auto& aggregation_input = input.input_aggregation_object;
    aggregation_state_ct previous_aggregation;

    // If we have previously recursively verified proofs, `is_aggregation_object_nonzero = true`
    // For now this is a complile-time constant i.e. whether this is true/false is fixed for the circuit!
    bool inner_aggregation_indices_all_zero = true;
    for (const auto& idx : aggregation_input) {
        inner_aggregation_indices_all_zero &= (idx == 0);
    }
    if (!inner_aggregation_indices_all_zero) {
        std::array<bn254::fq_ct, 4> aggregation_elements;
        for (size_t i = 0; i < 4; ++i) {
            aggregation_elements[i] =
                bn254::fq_ct(field_ct::from_witness_index(&composer, aggregation_input[4 * i]),
                             field_ct::from_witness_index(&composer, aggregation_input[4 * i + 1]),
                             field_ct::from_witness_index(&composer, aggregation_input[4 * i + 2]),
                             field_ct::from_witness_index(&composer, aggregation_input[4 * i + 3]));
            aggregation_elements[i].assert_is_in_field();
        }
        // If we have a previous aggregation object, assign it to `previous_aggregation` so that it is included
        // in stdlib::recursion::verify_proof
        previous_aggregation.P0 = bn254::g1_ct(aggregation_elements[0], aggregation_elements[1]);
        previous_aggregation.P1 = bn254::g1_ct(aggregation_elements[2], aggregation_elements[3]);
        previous_aggregation.has_data = true;
    } else {
        previous_aggregation.has_data = false;
    }

    transcript::Manifest manifest = Composer::create_unrolled_manifest(input.public_inputs.size());

    std::vector<field_ct> key_fields;
    key_fields.reserve(input.key.size());
    for (const auto& idx : input.key) {
        key_fields.emplace_back(field_ct::from_witness_index(&composer, idx));
    }

    std::vector<field_ct> proof_fields;
    proof_fields.reserve(input.proof.size());
    for (const auto& idx : input.proof) {
        proof_fields.emplace_back(field_ct::from_witness_index(&composer, idx));
    }

    // recursively verify the proof
    std::shared_ptr<verification_key_ct> vkey = verification_key_ct::from_field_pt_vector(
        &composer, key_fields, inner_proof_contains_recursive_proof, nested_aggregation_indices);
    vkey->program_width = noir_recursive_settings::program_width;
    Transcript_ct transcript(&composer, manifest, proof_fields, input.public_inputs.size());
    aggregation_state_ct result = proof_system::plonk::stdlib::recursion::verify_proof_<bn254, noir_recursive_settings>(
        &composer, vkey, transcript, previous_aggregation);

    // Assign correct witness value to the verification key hash
    vkey->compress().assert_equal(field_ct::from_witness_index(&composer, input.key_hash));

    ASSERT(result.public_inputs.size() == input.public_inputs.size());

    // Assign the `public_input` field to the public input of the inner proof
    for (size_t i = 0; i < input.public_inputs.size(); ++i) {
        result.public_inputs[i].assert_equal(field_ct::from_witness_index(&composer, input.public_inputs[i]));
    }

    // Assign the recursive proof outputs to `output_aggregation_object`
    for (size_t i = 0; i < result.proof_witness_indices.size(); ++i) {
        const auto lhs = field_ct::from_witness_index(&composer, result.proof_witness_indices[i]);
        const auto rhs = field_ct::from_witness_index(&composer, input.output_aggregation_object[i]);
        lhs.assert_equal(rhs);
    }
}

template void create_recursion_constraints<false>(Composer&, const RecursionConstraint&);
template void create_recursion_constraints<true>(Composer&, const RecursionConstraint&);

/**
 * @brief When recursively verifying proofs, we represent the verification key using field elements.
 *        This method exports the key formatted in the manner our recursive verifier expects.
 *        NOTE: only used by the dsl at the moment. Might be cleaner to make this a dsl function?
 *
 * @return std::vector<barretenberg::fr>
 */
std::vector<barretenberg::fr> export_key_in_recursion_format(std::shared_ptr<verification_key> const& vkey)
{
    std::vector<fr> output;
    output.emplace_back(vkey->domain.root);
    output.emplace_back(vkey->domain.domain);
    output.emplace_back(vkey->domain.generator);
    output.emplace_back(vkey->circuit_size);
    output.emplace_back(vkey->num_public_inputs);
    output.emplace_back(vkey->contains_recursive_proof);
    for (size_t i = 0; i < 16; ++i) {
        if (vkey->recursive_proof_public_input_indices.size() > i) {
            output.emplace_back(vkey->recursive_proof_public_input_indices[i]);
        } else {
            output.emplace_back(0);
            ASSERT(vkey->contains_recursive_proof == false);
        }
    }
    for (const auto& descriptor : vkey->polynomial_manifest.get()) {
        if (descriptor.source == PolynomialSource::SELECTOR || descriptor.source == PolynomialSource::PERMUTATION) {
            const auto element = vkey->commitments.at(std::string(descriptor.commitment_label));
            const uint256_t x = element.x;
            const uint256_t y = element.y;
            const barretenberg::fr x_lo = x.slice(0, 136);
            const barretenberg::fr x_hi = x.slice(136, 272);
            const barretenberg::fr y_lo = y.slice(0, 136);
            const barretenberg::fr y_hi = y.slice(136, 272);
            output.emplace_back(x_lo);
            output.emplace_back(x_hi);
            output.emplace_back(y_lo);
            output.emplace_back(y_hi);
        }
    }

    verification_key_data vkey_data{
        .composer_type = vkey->composer_type,
        .circuit_size = static_cast<uint32_t>(vkey->circuit_size),
        .num_public_inputs = static_cast<uint32_t>(vkey->num_public_inputs),
        .commitments = vkey->commitments,
        .contains_recursive_proof = vkey->contains_recursive_proof,
        .recursive_proof_public_input_indices = vkey->recursive_proof_public_input_indices,
    };
    output.emplace_back(vkey_data.compress_native(0)); // key_hash
    return output;
}

/**
 * @brief When recursively verifying proofs, we represent the verification key using field elements.
 *        This method exports the key formatted in the manner our recursive verifier expects.
 *        A dummy key is used when building a circuit without a valid witness assignment.
 *        We want the transcript to contain valid G1 points to prevent on-curve errors being thrown.
 *        We want a non-zero circuit size as this element will be inverted by the circuit
 *        and we do not want an "inverting 0" error thrown
 *
 * @return std::vector<barretenberg::fr>
 */
std::vector<barretenberg::fr> export_dummy_key_in_recursion_format(const PolynomialManifest& polynomial_manifest,
                                                                   const bool contains_recursive_proof)
{
    std::vector<fr> output;
    output.emplace_back(1); // domain.domain (will be inverted)
    output.emplace_back(1); // domain.root (will be inverted)
    output.emplace_back(1); // domain.generator (will be inverted)

    output.emplace_back(1); // circuit size
    output.emplace_back(1); // num public inputs

    output.emplace_back(contains_recursive_proof); // contains_recursive_proof
    for (size_t i = 0; i < 16; ++i) {
        output.emplace_back(0); // recursive_proof_public_input_indices
    }

    for (const auto& descriptor : polynomial_manifest.get()) {
        if (descriptor.source == PolynomialSource::SELECTOR || descriptor.source == PolynomialSource::PERMUTATION) {
            // the std::biggroup class creates unsatisfiable constraints when identical points are added/subtracted.
            // (when verifying zk proofs this is acceptable as we make sure verification key points are not identical.
            // And prover points should contain randomness for an honest Prover).
            // This check can also trigger a runtime error due to causing 0 to be inverted.
            // When creating dummy verification key points we must be mindful of the above and make sure that each
            // transcript point is unique.
            auto scalar = barretenberg::fr::random_element();
            const auto element = barretenberg::g1::affine_element(barretenberg::g1::one * scalar);
            const uint256_t x = element.x;
            const uint256_t y = element.y;
            const barretenberg::fr x_lo = x.slice(0, 136);
            const barretenberg::fr x_hi = x.slice(136, 272);
            const barretenberg::fr y_lo = y.slice(0, 136);
            const barretenberg::fr y_hi = y.slice(136, 272);
            output.emplace_back(x_lo);
            output.emplace_back(x_hi);
            output.emplace_back(y_lo);
            output.emplace_back(y_hi);
        }
    }

    output.emplace_back(0); // key_hash

    return output;
}

} // namespace acir_format
