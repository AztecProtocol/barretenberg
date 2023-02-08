#pragma once
#include "relation.hpp"
#include <proof_system/flavor/flavor.hpp>

namespace honk::sumcheck {

template <typename FF> class GrandProductComputationRelation : public Relation<FF> {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 5;
    using MULTIVARIATE = StandardHonk::MULTIVARIATE;

    GrandProductComputationRelation() = default;
    explicit GrandProductComputationRelation(auto){}; // TODO(luke): should just be default?

    /**
     * @brief Compute contribution of the permutation relation for a given edge (internal function)
     *
     * @details There are 2 relations associated with enforcing the wire copy relations
     * This file handles the relation that confirms faithful calculation of the grand
     * product polynomial Z_perm. (Initialization relation Z_perm(0) = 1 is handled elsewhere).
     *
     *  C(X) = ( z_perm(X) + lagrange_first(X) )*P(X) - ( z_perm_shift(X) + delta * lagrange_last(X) )*Q(X),
     *   where
     *      P(X) = Prod_{i=1:3} w_i(X) + β*(n*(i-1) + idx(X)) + γ
     *      Q(X) = Prod_{i=1:3} w_i(X) + β*σ_i(X) + γ
     *      delta is the public input correction term
     *
     * @param acc transformed to `acc + C(variables(X)...)*scaling_factor`
     * @param variables an std::array containing the variables for this gate.
     * Can be either FF or UnivariateView<FF> or UnivariateExpr<FF>.
     * @param parameters contains beta, gamma, and public_input_delta.
     * @param scaling_factor optional term to scale the evaluation before adding to acc.
     */
    static void accumulate_relation_evaluation(auto& acc,
                                               const auto& variables,
                                               const RelationParameters<FF>& parameters,
                                               const FF& scaling_factor = FF::one())
    {
        const auto& beta = parameters.beta;
        const auto& gamma = parameters.gamma;
        const auto& public_input_delta = parameters.public_input_delta;
        const auto& n = parameters.subgroup_size;


        const auto& w_1 = variables[MULTIVARIATE::W_L];
        const auto& w_2 = variables[MULTIVARIATE::W_R];
        const auto& w_3 = variables[MULTIVARIATE::W_O];
        const auto& sigma_1 = variables[MULTIVARIATE::SIGMA_1];
        const auto& sigma_2 = variables[MULTIVARIATE::SIGMA_2];
        const auto& sigma_3 = variables[MULTIVARIATE::SIGMA_3];
        const auto& id = variables[MULTIVARIATE::ID];
        const auto& z_perm = variables[MULTIVARIATE::Z_PERM];
        const auto& z_perm_shift = variables[MULTIVARIATE::Z_PERM_SHIFT];
        const auto& lagrange_first = variables[MULTIVARIATE::LAGRANGE_FIRST];
        const auto& lagrange_last = variables[MULTIVARIATE::LAGRANGE_LAST];


        acc += ((z_perm + lagrange_first) * (w_1 + id * beta + gamma) * (w_2 + id * beta + (beta * n + gamma)) *
                    (w_3 + id * beta + (beta * (n + n) + gamma)) -
                (z_perm_shift + lagrange_last * public_input_delta) * (w_1 + sigma_1 * beta + gamma) *
                    (w_2 + sigma_2 * beta + gamma) * (w_3 + sigma_3 * beta + gamma)) *
               scaling_factor;
    }
};
} // namespace honk::sumcheck