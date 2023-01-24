#include "prover.hpp"
#include <cstddef>
#include <honk/sumcheck/sumcheck.hpp> // will need
#include <array>
#include <honk/sumcheck/polynomials/univariate.hpp> // will go away
#include <honk/pcs/commitment_key.hpp>
#include <memory>
#include <vector>
#include "ecc/curves/bn254/fr.hpp"
#include "ecc/curves/bn254/g1.hpp"
#include <honk/sumcheck/polynomials/multivariates.hpp>
#include <honk/sumcheck/relations/arithmetic_relation.hpp>
#include <honk/sumcheck/relations/grand_product_computation_relation.hpp>
#include <honk/sumcheck/relations/grand_product_initialization_relation.hpp>
#include "plonk/proof_system/types/polynomial_manifest.hpp"
#include "polynomials/polynomial.hpp"
#include "proof_system/flavor/flavor.hpp"
#include "transcript/transcript_wrappers.hpp"
#include <string>
#include <honk/pcs/claim.hpp>

namespace honk {

using Fr = barretenberg::fr;
using Polynomial = barretenberg::Polynomial<Fr>;

/**
 * Create Prover from proving key, witness and manifest.
 *
 * @param input_key Proving key.
 * @param input_manifest Input manifest
 *
 * @tparam settings Settings class.
 * */
template <typename settings>
Prover<settings>::Prover(std::shared_ptr<waffle::proving_key> input_key, const transcript::Manifest& input_manifest)
    : n(input_key == nullptr ? 0 : input_key->n)
    , transcript(input_manifest, settings::hash_type, settings::num_challenge_bytes)
    , proving_key(input_key)
    , commitment_key(nullptr) // TODO(Cody): Need better constructors for prover.
// , queue(proving_key.get(), &transcript) // TODO(Adrian): explore whether it's needed 
{}

/**
 * For Plonk systems:
 * - Compute commitments to wires 1,2,3
 * - Get public inputs (which were stored in w_2_lagrange) and add to transcript
 *
 * For Honk, we should
 * - Commit to wires 1,2,3
 * - Add PI to transcript (I guess PI will stay in w_2 for now?)
 *
 * */
template <typename settings> void Prover<settings>::compute_wire_commitments()
{
    // TODO(luke): Compute wire commitments
    for (size_t i = 0; i < settings::program_width; ++i) {
        std::string wire_tag = "w_" + std::to_string(i + 1) + "_lagrange";
        std::string commit_tag = "W_" + std::to_string(i + 1);

        std::span<Fr> wire_polynomial = proving_key->polynomial_cache.get(wire_tag);
        auto commitment = commitment_key->commit(wire_polynomial);

        transcript.add_element(commit_tag, commitment.to_buffer());
    }
}

/**
 * @brief Compute the permutation grand product polynomial Z_perm(X)
 * *
 * @detail (This description assumes program_width 3). Z_perm may be defined in terms of its values
 * on X_i = 0,1,...,n-1 as Z_perm[0] = 1 and for i = 1:n-1
 *
 *                  (w_1(j) + β⋅id_1(j) + γ) ⋅ (w_2(j) + β⋅id_2(j) + γ) ⋅ (w_3(j) + β⋅id_3(j) + γ)
 * Z_perm[i] = ∏ --------------------------------------------------------------------------------
 *                  (w_1(j) + β⋅σ_1(j) + γ) ⋅ (w_2(j) + β⋅σ_2(j) + γ) ⋅ (w_3(j) + β⋅σ_3(j) + γ)
 *
 * where ∏ := ∏_{j=0:i-1} and id_i(X) = id(X) + n*(i-1). These evaluations are constructed over the
 * course of four steps. For expositional simplicity, write Z_perm[i] as
 *
 *                A_1(j) ⋅ A_2(j) ⋅ A_3(j)
 * Z_perm[i] = ∏ --------------------------
 *                B_1(j) ⋅ B_2(j) ⋅ B_3(j)
 *
 * Step 1) Compute the 2*program_width length-n polynomials A_i and B_i
 * Step 2) Compute the 2*program_width length-n polynomials ∏ A_i(j) and ∏ B_i(j)
 * Step 3) Compute the two length-n polynomials defined by
 *          numer[i] = ∏ A_1(j)⋅A_2(j)⋅A_3(j), and denom[i] = ∏ B_1(j)⋅B_2(j)⋅B_3(j)
 * Step 4) Compute Z_perm[i+1] = numer[i]/denom[i] (recall: Z_perm[0] = 1)
 *
 * Note: Step (4) utilizes Montgomery batch inversion to replace n-many inversions with
 * one batch inversion (at the expense of more multiplications)
 */
template <typename settings>
void Prover<settings>::compute_grand_product_polynomial(barretenberg::fr beta, barretenberg::fr gamma)
{
    using barretenberg::polynomial_arithmetic::copy_polynomial;
    static const size_t program_width = settings::program_width;

    // Allocate scratch space for accumulators
    Fr* numererator_accum[program_width];
    Fr* denominator_accum[program_width];
    for (size_t i = 0; i < program_width; ++i) {
        numererator_accum[i] = static_cast<Fr*>(aligned_alloc(64, sizeof(Fr) * proving_key->n));
        denominator_accum[i] = static_cast<Fr*>(aligned_alloc(64, sizeof(Fr) * proving_key->n));
    }

    // Populate wire and permutation polynomials
    std::array<const Fr*, program_width> wires;
    std::array<const Fr*, program_width> sigmas;
    for (size_t i = 0; i < program_width; ++i) {
        std::string wire_id = "w_" + std::to_string(i + 1) + "_lagrange";
        std::string sigma_id = "sigma_" + std::to_string(i + 1) + "_lagrange";
        wires[i] = proving_key->polynomial_cache.get(wire_id).get_coefficients();
        sigmas[i] = proving_key->polynomial_cache.get(sigma_id).get_coefficients();
    }

    // Step (1)
    for (size_t i = 0; i < proving_key->n; ++i) {
        for (size_t k = 0; k < program_width; ++k) {
            // TODO(luke): maybe this idx is replaced by proper ID polys in the future
            Fr idx = k * proving_key->n + i;
            numererator_accum[k][i] = wires[k][i] + (idx * beta) + gamma;          // w_k(i) + β.(k*n+i) + γ
            denominator_accum[k][i] = wires[k][i] + (sigmas[k][i] * beta) + gamma; // w_k(i) + β.σ_k(i) + γ
        }
    }

    // Step (2)
    for (size_t k = 0; k < program_width; ++k) {
        for (size_t i = 0; i < proving_key->n - 1; ++i) {
            numererator_accum[k][i + 1] *= numererator_accum[k][i];
            denominator_accum[k][i + 1] *= denominator_accum[k][i];
        }
    }

    // Step (3)
    for (size_t i = 0; i < proving_key->n; ++i) {
        for (size_t k = 1; k < program_width; ++k) {
            numererator_accum[0][i] *= numererator_accum[k][i];
            denominator_accum[0][i] *= denominator_accum[k][i];
        }
    }

    // Step (4)
    // Use Montgomery batch inversion to compute z_perm[i+1] = numererator_accum[0][i] / denominator_accum[0][i]. At the
    // end of this computation, the quotient numererator_accum[0] / denominator_accum[0] is stored in
    // numererator_accum[0].
    Fr* inversion_coefficients = &denominator_accum[1][0]; // arbitrary scratch space
    Fr inversion_accumulator = Fr::one();
    for (size_t i = 0; i < proving_key->n; ++i) {
        inversion_coefficients[i] = numererator_accum[0][i] * inversion_accumulator;
        inversion_accumulator *= denominator_accum[0][i];
    }
    inversion_accumulator = inversion_accumulator.invert(); // perform single inversion per thread
    for (size_t i = proving_key->n - 1; i != size_t(0) - 1; --i) {
        // TODO(luke): What needs to be done Re the comment below:
        // We can avoid fully reducing z_perm[i + 1] as the inverse fft will take care of that for us
        numererator_accum[0][i] = inversion_accumulator * inversion_coefficients[i];
        inversion_accumulator *= denominator_accum[0][i];
    }

    // Construct permutation polynomial 'z_perm' in lagrange form as:
    // z_perm = [1 numererator_accum[0][0] numererator_accum[0][1] ... numererator_accum[0][n-2]]
    Polynomial z_perm(proving_key->n, proving_key->n);
    z_perm[0] = Fr::one();
    copy_polynomial(numererator_accum[0], &z_perm[1], proving_key->n - 1, proving_key->n - 1);

    // free memory allocated for scratch space
    for (size_t k = 0; k < program_width; ++k) {
        aligned_free(numererator_accum[k]);
        aligned_free(denominator_accum[k]);
    }

    // TODO(luke): Commit to z_perm here? This would match Plonk but maybe best to do separately?

    proving_key->polynomial_cache.put("z_perm_lagrange", std::move(z_perm));
}

/**
 * For Plonk systems:
 * - added some initial data to transcript: circuit size and PI size
 * - added randomness to lagrange wires
 * - performed ifft to get monomial wires
 *
 * For Honk:
 * - Add circuit size and PI size to transcript. That's it?
 *
 * */
template <typename settings> void Prover<settings>::execute_preamble_round()
{
    // Add some initial data to transcript (circuit size and PI size)
    // queue.flush_queue(); // NOTE: Don't remove; we may reinstate the queue

    transcript.add_element("circuit_size",
                           { static_cast<uint8_t>(n >> 24),
                             static_cast<uint8_t>(n >> 16),
                             static_cast<uint8_t>(n >> 8),
                             static_cast<uint8_t>(n) });

    transcript.add_element("public_input_size",
                           { static_cast<uint8_t>(proving_key->num_public_inputs >> 24),
                             static_cast<uint8_t>(proving_key->num_public_inputs >> 16),
                             static_cast<uint8_t>(proving_key->num_public_inputs >> 8),
                             static_cast<uint8_t>(proving_key->num_public_inputs) });

    transcript.apply_fiat_shamir("init");
}

/**
 * For Plonk systems:
 * - compute wire commitments
 * - add public inputs to transcript (done in compute_wire_commitments() for some reason)
 *
 * For Honk:
 * - compute wire commitments
 * - add public inputs to transcript (done explicitly in execute_first_round())
 * */
template <typename settings> void Prover<settings>::execute_wire_commitments_round()
{
    // queue.flush_queue(); // NOTE: Don't remove; we may reinstate the queue
    compute_wire_commitments();

    // Add public inputs to transcript
    const Polynomial& public_wires_source = proving_key->polynomial_cache.get("w_2_lagrange");
    std::vector<Fr> public_wires;
    for (size_t i = 0; i < proving_key->num_public_inputs; ++i) {
        public_wires.push_back(public_wires_source[i]);
    }
    transcript.add_element("public_inputs", ::to_buffer(public_wires));
}

/**
 * For Plonk systems:
 * - Do Fiat-Shamir to get "eta" challenge (done regardless of arithmetization but only required for Ultra)
 * - does stuff related only to lookups (compute 's' etc and do some RAM/ROM stuff with w_4).
 *
 * For Standard Honk, this is a non-op (just like for Standard/Turbo Plonk).
 * */
template <typename settings> void Prover<settings>::execute_tables_round()
{
    // queue.flush_queue(); // NOTE: Don't remove; we may reinstate the queue
    transcript.apply_fiat_shamir("eta");

    // No operations are needed here for Standard Honk
}

/**
 * For Plonk systems:
 * - Do Fiat-Shamir to get "beta" challenge
 * - Compute grand product polynomials (permutation and lookup) and commitments
 * - Compute wire polynomial coset FFTs
 *
 * For Honk:
 * - Do Fiat-Shamir to get "beta" challenge (Note: gamma = beta^2)
 * - Compute grand product polynomial (permutation only) and commitment
 * */
template <typename settings> void Prover<settings>::execute_grand_product_computation_round()
{
    // queue.flush_queue(); // NOTE: Don't remove; we may reinstate the queue

    transcript.apply_fiat_shamir("beta");

    auto beta = transcript.get_challenge_field_element("beta", 0);
    auto gamma = transcript.get_challenge_field_element("beta", 1);
    compute_grand_product_polynomial(beta, gamma);
    std::span<Fr> z_perm = proving_key->polynomial_cache.get("z_perm_lagrange");
    auto commitment = commitment_key->commit(z_perm);
    transcript.add_element("Z_PERM", commitment.to_buffer());
}

/**
 * For Plonk systems:
 * - Do Fiat-Shamir to get "alpha" challenge
 * - Compute coset_fft(L_1)
 * - Compute quotient polynomial (with blinding)
 * - Compute quotient polynomial commitment
 *
 * For Honk
 * - Do Fiat-Shamir to get "alpha" challenge
 * - Run Sumcheck resulting in u = (u_1,...,u_d) challenges and all
 *   evaluations at u being calculated.
 * */
template <typename settings> void Prover<settings>::execute_relation_check_rounds()
{
    // queue.flush_queue(); // NOTE: Don't remove; we may reinstate the queue

    using Multivariates = sumcheck::Multivariates<barretenberg::fr, waffle::STANDARD_HONK_TOTAL_NUM_POLYS>;
    using Transcript = transcript::StandardTranscript;
    using Sumcheck = sumcheck::Sumcheck<Multivariates,
                                        Transcript,
                                        sumcheck::ArithmeticRelation,
                                        // sumcheck::GrandProductComputationRelation,
                                        sumcheck::GrandProductInitializationRelation>;

    // Compute alpha challenge
    transcript.apply_fiat_shamir("alpha");

    auto multivariates = Multivariates(proving_key);
    auto sumcheck = Sumcheck(multivariates, transcript);

    sumcheck.execute_prover();
}

/**
 * For Plonk: the polynomials are univariate, so this is a no-op.
 * For Honk:
 * - Get rho challenge
 * - Compute Fold polynomials and commitments.
 *
 * */
template <typename settings> void Prover<settings>::execute_univariatization_round()
{
    using Gemini = pcs::gemini::MultilinearReductionScheme<pcs::kzg::Params>;
    using MLEOpeningClaim = pcs::MLEOpeningClaim<pcs::kzg::Params>;

    // Construct inputs for Gemini:
    // - Multivariate opening point u = (u_1, ..., u_d)
    // - MLE opening claim = {commitment, eval} for each multivariate and shifted multivariate polynomial
    // - Pointers to multivariate and shifted multivariate polynomials
    std::vector<Fr> opening_point;
    std::vector<MLEOpeningClaim> opening_claims;
    std::vector<MLEOpeningClaim> opening_claims_shifted;
    std::vector<Polynomial*> multivariate_polynomials;
    std::vector<Polynomial*> multivariate_polynomials_shifted;
    // TODO(luke): Currently feeding in mock commitments for non-WITNESS polynomials. This may be sufficient for simple
    // proof verification since the other commitments are only needed to produce 'claims' in gemini.reduce_prove, they
    // are not needed in the proof itself.

    // Construct MLE opening point
    for (size_t round_idx = 0; round_idx < proving_key->log_n; round_idx++) {
        std::string label = "u_" + std::to_string(round_idx + 1);
        opening_point.emplace_back(transcript.get_challenge_field_element(label));
    }

    // Construct opening claims and polynomials
    for (auto& entry : proving_key->polynomial_manifest.get()) {
        std::string label(entry.polynomial_label);
        std::string commitment_label(entry.commitment_label);
        auto evaluation = Fr::serialize_from_buffer(&transcript.get_element(label)[0]);
        barretenberg::g1::affine_element commitment;
        if (entry.source == waffle::WITNESS) {
            commitment =
                barretenberg::g1::affine_element::serialize_from_buffer(&transcript.get_element(commitment_label)[0]);
        } else {                                       // SELECTOR, PERMUTATION, OTHER
            commitment = barretenberg::g1::affine_one; // mock commitment
        }
        opening_claims.emplace_back(commitment, evaluation);
        multivariate_polynomials.emplace_back(&proving_key->polynomial_cache.get(label));
        if (entry.requires_shifted_evaluation) {
            // Note: For a polynomial p for which we need the shift p_shift, we provide Gemini with the SHIFTED
            // evaluation p_shift(u), but the UNSHIFTED polynomial p and its UNSHIFTED commitment [p].
            auto shifted_evaluation = Fr::serialize_from_buffer(&transcript.get_element(label + "_shift")[0]);
            opening_claims_shifted.emplace_back(commitment, shifted_evaluation);
            multivariate_polynomials_shifted.emplace_back(&proving_key->polynomial_cache.get(label));
        }
    }

    gemini_output = Gemini::reduce_prove(commitment_key,
                                         opening_point,
                                         opening_claims,
                                         opening_claims_shifted,
                                         multivariate_polynomials,
                                         multivariate_polynomials_shifted,
                                         &transcript);
}

/**
 * For Plonk systems:
 * - Do Fiat-Shamir to get "frak-z" challenge
 * - Compute linearization or evaluation of quotient polynomial.
 *
 * For Honk:
 * - Do Fiat-Shamir to get "r" challenge
 * - Compute evaluations of folded polynomials.
 * */
template <typename settings> void Prover<settings>::execute_pcs_evaluation_round()
{
    // TODO(luke): This functionality is performed within Gemini::reduce_prove(), called in the previous round. In the
    // future we could (1) split the Gemini functionality to match the round structure defined here, or (2) remove this
    // function from the prover. The former may be necessary to maintain the work_queue paradigm.
}

/**
 * For Plonk: Batching is combined with generation of opening proof polynomial commitments.
 *
 * For Honk:
 * - Do Fiat-Shamir to get "nu" challenge.
 * - Compute Shplonk batched quotient commitment [Q]_1.
 * */
template <typename settings> void Prover<settings>::execute_shplonk_round()
{
    using Shplonk = pcs::shplonk::SingleBatchOpeningScheme<pcs::kzg::Params>;
    shplonk_output = Shplonk::reduce_prove(commitment_key, gemini_output.claim, gemini_output.witness, &transcript);
}

/**
 * For Plonk systems:
 * - Do Fiat-Shamir to get "nu" challenge
 * - Compute KZG batch opening polynomial commitments.
 *
 * For Honk:
 * - Get "z" challenge.
 * - Compute KZG quotient [W]_1.
 *
 * */
template <typename settings> void Prover<settings>::execute_kzg_round()
{
    // Note(luke): Fiat-Shamir to get "z" challenge is done in Shplonk::reduce_prove
    // TODO(luke): Get KZG opening point [W]_1
    using KZG = pcs::kzg::UnivariateOpeningScheme<pcs::kzg::Params>;
    using KzgOutput = pcs::kzg::UnivariateOpeningScheme<pcs::kzg::Params>::Output;
    KzgOutput kzg_output = KZG::reduce_prove(commitment_key, shplonk_output.claim, shplonk_output.witness);

    auto W_commitment = static_cast<barretenberg::g1::affine_element>(kzg_output.proof).to_buffer();

    transcript.add_element("W", W_commitment);
}

template <typename settings> waffle::plonk_proof& Prover<settings>::export_proof()
{
    proof.proof_data = transcript.export_transcript();
    return proof;
}

template <typename settings> waffle::plonk_proof& Prover<settings>::construct_proof()
{
    // Add circuit size and public input size to transcript.
    execute_preamble_round();
    // queue.process_queue(); // NOTE: Don't remove; we may reinstate the queue

    // Compute wire commitments; Add PI to transcript
    execute_wire_commitments_round();
    // queue.process_queue(); // NOTE: Don't remove; we may reinstate the queue

    // Currently a no-op; may execute some "random widgets", commit to W_4, do RAM/ROM stuff
    // if this prover structure is kept when we bring tables to Honk.
    execute_tables_round();
    // queue.process_queue(); // NOTE: Don't remove; we may reinstate the queue

    // Fiat-Shamir: beta & gamma
    // Compute grand product(s) and commitments.
    execute_grand_product_computation_round();
    // queue.process_queue(); // NOTE: Don't remove; we may reinstate the queue

    // Fiat-Shamir: alpha
    // Run sumcheck subprotocol.
    execute_relation_check_rounds();
    // // queue currently only handles commitments, not partial multivariate evaluations.
    // queue.process_queue(); // NOTE: Don't remove; we may reinstate the queue

    // // Fiat-Shamir: rho
    // // Compute Fold polynomials and their commitments.
    // execute_univariatization_round();
    // // queue.process_queue(); // NOTE: Don't remove; we may reinstate the queue

    // // Fiat-Shamir: r
    // // Compute Fold evaluations
    // execute_pcs_evaluation_round();

    // // Fiat-Shamir: nu
    // // Compute Shplonk batched quotient commitment
    // execute_shplonk_round();
    // // queue.process_queue(); // NOTE: Don't remove; we may reinstate the queue

    // // Fiat-Shamir: z
    // // Compute KZG quotient commitment
    // execute_kzg_round();
    // // queue.process_queue(); // NOTE: Don't remove; we may reinstate the queue

    // // queue.flush_queue(); // NOTE: Don't remove; we may reinstate the queue

    return export_proof();
}

// TODO(luke): Need to define a 'standard_settings' analog for Standard Honk
template class Prover<waffle::standard_settings>;

} // namespace honk
