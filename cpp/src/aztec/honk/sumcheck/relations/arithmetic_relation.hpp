#pragma once
#include <array>
#include <tuple>

#include <proof_system/flavor/flavor.hpp>
#include "relation.hpp"

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
     * @brief Expression for the StandardArithmetic gate.
     * @details Let C(variables...) =
     *    (q_m * w_r * w_l) + (q_l * w_l) + (q_r * w_r) + (q_o * w_o) + q_c
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
        // OPTIMIZATION?: Karatsuba in general, at least for some degrees?
        //       See https://hackmd.io/xGLuj6biSsCjzQnYN-pEiA?both
        const auto& q_m = variables[MULTIVARIATE::Q_M];
        const auto& q_l = variables[MULTIVARIATE::Q_L];
        const auto& q_r = variables[MULTIVARIATE::Q_R];
        const auto& q_o = variables[MULTIVARIATE::Q_O];
        const auto& q_c = variables[MULTIVARIATE::Q_C];
        const auto& w_l = variables[MULTIVARIATE::W_L];
        const auto& w_r = variables[MULTIVARIATE::W_R];
        const auto& w_o = variables[MULTIVARIATE::W_O];
        acc += (w_l * (q_m * w_r + q_l) + q_r * w_r + q_o * w_o + q_c) * scaling_factor;
    }
};
} // namespace honk::sumcheck
