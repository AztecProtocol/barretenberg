#pragma once
#include "sumcheck_round.hpp"
#include "relations/relation.hpp"
#include "polynomials/pow.hpp"
#include "polynomials/univariate.hpp"
#include "barretenberg/honk/transcript/transcript.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"
#include "barretenberg/numeric/bitop/get_msb.hpp"

#include <algorithm>
#include <optional>
#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace honk::sumcheck {

/**
 * @brief Contains the multi-linear `evaluations` of the polynomials at the `evaluation_point`.
 * These are computed by the prover and need to be checked using a multi-linear PCS like Gemini.
 */
template <typename FF> struct SumcheckOutput {
    // u = (u_0, ..., u_{d-1})
    std::vector<FF> evaluation_point;
    // Evaluations in `u` of the polynomials used in Sumcheck
    std::array<FF, bonk::StandardArithmetization::NUM_POLYNOMIALS> evaluations;

    bool operator==(const SumcheckOutput& other) const = default;
};

/**
 * @brief Partially evaluate the multi-linear polynomial contained in `source`, at the point `round_challenge`,
 * storing the result in `destination`.
 *
 * @param destination vector to store the resulting evaluations. It is resized to half the size of `source`.
 *   P'(X_1, ..., X_{d-1}) = P(u_0, X_1, ..., X_{d-1}), with n/2 cofficients [v'_0, ..., v'_{2^{d-1}-1}], where
 *   v'_i = v_{2i} * (1 - u_0) + v_{2i+1} * u_0.
 * @param source coefficients of the polynomial we want to evaluate.
 *   P(X_0, X_1, ..., X_{d-1}) with n=2^d coefficients [v_0, ..., v_{2^{d}-1}].
 * @param round_challenge partial evaluation point `u_0`
 *
 * @details Illustration of layout in example of first round when d==3:
 *
 * groups    vertex terms              collected vertex terms               groups after partial evaluation
 *     g0 -- v0 (1-X0)(1-X1)(1-X2) --- (v0(1-X0) + v1 X0) (1-X1)(1-X2) ---- (v0(1-u0) + v1 u0) (1-X1)(1-X2)
 *        \- v1   X0  (1-X1)(1-X2) --/                                  --- (v2(1-u0) + v3 u0)   X1  (1-X2)
 *     g1 -- v2 (1-X0)  X1  (1-X2) --- (v2(1-X0) + v3 X0)   X1  (1-X2)-/ -- (v4(1-u0) + v5 u0) (1-X1)  X2
 *        \- v3   X0    X1  (1-X2) --/                                  / - (v6(1-u0) + v7 u0)   X1    X2
 *     g2 -- v4 (1-X0)(1-X1)  X2   --- (v4(1-X0) + v5 X0) (1-X1)  X2  -/ /
 *        \- v5   X0  (1-X1)  X2   --/                                  /
 *     g3 -- v6 (1-X0)  X1    X2   --- (v6(1-X0) + v7 X0)   X1    X2  -/
 *        \- v7   X0    X1    X2   --/
 */
template <typename FF>
static void partially_evaluate(std::vector<FF>& destination, std::span<const FF> source, FF round_challenge)
{
    const size_t source_size = source.size();         // = 2^{d}
    const size_t destination_size = source_size >> 1; // = 2^{d-1}
    if (destination.size() < destination_size) {
        // resizing may invalidate `source`
        ASSERT(destination.data() != source.data());
        destination.resize(destination_size);
    }
    for (size_t i = 0; i < destination_size; ++i) {
        const FF coefficient_even = source[i << 1];      // v_{ 2i }
        const FF coefficient_odd = source[(i << 1) + 1]; // v_{2i+1}
        // v'_i = (1-u_0) * v_{ 2i } + u_0 * v_{2i+1} = v_{ 2i } + u_0 * (v_{2i+1} - v_{ 2i })
        destination[i] = coefficient_even + round_challenge * (coefficient_odd - coefficient_even);
    }
    destination.resize(destination_size);
}

