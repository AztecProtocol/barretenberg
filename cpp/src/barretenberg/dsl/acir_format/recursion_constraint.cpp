#include "recursion_constraint.hpp"
#include "barretenberg/stdlib/recursion/aggregation_state/aggregation_state.hpp"
#include "barretenberg/stdlib/recursion/verifier/verifier.hpp"

using namespace proof_system::plonk::stdlib::types;

using verification_key_ct = proof_system::plonk::stdlib::recursion::verification_key<bn254>;
using aggregation_state_ct = proof_system::plonk::stdlib::recursion::aggregation_state<bn254>;
using noir_recursive_settings = proof_system::plonk::stdlib::recursion::recursive_ultra_verifier_settings<bn254>;
namespace acir_format {

/**
 * @brief Add constraints required to recursively verify an UltraPlonk proof
 *
 * @param composer
 * @param input
 */
void create_recursion_constraints(Composer& composer, const RecursionConstraint& input)
{
    // The env_crs should be ok, we use this when generating the constraint system in dsl
    auto env_crs = std::make_unique<proof_system::EnvReferenceStringFactory>();

    // Construct an in-circuit representation of the verification key.
    // For now, the v-key is a circuit constant and is fixed for the circuit.
    // (We may need a separate recursion opcode for this to vary, or add more config witnesses to this opcode)
    const auto& aggregation_input = input.input_aggregation_object;
    aggregation_state_ct previous_aggregation;

    // If we have previously recursively verified proofs, `is_aggregation_object_nonzero = true`
    // For now this is a complile-time constant i.e. whether this is true/false is fixed for the circuit!
    if (input.is_aggregation_object_nonzero) {
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
    aggregation_state_ct result = proof_system::plonk::stdlib::recursion::verify_proof<bn254, noir_recursive_settings>(
        &composer, manifest, key_fields, proof_fields, previous_aggregation);

    // Assign the output aggregation object to the proof public inputs (16 field elements representing two G1 points)
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

} // namespace acir_format
