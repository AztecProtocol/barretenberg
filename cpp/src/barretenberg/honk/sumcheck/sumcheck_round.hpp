#pragma once
#include "polynomials/barycentric_data.hpp"
#include "polynomials/univariate.hpp"
#include "polynomials/pow.hpp"
#include "relations/relation.hpp"
#include <array>
#include <algorithm>
#include <tuple>

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

    static constexpr size_t NUM_RELATIONS = sizeof...(Relations);
    static constexpr std::array<size_t, NUM_RELATIONS> RELATION_LENGTHS = { Relations<FF>::RELATION_LENGTH... };
    static constexpr size_t MAX_RELATION_LENGTH = std::max({ Relations<FF>::RELATION_LENGTH... });

    static constexpr std::tuple<Relations<FF>...> relations = std::tuple{ Relations<FF>()... };

    /**
     * @brief Given a tuple t = (t_0, t_1, ..., t_{NUM_RELATIONS-1}) and a challenge α,
     * modify the tuple in place to (t_0, αt_1, ..., α^{NUM_RELATIONS-1}t_{NUM_RELATIONS-1}).
     */
    template <size_t idx = 0> static void scale_tuple(auto& tuple, FF challenge, FF running_challenge)
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
     * @param univariate_accumulators tuple of Univariates for each relation. Each one is of size of the relation's
     * length
     * @param challenge the relation separation challenge
     * @return T
     */
    template <typename T> static T batch_over_relations(auto& univariate_accumulators, FF challenge)
    {
        std::array<Univariate<FF, MAX_RELATION_LENGTH>, NUM_RELATIONS> extended_univariates;
        FF running_challenge = 1;
        scale_tuple<>(univariate_accumulators, challenge, running_challenge);
        extend_univariate_accumulators<>(extended_univariates, univariate_accumulators);
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
     * @param extended_edges array of univariates we store the extended edges into
     * @param multivariates array of multivariate polynomials
     * @param edge_idx index in the hypercube of the edges we are extending
     */
    static void extend_edges(auto& extended_edges, const auto& multivariates, size_t edge_idx)
    {
        // TODO(#224)(Cody): this should go away and we should use constexpr method to extend
        static BarycentricData<FF, 2, MAX_RELATION_LENGTH> barycentric_2_to_max =
            BarycentricData<FF, 2, MAX_RELATION_LENGTH>();
        for (size_t idx = 0; idx < num_multivariates; idx++) {
            auto edge = Univariate<FF, 2>({ multivariates[idx][edge_idx], multivariates[idx][edge_idx + 1] });
            extended_edges[idx] = barycentric_2_to_max.extend(edge);
        }
    }

    /**
     * @brief After executing each widget on each edge, producing a tuple of univariates of differing lenghths,
     * extend all univariates to the max of the lenghths required by the largest relation.
     *
     * @tparam relation_idx
     * @param extended_univariates array of Univariate of size MAX_RELATION_LENGTHS in which we store the result
     * @param univariate_accumulators tuple of Univariates we want to extend.
     */
    template <size_t relation_idx = 0>
    static void extend_univariate_accumulators(auto& extended_univariates, const auto& univariate_accumulators)
    {
        // TODO(#224)(Cody): this barycentric stuff should be more built-in?
        static auto barycentric_utils = BarycentricData<FF, RELATION_LENGTHS[relation_idx], MAX_RELATION_LENGTH>();

        extended_univariates[relation_idx] = barycentric_utils.extend(std::get<relation_idx>(univariate_accumulators));

        // Repeat for the next relation.
        if constexpr (relation_idx + 1 < NUM_RELATIONS) {
            extend_univariate_accumulators<relation_idx + 1>(extended_univariates, univariate_accumulators);
        }
    }

  public:
    /**
     * @brief Return the evaluations of the univariate restriction (S_l(X_l) in the thesis) at num_multivariates-many
     * values. Most likely this will end up being S_l(0), ... , S_l(t-1) where t is around 12.
     *
     * @param polynomials container of spans with the coefficients of
     * @param round_size number of evaluations in each polynomial
     * @param relation_parameters parameters for the relations
     * @param pow_univariate container for the pow polynomial
     * @param alpha relation separation challenge
     * @return Univariate<FF, MAX_RELATION_LENGTH> S_l(X), as evaluations over {0,1,...,t-1}
     */
    static Univariate<FF, MAX_RELATION_LENGTH> compute_univariate(auto& polynomials,
                                                                  const size_t round_size,
                                                                  const RelationParameters<FF>& relation_parameters,
                                                                  const PowUnivariate<FF>& pow_univariate,
                                                                  const FF alpha)
    {
        auto univariate_accumulators = std::tuple{ Univariate<FF, Relations<FF>::RELATION_LENGTH>()... };
        std::array<Univariate<FF, MAX_RELATION_LENGTH>, num_multivariates> extended_edges;

        // For each edge_idx = 2i, we need to multiply the whole contribution by zeta^{2^{2i}}
        // This means that each univariate for each relation needs an extra multiplication.
        FF pow_challenge = pow_univariate.partial_evaluation_constant;
        for (size_t edge_idx = 0; edge_idx < round_size; edge_idx += 2) {
            extend_edges(extended_edges, polynomials, edge_idx);

            // Compute the i-th edge's univariate contribution,
            // scale it by the pow polynomial's constant and zeta power "c_l ⋅ ζ_{l+1}ⁱ"
            // and add it to the accumulators for Sˡ(Xₗ)
            accumulate_relation_univariates<>(
                univariate_accumulators, extended_edges, relation_parameters, pow_challenge);
            // Update the pow polynomial's contribution c_l ⋅ ζ_{l+1}ⁱ for the next edge.
            pow_challenge *= pow_univariate.zeta_pow_sqr;
        }

        auto result = batch_over_relations<Univariate<FF, MAX_RELATION_LENGTH>>(univariate_accumulators, alpha);

        return result;
    }

  private:
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
     * @param univariate_accumulators tuple containing the accumulated evaluations for each relation
     * @param extended_edges array of Univariates, extended to MAX_RELATION_LENGTH
     */
    template <size_t relation_idx = 0>
    static void accumulate_relation_univariates(auto& univariate_accumulators,
                                                const auto& extended_edges,
                                                const RelationParameters<FF>& relation_parameters,
                                                const FF& scaling_factor)
    {
        std::get<relation_idx>(relations).add_edge_contribution(
            std::get<relation_idx>(univariate_accumulators), extended_edges, relation_parameters, scaling_factor);

        // Repeat for the next relation.
        if constexpr (relation_idx + 1 < NUM_RELATIONS) {
            accumulate_relation_univariates<relation_idx + 1>(
                univariate_accumulators, extended_edges, relation_parameters, scaling_factor);
        }
    }
};
} // namespace honk::sumcheck
