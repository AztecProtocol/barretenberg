#pragma once
#include <array>
#include <tuple>

#include <proof_system/flavor/flavor.hpp>
#include "relation.hpp"
#include "../polynomials/multivariates.hpp"
#include "../polynomials/barycentric_data.hpp"
#include "../polynomials/univariate.hpp"

namespace honk::sumcheck {

template <typename FF> class ArithmeticRelation : public Relation<FF> {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 4;
    using MULTIVARIATE = StandardHonk::MULTIVARIATE; // could just get from StandardArithmetization

    // FUTURE OPTIMIZATION: successively extend as needed?

    // This relation takes no randomness, so it will not receive challenges.
    ArithmeticRelation() = default;
    explicit ArithmeticRelation(auto){}; // NOLINT(readability-named-parameter)

    /**
     * @brief External function used by sumcheck round
     *
     * @param extended_edges Contain inputs for the relation
     * @return the resulting univariate polynomial
     *
     * The final parameter is left to conform to the general argument structure (input,output, challenges) even though
     * we don't need challenges in this relation.
     */
    template <typename T> Univariate<FF, RELATION_LENGTH> get_edge_contribution(auto& extended_edges, T)
    {
        return get_edge_contribution_internal(extended_edges);
    };

    /**
     * @brief Same as get_edge_contribution but is used for testing
     *
     * @details Arithmetic relation doesn't require challenges but it needs the same interface as those relations that
     * do
     *
     * @tparam T
     * @param extended_edges
     * @param evals
     * @param challenges
     */
    // TODO(kesha): Change once challenges are being supplied to regular contribution
    template <typename T> Univariate<FF, RELATION_LENGTH> get_edge_contribution_testing(auto& extended_edges, T)
    {
        return get_edge_contribution_internal(extended_edges);
    };

    // OPTIMIZATION?: Karatsuba in general, at least for some degrees?
    //       See https://hackmd.io/xGLuj6biSsCjzQnYN-pEiA?both
    Univariate<FF, RELATION_LENGTH> get_edge_contribution_internal(auto& extended_edges)
    {
        auto w_l = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_L]);
        auto w_r = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_R]);
        auto w_o = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_O]);
        auto q_m = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_M]);
        auto q_l = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_L]);
        auto q_r = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_R]);
        auto q_o = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_O]);
        auto q_c = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_C]);

        return w_l * (q_m * w_r + q_l) + q_r * w_r + q_o * w_o + q_c;
    };

    template <typename T> FF get_full_relation_value_contribution(auto& purported_evaluations, T)
    {
        auto w_l = purported_evaluations[MULTIVARIATE::W_L];
        auto w_r = purported_evaluations[MULTIVARIATE::W_R];
        auto w_o = purported_evaluations[MULTIVARIATE::W_O];
        auto q_m = purported_evaluations[MULTIVARIATE::Q_M];
        auto q_l = purported_evaluations[MULTIVARIATE::Q_L];
        auto q_r = purported_evaluations[MULTIVARIATE::Q_R];
        auto q_o = purported_evaluations[MULTIVARIATE::Q_O];
        auto q_c = purported_evaluations[MULTIVARIATE::Q_C];

        return w_l * (q_m * w_r + q_l) + q_r * w_r + q_o * w_o + q_c;
    };
};
} // namespace honk::sumcheck
