#include "prover.hpp"
#include "barretenberg/honk/proof_system/prover_library.hpp"
#include "barretenberg/honk/sumcheck/sumcheck.hpp"
#include "barretenberg/honk/transcript/transcript.hpp"
#include "barretenberg/honk/utils/power_polynomial.hpp"

namespace proof_system::honk {

/**
 * Create Prover from proving key, witness and manifest.
 *
 * @param input_key Proving key.
 * @param input_manifest Input manifest
 *
 * @tparam settings Settings class.
 * */
template <StandardFlavor Flavor>
StandardProver_<Flavor>::StandardProver_(const std::shared_ptr<ProvingKey> input_key,
                                         const std::shared_ptr<PCSCommitmentKey> commitment_key)
    : key(input_key)
    , queue(commitment_key, transcript)
    , pcs_commitment_key(commitment_key)
{
    prover_polynomials.q_c = key->q_c;
    prover_polynomials.q_l = key->q_l;
    prover_polynomials.q_r = key->q_r;
    prover_polynomials.q_o = key->q_o;
    prover_polynomials.q_m = key->q_m;
    prover_polynomials.sigma_1 = key->sigma_1;
    prover_polynomials.sigma_2 = key->sigma_2;
    prover_polynomials.sigma_3 = key->sigma_3;
    prover_polynomials.id_1 = key->id_1;
    prover_polynomials.id_2 = key->id_2;
    prover_polynomials.id_3 = key->id_3;
    prover_polynomials.lagrange_first = key->lagrange_first;
    prover_polynomials.lagrange_last = key->lagrange_last;
    prover_polynomials.w_l = key->w_l;
    prover_polynomials.w_r = key->w_r;
    prover_polynomials.w_o = key->w_o;

    // Add public inputs to transcript from the second wire polynomial
    std::span<FF> public_wires_source = prover_polynomials.w_r;

    for (size_t i = 0; i < key->num_public_inputs; ++i) {
        public_inputs.emplace_back(public_wires_source[i]);
    }
}

/**
 * - Add commitment to wires 1,2,3 to work queue
 * - Add PI to transcript (I guess PI will stay in w_2 for now?)
 *
 * */
template <StandardFlavor Flavor> void StandardProver_<Flavor>::compute_wire_commitments()
{
    size_t wire_idx = 0; // TODO(#391) zip
    auto wire_polys = key->get_wires();
    for (auto& label : commitment_labels.get_wires()) {
        queue.add_commitment(wire_polys[wire_idx], label);
        ++wire_idx;
    }
}

/**
 * @brief Compute the permutation grand product polynomial \f$\Zperm(Xf)\f$
 *
 * @details The grand product polynomial is defined via Lagrange interpolation by specifying
 *     \f[ Z(ω^i) =
 *          \frac
 *           {\prod_{k=1}^{i}\prod_{j=1}^{\numwires} (w_j(ω^k) + β         \ID_j(ω^k) + γ )}
 *           {\prod_{k=1}^{i}\prod_{j=1}^{\numwires} (w_j(ω^k) + β S_{\sigma, j}(ω^k) + γ)}
 *                                                   \qquad i=1,\ldots,\numgates.         \f]
 * One argues that the polynomial is well formed by showing that
 *     \f[   Z(X)  ⋅ \prod_{j=1}^{\numwires} (w_j(X) +          β \ID_j(X) + γ)
 *         = Z(Xω) ⋅ \prod_{j=1}^{\numwires} (w_j(X) + β  S_{\sigma, j}(X) + γ), \f]
 * on \f$H\f$ and showing that
 *  \f[Z(ω^\numgates) = 1.\f]
 */
// TODO(#222)(luke): Parallelize
template <typename settings> Polynomial Prover<settings>::compute_grand_product_polynomial(Fr beta, Fr gamma)
{
    /**
     * Notation: ω^k is written as evaluation at `k` as in `(k)`, and this corresponds to extracting the
     *           `k`-th index of a vector.
     *
     * Step 1) Compute the 2*program_width length-n polynomials \f$A_i\f$ and \f$B_i\f$
     *
     * Step 2) Compute the 2*program_width length-n polynomials \f$\prod A_i(j)\f$ and \f$\prod B_i(j)\f$
     *
     * Step 3) Compute the two length-n polynomials defined by
     *          \f$\numer[i] = ∏ A_1(j)⋅A_2(j)⋅A_3(j)\f$, and \f$\denom[i] = ∏ B_1(j)⋅B_2(j)⋅B_3(j)\f$
     *
     * Step 4) Compute \f$\Zperm[i+1] = \numer[i]/\denom[i]\f$ (recall: \f$\Zperm[0] = 1\f$)
     *
     * Note: Step (4) utilizes Montgomery batch inversion to replace n-many inversions with
     * one batch inversion (at the expense of more multiplications)
     * @todo This is function is shared, right? So move out of `honk` namespace.
     */
    using barretenberg::polynomial_arithmetic::copy_polynomial;
    static const size_t program_width = settings::program_width;

    // Allocate scratch space for accumulators
    std::array<Fr*, program_width> numerator_accumulator;
    std::array<Fr*, program_width> denominator_accumulator;
    for (size_t i = 0; i < program_width; ++i) {
        numerator_accumulator[i] = static_cast<Fr*>(aligned_alloc(64, sizeof(Fr) * key->circuit_size));
        denominator_accumulator[i] = static_cast<Fr*>(aligned_alloc(64, sizeof(Fr) * key->circuit_size));
    }

    // Populate wire and permutation polynomials
    std::array<std::span<const Fr>, program_width> wires;
    std::array<std::span<const Fr>, program_width> sigmas;
    for (size_t i = 0; i < program_width; ++i) {
        std::string sigma_id = "sigma_" + std::to_string(i + 1) + "_lagrange";
        wires[i] = wire_polynomials[i];
        sigmas[i] = key->polynomial_store.get(sigma_id);
    }

    // Step (1)
    // TODO(#222)(kesha): Change the order to engage automatic prefetching and get rid of redundant computation
    for (size_t i = 0; i < key->circuit_size; ++i) {
        for (size_t k = 0; k < program_width; ++k) {
            // Note(luke): this idx could be replaced by proper ID polys if desired
            Fr idx = k * key->circuit_size + i;
            numerator_accumulator[k][i] = wires[k][i] + (idx * beta) + gamma;            // w_k(i) + β.(k*n+i) + γ
            denominator_accumulator[k][i] = wires[k][i] + (sigmas[k][i] * beta) + gamma; // w_k(i) + β.σ_k(i) + γ
        }
    }

    // Step (2)
    for (size_t k = 0; k < program_width; ++k) {
        for (size_t i = 0; i < key->circuit_size - 1; ++i) {
            numerator_accumulator[k][i + 1] *= numerator_accumulator[k][i];
            denominator_accumulator[k][i + 1] *= denominator_accumulator[k][i];
        }
    }

    // Step (3)
    for (size_t i = 0; i < key->circuit_size; ++i) {
        for (size_t k = 1; k < program_width; ++k) {
            numerator_accumulator[0][i] *= numerator_accumulator[k][i];
            denominator_accumulator[0][i] *= denominator_accumulator[k][i];
        }
    }

    // Step (4)
    // Use Montgomery batch inversion to compute z_perm[i+1] = numerator_accumulator[0][i] /
    // denominator_accumulator[0][i]. At the end of this computation, the quotient numerator_accumulator[0] /
    // denominator_accumulator[0] is stored in numerator_accumulator[0].
    Fr* inversion_coefficients = &denominator_accumulator[1][0]; // arbitrary scratch space
    Fr inversion_accumulator = Fr::one();
    for (size_t i = 0; i < key->circuit_size; ++i) {
        inversion_coefficients[i] = numerator_accumulator[0][i] * inversion_accumulator;
        inversion_accumulator *= denominator_accumulator[0][i];
    }
    inversion_accumulator = inversion_accumulator.invert(); // perform single inversion per thread
    for (size_t i = key->circuit_size - 1; i != std::numeric_limits<size_t>::max(); --i) {
        numerator_accumulator[0][i] = inversion_accumulator * inversion_coefficients[i];
        inversion_accumulator *= denominator_accumulator[0][i];
    }

    // Construct permutation polynomial 'z_perm' in lagrange form as:
    // z_perm = [0 numerator_accumulator[0][0] numerator_accumulator[0][1] ... numerator_accumulator[0][n-2] 0]
    Polynomial z_perm(key->circuit_size);
    // We'll need to shift this polynomial to the left by dividing it by X in gemini, so the the 0-th coefficient should
    // stay zero
    copy_polynomial(numerator_accumulator[0], &z_perm[1], key->circuit_size - 1, key->circuit_size - 1);

    // free memory allocated for scratch space
    for (size_t k = 0; k < program_width; ++k) {
        aligned_free(numerator_accumulator[k]);
        aligned_free(denominator_accumulator[k]);
    }

    return z_perm;
}

/**
 * - Add circuit size, public input size, and public inputs to transcript
 *
 * */
template <StandardFlavor Flavor> void StandardProver_<Flavor>::execute_preamble_round()
{
    const auto circuit_size = static_cast<uint32_t>(key->circuit_size);
    const auto num_public_inputs = static_cast<uint32_t>(key->num_public_inputs);

    transcript.send_to_verifier("circuit_size", circuit_size);
    transcript.send_to_verifier("public_input_size", num_public_inputs);

    for (size_t i = 0; i < key->num_public_inputs; ++i) {
        auto public_input_i = public_inputs[i];
        transcript.send_to_verifier("public_input_" + std::to_string(i), public_input_i);
    }
}

/**
 * - compute wire commitments
 * */
template <StandardFlavor Flavor> void StandardProver_<Flavor>::execute_wire_commitments_round()
{
    compute_wire_commitments();
}

/**
 * For Standard Honk, this is a non-op (just like for Standard/Turbo Plonk).
 * */
template <StandardFlavor Flavor> void StandardProver_<Flavor>::execute_tables_round()
{
    // No operations are needed here for Standard Honk
}

/**
 * - Do Fiat-Shamir to get "beta" challenge (Note: gamma = beta^2)
 * - Compute grand product polynomial (permutation only) and commitment
 * */
template <StandardFlavor Flavor> void StandardProver_<Flavor>::execute_grand_product_computation_round()
{
    // Compute and store parameters required by relations in Sumcheck
    auto [beta, gamma] = transcript.get_challenges("beta", "gamma");

    auto public_input_delta = compute_public_input_delta<FF>(public_inputs, beta, gamma, key->circuit_size);

    relation_parameters = sumcheck::RelationParameters<FF>{
        .beta = beta,
        .gamma = gamma,
        .public_input_delta = public_input_delta,
    };

    key->z_perm = prover_library::compute_permutation_grand_product<Flavor>(key, beta, gamma);

    queue.add_commitment(key->z_perm, commitment_labels.z_perm);

    prover_polynomials.z_perm = key->z_perm;
    prover_polynomials.z_perm_shift = key->z_perm.shifted();
}

/**
 * - Do Fiat-Shamir to get "alpha" challenge
 * - Run Sumcheck resulting in u = (u_1,...,u_d) challenges and all
 *   evaluations at u being calculated.
 * */
template <StandardFlavor Flavor> void StandardProver_<Flavor>::execute_relation_check_rounds()
{
    using Sumcheck = sumcheck::Sumcheck<Flavor, ProverTranscript<FF>>;

    auto sumcheck = Sumcheck(key->circuit_size, transcript);

    sumcheck_output = sumcheck.execute_prover(prover_polynomials, relation_parameters);
}

/**
 * - Get rho challenge
 * - Compute d+1 Fold polynomials and their evaluations.
 *
 * */
template <StandardFlavor Flavor> void StandardProver_<Flavor>::execute_univariatization_round()
{
    const size_t NUM_POLYNOMIALS = Flavor::NUM_ALL_ENTITIES;

    // Generate batching challenge ρ and powers 1,ρ,…,ρᵐ⁻¹
    FF rho = transcript.get_challenge("rho");
    std::vector<FF> rhos = Gemini::powers_of_rho(rho, NUM_POLYNOMIALS);

    // Batch the unshifted polynomials and the to-be-shifted polynomials using ρ
    Polynomial batched_poly_unshifted(key->circuit_size); // batched unshifted polynomials
    size_t poly_idx = 0;                                  // TODO(#391) zip
    for (auto& unshifted_poly : prover_polynomials.get_unshifted()) {
        batched_poly_unshifted.add_scaled(unshifted_poly, rhos[poly_idx]);
        ++poly_idx;
    }

    Polynomial batched_poly_to_be_shifted(key->circuit_size); // batched to-be-shifted polynomials
    for (auto& to_be_shifted_poly : prover_polynomials.get_to_be_shifted()) {
        batched_poly_to_be_shifted.add_scaled(to_be_shifted_poly, rhos[poly_idx]);
        ++poly_idx;
    };

    // Compute d-1 polynomials Fold^(i), i = 1, ..., d-1.
    fold_polynomials = Gemini::compute_fold_polynomials(
        sumcheck_output.challenge_point, std::move(batched_poly_unshifted), std::move(batched_poly_to_be_shifted));

    // Compute and add to trasnscript the commitments [Fold^(i)], i = 1, ..., d-1
    for (size_t l = 0; l < key->log_circuit_size - 1; ++l) {
        queue.add_commitment(fold_polynomials[l + 2], "Gemini:FOLD_" + std::to_string(l + 1));
    }
}

/**
 * - Do Fiat-Shamir to get "r" challenge
 * - Compute remaining two partially evaluated Fold polynomials Fold_{r}^(0) and Fold_{-r}^(0).
 * - Compute and aggregate opening pairs (challenge, evaluation) for each of d Fold polynomials.
 * - Add d-many Fold evaluations a_i, i = 0, ..., d-1 to the transcript, excluding eval of Fold_{r}^(0)
 * */
template <StandardFlavor Flavor> void StandardProver_<Flavor>::execute_pcs_evaluation_round()
{
    const FF r_challenge = transcript.get_challenge("Gemini:r");
    gemini_output = Gemini::compute_fold_polynomial_evaluations(
        sumcheck_output.challenge_point, std::move(fold_polynomials), r_challenge);

    for (size_t l = 0; l < key->log_circuit_size; ++l) {
        std::string label = "Gemini:a_" + std::to_string(l);
        const auto& evaluation = gemini_output.opening_pairs[l + 1].evaluation;
        transcript.send_to_verifier(label, evaluation);
    }
}

/**
 * - Do Fiat-Shamir to get "nu" challenge.
 * - Compute commitment [Q]_1
 * */
template <StandardFlavor Flavor> void StandardProver_<Flavor>::execute_shplonk_batched_quotient_round()
{
    nu_challenge = transcript.get_challenge("Shplonk:nu");

    batched_quotient_Q =
        Shplonk::compute_batched_quotient(gemini_output.opening_pairs, gemini_output.witnesses, nu_challenge);

    // commit to Q(X) and add [Q] to the transcript
    queue.add_commitment(batched_quotient_Q, "Shplonk:Q");
}

/**
 * - Do Fiat-Shamir to get "z" challenge.
 * - Compute polynomial Q(X) - Q_z(X)
 * */
template <StandardFlavor Flavor> void StandardProver_<Flavor>::execute_shplonk_partial_evaluation_round()
{
    const FF z_challenge = transcript.get_challenge("Shplonk:z");
    shplonk_output = Shplonk::compute_partially_evaluated_batched_quotient(
        gemini_output.opening_pairs, gemini_output.witnesses, std::move(batched_quotient_Q), nu_challenge, z_challenge);
}

/**
 * - Compute final PCS opening proof:
 * - For KZG, this is the quotient commitment [W]_1
 * - For IPA, the vectors L and R
 * */
template <StandardFlavor Flavor> void StandardProver_<Flavor>::execute_final_pcs_round()
{
    PCS::compute_opening_proof(pcs_commitment_key, shplonk_output.opening_pair, shplonk_output.witness, transcript);
}

template <StandardFlavor Flavor> plonk::proof& StandardProver_<Flavor>::export_proof()
{
    proof.proof_data = transcript.proof_data;
    return proof;
}

//! [ConstructProof]
template <StandardFlavor Flavor> plonk::proof& StandardProver_<Flavor>::construct_proof()
{
    // Add circuit size and public input size to transcript.
    execute_preamble_round();

    // Compute wire commitments; Add PI to transcript
    execute_wire_commitments_round();
    queue.process_queue();

    // Currently a no-op; may execute some "random widgets", commit to W_4, do RAM/ROM stuff
    // if this prover structure is kept when we bring tables to Honk.
    // Suggestion: Maybe we shouldn't mix and match proof creation for different systems and
    // instead instatiate construct_proof differently for each?
    execute_tables_round();

    // Fiat-Shamir: beta & gamma
    // Compute grand product(s) and commitments.
    execute_grand_product_computation_round();
    queue.process_queue();

    // Fiat-Shamir: alpha
    // Run sumcheck subprotocol.
    execute_relation_check_rounds();

    // Fiat-Shamir: rho
    // Compute Fold polynomials and their commitments.
    execute_univariatization_round();
    queue.process_queue();

    // Fiat-Shamir: r
    // Compute Fold evaluations
    execute_pcs_evaluation_round();

    // Fiat-Shamir: nu
    // Compute Shplonk batched quotient commitment Q
    execute_shplonk_batched_quotient_round();
    queue.process_queue();

    // Fiat-Shamir: z
    // Compute partial evaluation Q_z
    execute_shplonk_partial_evaluation_round();

    // Fiat-Shamir: z
    // Compute final PCS opening proof (this is KZG quotient commitment or IPA opening proof)
    execute_final_pcs_round();
    // TODO(#479): queue.process_queue after the work_queue has been (re)added to KZG/IPA

    return export_proof();
}
//! [ConstructProof]

template class StandardProver_<flavor::Standard>;
template class StandardProver_<flavor::StandardGrumpkin>;

} // namespace proof_system::honk
