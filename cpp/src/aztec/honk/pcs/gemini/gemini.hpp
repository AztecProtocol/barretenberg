#pragma once

#include "../claim.hpp"
#include "common/log.hpp"
#include "honk/pcs/commitment_key.hpp"
#include "polynomials/polynomial.hpp"

#include <common/assert.hpp>
#include <memory>
#include <vector>

/**
 * @brief Protocol for opening several multi-linear polynomials at the same point.
 *
 *
 * m = number of variables
 * n = 2ᵐ
 * u = (u₀,...,uₘ₋₁)
 * f₀, …, fₖ₋₁ = multilinear polynomials,
 * g₀, …, gₕ₋₁ = shifted multilinear polynomial,
 *  Each gⱼ is the left-shift of some f↺ᵢ, and gⱼ points to the same memory location as fᵢ.
 * v₀, …, vₖ₋₁, v↺₀, …, v↺ₕ₋₁ = multilinear evalutions s.t. fⱼ(u) = vⱼ, and gⱼ(u) = f↺ⱼ(u) = v↺ⱼ
 *
 * We use a challenge ρ to create a random linear combination of all fⱼ,
 * and actually define A₀ = F + G↺, where
 *   F  = ∑ⱼ ρʲ fⱼ
 *   G  = ∑ⱼ ρᵏ⁺ʲ gⱼ,
 *   G↺ = is the shift of G
 * where fⱼ is normal, and gⱼ is shifted.
 * The evaluations are also batched, and
 *   v  = ∑ ρʲ⋅vⱼ + ∑ ρᵏ⁺ʲ⋅v↺ⱼ = F(u) + G↺(u)
 *
 * The prover then creates the folded polynomials A₀, ..., Aₘ₋₁,
 * commits and opens them at different points, as univariates.
 *
 * We open A₀ as univariate at r and -r.
 * Since A₀ = F + G↺, but the verifier only has commitments to the gⱼs,
 * we need to partially evaluate A₀ at both evaluation points.
 * As univariate, we have
 *  A₀(X) = F(X) + G↺(X) = F(X) + G(X)/X
 * So we define
 *  - A₀₊(X) = F(X) + G(X)/r
 *  - A₀₋(X) = F(X) − G(X)/r
 * So that A₀₊(r) = A₀(r) and A₀₋(-r) = A₀(-r).
 * The verifier is able to computed the simulated commitments to A₀₊(X) and A₀₋(X)
 * since they are linear-combinations of the commitments [fⱼ] and [gⱼ].
 */
