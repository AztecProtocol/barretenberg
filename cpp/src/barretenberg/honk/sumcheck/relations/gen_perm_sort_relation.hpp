#pragma once
#include <array>
#include <tuple>

#include "../polynomials/univariate.hpp"
#include "relation.hpp"

namespace proof_system::honk::sumcheck {

template <typename FF> class GenPermSortRelation {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 6; // degree(q_sort * D(D - 1)(D - 2)(D - 3)) = 5

    /**
     * @brief Expression for the generalized permutation sort gate.
     * @details The relation is defined as C(extended_edges(X)...) =
     *    q_sort * \sum{ i = [0, 3]} \alpha^i D_i(D_i - 1)(D_i - 2)(D_i - 3)
     *      where
     *      D_0 = w_2 - w_1
     *      D_1 = w_3 - w_2
     *      D_2 = w_4 - w_3
     *      D_3 = w_1_shift - w_4
     *
     * @param evals transformed to `evals + C(extended_edges(X)...)*scaling_factor`
     * @param extended_edges an std::array containing the fully extended Univariate edges.
     * @param parameters contains beta, gamma, and public_input_delta, ....
     * @param scaling_factor optional term to scale the evaluation before adding to evals.
     */
    void add_edge_contribution(Univariate<FF, RELATION_LENGTH>& evals,
                               const auto& extended_edges,
                               const RelationParameters<FF>&,
                               const FF& scaling_factor) const
    {
        // OPTIMIZATION?: Karatsuba in general, at least for some degrees?
        //       See https://hackmd.io/xGLuj6biSsCjzQnYN-pEiA?both

        auto w_1 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.w_l);
        auto w_2 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.w_r);
        auto w_3 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.w_o);
        auto w_4 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.w_4);
        auto w_1_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges.w_l_shift);
        auto q_sort = UnivariateView<FF, RELATION_LENGTH>(extended_edges.q_sort);

        info("w_1 = ", w_1);
        info("w_2 = ", w_2);
        info("w_3 = ", w_3);
        info("w_4 = ", w_4);
        info("w_1_shift = ", w_1_shift);
        info("q_sort = ", q_sort);

        static const FF minus_one = FF(-1);
        static const FF minus_two = FF(-2);
        static const FF minus_three = FF(-3);

        // WORKTODO: Eventually this would be based on real alpha but this is not a full solution
        // since utilizing powers of alpha internal to a relation results in incorrect powers
        // being used in the ultimate univariate batching. i.e we'd wind up reusing the same power
        // of alpha in multiple relations.
        static const FF fake_alpha_1 = FF(1);
        static const FF fake_alpha_2 = fake_alpha_1 * fake_alpha_1;
        static const FF fake_alpha_3 = fake_alpha_2 * fake_alpha_1;
        static const FF fake_alpha_4 = fake_alpha_3 * fake_alpha_1;

        // Compute wire differences
        auto delta_1 = w_2 - w_1;
        auto delta_2 = w_3 - w_2;
        auto delta_3 = w_4 - w_3;
        auto delta_4 = w_1_shift - w_4;

        info("delta_1 = ", delta_1);
        info("delta_2 = ", delta_2);
        info("delta_3 = ", delta_3);
        info("delta_4 = ", delta_4);

        auto tmp = delta_1;
        tmp *= (delta_1 + minus_one);
        tmp *= (delta_1 + minus_two);
        tmp *= (delta_1 + minus_three);
        tmp *= fake_alpha_1; // WORKTODO * 1
        evals += tmp;

        info("tmp = ", tmp);

        tmp = delta_2;
        tmp *= (delta_2 + minus_one);
        tmp *= (delta_2 + minus_two);
        tmp *= (delta_2 + minus_three);
        tmp *= fake_alpha_2; // WORKTODO * alpha
        evals += tmp;

        info("tmp = ", tmp);

        tmp = delta_3;
        tmp *= (delta_3 + minus_one);
        tmp *= (delta_3 + minus_two);
        tmp *= (delta_3 + minus_three);
        tmp *= fake_alpha_3; // WORKTODO * alpha^2
        evals += tmp;

        info("tmp = ", tmp);

        tmp = delta_4;
        tmp *= (delta_4 + minus_one);
        tmp *= (delta_4 + minus_two);
        tmp *= (delta_4 + minus_three);
        tmp *= fake_alpha_4; // WORKTODO * alpha^3
        evals += tmp;

        info("tmp = ", tmp);

        evals *= q_sort;
        evals *= scaling_factor;
    };

    void add_full_relation_value_contribution(FF& full_honk_relation_value,
                                              const auto& purported_evaluations,
                                              const RelationParameters<FF>&) const
    {
        auto w_1 = purported_evaluations.w_l;
        auto w_2 = purported_evaluations.w_r;
        auto w_3 = purported_evaluations.w_o;
        auto w_4 = purported_evaluations.w_4;
        auto w_1_shift = purported_evaluations.w_l_shift;
        auto q_sort = purported_evaluations.q_sort;

        static const FF minus_one = FF(-1);
        static const FF minus_two = FF(-2);
        static const FF minus_three = FF(-3);

        // WORKTODO: Eventually this would be based on real alpha but this is not a full solution
        // since utilizing powers of alpha internal to a relation results in incorrect powers
        // being used in the ultimate univariate batching. i.e we'd wind up reusing the same power
        // of alpha in multiple relations.
        static const FF fake_alpha_1 = FF(1);
        static const FF fake_alpha_2 = fake_alpha_1 * fake_alpha_1;
        static const FF fake_alpha_3 = fake_alpha_2 * fake_alpha_1;
        static const FF fake_alpha_4 = fake_alpha_3 * fake_alpha_1;

        // Compute wire differences
        auto delta_1 = w_2 - w_1;
        auto delta_2 = w_3 - w_2;
        auto delta_3 = w_4 - w_3;
        auto delta_4 = w_1_shift - w_4;

        auto tmp = delta_1;
        tmp *= (delta_1 + minus_one);
        tmp *= (delta_1 + minus_two);
        tmp *= (delta_1 + minus_three);
        tmp *= fake_alpha_1; // WORKTODO * 1
        full_honk_relation_value += tmp;

        tmp = delta_2;
        tmp *= (delta_2 + minus_one);
        tmp *= (delta_2 + minus_two);
        tmp *= (delta_2 + minus_three);
        tmp *= fake_alpha_2; // WORKTODO * alpha
        full_honk_relation_value += tmp;

        tmp = delta_3;
        tmp *= (delta_3 + minus_one);
        tmp *= (delta_3 + minus_two);
        tmp *= (delta_3 + minus_three);
        tmp *= fake_alpha_3; // WORKTODO * alpha^2
        full_honk_relation_value += tmp;

        tmp = delta_4;
        tmp *= (delta_4 + minus_one);
        tmp *= (delta_4 + minus_two);
        tmp *= (delta_4 + minus_three);
        tmp *= fake_alpha_4; // WORKTODO * alpha^3
        full_honk_relation_value += tmp;

        full_honk_relation_value *= q_sort;
    };
};
} // namespace proof_system::honk::sumcheck
