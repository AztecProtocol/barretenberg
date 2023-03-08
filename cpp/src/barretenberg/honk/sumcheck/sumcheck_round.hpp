#pragma once
#include "barretenberg/common/log.hpp"
#include <array>
#include <algorithm>
#include <tuple>
#include "polynomials/barycentric_data.hpp"
#include "polynomials/univariate.hpp"
#include "polynomials/pow.hpp"
#include "relations/relation.hpp"

namespace honk::sumcheck {

/*
 Notation: The polynomial P(X0, X1) that is the low-degree extension of its values vij = P(i,j)
 for i,j ∈ H = {0,1} is conveniently recorded as follows:
     (0,1)-----(1,1)   v01 ------ v11
       |          |     |          |  P(X0,X1) =   (v00 * (1-X0) + v10 * X0) * (1-X1)
   X1  |   H^2    |     | P(X0,X1) |             + (v01 * (1-X0) + v11 * X0) *   X1
       |          |     |          |
     (0,0) ---- (1,0)  v00 -------v10
            X0
*/

/*
 Example: There are two low-degree extensions Y1, Y2 over the square H^2 in the Cartesian plane.

    3 -------- 7   4 -------- 8
    |          |   |          | Let F(X0, X1) = G(Y1, Y2) =     G0(Y1(X0, X1), Y2(X0, X1))
    |    Y1    |   |    Y2    |                             + α G1(Y1(X0, X1), Y2(X0, X1)),
    |          |   |          |  where the relations are G0(Y1, Y2) = Y1 * Y2
    1 -------- 5   2 -------- 6                      and G1(Y1, Y2) = Y1 + Y2.

 G1, G2 together comprise the Relations.

 In the first round, the computations will relate elements along horizontal lines. As a mnemonic, we
 use the term "edge" for the linear, univariate polynomials corresponding to the four lines
  1 - 5
  2 - 6
  3 - 7
  4 - 8

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

    FF target_total_sum = 0;

    // TODO(Cody): this barycentric stuff should be more built-in?
    std::tuple<BarycentricData<FF, Relations<FF>::RELATION_LENGTH, MAX_RELATION_LENGTH>...> barycentric_utils;
    std::tuple<Univariate<FF, Relations<FF>::RELATION_LENGTH>...> univariate_accumulators;
    std::array<FF, NUM_RELATIONS> evaluations;
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
        , univariate_accumulators(Univariate<FF, Relations<FF>::RELATION_LENGTH>()...)
    {
        // FF's default constructor may not initialize to zero (e.g., barretenberg::fr), hence we can't rely on
        // aggregate initialization of the evaluations array.
        std::fill(evaluations.begin(), evaluations.end(), FF(0));
    };

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
    // IMPROVEMENT(Cody): This is kind of ugly. There should be a one-liner with folding
    // or std::apply or something.

    /**
     * @brief Given a tuple t = (t_0, t_1, ..., t_{NUM_RELATIONS-1}) and a challenge α,
     * modify the tuple in place to (t_0, αt_1, ..., α^{NUM_RELATIONS-1}t_{NUM_RELATIONS-1}).
     */
    template <size_t idx = 0> void scale_tuple(auto& tuple, FF challenge, FF running_challenge)
    {
        std::get<idx>(tuple) *= running_challenge;
        running_challenge *= challenge;
        if constexpr (idx + 1 < NUM_RELATIONS) {
            scale_tuple<idx + 1>(tuple, challenge, running_challenge);
        }
    };