namespace honk::pcs::gemini {

/**
 * @brief A Gemini proof contains the m-1 commitments to the
 * folded univariates, and corresponding evaluations
 * at -r, -r², …, r^{2ᵐ⁻¹}.
 *
 * The evaluations allow the verifier to reconstruct the evaluation of A₀(r).
 *
 * @tparam Params CommitmentScheme parameters
 */
template <typename Params> struct Proof {
    /** @brief Commitments to folded polynomials (size = m-1)
     *
     * [ C₁, …,  Cₘ₋₁], where Cₗ = commit(Aₗ(X)) of size 2ᵐ⁻ˡ
     */
    std::vector<typename Params::Commitment> commitments;

    /**
     * @brief Evaluations of batched and folded polynomials (size m)
     *
     * [A₀(-r) , ..., Aₘ₋₁(-r^{2ᵐ⁻¹})]
     */
    std::vector<typename Params::Fr> evaluations;
};

/**
 * @brief Univariate opening claims for multiple polynomials,
 * each opened at a single different point (size = m+1).
 *
 * [
 *   (C₀₊ , A₀  ( r)       ,  r )
 *   (C₀₋ , A₀  (-r)       , -r )
 *   (C₁  , A₁  (-r²)      , -r²)
 *   ...
 *   (Cₘ₋₁, Aₘ₋₁(-r^{2ᵐ⁻¹}), -r^{2ᵐ⁻¹})
 * ]
 * where
 *   C₀₊ is a simulated commitment to A₀ partially evaluated at r
 *   C₀₋ is a simulated commitment to A₀ partially evaluated at -r
 *
 * @tparam Params CommitmentScheme parameters
 */
template <typename Params> using OutputClaim = std::vector<OpeningClaim<Params>>;

/**
 * @brief Vector of opening pairs {r, v = p(r)}
 * @tparam Params CommitmentScheme parameters
 */
template <typename Params> using OutputPair = std::vector<OpeningPair<Params>>;

/**
 * @brief Univariate witness polynomials for opening all the
 *
 * [
 *   A₀₊(X) = F(X) + r⁻¹⋅G(X)
 *   A₀₋(X) = F(X) - r⁻¹⋅G(X)
 *   A₁(X) = (1-u₀)⋅even(A₀)(X) + u₀⋅odd(A₀)(X)
 *   ...
 *   Aₘ₋₁(X) = (1-uₘ₋₂)⋅even(Aₘ₋₂)(X) + uₘ₋₂⋅odd(Aₘ₋₂)(X)
 * ]
 * where
 *           /  r ⁻¹ ⋅ fⱼ(X)  if fⱼ is a shift
 * fⱼ₊(X) = |
 *           \  fⱼ(X)         otherwise
 *
 *           / (-r)⁻¹ ⋅ fⱼ(X) if fⱼ is a shift
 * fⱼ₋(X) = |
 *           \  fⱼ(X)         otherwise
 *
 *
 * @tparam Params CommitmentScheme parameters
 */
template <typename Params> using OutputWitness = std::vector<barretenberg::Polynomial<typename Params::Fr>>;

/**
 * @brief Prover output (evalutation pair, witness) that can be passed on to Shplonk batch opening.
 *
 * @tparam Params CommitmentScheme parameters
 */
template <typename Params> struct ProverOutput {
    OutputPair<Params> opening_pairs;
    OutputWitness<Params> witnesses;
};

template <typename Params> class MultilinearReductionScheme {
    using CK = typename Params::CK;

    using Fr = typename Params::Fr;
    using Commitment = typename Params::Commitment;
    using CommitmentAffine = typename Params::C; // TODO(luke): find a better name
    using Polynomial = barretenberg::Polynomial<Fr>;

  public:
    /**
     * @brief reduces claims about multiple (shifted) MLE evaluation
     *
     * @param ck is the commitment key for creating the new commitments
     * @param mle_opening_point = u =(u₀,...,uₘ₋₁) is the MLE opening point
     * @param evaluations evaluations of each multivariate at the point u
     * @param polynomials_f the unshifted multivariate polynomials
     * @param polynomials_g the to-be-shifted multivariate polynomials
     * @param transcript
     * @return Output (opening pairs, folded_witness_polynomials)
     *
     */
    static ProverOutput<Params> reduce_prove(std::shared_ptr<CK> ck,
                                             std::span<const Fr> mle_opening_point,
                                             std::span<const Fr> evaluations,          /* all */
                                             const Polynomial&& batched_shifted,       /* unshifted */
                                             const Polynomial&& batched_to_be_shifted, /* to-be-shifted */
                                             std::span<Fr> rhos,                       /* to-be-shifted */
                                             const auto& transcript)
    {
        const size_t num_variables = mle_opening_point.size(); // m

        // Allocate space for m+1 Fold polynomials
        //
        // At the end, the first two will contain the batched polynomial
        // partially evaluated at the challenges r,-r.
        // The other m-1 polynomials correspond to the foldings of A₀
        std::vector<Polynomial> fold_polynomials;
        fold_polynomials.reserve(num_variables + 1);
        // F(X) = ∑ⱼ ρʲ   fⱼ(X)
        Polynomial& batched_F = fold_polynomials.emplace_back(batched_shifted);
        // G(X) = ∑ⱼ ρᵏ⁺ʲ gⱼ(X)
        Polynomial& batched_G = fold_polynomials.emplace_back(batched_to_be_shifted);

        // A₀(X) = F(X) + G↺(X) = F(X) + G(X)/X.
        Polynomial A_0(batched_F);
        A_0 += batched_G.shifted();

        // Create the folded polynomials A₁(X),…,Aₘ₋₁(X)
        //
        // A_l = Aₗ(X) is the polynomial being folded
        // in the first iteration, we take the batched polynomial
        // in the next iteration, it is the previously folded one
        Fr* A_l = A_0.get_coefficients();
        for (size_t l = 0; l < num_variables - 1; ++l) {
            const Fr u_l = mle_opening_point[l];

            // size of the previous polynomial/2
            const size_t n_l = 1 << (num_variables - l - 1);

            // A_l_fold = Aₗ₊₁(X) = (1-uₗ)⋅even(Aₗ)(X) + uₗ⋅odd(Aₗ)(X)
            Fr* A_l_fold = fold_polynomials.emplace_back(Polynomial(n_l)).get_coefficients();

            // fold the previous polynomial with odd and even parts
            for (size_t i = 0; i < n_l; ++i) {
                // TODO parallelize

                // fold(Aₗ)[i] = (1-uₗ)⋅even(Aₗ)[i] + uₗ⋅odd(Aₗ)[i]
                //            = (1-uₗ)⋅Aₗ[2i]      + uₗ⋅Aₗ[2i+1]
                //            = Aₗ₊₁[i]
                A_l_fold[i] = A_l[i << 1] + u_l * (A_l[(i << 1) + 1] - A_l[i << 1]);
            }

            // set Aₗ₊₁ = Aₗ for the next iteration
            A_l = A_l_fold;
        }

        /*
         * Create commitments C₁,…,Cₘ₋₁ to polynomials FOLD_i, i = 1,...,d-1 and add to transcript
         */
        std::vector<Commitment> commitments;
        commitments.reserve(num_variables - 1);
        for (size_t l = 0; l < num_variables - 1; ++l) {
            commitments.emplace_back(ck->commit(fold_polynomials[l + 2]));
            transcript->add_element("FOLD_" + std::to_string(l + 1),
                                    static_cast<CommitmentAffine>(commitments[l]).to_buffer());
        }

        /*
         * Generate evaluation challenge r, and compute rₗ = r^{2ˡ} for l = 0, 1, ..., m-1
         */
        transcript->apply_fiat_shamir("r");
        const Fr r_challenge = Fr::serialize_from_buffer(transcript->get_challenge("r").begin());
        std::vector<Fr> r_squares = squares_of_r(r_challenge, num_variables);

        /*
         * Compute the witness polynomials for the resulting claim
         *
         *
         * We are batching all polynomials together, and linearly combining them with
         * powers of ρ
         */

        // 2 simulated polynomials and (m-1) polynomials from this round
        Fr r_inv = r_challenge.invert();
        // G(X) *= r⁻¹
        batched_G *= r_inv;

        // To avoid an extra allocation, we reuse the following polynomials
        // but rename them to represent the result.
        // tmp     = A₀(X) (&tmp     == &A_0)
        // A_0_pos = F(X)  (&A_0_pos == &batched_F)
        Polynomial& tmp = A_0;
        Polynomial& A_0_pos = fold_polynomials[0];

        tmp = batched_F;
        // A₀₊(X) = F(X) + G(X)/r, s.t. A₀₊(r) = A₀(r)
        A_0_pos += batched_G;

        std::swap(tmp, batched_G);
        // After the swap, we have
        // tmp = G(X)/r
        // A_0_neg = F(X) (since &batched_F == &A_0_neg)
        Polynomial& A_0_neg = fold_polynomials[1];

        // A₀₋(X) = F(X) - G(X)/r, s.t. A₀₋(-r) = A₀(-r)
        A_0_neg -= tmp;

        /*
         * Compute the m evaluations a_0 = Fold_{-r}^(0)(-r), and a_l = Fold^(l)(-r^{2^l}), l = 1, ..., m-1.
         * Add them to the transcript
         */
        std::vector<Fr> fold_polynomial_evals;
        fold_polynomial_evals.reserve(num_variables);
        for (size_t l = 0; l < num_variables; ++l) {
            const Polynomial& A_l = fold_polynomials[l + 1];

            fold_polynomial_evals.emplace_back(A_l.evaluate(-r_squares[l]));
            transcript->add_element("a_" + std::to_string(l), fold_polynomial_evals[l].to_buffer());
        }

        /*
         * Compute evaluation A₀(r)
         */
        auto a_0_pos = compute_eval_pos(evaluations, mle_opening_point, rhos, r_squares, fold_polynomial_evals);

        std::vector<OpeningPair<Params>> fold_poly_opening_pairs;
        fold_poly_opening_pairs.reserve(num_variables + 1);

        // ( [A₀₊], r, A₀(r) )
        fold_poly_opening_pairs.emplace_back(OpeningPair<Params>{ r_challenge, a_0_pos });
        // ([A₀₋], -r, Aₗ(−r^{2ˡ}) )
        for (size_t l = 0; l < num_variables; ++l) {
            fold_poly_opening_pairs.emplace_back(OpeningPair<Params>{ -r_squares[l], fold_polynomial_evals[l] });
        }

        return { fold_poly_opening_pairs, std::move(fold_polynomials) };
    };

    /**
     * @brief Checks that all MLE evaluations vⱼ contained in the list of m MLE opening claims
     * is correct, and returns univariate polynomial opening claims to be checked later
     *
     * @param mle_opening_point the MLE evaluation point for all claims
     * @param evaluations evaluations of each multivariate at the point u
     * @param commitments_f commitments to unshifted multivariate polynomials
     * @param commitments_g commitments to to-be-shifted multivariate polynomials
     * @param proof commitments to the m-1 folded polynomials, and alleged evaluations.
     * @param transcript
     * @return Fold polynomial opening claims
     */
    static OutputClaim<Params> reduce_verify(std::span<const Fr> mle_opening_point,     /* u */
                                             std::span<const Fr> evaluations,           /* all */
                                             std::span<const Commitment> commitments_f, /* unshifted */
                                             std::span<const Commitment> commitments_g, /* to-be-shifted */
                                             const Proof<Params>& proof,
                                             const auto& transcript)
    {
        const size_t num_variables = mle_opening_point.size();
        const size_t num_claims = evaluations.size();

        // compute vector of powers of batching challenge ρ
        const Fr rho = Fr::serialize_from_buffer(transcript->get_challenge("rho").begin());
        std::vector<Fr> rhos = powers_of_rho(rho, num_claims);

        // compute vector of powers of random evaluation point r
        const Fr r = Fr::serialize_from_buffer(transcript->get_challenge("r").begin());
        std::vector<Fr> r_squares = squares_of_r(r, num_variables);

        auto a_0_pos = compute_eval_pos(evaluations, mle_opening_point, rhos, r_squares, proof.evaluations);

        // C₀_r_pos = ∑ⱼ ρʲ⋅[fⱼ] + r⁻¹⋅∑ⱼ ρᵏ⁺ʲ [gⱼ]
        // C₀_r_pos = ∑ⱼ ρʲ⋅[fⱼ] - r⁻¹⋅∑ⱼ ρᵏ⁺ʲ [gⱼ]
        auto [c0_r_pos, c0_r_neg] = compute_simulated_commitments(commitments_f, commitments_g, rhos, r);

        std::vector<OpeningClaim<Params>> fold_polynomial_opening_claims;
        fold_polynomial_opening_claims.reserve(num_variables + 1);

        // ( [A₀₊], r, A₀(r) )
        fold_polynomial_opening_claims.emplace_back(OpeningClaim<Params>{ { r, a_0_pos }, c0_r_pos });
        // ( [A₀₋], -r, A₀(-r) )
        fold_polynomial_opening_claims.emplace_back(OpeningClaim<Params>{ { -r, proof.evaluations[0] }, c0_r_neg });
        for (size_t l = 0; l < num_variables - 1; ++l) {
            // ([A₀₋], −r^{2ˡ}, Aₗ(−r^{2ˡ}) )
            fold_polynomial_opening_claims.emplace_back(
                OpeningClaim<Params>{ { -r_squares[l + 1], proof.evaluations[l + 1] }, proof.commitments[l] });
        }

        return fold_polynomial_opening_claims;
    };

    /**
     * @brief Reconstruct Gemini proof from transcript
     *
     * @param transcript
     * @return Proof
     * @details Proof consists of:
     * - d Fold poly evaluations a_0, ..., a_{d-1}
     * - (d-1) Fold polynomial commitments [Fold^(1)], ..., [Fold^(d-1)]
     */
    static Proof<Params> reconstruct_proof_from_transcript(const auto& transcript, const size_t log_n)
    {
        Proof<Params> proof;
        for (size_t i = 0; i < log_n; i++) {
            std::string label = "a_" + std::to_string(i);
            proof.evaluations.emplace_back(transcript->get_field_element(label));
        };
        for (size_t i = 1; i < log_n; i++) {
            std::string label = "FOLD_" + std::to_string(i);
            proof.commitments.emplace_back(transcript->get_group_element(label));
        };

        return proof;
    }

    static std::vector<Fr> powers_of_rho(const Fr rho, const size_t num_powers)
    {
        std::vector<Fr> rhos = { Fr(1), rho };
        rhos.reserve(num_powers);
        for (size_t j = 2; j < num_powers; j++) {
            rhos.emplace_back(rhos[j - 1] * rho);
        }
        return rhos;
    };

  private:
    /**
     * @brief computes the output pair given the transcript.
     * This method is common for both prover and verifier.
     *
     * @param evaluations evaluations of each multivariate
     * @param mle_vars MLE opening point u
     * @param rhos powers of the initial batching challenge ρ
     * @param r_squares squares of r, r², ..., r^{2ᵐ⁻¹}
     * @return evaluation A₀(r)
     */
    static Fr compute_eval_pos(std::span<const Fr> evaluations,
                               std::span<const Fr> mle_vars,
                               std::span<const Fr> rhos,
                               std::span<const Fr> r_squares,
                               std::span<const Fr> fold_polynomial_evals)
    {
        const size_t num_variables = mle_vars.size();
        const size_t num_claims = evaluations.size();

        const auto& evals = fold_polynomial_evals;

        // compute the batched MLE evaluation
        // v = ∑ⱼ ρʲ vⱼ + ∑ⱼ ρᵏ⁺ʲ v↺ⱼ
        Fr mle_eval{ Fr::zero() };

        // construct batched evaluation
        for (size_t j = 0; j < num_claims; ++j) {
            mle_eval += evaluations[j] * rhos[j];
        }

        // For l = m, ..., 1
        // Initialize eval_pos = Aₘ(r^2ᵐ) = v
        Fr eval_pos = mle_eval;
        for (size_t l = num_variables; l != 0; --l) {
            const Fr r = r_squares[l - 1];    // = rₗ₋₁ = r^{2ˡ⁻¹}
            const Fr eval_neg = evals[l - 1]; // = Aₗ₋₁(−r^{2ˡ⁻¹})
            const Fr u = mle_vars[l - 1];     // = uₗ₋₁

            // The folding property ensures that
            //                     Aₗ₋₁(r^{2ˡ⁻¹}) + Aₗ₋₁(−r^{2ˡ⁻¹})      Aₗ₋₁(r^{2ˡ⁻¹}) - Aₗ₋₁(−r^{2ˡ⁻¹})
            // Aₗ(r^{2ˡ}) = (1-uₗ₋₁) ----------------------------- + uₗ₋₁ -----------------------------
            //                                   2                                2r^{2ˡ⁻¹}
            // We solve the above equation in Aₗ₋₁(r^{2ˡ⁻¹}), using the previously computed Aₗ(r^{2ˡ}) in eval_pos
            // and using Aₗ₋₁(−r^{2ˡ⁻¹}) sent by the prover in the proof.
            eval_pos = ((r * eval_pos * 2) - eval_neg * (r * (Fr(1) - u) - u)) / (r * (Fr(1) - u) + u);
        }

        return eval_pos; // return A₀(r)
    };

    static std::vector<Fr> squares_of_r(const Fr r, const size_t num_squares)
    {
        std::vector<Fr> squares = { r };
        squares.reserve(num_squares);
        for (size_t j = 1; j < num_squares; j++) {
            squares.emplace_back(squares[j - 1].sqr());
        }
        return squares;
    };

    /**
     * @brief Computes two commitments to A₀ partially evaluated in r and -r.
     *
     * @param claims_f array of claims containing commitments to non-shifted polynomials
     * @param claims_g array of claims containing commitments to shifted polynomials
     * @param rhos vector of m powers of rho used for linear combination
     * @param r evaluation point at which we have partially evaluated A₀ at r and -r.
     * @return std::pair<Commitment, Commitment>  c0_r_pos, c0_r_neg
     */
    static std::pair<Commitment, Commitment> compute_simulated_commitments(std::span<const Commitment> commitments_f,
                                                                           std::span<const Commitment> commitments_g,
                                                                           std::span<const Fr> rhos,
                                                                           Fr r)
    {
        const size_t num_commitments_f = commitments_f.size();
        const size_t num_commitments_g = commitments_g.size();

        Fr r_inv = r.invert();

        // Commitment to F(X), G(X)
        Commitment batched_f = Commitment::zero();
        std::span rhos_f = rhos.subspan(0, num_commitments_f);
        for (size_t j = 0; j < num_commitments_f; ++j) {
            batched_f += commitments_f[j] * rhos_f[j];
        }

        // Commitment to G(X)
        Commitment batched_g = Commitment::zero();
        std::span rhos_g = rhos.subspan(num_commitments_f, num_commitments_g);
        for (size_t j = 0; j < num_commitments_g; ++j) {
            batched_g += commitments_g[j] * rhos_g[j];
        }

        // C₀ᵣ₊ = [F] + r⁻¹⋅[G]
        Commitment C0_r_pos = batched_f;
        // C₀ᵣ₋ = [F] - r⁻¹⋅[G]
        Commitment C0_r_neg = batched_f;
        if (!batched_g.is_point_at_infinity()) {
            batched_g *= r_inv;
            C0_r_pos += batched_g;
            C0_r_neg -= batched_g;
        }
        return { C0_r_pos, C0_r_neg };
    };
};
} // namespace honk::pcs::gemini