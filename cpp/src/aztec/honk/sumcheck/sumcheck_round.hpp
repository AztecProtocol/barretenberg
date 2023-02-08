#pragma once
#include <common/log.hpp>
#include <array>
#include <algorithm>
#include <tuple>
#include "polynomials/barycentric_data.hpp"
#include "polynomials/univariate.hpp"
#include "polynomials/pow.hpp"
#include "relations/relation.hpp"

namespace honk::sumcheck {

/*
 Notation: The polynomial P(X1, X2) that is the low-degree extension of its values vij = P(i,j)
 for i,j ∈ H = {0,1} is conveniently recorded as follows:
     (0,1)-----(1,1)   v01 ------ v11
       |          |     |          |  P(X1,X2) =   (v01 * (1-X1) + v11 * X1) *   X2
       |   H^2    |     | P(X1,X2) |             + (v00 * (1-X1) + v10 * X1) * (1-X2)
       |          |     |          |
     (0,0) ---- (1,0)  v00 -------v10
*/

/*
 Example: There are two low-degree extensions Y1, Y2 over the square H^2 in the Cartesian plane.

    3 -------- 7   4 -------- 8
    |          |   |          | Let F(X1, X2) = G(Y1, Y2) =     G0(Y1(X1, X2) + Y2(X1, X2))
    |    Y1    |   |    Y2    |                             + α G1(Y1(X1, X2) + Y2(X1, X2)),
    |          |   |          |  where the relations are G0(Y1, Y2) = Y1 * Y2
    1 -------- 5   2 -------- 6                      and G1(Y1, Y2) = Y1 + Y2.

 G1, G2 together comprise the Relations.

 In the first round, the computations will relate elements along veritcal lines. As a mnemonic, we
 use the term "edge" for the linear, univariate polynomials corresponding to the four lines
  3   4   7   8
  | , | , | , | .
  1   2   5   6

 The polynomials Y1, Y2 are stored in an array in Multivariates. In the first round, these are arrays
 of spans living outside of the Multivariates object, and in sebsequent rounds these are arrays of field
 elements that are stored in the Multivariates. The rationale for adopting this model is to
 avoid copying the full-length polynomials; this way, the largest polynomial array stores in a
 Multivariates class is multivariates_n/2.

 Note: This class uses recursive function calls with template parameters. This is a common trick that is used to force
 the compiler to unroll loops. The idea is that a function that is only called once will always be inlined, and since
 template functions always create different functions, this is guaranteed.
 */

template <class FF, size_t num_multivariates, template <class> class... Relations> class SumcheckRound {

  public:
    bool round_failed = false;
    size_t round_size; // a power of 2

    std::tuple<Relations<FF>...> relations;
    static constexpr size_t NUM_RELATIONS = sizeof...(Relations);
    static constexpr size_t MAX_RELATION_LENGTH = std::max({ Relations<FF>::RELATION_LENGTH... });
    static constexpr std::array<size_t, NUM_RELATIONS> RELATION_LENGTHS{ { Relations<FF>::RELATION_LENGTH... } };

    FF target_total_sum = 0;

    // TODO(Cody): this barycentric stuff should be more built-in?
    std::tuple<BarycentricData<FF, Relations<FF>::RELATION_LENGTH, MAX_RELATION_LENGTH>...> barycentric_utils;
    std::tuple<Univariate<FF, Relations<FF>::RELATION_LENGTH>...> univariate_accumulators;
    std::array<Univariate<FF, MAX_RELATION_LENGTH>, num_multivariates> extended_edges;
    std::array<Univariate<FF, MAX_RELATION_LENGTH>, NUM_RELATIONS> extended_univariates;

    // TODO(Cody): this should go away and we should use constexpr method to extend
    BarycentricData<FF, 2, MAX_RELATION_LENGTH> barycentric_2_to_max = BarycentricData<FF, 2, MAX_RELATION_LENGTH>();

    // Prover constructor
    SumcheckRound(size_t initial_round_size, auto&& relations)
        : round_size(initial_round_size)
        , relations(relations)
        , barycentric_utils(BarycentricData<FF, Relations<FF>::RELATION_LENGTH, MAX_RELATION_LENGTH>()...)
        , univariate_accumulators(Univariate<FF, Relations<FF>::RELATION_LENGTH>()...)
    {}

    // Verifier constructor
    explicit SumcheckRound(auto relations)
        : relations(relations)
        // TODO(Cody): this is a hack; accumulators not needed by verifier
        , univariate_accumulators(Univariate<FF, Relations<FF>::RELATION_LENGTH>()...){};

    /**
     * @brief After computing the round univariate, it is necessary to zero-out the accumulators used to compute it.
     */
    template <size_t idx = 0> void reset_accumulators()
    {
        auto& univariate = std::get<idx>(univariate_accumulators);
        std::fill(univariate.evaluations.begin(), univariate.evaluations.end(), FF(0));

        if constexpr (idx + 1 < NUM_RELATIONS) {
            reset_accumulators<idx + 1>();
        }
    };

    /**
     * @brief Compute the sum of all the elements in the accumulators.
     *
     * @tparam T Container type
     * @tparam N number of relations that have been evaluated.
     * @param accumulators array of N elements to combine
     * @return T = ∑_i accumulators[i] * alpha^i
     */
    template <typename T, size_t N> T batch_over_relations(const std::array<T, N>& accumulators, const FF& alpha)
    {
        // Iterate over the array in reverse order to avoid the extra multiplication by alpha
        T result{ 0 };
        for (size_t i = N - 1; i != 0; --i) {
            result += accumulators[i];
            result *= alpha;
        }
        result += accumulators[0];
        return result;
    }

    /**
     * @brief Evaluate some relations by evaluating each edge in the edge group at
     * Univariate::length-many values. Store each value separately in the corresponding
     * entry of relation_evals.
     *
     * @details Should only be called externally with relation_idx equal to 0.
     *
     */
    void extend_edges(auto& multivariates, size_t edge_idx)
    {
        for (size_t idx = 0; idx < num_multivariates; idx++) {
            auto edge = Univariate<FF, 2>({ multivariates[idx][edge_idx], multivariates[idx][edge_idx + 1] });
            extended_edges[idx] = barycentric_2_to_max.extend(edge);
        }
    }

    /**
     * @brief Evaluate the specified relation using the polynomials in `columns`, scale it,
     * and add it to the corresponding accumulator.
     *
     * @details This function is almost uniform over FF and Univariate. To minimize the
     * amount of work required when we are running the prover, we need to first
     * transform the columns into UnivariateView which shrinks the length.
     *
     * @post accumulator[r_idx] += scaling_factor * relation.evaluate(columns)
     *
     * @tparam relation_idx Index of the current relation we are evaluating
     * @param accumulators std::array indexed by each relations containing the accumulated (randomized) sum of
     * evaluations of this relation.
     * @param columns std::array of evaluations (FF or Univariate) indexed by the number of polynomials.
     * @param relation_parameters Generic struct with challenges
     * @param scaling_factor Either 1 (if we are evaluating as the verifier) or (zeta^2^{l+1})^i (as the prover),
     * multiplies the evaluated relation before it is added to the accumulator
     */
    template <size_t relation_idx>
    void accumulate_relations_at_idx(auto& accumulators,
                                     const auto& columns,
                                     const RelationParameters<FF>& relation_parameters,
                                     const FF scaling_factor)
    {
        auto& accumulator = std::get<relation_idx>(accumulators);
        const auto& relation = std::get<relation_idx>(relations);

        // TODO: For now, we need to distinguish between the case where we are summing Field elements or Univariates.
        if constexpr (std::is_same_v<decltype(accumulator), FF&>) {
            relation.accumulate_relation_evaluation(accumulator, columns, relation_parameters, scaling_factor);
        } else {
            constexpr size_t relation_length = RELATION_LENGTHS[relation_idx];

            // Create UnivariateView's from all the Univariates, where the size of the UnivariateView corresponds to
            // this relation's "length"
            const auto columns_view = array_to_array<UnivariateView<FF, relation_length>>(columns);

            relation.accumulate_relation_evaluation(accumulator, columns_view, relation_parameters, scaling_factor);
        }
    }

    /**
     * @brief For a given edge, calculate the contribution of each relation to the prover round univariate (S_l in the
     * thesis).
     *
     * @details In Round l, the univariate S_l computed by the prover is computed as follows:
     *   - Outer loop: iterate through the points on the boolean hypercube of dimension = log(round_size), skipping
     *                 every other point. On each iteration, create a Univariate<FF, 2> (an 'edge') for each
     *                 multivariate.
     *   - Inner loop: iterate through the relations, feeding each relation the present collection of edges. Each
     *                 relation adds a contribution
     *
     * Result: for each relation, a univariate of some degree is computed by accumulating the contributions of each
     * group of edges. These are stored in `univariate_accumulators`. Adding these univariates together, with
     * appropriate scaling factors, produces S_l.
     *
     * @note This function is essentially performing an unrolled loop. The actual body is contained in
     * add_scaled_expression_at_idx.
     */
    template <size_t relation_idx = 0>
    void accumulate_relations(auto& accumulators,
                              const auto& columns,
                              const RelationParameters<FF>& relation_parameters,
                              const FF scaling_factor)
    {
        accumulate_relations_at_idx<relation_idx>(accumulators, columns, relation_parameters, scaling_factor);

        // Repeat for the next relation.
        if constexpr (relation_idx + 1 < NUM_RELATIONS) {
            accumulate_relations<relation_idx + 1>(accumulators, columns, relation_parameters, scaling_factor);
        }
    }

    /**
     * @brief After executing each widget on each edge, producing a tuple of univariates of differing lenghths,
     * extend all univariates to the max of the lenghths required by the largest relation.
     *
     * @tparam relation_idx
     */
    template <size_t relation_idx = 0> void extend_univariate_accumulators()
    {
        extended_univariates[relation_idx] =
            std::get<relation_idx>(barycentric_utils).extend(std::get<relation_idx>(univariate_accumulators));

        // Repeat for the next relation.
        if constexpr (relation_idx + 1 < NUM_RELATIONS) {
            extend_univariate_accumulators<relation_idx + 1>();
        }
    }

    /**
     * @brief Return the evaluations of the univariate restriction (S_l(X_l) in the thesis) at num_multivariates-many
     * values. Most likely this will end up being S_l(0), ... , S_l(t-1) where t is around 12. At the end, reset all
     * univariate accumulators to be zero.
     */
    Univariate<FF, MAX_RELATION_LENGTH> compute_univariate(auto& polynomials,
                                                           const RelationParameters<FF>& relation_parameters,
                                                           const PowUnivariate<FF>& pow_univariate)
    {
        // For each edge_idx = 2i, we need to multiply the whole contribution by zeta^{2^{2i}}
        // This means that each univariate for each relation needs an extra multiplication.
        FF pow_challenge = pow_univariate.partial_evaluation_constant;
        for (size_t edge_idx = 0; edge_idx < round_size; edge_idx += 2) {
            extend_edges(polynomials, edge_idx);
            // Compute the i-th edge's univariate contribution,
            // scale it by the pow polynomial's constant and zeta power "cₗ ⋅ ζₗ₋₁ⁱ"
            // and add it to the accumulators for Sˡ(Xₗ)
            accumulate_relations(univariate_accumulators, extended_edges, relation_parameters, pow_challenge);
            // Update the pow polynomial's contribution ζₗ₋₁ⁱ for the next edge.
            pow_challenge *= pow_univariate.zeta_pow_sqr;
        }

        // Univariates have been computed for each relation.
        // Extend their evaluations so we can sum them all together.
        extend_univariate_accumulators<>();

        // Compute the linear combination of the (now extended) univariates for each relation, using powers of the alpha
        // challenge.
        auto result = batch_over_relations(extended_univariates, relation_parameters.alpha);

        reset_accumulators<>();

        return result;
    }

    /**
     * @brief Calculate the contribution of each relation to the expected value of the full Honk relation.
     *
     * @details For each relation, use the purported values (supplied by the prover) of the multivariates to calculate
     * a contribution to the purported value of the full Honk relation. These are stored in `evaluations`. Adding these
     * together, with appropriate scaling factors, produces the expected value of the full Honk relation. This value is
     * checked against the final value of the target total sum (called sigma_0 in the thesis).
     */
    // TODO(Cody): Input should be an array? Then challenge container has to know array length.
    FF compute_full_honk_relation_purported_value(std::vector<FF>& purported_evaluations,
                                                  const RelationParameters<FF>& relation_parameters,
                                                  const PowUnivariate<FF>& pow_univariate)
    {
        // initialize the accumulator for each relation and set it to zero.
        std::array<FF, NUM_RELATIONS> evaluation_accumulators;
        std::fill_n(evaluation_accumulators.begin(), NUM_RELATIONS, FF::zero());
        // iterate over each relation, evaluate it in the points given in purported_evaluations,
        // and scale using the alpha challenge in relation_parameters
        accumulate_relations(evaluation_accumulators, purported_evaluations, relation_parameters, FF::one());
        // sum all the evaluations, and multiply by the already evaluated pow polynomial.
        return batch_over_relations(evaluation_accumulators, relation_parameters.alpha) *
               pow_univariate.partial_evaluation_constant;
    }

    /**
     * @brief check if S^{l}(0) + S^{l}(1) = S^{l+1}(u_{l+1})
     *
     * @param univariate T^{l}(X), the round univariate that is equal to S^{l}(X)/( (1−X) + X⋅ζ^{ 2^{d-l-1} } )
     */
    bool check_sum(Univariate<FF, MAX_RELATION_LENGTH>& univariate, const PowUnivariate<FF>& pow_univariate)
    {
        // S^{l}(0) = ( (1−0) + 0⋅ζ^{ 2^{d-l-1} } ) ⋅ T^{l}(0) = T^{l}(0)
        // S^{l}(1) = ( (1−1) + 1⋅ζ^{ 2^{d-l-1} } ) ⋅ T^{l}(1) = ζ^{ 2^{d-l-1} } ⋅ T^{l}(1)
        FF total_sum = univariate.value_at(0) + (pow_univariate.zeta_pow * univariate.value_at(1));
        bool sumcheck_round_failed = (target_total_sum != total_sum);
        round_failed = round_failed || sumcheck_round_failed;
        return !sumcheck_round_failed;
    };

    /**
     * @brief After checking that the univariate is good for this round, compute the next target sum.
     *
     * @param univariate T^l(X), given by its evaluations over {0,1,2,...},
     * equal to S^{l}(X)/( (1−X) + X⋅ζ^{ 2^{d-l-1} } )
     * @param round_challenge u_l
     * @return FF sigma_{l} = S^l(u_l)
     */
    FF compute_next_target_sum(Univariate<FF, MAX_RELATION_LENGTH>& univariate,
                               FF& round_challenge,
                               const PowUnivariate<FF>& pow_univariate)
    {
        // IMPROVEMENT(Cody): Use barycentric static method, maybe implement evaluation as member
        // function on Univariate.
        auto barycentric = BarycentricData<FF, MAX_RELATION_LENGTH, MAX_RELATION_LENGTH>();
        // Evaluate T^{l}(u_{l})
        target_total_sum = barycentric.evaluate(univariate, round_challenge);
        // Evaluate (1−u_{l}) + u_{l}⋅ζ^{2^{d-l-1}} )
        FF pow_monomial_eval = pow_univariate.univariate_eval(round_challenge);
        // sigma_{l} = S^l(u_l) = (1−u_{l}) + u_{l}⋅ζ^{2^{d-l-1}} ) ⋅ T^{l}(u_{l})
        target_total_sum *= pow_monomial_eval;
        return target_total_sum;
    }
};
} // namespace honk::sumcheck