    /**
     * @brief Given a tuple t = (t_0, t_1, ..., t_{NUM_RELATIONS-1}) and a challenge α,
     * return t_0 + αt_1 + ... + α^{NUM_RELATIONS-1}t_{NUM_RELATIONS-1}).
     *
     * @tparam T : In practice, this is an FF or a Univariate<FF, MAX_NUM_RELATIONS>.
     */
    template <typename T> T batch_over_relations(FF challenge)
    {
        FF running_challenge = 1;
        scale_tuple<>(univariate_accumulators, challenge, running_challenge);
        extend_univariate_accumulators<>();
        auto result = T();
        for (size_t i = 0; i < NUM_RELATIONS; ++i) {
            result += extended_univariates[i];
        }
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

    // TODO(Cody): make private
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
     */
    template <size_t relation_idx = 0>
    void accumulate_relation_univariates(const RelationParameters<FF>& relation_parameters, const FF& scaling_factor)
    {
        std::get<relation_idx>(relations).add_edge_contribution(
            std::get<relation_idx>(univariate_accumulators), extended_edges, relation_parameters, scaling_factor);

        // Repeat for the next relation.
        if constexpr (relation_idx + 1 < NUM_RELATIONS) {
            accumulate_relation_univariates<relation_idx + 1>(relation_parameters, scaling_factor);
        }
    }

    // TODO(Cody): make private
    // TODO(Cody): make uniform with accumulate_relation_univariates
    /**
     * @brief Calculate the contribution of each relation to the expected value of the full Honk relation.
     *
     * @details For each relation, use the purported values (supplied by the prover) of the multivariates to calculate
     * a contribution to the purported value of the full Honk relation. These are stored in `evaluations`. Adding these
     * together, with appropriate scaling factors, produces the expected value of the full Honk relation. This value is
     * checked against the final value of the target total sum (called sigma_0 in the thesis).
     */
    template <size_t relation_idx = 0>
    // TODO(Cody): Input should be an array? Then challenge container has to know array length.
    void accumulate_relation_evaluations(std::vector<FF>& purported_evaluations,
                                         const RelationParameters<FF>& relation_parameters)
    {
        std::get<relation_idx>(relations).add_full_relation_value_contribution(
            evaluations[relation_idx], purported_evaluations, relation_parameters);

        // Repeat for the next relation.
        if constexpr (relation_idx + 1 < NUM_RELATIONS) {
            accumulate_relation_evaluations<relation_idx + 1>(purported_evaluations, relation_parameters);
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
            // scale it by the pow polynomial's constant and zeta power "c_l ⋅ ζ_{l+1}ⁱ"
            // and add it to the accumulators for Sˡ(Xₗ)
            accumulate_relation_univariates<>(relation_parameters, pow_challenge);
            // Update the pow polynomial's contribution c_l ⋅ ζ_{l+1}ⁱ for the next edge.
            pow_challenge *= pow_univariate.zeta_pow_sqr;
        }

        auto result = batch_over_relations<Univariate<FF, MAX_RELATION_LENGTH>>(relation_parameters.alpha);

        reset_accumulators<>();

        return result;
    }

    /**
     * @brief Calculate the contribution of each relation to the expected value of the full Honk relation.
     *
     * @details For each relation, use the purported values (supplied by the prover) of the multivariates to calculate
     * a contribution to the purported value of the full Honk relation. These are stored in `evaluations`. Adding these
     * together, with appropriate scaling factors, produces the expected value of the full Honk relation. This value is
     * checked against the final value of the target total sum, defined as sigma_d.
     */
    // TODO(Cody): Input should be an array? Then challenge container has to know array length.
    FF compute_full_honk_relation_purported_value(std::vector<FF>& purported_evaluations,
                                                  const RelationParameters<FF>& relation_parameters,
                                                  const PowUnivariate<FF>& pow_univariate)
    {
        accumulate_relation_evaluations<>(purported_evaluations, relation_parameters);

        // IMPROVEMENT(Cody): Reuse functions from univariate_accumulators batching?
        FF running_challenge = 1;
        FF output = 0;
        for (auto& evals : evaluations) {
            output += evals * running_challenge;
            running_challenge *= relation_parameters.alpha;
        }
        output *= pow_univariate.partial_evaluation_constant;

        return output;
    }

    /**
     * @brief check if S^{l}(0) + S^{l}(1) = S^{l-1}(u_{l-1}) = sigma_{l} (or 0 if l=0)
     *
     * @param univariate T^{l}(X), the round univariate that is equal to S^{l}(X)/( (1−X) + X⋅ζ^{ 2^l } )
     */
    bool check_sum(Univariate<FF, MAX_RELATION_LENGTH>& univariate, const PowUnivariate<FF>& pow_univariate)
    {
        // S^{l}(0) = ( (1−0) + 0⋅ζ^{ 2^l } ) ⋅ T^{l}(0) = T^{l}(0)
        // S^{l}(1) = ( (1−1) + 1⋅ζ^{ 2^l } ) ⋅ T^{l}(1) = ζ^{ 2^l } ⋅ T^{l}(1)
        FF total_sum = univariate.value_at(0) + (pow_univariate.zeta_pow * univariate.value_at(1));
        // target_total_sum = sigma_{l} =
        bool sumcheck_round_failed = (target_total_sum != total_sum);
        round_failed = round_failed || sumcheck_round_failed;
        return !sumcheck_round_failed;
    };

    /**
     * @brief After checking that the univariate is good for this round, compute the next target sum.
     *
     * @param univariate T^l(X), given by its evaluations over {0,1,2,...},
     * equal to S^{l}(X)/( (1−X) + X⋅ζ^{ 2^l } )
     * @param round_challenge u_l
     * @return FF sigma_{l+1} = S^l(u_l)
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
        // Evaluate (1−u_l) + u_l ⋅ ζ^{2^l} )
        FF pow_monomial_eval = pow_univariate.univariate_eval(round_challenge);
        // sigma_{l+1} = S^l(u_l) = (1−u_l) + u_l⋅ζ^{2^l} ) ⋅ T^{l}(u_l)
        target_total_sum *= pow_monomial_eval;
        return target_total_sum;
    }
};
} // namespace honk::sumcheck