template <typename FF, template <class> class... Relations> class Sumcheck {

  public:
    static constexpr size_t MAX_RELATION_LENGTH = std::max({ Relations<FF>::RELATION_LENGTH... });
    static constexpr size_t NUM_POLYNOMIALS = bonk::StandardArithmetization::NUM_POLYNOMIALS;

    using RoundUnivariate = Univariate<FF, MAX_RELATION_LENGTH>;

    /**
     * @brief Compute univariate restriction place in transcript, generate challenge, fold,... repeat until final round,
     * then compute multivariate evaluations and place in transcript.
     *
     * @details
     */
    static SumcheckOutput<FF> execute_prover(const size_t multivariate_d,
                                             const RelationParameters<FF>& relation_parameters,
                                             auto full_polynomials, // pass by value, not by reference
                                             ProverTranscript<FF>& transcript)
    {
        auto [alpha, zeta] = transcript.get_challenges("Sumcheck:alpha", "Sumcheck:zeta");
        /**
         * Suppose the Honk `full_polynomials` (multilinear in d variables) are called P_1, ..., P_{NUM_POLYNOMIALS}.
         * At initialization, we think of these as lying in a two-dimensional array, where each column records the value
         * of one P_i on H^d.
         * After the first round, the array will be updated ('partially evaluated'), so that the first n/2 rows will
         * represent the evaluations P_i(u0, X1, ..., X_{d-1}) as a low-degree extension on H^{d-1}.
         *
         *    We imagine all of the defining polynomial data in a matrix like this:
         *                | P_1 | P_2 | P_3 | P_4 | ... | P_N | N = NUM_POLYNOMIALS
         *                |-----------------------------------|
         *      group 0 --|  *  |  *  |  *  |  *  | ... |  *  | vertex 0
         *              \-|  *  |  *  |  *  |  *  | ... |  *  | vertex 1
         *      group 1 --|  *  |  *  |  *  |  *  | ... |  *  | vertex 2
         *              \-|  *  |  *  |  *  |  *  | ... |  *  | vertex 3
         *                |  *  |  *  |  *  |  *  | ... |  *  |
         *    group m-1 --|  *  |  *  |  *  |  *  | ... |  *  | vertex n-2
         *              \-|  *  |  *  |  *  |  *  | ... |  *  | vertex n-1
         *        m = n/2
         *
         * At the start of round l>0, the array contains the n/2^l evaluation of
         * P_i(u0, ..., u_{l-1}, X_l, ..., X_{d-1}), and at the end of the round it is evaluated in X_l = u_l,
         * where u_l is the challenge point sent by the verifier.
         *
         * Since the Honk prover requires the full description of the `full_polynomials` for the PCS opening, we store
         * the partial evaluations in `partially_evaluated_polynomials`, an array of temporary buffers.
         * To make the rounds uniform, we perform the Sumcheck rounds over `round_polynomials` which represents the
         * matrix of partially evaluated polynomials for the given round.
         *
         *
         * NOTE: With ~40 columns, prob only want to allocate 256 EdgeGroup's at once to keep stack under 1MB?
         * TODO(#224)(Cody): might want to just do C-style multidimensional array? for guaranteed adjacency?
         */
        std::array<std::span<const FF>, NUM_POLYNOMIALS> round_polynomials;
        for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
            round_polynomials[i] = full_polynomials[i];
        }

        /**
         * Array of temporary buffers containing the partial evaluations of the `full_polynomials`
         * The function `partially_evaluate` takes care of resizing the vectors after each partial evaluation.
         */
        std::array<std::vector<FF>, NUM_POLYNOMIALS> partially_evaluated_polynomials;

        // Store the multi-variate evaluation point u = (u_0, ..., u_{d-1})
        std::vector<FF> evaluation_points;
        evaluation_points.reserve(multivariate_d);

        // The pow polynomial and its partial evaluations can be efficiently computed by the PowUnivariate class.
        PowUnivariate<FF> pow_univariate(zeta);

        // Perform the d Sumcheck rounds
        for (size_t round_idx = 0; round_idx < multivariate_d; round_idx++) {
            const size_t round_size = 1 << (multivariate_d - round_idx);
            // Write the round univariate T_l(X) to the transcript
            // Multiply by [(1-X) + ζ^{ 2^{l} } X] to obtain the full round univariate S_l(X).
            RoundUnivariate round_univariate = SumcheckRound<FF, NUM_POLYNOMIALS, Relations...>::compute_univariate(
                round_polynomials, round_size, relation_parameters, pow_univariate, alpha);
            std::string round_univariate_label = "Sumcheck:T_" + std::to_string(round_idx);
            transcript.send_to_verifier(round_univariate_label, round_univariate);

            // Get the challenge
            std::string round_challenge_label = "Sumcheck:u_" + std::to_string(round_idx);
            const FF round_challenge = transcript.get_challenge(round_challenge_label);
            evaluation_points.emplace_back(round_challenge);

            // Perform partial evaluation of the round polynomials, in the evaluation point u_l
            for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
                // The polynomials in `partially_evaluated_polynomials` are resized appropriately.
                partially_evaluate(partially_evaluated_polynomials[i], round_polynomials[i], round_challenge);
                // Sets the polynomials for the next round.
                round_polynomials[i] = partially_evaluated_polynomials[i];
            }
            pow_univariate.partially_evaluate(round_challenge);
        }

        // Final round: Extract multivariate evaluations from folded_polynomials and add to transcript
        std::array<FF, NUM_POLYNOMIALS> multivariate_evaluations;
        for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
            ASSERT(partially_evaluated_polynomials[i].size() == 1);
            multivariate_evaluations[i] = partially_evaluated_polynomials[i][0];
        }
        transcript.send_to_verifier("Sumcheck:evaluations", multivariate_evaluations);

        return SumcheckOutput<FF>{ .evaluation_point = evaluation_points, .evaluations = multivariate_evaluations };
    };

    /**
     * @brief Extract round univariate, check sum, generate challenge, compute next target sum..., repeat until final
     * round, then use purported evaluations to generate purported full Honk relation value and check against final
     * target sum.
     */
    static std::optional<SumcheckOutput<FF>> execute_verifier(const size_t multivariate_d,
                                                              const RelationParameters<FF>& relation_parameters,
                                                              auto& transcript)
    {
        auto [alpha, zeta] = transcript.get_challenges("Sumcheck:alpha", "Sumcheck:zeta");

        PowUnivariate<FF> pow_univariate(zeta);

        // Used to evaluate T_l in each round.
        auto barycentric = BarycentricData<FF, MAX_RELATION_LENGTH, MAX_RELATION_LENGTH>();

        // Keep track of the evaluation points and so we can return them upon successful verification.
        std::vector<FF> evaluation_points;
        evaluation_points.reserve(multivariate_d);

        // initialize sigma_0 = 0
        FF target_sum{ 0 };
        for (size_t round_idx = 0; round_idx < multivariate_d; ++round_idx) {
            // Obtain the round univariate from the transcript
            std::string round_univariate_label = "Sumcheck:T_" + std::to_string(round_idx);
            auto T_l = transcript.template receive_from_prover<RoundUnivariate>(round_univariate_label);

            // The "real" round polynomial S_l(X) is ( (1−X) + X⋅ζ^{ 2^{l} } ) ⋅ T_l(X)
            // S_l(0) + S_l(1) = T^{l}(0) + ζ^{ 2^{l} } ⋅ T^{l}(1), since
            //   S^{l}(0) = ( (1−0) + 0⋅ζ^{ 2^{l} } ) ⋅ T^{l}(0) = T^{l}(0)
            //   S^{l}(1) = ( (1−1) + 1⋅ζ^{ 2^{l} } ) ⋅ T^{l}(1) = ζ^{ 2^{l} } ⋅ T^{l}(1)
            FF claimed_sum = T_l.value_at(0) + (pow_univariate.zeta_pow * T_l.value_at(1));

            // If the evaluation is wrong, abort and return nothing to indicate failure.
            if (claimed_sum != target_sum) {
                return std::nullopt;
            }

            // Get the next challenge point and store it.
            std::string round_challenge_label = "Sumcheck:u_" + std::to_string(round_idx);
            auto u_l = transcript.get_challenge(round_challenge_label);
            evaluation_points.emplace_back(u_l);

            // Compute next target_sum
            // sigma_{l+1} = S^l(u_l)
            //             = T^l(u_l) * [ (1−u_l) + u_l⋅ζ^{ 2^l } ]
            target_sum = barycentric.evaluate(T_l, u_l) * pow_univariate.univariate_eval(u_l);

            // Partially evaluate the pow_zeta polynomial
            pow_univariate.partially_evaluate(u_l);
        }

        // Final round
        auto purported_evaluations =
            transcript.template receive_from_prover<std::array<FF, NUM_POLYNOMIALS>>("Sumcheck:evaluations");

        // The full Honk identity evaluated in the opening point u.
        FF full_eval = compute_full_evaluation(
            purported_evaluations, pow_univariate.partial_evaluation_constant, relation_parameters, alpha);

        // Check against sigma_d
        if (full_eval != target_sum) {
            return std::nullopt;
        }

        return SumcheckOutput<FF>{ .evaluation_point = evaluation_points, .evaluations = purported_evaluations };
    }

    /**
     * @brief Given the multi-linear evaluations of the Sumcheck polynomials,
     * computes the evaluation of the full Honk identity.
     *
     * @param multivariate_evaluations container with polynomial
     * @param pow_zeta_eval Evaluation of pow, computed by the verifier which multiplies the full identity.
     * @param relation_parameters required by certain relations.
     * @param alpha relation separation challenge.
     * @return
     */
    static FF compute_full_evaluation(std::span<const FF, NUM_POLYNOMIALS> multivariate_evaluations,
                                      const FF pow_zeta_eval,
                                      const RelationParameters<FF>& relation_parameters,
                                      const FF alpha)
    {
        constexpr size_t NUM_RELATIONS = sizeof...(Relations);
        // Compute the evaluations of each relation by unpacking the type `Relations`
        std::array<FF, NUM_RELATIONS> relation_evals = { Relations<FF>::evaluate_full_relation_value_contribution(
            multivariate_evaluations, relation_parameters)... };

        // Do the alpha-linear combination
        FF alpha_pow{ alpha };
        FF full_eval{ relation_evals[0] };
        for (size_t i = 1; i < NUM_RELATIONS; ++i) {
            full_eval += relation_evals[i] * alpha_pow;
            alpha_pow *= alpha;
        }
        // Multiply by the evaluation of pow
        full_eval *= pow_zeta_eval;
        return full_eval;
    }
};
} // namespace honk::sumcheck
