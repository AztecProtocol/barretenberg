#pragma once

#include "sumcheck_round.hpp"

#include "honk/sumcheck/polynomials/univariate.hpp"
#include "honk/sumcheck/polynomials/pow.hpp"
#include "honk/sumcheck/relations/relation.hpp"
#include "honk/transcript/transcript.hpp"
#include "proof_system/flavor/flavor.hpp"

#include <optional>
#include <array>
#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <vector>
namespace honk::sumcheck {

template <typename FF> struct SumcheckOutput {
    std::vector<FF> evaluation_point;
    std::array<FF, bonk::StandardArithmetization::NUM_POLYNOMIALS> evaluations;

    bool operator==(const SumcheckOutput& other) const = default;
};

template <typename FF, template <class> class... Relations> class Sumcheck {

  public:
    // TODO(luke): this value is needed here but also lives in sumcheck_round
    static constexpr size_t MAX_RELATION_LENGTH = std::max({ Relations<FF>::RELATION_LENGTH... });
    static constexpr size_t NUM_POLYNOMIALS = bonk::StandardArithmetization::NUM_POLYNOMIALS;

    using RoundUnivariate = Univariate<FF, MAX_RELATION_LENGTH>;
    const size_t multivariate_n;
    const size_t multivariate_d;
    RelationParameters<FF> relation_parameters;

    /**
    *
    * @brief (folded_polynomials) Suppose the Honk polynomials (multilinear in d variables) are called P_1, ..., P_N.
    * At initialization,
    * we think of these as lying in a two-dimensional array, where each column records the value of one P_i on H^d.
    * After the first round, the array will be updated ('folded'), so that the first n/2 rows will represent the
    * evaluations P_i(u0, X1, ..., X_{d-1}) as a low-degree extension on H^{d-1}. In reality, we elude copying all
    * of the polynomial-defining data by only populating folded_multivariates after the first round. I.e.:

        We imagine all of the defining polynomial data in a matrix like this:
                    | P_1 | P_2 | P_3 | P_4 | ... | P_N | N = number of multivariatesk
                    |-----------------------------------|
          group 0 --|  *  |  *  |  *  |  *  | ... |  *  | vertex 0
                  \-|  *  |  *  |  *  |  *  | ... |  *  | vertex 1
          group 1 --|  *  |  *  |  *  |  *  | ... |  *  | vertex 2
                  \-|  *  |  *  |  *  |  *  | ... |  *  | vertex 3
                    |  *  |  *  |  *  |  *  | ... |  *  |
        group m-1 --|  *  |  *  |  *  |  *  | ... |  *  | vertex n-2
                  \-|  *  |  *  |  *  |  *  | ... |  *  | vertex n-1
            m = n/2
                                        *
            Each group consists of N edges |, and our construction of univariates and folding
                                        *
            operations naturally operate on these groups of edges

    *
    * NOTE: With ~40 columns, prob only want to allocate 256 EdgeGroup's at once to keep stack under 1MB?
    * TODO(Cody): might want to just do C-style multidimensional array? for guaranteed adjacency?
    */

    // prover instantiates sumcheck with circuit size and transcript
    Sumcheck(size_t multivariate_n, const RelationParameters<FF>& relation_parameters)
        : multivariate_n(multivariate_n)
        , multivariate_d(numeric::get_msb(multivariate_n))
        , relation_parameters(relation_parameters){};

    /**
     * @brief Compute univariate restriction place in transcript, generate challenge, fold,... repeat until final round,
     * then compute multivariate evaluations and place in transcript.
     *
     * @details
     */
    SumcheckOutput<FF> execute_prover(auto full_polynomials, // pass by value, not by reference
                                      ProverTranscript<FF>& transcript)
    {
        auto [alpha, zeta] = transcript.get_challenges("Sumcheck:alpha", "Sumcheck:zeta");

        SumcheckRound<FF, NUM_POLYNOMIALS, Relations...> round(multivariate_n, std::tuple(Relations<FF>()...));

        std::array<std::vector<FF>, NUM_POLYNOMIALS> folded_polynomials;
        for (auto& polynomial : folded_polynomials) {
            polynomial.resize(multivariate_n >> 1);
        }

        std::vector<FF> evaluation_points;
        evaluation_points.reserve(multivariate_d);

        PowUnivariate<FF> pow_univariate(zeta);

        // First round
        RoundUnivariate round_univariate =
            round.compute_univariate(full_polynomials, relation_parameters, pow_univariate, alpha);

        transcript.send_to_verifier("Sumcheck:T_0", round_univariate);
        FF round_challenge = transcript.get_challenge("Sumcheck:u_0");
        evaluation_points.emplace_back(round_challenge);

        // This populates folded_polynomials.
        fold(folded_polynomials, full_polynomials, multivariate_n, round_challenge);
        pow_univariate.partially_evaluate(round_challenge);
        round.round_size = round.round_size >> 1; // TODO(Cody): Maybe fold should do this and release memory?

        // All but final round
        // We operate on folded_polynomials in place.
        for (size_t round_idx = 1; round_idx < multivariate_d; round_idx++) {
            // Write the round univariate to the transcript
            round_univariate = round.compute_univariate(folded_polynomials, relation_parameters, pow_univariate, alpha);
            std::string round_univariate_label = "Sumcheck:T_" + std::to_string(round_idx);
            transcript.send_to_verifier(round_univariate_label, round_univariate);

            // Get the challenge
            std::string round_challenge_label = "Sumcheck:u_" + std::to_string(round_idx);
            round_challenge = transcript.get_challenge(round_challenge_label);
            evaluation_points.emplace_back(round_challenge);

            fold(folded_polynomials, folded_polynomials, round.round_size, round_challenge);
            pow_univariate.partially_evaluate(round_challenge);
            round.round_size = round.round_size >> 1;
        }

        // Final round: Extract multivariate evaluations from folded_polynomials and add to transcript
        std::array<FF, NUM_POLYNOMIALS> multivariate_evaluations;
        for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
            multivariate_evaluations[i] = folded_polynomials[i][0];
        }
        transcript.send_to_verifier("Sumcheck:evaluations", multivariate_evaluations);

        return SumcheckOutput<FF>{ .evaluation_point = evaluation_points, .evaluations = multivariate_evaluations };
    };

