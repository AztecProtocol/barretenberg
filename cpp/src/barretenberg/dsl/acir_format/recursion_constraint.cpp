#include "recursion_constraint.hpp"
#include "barretenberg/plonk/proof_system/verification_key/verification_key.hpp"
#include "barretenberg/stdlib/recursion/aggregation_state/aggregation_state.hpp"
#include "barretenberg/stdlib/recursion/verifier/verifier.hpp"
#include "barretenberg/transcript/transcript_wrappers.hpp"

using namespace proof_system::plonk::stdlib::types;

using verification_key_ct = proof_system::plonk::stdlib::recursion::verification_key<bn254>;
using aggregation_state_ct = proof_system::plonk::stdlib::recursion::aggregation_state<bn254>;
using noir_recursive_settings = proof_system::plonk::stdlib::recursion::recursive_ultra_verifier_settings<bn254>;
namespace acir_format {

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
template <bool has_valid_witness_assignment, bool inner_proof_contains_recursive_proof>
void create_recursion_constraints(Composer& composer, const RecursionConstraint& input)
{

    // If we do not have a witness, we must ensure that our dummy witness will not trigger
    // on-curve errors and inverting-zero errors
    {
        // get a fake key/proof that satisfies on-curve + inversion-zero checks
        const std::vector<fr> dummy_key = plonk::verification_key::export_dummy_key_in_recursion_format(
            PolynomialManifest(Composer::type), inner_proof_contains_recursive_proof);
        const auto manifest = Composer::create_unrolled_manifest(1);
        const std::vector<barretenberg::fr> dummy_proof =
            transcript::StandardTranscript::export_dummy_transcript_in_recursion_format(manifest);
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
    // The env_crs should be ok, we use this when generating the constraint system in dsl
    auto env_crs = std::make_unique<proof_system::EnvReferenceStringFactory>();

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

    transcript::Manifest manifest = Composer::create_unrolled_manifest(1);
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
    aggregation_state_ct result = proof_system::plonk::stdlib::recursion::
        verify_proof<bn254, noir_recursive_settings, inner_proof_contains_recursive_proof>(
            &composer, manifest, key_fields, proof_fields, previous_aggregation);
    // Assign the output aggregation object to the proof public inputs (16 field elements representing two
    // G1 points)
    result.add_proof_outputs_as_public_inputs();

    ASSERT(result.public_inputs.size() == 1);

    // Assign the `public_input` field to the public input of the inner proof
    result.public_inputs[0].assert_equal(field_ct::from_witness_index(&composer, input.public_input));

    // Assign the recursive proof outputs to `output_aggregation_object`
    for (size_t i = 0; i < result.proof_witness_indices.size(); ++i) {
        const auto lhs = field_ct::from_witness_index(&composer, result.proof_witness_indices[i]);
        const auto rhs = field_ct::from_witness_index(&composer, input.output_aggregation_object[i]);
        lhs.assert_equal(rhs);
    }
}

template void create_recursion_constraints<false, false>(plonk::stdlib::types::Composer&, const RecursionConstraint&);
template void create_recursion_constraints<false, true>(plonk::stdlib::types::Composer&, const RecursionConstraint&);
template void create_recursion_constraints<true, false>(plonk::stdlib::types::Composer&, const RecursionConstraint&);
template void create_recursion_constraints<true, true>(plonk::stdlib::types::Composer&, const RecursionConstraint&);

} // namespace acir_format
