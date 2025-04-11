#include "recursive_verifier.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>

#include "barretenberg/commitment_schemes/shplonk/shplemini.hpp"
#include "barretenberg/plonk_honk_shared/types/aggregation_object_type.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/polynomials/shared_shifted_virtual_zeroes_array.hpp"
#include "barretenberg/stdlib/primitives/field/field.hpp"
#include "barretenberg/transcript/transcript.hpp"
#include "barretenberg/vm2/common/aztec_constants.hpp"

namespace bb::avm2 {

template <typename Flavor>
AvmRecursiveVerifier_<Flavor>::AvmRecursiveVerifier_(
    Builder& builder, const std::shared_ptr<NativeVerificationKey>& native_verification_key)
    : key(std::make_shared<VerificationKey>(&builder, native_verification_key))
    , builder(builder)
{}

template <typename Flavor>
AvmRecursiveVerifier_<Flavor>::AvmRecursiveVerifier_(Builder& builder, const std::shared_ptr<VerificationKey>& vkey)
    : key(vkey)
    , builder(builder)
{}

// Evaluate the given public input column over the multivariate challenge points
template <typename Flavor>
Flavor::FF AvmRecursiveVerifier_<Flavor>::evaluate_public_input_column(const std::vector<FF>& points,
                                                                       const std::vector<FF>& challenges)
{
    auto coefficients = SharedShiftedVirtualZeroesArray<FF>{
        .start_ = 0,
        .end_ = points.size(),
        .virtual_size_ =
            static_cast<uint32_t>(key->circuit_size.get_value()), // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
        .backing_memory_ = std::static_pointer_cast<FF[]>(get_mem_slab(sizeof(FF) * points.size())),
    };

    memcpy(
        static_cast<void*>(coefficients.data()), static_cast<const void*>(points.data()), sizeof(FF) * points.size());

    return generic_evaluate_mle<FF>(challenges, coefficients);
}

template <typename Flavor>
AvmRecursiveVerifier_<Flavor>::AggregationObject AvmRecursiveVerifier_<Flavor>::verify_proof(
    const HonkProof& proof, const std::vector<std::vector<fr>>& public_inputs_vec_nt, AggregationObject agg_obj)
{
    StdlibProof<Builder> stdlib_proof = convert_native_proof_to_stdlib(&builder, proof);

    std::vector<std::vector<FF>> public_inputs_ct;
    public_inputs_ct.reserve(public_inputs_vec_nt.size());

    for (const auto& vec : public_inputs_vec_nt) {
        std::vector<FF> vec_ct;
        vec_ct.reserve(vec.size());
        for (const auto& el : vec) {
            vec_ct.push_back(stdlib::witness_t<Builder>(&builder, el));
        }
        public_inputs_ct.push_back(vec_ct);
    }

    return verify_proof(stdlib_proof, public_inputs_ct, agg_obj);
}

// TODO(#991): (see https://github.com/AztecProtocol/barretenberg/issues/991)
template <typename Flavor>
AvmRecursiveVerifier_<Flavor>::AggregationObject AvmRecursiveVerifier_<Flavor>::verify_proof(
    const StdlibProof<Builder>& stdlib_proof,
    const std::vector<std::vector<FF>>& public_inputs,
    AggregationObject agg_obj)
{
    using Curve = typename Flavor::Curve;
    using PCS = typename Flavor::PCS;
    using VerifierCommitments = typename Flavor::VerifierCommitments;
    using RelationParams = RelationParameters<typename Flavor::FF>;
    using Transcript = typename Flavor::Transcript;
    using Shplemini = ShpleminiVerifier_<Curve, Flavor::USE_PADDING>;
    using ClaimBatcher = ClaimBatcher_<Curve>;
    using ClaimBatch = ClaimBatcher::Batch;

    transcript = std::make_shared<Transcript>(stdlib_proof);

    RelationParams relation_parameters;
    VerifierCommitments commitments{ key };

    const auto circuit_size = transcript->template receive_from_prover<FF>("circuit_size");
    if (static_cast<uint32_t>(circuit_size.get_value()) != static_cast<uint32_t>(key->circuit_size.get_value())) {
        throw_or_abort("AvmRecursiveVerifier::verify_proof: proof circuit size does not match verification key!");
    }

    // Get commitments to VM wires
    for (auto [comm, label] : zip_view(commitments.get_wires(), commitments.get_wires_labels())) {
        comm = transcript->template receive_from_prover<Commitment>(label);
    }

    auto [beta, gamma] = transcript->template get_challenges<FF>("beta", "gamma");
    relation_parameters.beta = beta;
    relation_parameters.gamma = gamma;

    // Get commitments to inverses
    for (auto [label, commitment] : zip_view(commitments.get_derived_labels(), commitments.get_derived())) {
        commitment = transcript->template receive_from_prover<Commitment>(label);
    }

    // unconstrained
    const size_t log_circuit_size = numeric::get_msb(static_cast<uint32_t>(circuit_size.get_value()));
    auto sumcheck = SumcheckVerifier<Flavor>(log_circuit_size, transcript);

    FF alpha = transcript->template get_challenge<FF>("Sumcheck:alpha");

    auto gate_challenges = std::vector<FF>(log_circuit_size);
    for (size_t idx = 0; idx < log_circuit_size; idx++) {
        gate_challenges[idx] = transcript->template get_challenge<FF>("Sumcheck:gate_challenge_" + std::to_string(idx));
    }

    // No need to constrain that sumcheck_verified is true as this is guaranteed by the implementation of
    // when called over a "circuit field" types.
    SumcheckOutput<Flavor> output = sumcheck.verify(relation_parameters, alpha, gate_challenges);

    vinfo("verified sumcheck: ", (output.verified));

    // Public columns evaluation checks
    std::vector<FF> mle_challenge(output.challenge.begin(),
                                  output.challenge.begin() + static_cast<int>(log_circuit_size));

    // Simplified public input with a single column
    // TODO: Extend to multiple columns once public inputs are finalized
    FF execution_input_evaluation = evaluate_public_input_column(public_inputs[0], mle_challenge);
    execution_input_evaluation.assert_equal(output.claimed_evaluations.execution_input,
                                            "execution_input_evaluation failed");

    // Execute Shplemini rounds.
    ClaimBatcher claim_batcher{
        .unshifted = ClaimBatch{ commitments.get_unshifted(), output.claimed_evaluations.get_unshifted() },
        .shifted = ClaimBatch{ commitments.get_to_be_shifted(), output.claimed_evaluations.get_shifted() }
    };
    const BatchOpeningClaim<Curve> opening_claim = Shplemini::compute_batch_opening_claim(
        log_circuit_size, claim_batcher, output.challenge, Commitment::one(&builder), transcript);

    auto pairing_points = PCS::reduce_verify_batch_opening_claim(opening_claim, transcript);

    pairing_points[0] = pairing_points[0].normalize();
    pairing_points[1] = pairing_points[1].normalize();
    // TODO(https://github.com/AztecProtocol/barretenberg/issues/995): generate this challenge properly.
    typename Curve::ScalarField recursion_separator =
        Curve::ScalarField::from_witness_index(&builder, builder.add_variable(42));
    agg_obj.aggregate(pairing_points, recursion_separator);
    return agg_obj;
}

// TODO: Once Goblinized version is mature, we can remove this one and we only to template
// with MegaCircuitBuilder and therefore we can remove the templat in AvmRecursiveVerifier_.
template class AvmRecursiveVerifier_<AvmRecursiveFlavor_<UltraCircuitBuilder>>;
template class AvmRecursiveVerifier_<AvmRecursiveFlavor_<MegaCircuitBuilder>>;

} // namespace bb::avm2