    /**
     * @brief Extract round univariate, check sum, generate challenge, compute next target sum..., repeat until final
     * round, then use purported evaluations to generate purported full Honk relation value and check against final
     * target sum.
     */
    std::optional<SumcheckOutput<FF>> execute_verifier(VerifierTranscript<FF>& transcript)
    {

        auto [alpha, zeta] = transcript.get_challenges("Sumcheck:alpha", "Sumcheck:zeta");

        PowUnivariate<FF> pow_univariate{ zeta };

        // Used to evaluate T_l in each round.
        auto barycentric = BarycentricData<FF, MAX_RELATION_LENGTH, MAX_RELATION_LENGTH>();

        std::vector<FF> evaluation_points;
        evaluation_points.reserve(multivariate_d);

        // initialize sigma_0 = 0
        FF target_sum{ 0 };
        for (size_t round_l = 0; round_l < multivariate_d; ++round_l) {
            // Obtain the round univariate from the transcript
            std::string round_univariate_label = "Sumcheck:T_" + std::to_string(round_l);
            auto T_l = transcript.template receive_from_prover<RoundUnivariate>(round_univariate_label);

            // S^{l}(0) = ( (1−0) + 0⋅ζ^{ 2^{l} } ) ⋅ T^{l}(0) = T^{l}(0)
            // S^{l}(1) = ( (1−1) + 1⋅ζ^{ 2^{l} } ) ⋅ T^{l}(1) = ζ^{ 2^{l} } ⋅ T^{l}(1)
            FF claimed_sum = T_l.value_at(0) + (pow_univariate.zeta_pow * T_l.value_at(1));

            if (claimed_sum != target_sum) {
                return std::nullopt;
            }

            std::string round_challenge_label = "Sumcheck:u_" + std::to_string(round_l);
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

        FF full_eval = compute_full_evaluation(
            purported_evaluations, pow_univariate.partial_evaluation_constant, relation_parameters, alpha);

        // Check against sigma_d
        if (full_eval != target_sum) {
            return std::nullopt;
        }

        return SumcheckOutput<FF>{ .evaluation_point = evaluation_points, .evaluations = purported_evaluations };
    }

    // TODO(Cody): Rename. fold is not descriptive, and it's already in use in the Gemini context.
    //             Probably just call it partial_evaluation?
    /**
     * @brief Evaluate at the round challenge and prepare class for next round.
     * Illustration of layout in example of first round when d==3 (showing just one Honk polynomial,
     * i.e., what happens in just one column of our two-dimensional array):
     *
     * groups    vertex terms              collected vertex terms               groups after folding
     *     g0 -- v0 (1-X0)(1-X1)(1-X2) --- (v0(1-X0) + v1 X0) (1-X1)(1-X2) ---- (v0(1-u0) + v1 u0) (1-X1)(1-X2)
     *        \- v1   X0  (1-X1)(1-X2) --/                                  --- (v2(1-u0) + v3 u0)   X1  (1-X2)
     *     g1 -- v2 (1-X0)  X1  (1-X2) --- (v2(1-X0) + v3 X0)   X1  (1-X2)-/ -- (v4(1-u0) + v5 u0) (1-X1)  X2
     *        \- v3   X0    X1  (1-X2) --/                                  / - (v6(1-u0) + v7 u0)   X1    X2
     *     g2 -- v4 (1-X0)(1-X1)  X2   --- (v4(1-X0) + v5 X0) (1-X1)  X2  -/ /
     *        \- v5   X0  (1-X1)  X2   --/                                  /
     *     g3 -- v6 (1-X0)  X1    X2   --- (v6(1-X0) + v7 X0)   X1    X2  -/
     *        \- v7   X0    X1    X2   --/
     *
     * @param challenge
     */
    void fold(auto& folded_polynomials, const auto& polynomials, size_t round_size, FF round_challenge)
    {
        // after the first round, operate in place on folded_polynomials
        for (size_t j = 0; j < polynomials.size(); ++j) {
            // for (size_t j = 0; j < bonk::StandardArithmetization::NUM_POLYNOMIALS; ++j) {
            for (size_t i = 0; i < round_size; i += 2) {
                folded_polynomials[j][i >> 1] =
                    polynomials[j][i] + round_challenge * (polynomials[j][i + 1] - polynomials[j][i]);
            }
        }
    };

    static FF compute_full_evaluation(const std::array<FF, NUM_POLYNOMIALS>& multivariate_evaluations,
                                      const FF pow_zeta_eval,
                                      const RelationParameters<FF>& relation_parameters,
                                      const FF alpha)
    {
        // Compute the evaluations of each relation
        // Create an array of the same size as the `Relations` pack,
        std::array relation_evals = { Relations<FF>::evaluate_full_relation_value_contribution(
            multivariate_evaluations, relation_parameters)... };

        // Do the alpha-linear combination
        FF alpha_pow{ alpha };
        FF full_eval{ relation_evals[0] };
        for (size_t i = 1; i < relation_evals.size(); ++i) {
            full_eval += relation_evals[i] * alpha_pow;
            alpha_pow *= alpha;
        }
        // Multiply by the evaluation of pow
        full_eval *= pow_zeta_eval;
        return full_eval;
    }
};
} // namespace honk::sumcheck
