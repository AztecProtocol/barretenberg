#pragma once
#include <array>
#include <tuple>

#include "../polynomials/univariate.hpp"
#include "relation_parameters.hpp"
#include "relation_types.hpp"

namespace proof_system::honk::sumcheck {

// WORKTODO: ADD zero check on op wires
template <typename FF> class EccOpQueueRelationBase {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 3; //

    static constexpr size_t LEN_1 = 3; // range constrain sub-relation 1
    static constexpr size_t LEN_2 = 3; // range constrain sub-relation 2
    static constexpr size_t LEN_3 = 3; // range constrain sub-relation 3
    static constexpr size_t LEN_4 = 3; // range constrain sub-relation 4
    template <template <size_t...> typename AccumulatorTypesContainer>
    using AccumulatorTypesBase = AccumulatorTypesContainer<LEN_1, LEN_2, LEN_3, LEN_4>;

    /**
     * @brief Expression for the generalized permutation sort gate.
     * @details The relation is defined as C(extended_edges(X)...) =
     *    TODO
     *
     * @param evals transformed to `evals + C(extended_edges(X)...)*scaling_factor`
     * @param extended_edges an std::array containing the fully extended Univariate edges.
     * @param parameters contains beta, gamma, and public_input_delta, ....
     * @param scaling_factor optional term to scale the evaluation before adding to evals.
     */
    template <typename AccumulatorTypes>
    void static add_edge_contribution_impl(typename AccumulatorTypes::Accumulators& accumulators,
                                           const auto& extended_edges,
                                           const RelationParameters<FF>&,
                                           const FF& scaling_factor)
    {
        // OPTIMIZATION?: Karatsuba in general, at least for some degrees?
        //       See https://hackmd.io/xGLuj6biSsCjzQnYN-pEiA?both

        using View = typename std::tuple_element<0, typename AccumulatorTypes::AccumulatorViews>::type;
        auto w_1 = View(extended_edges.w_l);
        auto w_2 = View(extended_edges.w_r);
        auto w_3 = View(extended_edges.w_o);
        auto w_4 = View(extended_edges.w_4);
        auto op_wire_1 = View(extended_edges.ecc_op_wire_1);
        auto op_wire_2 = View(extended_edges.ecc_op_wire_2);
        auto op_wire_3 = View(extended_edges.ecc_op_wire_3);
        auto op_wire_4 = View(extended_edges.ecc_op_wire_4);
        auto q_ecc_op_queue = View(extended_edges.q_ecc_op_queue);

        // Contribution (1)
        auto tmp = op_wire_1 - w_1;
        tmp *= q_ecc_op_queue;
        tmp *= scaling_factor;
        std::get<0>(accumulators) += tmp;

        // Contribution (2)
        tmp = op_wire_2 - w_2;
        tmp *= q_ecc_op_queue;
        tmp *= scaling_factor;
        std::get<1>(accumulators) += tmp;

        // Contribution (3)
        tmp = op_wire_3 - w_3;
        tmp *= q_ecc_op_queue;
        tmp *= scaling_factor;
        std::get<2>(accumulators) += tmp;

        // Contribution (4)
        tmp = op_wire_4 - w_4;
        tmp *= q_ecc_op_queue;
        tmp *= scaling_factor;
        std::get<3>(accumulators) += tmp;
    };
};

template <typename FF> using EccOpQueueRelation = RelationWrapper<FF, EccOpQueueRelationBase>;

} // namespace proof_system::honk::sumcheck
