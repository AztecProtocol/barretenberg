#pragma once
#include <array>
#include <tuple>

#include "../polynomials/univariate.hpp"
#include "relation_parameters.hpp"
#include "relation_types.hpp"

namespace proof_system::honk::sumcheck {

template <typename FF> class ArithmeticRelation {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 4;

    static constexpr size_t LEN_1 = 4; // arithmetic sub-relation
    using LENGTHS = LengthsWrapper<LEN_1>;

    using UnivariateAccumTypes = UnivariateAccumulatorTypes<FF, LENGTHS>;
    using ValueAccumTypes = ValueAccumulatorTypes<FF, LENGTHS>;

    using RelationUnivariates = typename UnivariateAccumTypes::Accumulators;
    using RelationValues = typename ValueAccumTypes::Accumulators;

    /**
     * @brief Expression for the StandardArithmetic gate.
     * @details The relation is defined as C(extended_edges(X)...) =
     *    (q_m * w_r * w_l) + (q_l * w_l) + (q_r * w_r) + (q_o * w_o) + q_c
     *
     * @param evals transformed to `evals + C(extended_edges(X)...)*scaling_factor`
     * @param extended_edges an std::array containing the fully extended Univariate edges.
     * @param parameters contains beta, gamma, and public_input_delta, ....
     * @param scaling_factor optional term to scale the evaluation before adding to evals.
     */
    template <typename TypeMuncher>
    void add_edge_contribution_impl(typename TypeMuncher::Accumulators& accumulator,
                                    const auto& extended_edges,
                                    const RelationParameters<FF>&,
                                    const FF& scaling_factor) const
    {
        // OPTIMIZATION?: Karatsuba in general, at least for some degrees?
        //       See https://hackmd.io/xGLuj6biSsCjzQnYN-pEiA?both

        using View = typename std::tuple_element<0, typename TypeMuncher::AccumulatorViews>::type;
        auto w_l = View(extended_edges.w_l);
        auto w_r = View(extended_edges.w_r);
        auto w_o = View(extended_edges.w_o);
        auto q_m = View(extended_edges.q_m);
        auto q_l = View(extended_edges.q_l);
        auto q_r = View(extended_edges.q_r);
        auto q_o = View(extended_edges.q_o);
        auto q_c = View(extended_edges.q_c);

        auto tmp = w_l * (q_m * w_r + q_l);
        tmp += q_r * w_r;
        tmp += q_o * w_o;
        tmp += q_c;
        tmp *= scaling_factor;
        std::get<0>(accumulator) += tmp;
    };

    inline void add_edge_contribution(auto& accumulator,
                                      const auto& input,
                                      const RelationParameters<FF>& relation_parameters,
                                      const FF& scaling_factor) const
    {
        add_edge_contribution_impl<UnivariateAccumTypes>(accumulator, input, relation_parameters, scaling_factor);
    }

    void add_full_relation_value_contribution(RelationValues& accumulator,
                                              auto& input,
                                              const RelationParameters<FF>& relation_parameters,
                                              const FF& scaling_factor = 1) const
    {
        add_edge_contribution_impl<ValueAccumTypes>(accumulator, input, relation_parameters, scaling_factor);
    }
};
} // namespace proof_system::honk::sumcheck
