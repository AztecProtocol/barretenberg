#pragma once
#include <array>
#include <tuple>

#include "../polynomials/univariate.hpp"
#include "relation_parameters.hpp"
#include "relation_types.hpp"

namespace proof_system::honk::sumcheck {

template <typename FF> class GenPermSortRelation {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 6; // degree(q_sort * D(D - 1)(D - 2)(D - 3)) = 5

    static constexpr size_t LEN_1 = 6; // range constrain sub-relation 1
    static constexpr size_t LEN_2 = 6; // range constrain sub-relation 2
    static constexpr size_t LEN_3 = 6; // range constrain sub-relation 3
    static constexpr size_t LEN_4 = 6; // range constrain sub-relation 4
    using LENGTHS = LengthsWrapper<LEN_1, LEN_2, LEN_3, LEN_4>;

    using UnivariateAccumulatorTypes = UnivariateAccumulatorTypes<FF, LENGTHS>;
    using ValueAccumulatorTypes = ValueAccumulatorTypes<FF, LENGTHS>;

    using RelationUnivariates = typename UnivariateAccumulatorTypes::Accumulators;
    using RelationValues = typename ValueAccumulatorTypes::Accumulators;

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
    template <typename TypeMuncher>
    void add_edge_contribution_impl(typename TypeMuncher::Accumulators& accumulators,
                                    const auto& extended_edges,
                                    const RelationParameters<FF>&,
                                    const FF& scaling_factor) const
    {
        // OPTIMIZATION?: Karatsuba in general, at least for some degrees?
        //       See https://hackmd.io/xGLuj6biSsCjzQnYN-pEiA?both

        using View = typename std::tuple_element<0, typename TypeMuncher::AccumulatorViews>::type;
        auto w_1 = View(extended_edges.w_l);
        auto w_2 = View(extended_edges.w_r);
        auto w_3 = View(extended_edges.w_o);
        auto w_4 = View(extended_edges.w_4);
        auto w_1_shift = View(extended_edges.w_l_shift);
        auto q_sort = View(extended_edges.q_sort);

        static const FF minus_one = FF(-1);
        static const FF minus_two = FF(-2);
        static const FF minus_three = FF(-3);

        // Compute wire differences
        auto delta_1 = w_2 - w_1;
        auto delta_2 = w_3 - w_2;
        auto delta_3 = w_4 - w_3;
        auto delta_4 = w_1_shift - w_4;

        // Contribution (1)
        auto tmp_1 = delta_1;
        tmp_1 *= (delta_1 + minus_one);
        tmp_1 *= (delta_1 + minus_two);
        tmp_1 *= (delta_1 + minus_three);
        tmp_1 *= q_sort;
        tmp_1 *= scaling_factor;
        std::get<0>(accumulators) += tmp_1;

        // Contribution (2)
        auto tmp_2 = delta_2;
        tmp_2 *= (delta_2 + minus_one);
        tmp_2 *= (delta_2 + minus_two);
        tmp_2 *= (delta_2 + minus_three);
        tmp_2 *= q_sort;
        tmp_2 *= scaling_factor;
        std::get<1>(accumulators) += tmp_2;

        // Contribution (3)
        auto tmp_3 = delta_3;
        tmp_3 *= (delta_3 + minus_one);
        tmp_3 *= (delta_3 + minus_two);
        tmp_3 *= (delta_3 + minus_three);
        tmp_3 *= q_sort;
        tmp_3 *= scaling_factor;
        std::get<2>(accumulators) += tmp_3;

        // Contribution (4)
        auto tmp_4 = delta_4;
        tmp_4 *= (delta_4 + minus_one);
        tmp_4 *= (delta_4 + minus_two);
        tmp_4 *= (delta_4 + minus_three);
        tmp_4 *= q_sort;
        tmp_4 *= scaling_factor;
        std::get<3>(accumulators) += tmp_4;
    };

    inline void add_edge_contribution(auto& accumulator,
                                      const auto& input,
                                      const RelationParameters<FF>& relation_parameters,
                                      const FF& scaling_factor) const
    {
        add_edge_contribution_impl<UnivariateAccumulatorTypes>(accumulator, input, relation_parameters, scaling_factor);
    }

    void add_full_relation_value_contribution(RelationValues& accumulator,
                                              auto& input,
                                              const RelationParameters<FF>& relation_parameters,
                                              const FF& scaling_factor = 1) const
    {
        add_edge_contribution_impl<ValueAccumulatorTypes>(accumulator, input, relation_parameters, scaling_factor);
    }
};
} // namespace proof_system::honk::sumcheck
