#include "./eccvm_verifier.hpp"
#include "barretenberg/honk/flavor/standard.hpp"
#include "barretenberg/honk/transcript/transcript.hpp"
#include "barretenberg/honk/utils/power_polynomial.hpp"
#include "barretenberg/numeric/bitop/get_msb.hpp"

using namespace barretenberg;
using namespace proof_system::honk::sumcheck;

namespace proof_system::honk {
template <typename Flavor>
ECCVMVerifier_<Flavor>::ECCVMVerifier_(std::shared_ptr<typename Flavor::VerificationKey> verifier_key)
    : key(verifier_key)
{}

template <typename Flavor>
ECCVMVerifier_<Flavor>::ECCVMVerifier_(ECCVMVerifier_&& other) noexcept
    : key(std::move(other.key))
    , pcs_verification_key(std::move(other.pcs_verification_key))
{}

template <typename Flavor> ECCVMVerifier_<Flavor>& ECCVMVerifier_<Flavor>::operator=(ECCVMVerifier_&& other) noexcept
{
    key = other.key;
    pcs_verification_key = (std::move(other.pcs_verification_key));
    commitments.clear();
    pcs_fr_elements.clear();
    return *this;
}

/**
 * @brief This function verifies an ECCVM Honk proof for given program settings.
 *
 */
template <typename Flavor> bool ECCVMVerifier_<Flavor>::verify_proof(const plonk::proof& proof)
{
    using FF = typename Flavor::FF;
    using GroupElement = typename Flavor::GroupElement;
    using Commitment = typename Flavor::Commitment;
    using PCSParams = typename Flavor::PCSParams;
    using PCS = typename Flavor::PCS;
    using Gemini = pcs::gemini::MultilinearReductionScheme<PCSParams>;
    using Shplonk = pcs::shplonk::SingleBatchOpeningScheme<PCSParams>;
    using VerifierCommitments = typename Flavor::VerifierCommitments;
    using CommitmentLabels = typename Flavor::CommitmentLabels;

    RelationParameters<FF> relation_parameters;

    transcript = VerifierTranscript<FF>{ proof.proof_data };

    auto commitments = VerifierCommitments(key, transcript);
    auto commitment_labels = CommitmentLabels();

    // TODO(Adrian): Change the initialization of the transcript to take the VK hash?
    const auto circuit_size = transcript.template receive_from_prover<uint32_t>("circuit_size");

    if (circuit_size != key->circuit_size) {
        return false;
    }

    // Get commitments to VM wires
    commitments.q_transcript_add =
        transcript.template receive_from_prover<Commitment>(commitment_labels.q_transcript_add);
    commitments.q_transcript_mul =
        transcript.template receive_from_prover<Commitment>(commitment_labels.q_transcript_mul);
    commitments.q_transcript_eq =
        transcript.template receive_from_prover<Commitment>(commitment_labels.q_transcript_eq);
    commitments.q_transcript_accumulate =
        transcript.template receive_from_prover<Commitment>(commitment_labels.q_transcript_accumulate);
    commitments.q_transcript_msm_transition =
        transcript.template receive_from_prover<Commitment>(commitment_labels.q_transcript_msm_transition);
    commitments.transcript_pc = transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_pc);
    commitments.transcript_msm_count =
        transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_msm_count);
    commitments.transcript_x = transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_x);
    commitments.transcript_y = transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_y);
    commitments.transcript_z1 = transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_z1);
    commitments.transcript_z2 = transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_z2);
    commitments.transcript_z1zero =
        transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_z1zero);
    commitments.transcript_z2zero =
        transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_z2zero);
    commitments.transcript_op = transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_op);
    commitments.transcript_accumulator_x =
        transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_accumulator_x);
    commitments.transcript_accumulator_y =
        transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_accumulator_y);
    commitments.transcript_msm_x =
        transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_msm_x);
    commitments.transcript_msm_y =
        transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_msm_y);
    commitments.table_pc = transcript.template receive_from_prover<Commitment>(commitment_labels.table_pc);
    commitments.table_point_transition =
        transcript.template receive_from_prover<Commitment>(commitment_labels.table_point_transition);
    commitments.table_round = transcript.template receive_from_prover<Commitment>(commitment_labels.table_round);
    commitments.table_scalar_sum =
        transcript.template receive_from_prover<Commitment>(commitment_labels.table_scalar_sum);
    commitments.table_s1 = transcript.template receive_from_prover<Commitment>(commitment_labels.table_s1);
    commitments.table_s2 = transcript.template receive_from_prover<Commitment>(commitment_labels.table_s2);
    commitments.table_s3 = transcript.template receive_from_prover<Commitment>(commitment_labels.table_s3);
    commitments.table_s4 = transcript.template receive_from_prover<Commitment>(commitment_labels.table_s4);
    commitments.table_s5 = transcript.template receive_from_prover<Commitment>(commitment_labels.table_s5);
    commitments.table_s6 = transcript.template receive_from_prover<Commitment>(commitment_labels.table_s6);
    commitments.table_s7 = transcript.template receive_from_prover<Commitment>(commitment_labels.table_s7);
    commitments.table_s8 = transcript.template receive_from_prover<Commitment>(commitment_labels.table_s8);
    commitments.table_skew = transcript.template receive_from_prover<Commitment>(commitment_labels.table_skew);
    commitments.table_dx = transcript.template receive_from_prover<Commitment>(commitment_labels.table_dx);
    commitments.table_dy = transcript.template receive_from_prover<Commitment>(commitment_labels.table_dy);
    commitments.table_tx = transcript.template receive_from_prover<Commitment>(commitment_labels.table_tx);
    commitments.table_ty = transcript.template receive_from_prover<Commitment>(commitment_labels.table_ty);
    commitments.q_msm_transition =
        transcript.template receive_from_prover<Commitment>(commitment_labels.q_msm_transition);
    commitments.msm_q_add = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_q_add);
    commitments.msm_q_double = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_q_double);
    commitments.msm_q_skew = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_q_skew);
    commitments.msm_accumulator_x =
        transcript.template receive_from_prover<Commitment>(commitment_labels.msm_accumulator_x);
    commitments.msm_accumulator_y =
        transcript.template receive_from_prover<Commitment>(commitment_labels.msm_accumulator_y);
    commitments.msm_pc = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_pc);
    commitments.msm_size_of_msm =
        transcript.template receive_from_prover<Commitment>(commitment_labels.msm_size_of_msm);
    commitments.msm_count = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_count);
    commitments.msm_round = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_round);
    commitments.msm_q_add1 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_q_add1);
    commitments.msm_q_add2 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_q_add2);
    commitments.msm_q_add3 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_q_add3);
    commitments.msm_q_add4 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_q_add4);
    commitments.msm_x1 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_x1);
    commitments.msm_y1 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_y1);
    commitments.msm_x2 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_x2);
    commitments.msm_y2 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_y2);
    commitments.msm_x3 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_x3);
    commitments.msm_y3 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_y3);
    commitments.msm_x4 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_x4);
    commitments.msm_y4 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_y4);
    commitments.msm_collision_x1 =
        transcript.template receive_from_prover<Commitment>(commitment_labels.msm_collision_x1);
    commitments.msm_collision_x2 =
        transcript.template receive_from_prover<Commitment>(commitment_labels.msm_collision_x2);
    commitments.msm_collision_x3 =
        transcript.template receive_from_prover<Commitment>(commitment_labels.msm_collision_x3);
    commitments.msm_collision_x4 =
        transcript.template receive_from_prover<Commitment>(commitment_labels.msm_collision_x4);
    commitments.msm_lambda1 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_lambda1);
    commitments.msm_lambda2 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_lambda2);
    commitments.msm_lambda3 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_lambda3);
    commitments.msm_lambda4 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_lambda4);
    commitments.msm_slice1 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_slice1);
    commitments.msm_slice2 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_slice2);
    commitments.msm_slice3 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_slice3);
    commitments.msm_slice4 = transcript.template receive_from_prover<Commitment>(commitment_labels.msm_slice4);
    commitments.transcript_accumulator_empty =
        transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_accumulator_empty);
    commitments.transcript_q_reset_accumulator =
        transcript.template receive_from_prover<Commitment>(commitment_labels.transcript_q_reset_accumulator);
    commitments.q_wnaf = transcript.template receive_from_prover<Commitment>(commitment_labels.q_wnaf);
    commitments.lookup_read_counts_0 =
        transcript.template receive_from_prover<Commitment>(commitment_labels.lookup_read_counts_0);
    commitments.lookup_read_counts_1 =
        transcript.template receive_from_prover<Commitment>(commitment_labels.lookup_read_counts_1);

    // Get challenge for sorted list batching and wire four memory records
    auto [eta, gamma] = transcript.get_challenges("beta", "gamma");
    relation_parameters.gamma = gamma;
    auto eta_sqr = eta * eta;
    relation_parameters.eta = eta;
    relation_parameters.eta_sqr = eta_sqr;
    relation_parameters.eta_cube = eta_sqr * eta;
    relation_parameters.permutation_offset =
        gamma * (gamma + eta_sqr) * (gamma + eta_sqr + eta_sqr) * (gamma + eta_sqr + eta_sqr + eta_sqr);
    relation_parameters.permutation_offset = relation_parameters.permutation_offset.invert();

    // Get commitment to permutation and lookup grand products
    commitments.lookup_inverses =
        transcript.template receive_from_prover<Commitment>(commitment_labels.lookup_inverses);
    commitments.z_perm = transcript.template receive_from_prover<Commitment>(commitment_labels.z_perm);

    // Execute Sumcheck Verifier
    auto sumcheck = Sumcheck<Flavor, VerifierTranscript<FF>>(circuit_size, transcript);

    std::optional sumcheck_output = sumcheck.execute_verifier(relation_parameters);

    // If Sumcheck does not return an output, sumcheck verification has failed
    if (!sumcheck_output.has_value()) {
        return false;
    }

    auto [multivariate_challenge, purported_evaluations] = *sumcheck_output;

    // Execute Gemini/Shplonk verification:

    // Construct inputs for Gemini verifier:
    // - Multivariate opening point u = (u_0, ..., u_{d-1})
    // - batched unshifted and to-be-shifted polynomial commitments
    auto batched_commitment_unshifted = GroupElement::zero();
    auto batched_commitment_to_be_shifted = GroupElement::zero();

    // Compute powers of batching challenge rho
    FF rho = transcript.get_challenge("rho");
    std::vector<FF> rhos = Gemini::powers_of_rho(rho, Flavor::NUM_ALL_ENTITIES);

    // Compute batched multivariate evaluation
    FF batched_evaluation = FF::zero();
    size_t evaluation_idx = 0;
    for (auto& value : purported_evaluations.get_unshifted()) {
        batched_evaluation += value * rhos[evaluation_idx];
        ++evaluation_idx;
    }
    for (auto& value : purported_evaluations.get_shifted()) {
        batched_evaluation += value * rhos[evaluation_idx];
        ++evaluation_idx;
    }

    // Construct batched commitment for NON-shifted polynomials
    size_t commitment_idx = 0;
    for (auto& commitment : commitments.get_unshifted()) {
        // very lazy point at infinity check. not complete. fix.
        if (commitment.y != 0) {
            batched_commitment_unshifted += commitment * rhos[commitment_idx];
        } else {
            std::cout << "point at infinity (unshifted)" << std::endl;
        }
        ++commitment_idx;
    }

    // Construct batched commitment for to-be-shifted polynomials
    for (auto& commitment : commitments.get_to_be_shifted()) {
        // very lazy point at infinity check. not complete. fix.
        if (commitment.y != 0) {
            batched_commitment_to_be_shifted += commitment * rhos[commitment_idx];
        } else {
            std::cout << "point at infinity (to be shifted)" << std::endl;
        }
        ++commitment_idx;
    }

    // Produce a Gemini claim consisting of:
    // - d+1 commitments [Fold_{r}^(0)], [Fold_{-r}^(0)], and [Fold^(l)], l = 1:d-1
    // - d+1 evaluations a_0_pos, and a_l, l = 0:d-1
    auto gemini_claim = Gemini::reduce_verify(multivariate_challenge,
                                              batched_evaluation,
                                              batched_commitment_unshifted,
                                              batched_commitment_to_be_shifted,
                                              transcript);

    // Produce a Shplonk claim: commitment [Q] - [Q_z], evaluation zero (at random challenge z)
    auto shplonk_claim = Shplonk::reduce_verify(gemini_claim, transcript);

    // // Verify the Shplonk claim with KZG or IPA
    return PCS::verify(pcs_verification_key, shplonk_claim, transcript);
}

template class ECCVMVerifier_<honk::flavor::ECCVM>;

} // namespace proof_system::honk
