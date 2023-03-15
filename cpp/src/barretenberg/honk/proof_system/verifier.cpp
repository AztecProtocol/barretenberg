
#include "./verifier.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/honk/pcs/gemini/gemini.hpp"
#include "barretenberg/honk/pcs/kzg/kzg.hpp"
#include "barretenberg/honk/pcs/shplonk/shplonk_single.hpp"
#include "barretenberg/honk/sumcheck/sumcheck.hpp"
#include "barretenberg/honk/sumcheck/relations/arithmetic_relation.hpp"
#include "barretenberg/honk/sumcheck/relations/grand_product_computation_relation.hpp"
#include "barretenberg/honk/sumcheck/relations/grand_product_initialization_relation.hpp"
#include "barretenberg/honk/utils/public_inputs.hpp"
#include "barretenberg/numeric/bitop/get_msb.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"
#include "barretenberg/ecc/curves/bn254/scalar_multiplication/scalar_multiplication.hpp"

#include <string>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace honk {
template <typename program_settings>
Verifier<program_settings>::Verifier(std::shared_ptr<bonk::verification_key> verifier_key,
                                     const transcript::Manifest& input_manifest)
    : manifest(input_manifest)
    , key(verifier_key)
{}

template <typename program_settings>
Verifier<program_settings>::Verifier(Verifier&& other)
    : manifest(other.manifest)
    , key(other.key)
    , kate_verification_key(std::move(other.kate_verification_key))
{}

template <typename program_settings> Verifier<program_settings>& Verifier<program_settings>::operator=(Verifier&& other)
{
    key = other.key;
    manifest = other.manifest;
    kate_verification_key = (std::move(other.kate_verification_key));
    kate_g1_elements.clear();
    kate_fr_elements.clear();
    return *this;
}

