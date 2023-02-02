#pragma once
#include "relation.hpp"
#include <proof_system/flavor/flavor.hpp>

namespace honk::sumcheck {

template <typename FF> class GrandProductInitializationRelation : public Relation<FF> {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 3;
    using MULTIVARIATE = StandardHonk::MULTIVARIATE; // could just get from StandardArithmetization

    GrandProductInitializationRelation() = default;
    explicit GrandProductInitializationRelation(auto){}; // NOLINT(readability-named-parameter)

    /**
     * @brief Add contribution of the permutation relation for a given edge
     *
     * @detail There are 2 relations associated with enforcing the wire copy relations
     * This file handles the relation Z_perm_shift(n_last) = 0 via the relation:
     *
     *                      C(X) = L_LAST(X) * Z_perm_shift(X)
     *
     * @param acc transformed to `acc + C(variables...)*scaling_factor`
     * @param variables an std::array containing the variables for this gate.
     * Can be either FF or UnivariateView<FF> or UnivariateExpr<FF>.
     * @param parameters unused
     * @param scaling_factor optional term to scale the evaluation before adding to acc.
     */
    static void accumulate_relation_evaluation(auto& acc,
                                               const auto& variables,
                                               const RelationParameters<FF>& = {},
                                               const FF& scaling_factor = FF::one())
    {
        const auto& z_perm_shift = variables[MULTIVARIATE::Z_PERM_SHIFT];
        const auto& lagrange_last = variables[MULTIVARIATE::LAGRANGE_LAST];

        acc += (lagrange_last * z_perm_shift) * scaling_factor;
    }
};
} // namespace honk::sumcheck
