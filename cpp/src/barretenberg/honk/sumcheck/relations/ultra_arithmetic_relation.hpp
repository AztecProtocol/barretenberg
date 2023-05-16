#pragma once
#include <array>
#include <tuple>

#include "../polynomials/univariate.hpp"
#include "relation_parameters.hpp"

namespace proof_system::honk::sumcheck {

// WORKTODO: fix formatting around scopes in this file
template <typename FF> class UltraArithmeticRelation {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 6; // degree(q_arith^2 * q_m * w_r * w_l) = 5

    static constexpr size_t NUM_CONSTRAINTS = 2;
    static constexpr std::array<size_t, NUM_CONSTRAINTS> CONSTRAINT_LENGTH = { 6, 5 };

    using RelationUnivariates = std::tuple<Univariate<FF, CONSTRAINT_LENGTH[0]>, Univariate<FF, CONSTRAINT_LENGTH[1]>>;
    using RelationValues = std::array<FF, NUM_CONSTRAINTS>;

    /**
     * @brief Expression for the Ultra Arithmetic gate.
     * @details The relation is defined as C(extended_edges(X)...) =
     *    q_arith *
     *      [ -1/2(q_arith - 3)(q_m * w_r * w_l) +
     *        (q_l * w_l) + (q_r * w_r) + (q_o * w_o) + (q_4 * w_4) + q_c +
     *        (q_arith - 1)w_4_shift ]
     *
     *    q_arith *
     *      (q_arith - 2) * (q_arith - 1) * (w_l + w_4 - w_l_shift + q_m)
     *
     * @param evals transformed to `evals + C(extended_edges(X)...)*scaling_factor`
     * @param extended_edges an std::array containing the fully extended Univariate edges.
     * @param parameters contains beta, gamma, and public_input_delta, ....
     * @param scaling_factor optional term to scale the evaluation before adding to evals.
     */
    void add_edge_contribution(RelationUnivariates& evals,
                               const auto& extended_edges,
                               const RelationParameters<FF>&,
                               const FF& scaling_factor) const {
        // OPTIMIZATION?: Karatsuba in general, at least for some degrees?
        //       See https://hackmd.io/xGLuj6biSsCjzQnYN-pEiA?both
        // clang-format off
        // Contribution 1
        {   
            static constexpr size_t LENGTH = CONSTRAINT_LENGTH[0];
            auto w_l = UnivariateView<FF, LENGTH>(extended_edges.w_l);
            auto w_r = UnivariateView<FF, LENGTH>(extended_edges.w_r);
            auto w_o = UnivariateView<FF, LENGTH>(extended_edges.w_o);
            auto w_4 = UnivariateView<FF, LENGTH>(extended_edges.w_4);
            auto w_4_shift = UnivariateView<FF, LENGTH>(extended_edges.w_4_shift);
            auto q_m = UnivariateView<FF, LENGTH>(extended_edges.q_m);
            auto q_l = UnivariateView<FF, LENGTH>(extended_edges.q_l);
            auto q_r = UnivariateView<FF, LENGTH>(extended_edges.q_r);
            auto q_o = UnivariateView<FF, LENGTH>(extended_edges.q_o);
            auto q_4 = UnivariateView<FF, LENGTH>(extended_edges.q_4);
            auto q_c = UnivariateView<FF, LENGTH>(extended_edges.q_c);
            auto q_arith = UnivariateView<FF, LENGTH>(extended_edges.q_arith);

            static const FF neg_half = FF(-2).invert();

            auto tmp = (q_arith - 3) * (q_m * w_r * w_l) * neg_half;
            tmp += (q_l * w_l) + (q_r * w_r) + (q_o * w_o) + (q_4 * w_4) + q_c;
            tmp += (q_arith - 1) * w_4_shift;
            tmp *= q_arith;
            tmp *= scaling_factor;
            std::get<0>(evals) += tmp;
        }
        // Contribution 2
        {
            static constexpr size_t LENGTH = CONSTRAINT_LENGTH[1];
            auto w_l = UnivariateView<FF, LENGTH>(extended_edges.w_l);
            auto w_4 = UnivariateView<FF, LENGTH>(extended_edges.w_4);
            auto w_l_shift = UnivariateView<FF, LENGTH>(extended_edges.w_l_shift);
            auto q_m = UnivariateView<FF, LENGTH>(extended_edges.q_m);
            auto q_arith = UnivariateView<FF, LENGTH>(extended_edges.q_arith);

            auto tmp = w_l + w_4 - w_l_shift + q_m;
            tmp *= (q_arith - 2);
            tmp *= (q_arith - 1);
            tmp *= q_arith;
            tmp *= scaling_factor;
            std::get<1>(evals) += tmp;
        }
    }; // namespace proof_system::honk::sumcheck

    void add_full_relation_value_contribution(RelationValues& full_honk_relation_value,
                                          const auto& purported_evaluations,
                                          const RelationParameters<FF>&) const
    {
        auto w_l = purported_evaluations.w_l;
        auto w_l_shift = purported_evaluations.w_l_shift;
        auto w_r = purported_evaluations.w_r;
        auto w_o = purported_evaluations.w_o;
        auto w_4 = purported_evaluations.w_4;
        auto w_4_shift = purported_evaluations.w_4_shift;
        auto q_m = purported_evaluations.q_m;
        auto q_l = purported_evaluations.q_l;
        auto q_r = purported_evaluations.q_r;
        auto q_o = purported_evaluations.q_o;
        auto q_4 = purported_evaluations.q_4;
        auto q_c = purported_evaluations.q_c;
        auto q_arith = purported_evaluations.q_arith;

        static const FF neg_half = FF(-2).invert();

        // Contribution 1
        auto tmp = (q_arith - 3) * (q_m * w_r * w_l) * neg_half;
        tmp += (q_l * w_l) + (q_r * w_r) + (q_o * w_o) + (q_4 * w_4) + q_c;
        tmp += (q_arith - 1) * w_4_shift;
        tmp *= q_arith;
        std::get<0>(full_honk_relation_value) += tmp;

        // Contribution 2
        tmp = w_l + w_4 - w_l_shift + q_m;
        tmp *= (q_arith - 2);
        tmp *= (q_arith - 1);
        tmp *= q_arith;
        std::get<1>(full_honk_relation_value) += tmp;
    };
};
// clang-format on
} // namespace proof_system::honk::sumcheck