/**
* @brief This function verifies a Honk proof for given program settings.
*
* @details A Standard Honk proof contains the following:
    Multilinear evaluations:
        w_i(X),        i = 1,2,3
        sigma_i(X),    i = 1,2,3
        q_i(X),        i = 1,2,3,4,5
        z_perm(X),
        L_0(X),
        id(X)

    Univariate evaluations:
        a_0 = Fold_{-r}^(0)(-r),
        a_l = Fold^(l)(-r^{2^l}), i = 1,...,d-1

    Univariate polynomials (evaluations over MAX_RELATION_LENGTH-many points):
        S_l, l = 0,...,d-1

    Commitments:
        [w_i]_1,        i = 1,2,3
        [z_perm]_1,
        [Fold^(l)]_1,   l = 1,...,d-1
        [Q]_1,
        [W]_1
*/
template <typename program_settings> bool Verifier<program_settings>::verify_proof(const plonk::proof& proof)
{
    using FF = typename program_settings::fr;
    using Commitment = barretenberg::g1::element;
    using CommitmentAffine = barretenberg::g1::affine_element;

    using Sumcheck = sumcheck::Sumcheck<FF,
                                        sumcheck::ArithmeticRelation,
                                        sumcheck::GrandProductComputationRelation,
                                        sumcheck::GrandProductInitializationRelation>;
    using Gemini = pcs::gemini::MultilinearReductionScheme<pcs::kzg::Params>;
    using Shplonk = pcs::shplonk::SingleBatchOpeningScheme<pcs::kzg::Params>;
    using KZG = pcs::kzg::UnivariateOpeningScheme<pcs::kzg::Params>;

    const size_t NUM_POLYNOMIALS = bonk::StandardArithmetization::NUM_POLYNOMIALS;
    const size_t NUM_UNSHIFTED = bonk::StandardArithmetization::NUM_UNSHIFTED_POLYNOMIALS;
    const size_t NUM_PRECOMPUTED = bonk::StandardArithmetization::NUM_PRECOMPUTED_POLYNOMIALS;

    constexpr auto width = program_settings::program_width;

    const auto circuit_size = static_cast<uint32_t>(key->circuit_size);
    const auto num_public_inputs = static_cast<uint32_t>(key->num_public_inputs);
    const size_t log_n(numeric::get_msb(circuit_size));
    // Add the proof data to the transcript, according to the manifest. Also initialise the transcript's hash type
    // and challenge bytes.
    auto transcript = transcript::StandardTranscript(
        proof.proof_data, manifest, program_settings::hash_type, program_settings::num_challenge_bytes);

    // Add the circuit size and the number of public inputs) to the transcript.
    transcript.add_element("circuit_size",
                           { static_cast<uint8_t>(circuit_size >> 24),
                             static_cast<uint8_t>(circuit_size >> 16),
                             static_cast<uint8_t>(circuit_size >> 8),
                             static_cast<uint8_t>(circuit_size) });

    transcript.add_element("public_input_size",
                           { static_cast<uint8_t>(num_public_inputs >> 24),
                             static_cast<uint8_t>(num_public_inputs >> 16),
                             static_cast<uint8_t>(num_public_inputs >> 8),
                             static_cast<uint8_t>(num_public_inputs) });

    // Compute challenges from the proof data, based on the manifest, using the Fiat-Shamir heuristic
    transcript.apply_fiat_shamir("init");
    transcript.apply_fiat_shamir("eta");
    transcript.apply_fiat_shamir("beta");
    transcript.apply_fiat_shamir("alpha");
    for (size_t idx = 0; idx < log_n; idx++) {
        transcript.apply_fiat_shamir("u_" + std::to_string(idx));
    }
    transcript.apply_fiat_shamir("rho");
    transcript.apply_fiat_shamir("r");
    transcript.apply_fiat_shamir("nu");
    transcript.apply_fiat_shamir("z");
    transcript.apply_fiat_shamir("separator");

    std::vector<FF> public_inputs = many_from_buffer<FF>(transcript.get_element("public_inputs"));
    ASSERT(public_inputs.size() == num_public_inputs);

    // Get commitments to the wires
    std::array<CommitmentAffine, width> wire_commitments;
    for (size_t i = 0; i < width; ++i) {
        wire_commitments[i] =
            transcript.get_group_element(bonk::StandardArithmetization::ENUM_TO_COMM[NUM_PRECOMPUTED + i]);
    }

    // Get commitment to Z_PERM
    auto z_permutation_commitment = transcript.get_group_element("Z_PERM");

    // Get permutation challenges
    const FF beta = FF::serialize_from_buffer(transcript.get_challenge("beta").begin());
    const FF gamma = FF::serialize_from_buffer(transcript.get_challenge("beta", 1).begin());
    const FF public_input_delta = compute_public_input_delta<FF>(public_inputs, beta, gamma, circuit_size);

    sumcheck::RelationParameters<FF> relation_parameters{
        .beta = beta,
        .gamma = gamma,
        .public_input_delta = public_input_delta,
    };

    // TODO(Cody): Compute some basic public polys like id(X), pow(X), and any required Lagrange polys

    // Execute Sumcheck Verifier
    std::optional sumcheck_result = Sumcheck::execute_verifier(log_n, relation_parameters, transcript);

    // Abort if sumcheck failed and returned nothing
    if (!sumcheck_result.has_value()) {
        return false;
    }

    // Get opening point and vector of multivariate evaluations produced by Sumcheck
    // - Multivariate opening point u = (u_0, ..., u_{d-1})
    auto [opening_point, multivariate_evaluations] = *sumcheck_result;
    // Execute Gemini/Shplonk verification:

    // Construct inputs for Gemini verifier:
    // - Multivariate opening point u = (u_0, ..., u_{d-1})
    // - batched unshifted and to-be-shifted polynomial commitments

    // Compute powers of batching challenge rho
    FF rho = FF::serialize_from_buffer(transcript.get_challenge("rho").begin());
    std::vector<FF> rhos = Gemini::powers_of_rho(rho, NUM_POLYNOMIALS);

    // Compute batched multivariate evaluation
    FF batched_evaluation = FF::zero();
    for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
        batched_evaluation += multivariate_evaluations[i] * rhos[i];
    }

    // Construct batched commitment for NON-shifted polynomials
    Commitment batched_commitment_unshifted = Commitment::zero();
    {
        // add precomputed commitments
        for (size_t i = 0; i < NUM_PRECOMPUTED; ++i) {
            // if precomputed, commitment comes from verification key
            Commitment commitment = key->commitments[bonk::StandardArithmetization::ENUM_TO_COMM[i]];
            batched_commitment_unshifted += commitment * rhos[i];
        }
        // add wire commitment
        for (size_t i = 0; i < width; ++i) {
            batched_commitment_unshifted += wire_commitments[i] * rhos[NUM_PRECOMPUTED + i];
        }
        // add z_perm
        batched_commitment_unshifted += z_permutation_commitment * rhos[NUM_PRECOMPUTED + width];
    }
    // Construct batched commitment for to-be-shifted polynomials (for now only Z_PERM)
    Commitment batched_commitment_to_be_shifted = z_permutation_commitment * rhos[NUM_UNSHIFTED];

    // Reconstruct the Gemini Proof from the transcript
    auto gemini_proof = Gemini::reconstruct_proof_from_transcript(&transcript, log_n);

    // Produce a Gemini claim consisting of:
    // - d+1 commitments [Fold_{r}^(0)], [Fold_{-r}^(0)], and [Fold^(l)], l = 1:d-1
    // - d+1 evaluations a_0_pos, and a_l, l = 0:d-1
    auto gemini_claim = Gemini::reduce_verify(opening_point,
                                              batched_evaluation,
                                              batched_commitment_unshifted,
                                              batched_commitment_to_be_shifted,
                                              gemini_proof,
                                              &transcript);

    // Reconstruct the Shplonk Proof (commitment [Q]) from the transcript
    auto shplonk_proof = transcript.get_group_element("Q");

    // Produce a Shplonk claim: commitment [Q] - [Q_z], evaluation zero (at random challenge z)
    auto shplonk_claim = Shplonk::reduce_verify(gemini_claim, shplonk_proof, &transcript);

    // Reconstruct the KZG Proof (commitment [W]_1) from the transcript
    auto kzg_proof = transcript.get_group_element("W");

    // Aggregate inputs [Q] - [Q_z] and [W] into an 'accumulator' (can perform pairing check on result)
    auto kzg_claim = KZG::reduce_verify(shplonk_claim, kzg_proof);

    // Do final pairing check
    bool pairing_result = kzg_claim.verify(kate_verification_key);

    bool result = sumcheck_result && pairing_result;

    return result;
}

template class Verifier<honk::standard_verifier_settings>;

} // namespace honk
