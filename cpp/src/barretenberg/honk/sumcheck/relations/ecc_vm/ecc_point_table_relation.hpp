#pragma once
#include <array>
#include <tuple>

#include "../../polynomials/univariate.hpp"
#include "../relation.hpp"
#include "barretenberg/numeric/uint256/uint256.hpp"

namespace proof_system::honk::sumcheck {

template <typename FF, template <typename> typename TypeMuncher> class ECCVMPointTableRelationBase {
  public:
    constexpr bool scale_by_random_polynomial() { return true; }

    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 6;

    using UnivariateView = TypeMuncher<FF>::template RelationType<RELATION_LENGTH>::UnivariateView;
    using Univariate = TypeMuncher<FF>::template RelationType<RELATION_LENGTH>::Univariate;
    using RelationParameters = proof_system::honk::sumcheck::RelationParameters<FF>;

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
    void add_edge_contribution(Univariate& evals,
                               const auto& extended_edges,
                               const RelationParameters& /*unused*/,
                               const FF& scaling_factor) const
    {

        const auto& Tx = UnivariateView(extended_edges.table_tx);
        const auto& Tx_shift = UnivariateView(extended_edges.table_tx_shift);
        const auto& Ty = UnivariateView(extended_edges.table_ty);
        const auto& Ty_shift = UnivariateView(extended_edges.table_ty_shift);
        const auto& Dx = UnivariateView(extended_edges.table_dx);
        const auto& Dx_shift = UnivariateView(extended_edges.table_dx_shift);
        const auto& Dy = UnivariateView(extended_edges.table_dy);
        const auto& Dy_shift = UnivariateView(extended_edges.table_dy_shift);
        const auto& q_transition = UnivariateView(extended_edges.table_point_transition);

        Univariate evaluation(0);
        // Validate [D] = 2[T] if q_transition == 1

        // L = 3x^2 / 2y
        // x3 = L.L - 2x
        // (x3 + 2x) * 4y.y - 9x.x.x.x = 0
        auto two_x = Tx + Tx;
        auto three_x = two_x + Tx;
        auto three_xx = Tx * three_x;
        auto nine_xxxx = three_xx * three_xx;
        auto two_y = Ty + Ty;
        auto four_yy = two_y * two_y;

        auto x_double_check = (Dx + two_x) * four_yy - nine_xxxx;

        // y3 = L.(x - x3) - y
        // (y3 + y).(2y) - 3xx.(x3 - x) = 0
        auto y_double_check = (Ty + Dy) * two_y + three_xx * (Dx - Tx);

        evaluation += q_transition * x_double_check;
        evaluation += q_transition * y_double_check;

        // if q_transition = 0, validate [D_shift] = [D]
        evaluation += (-q_transition + 1) * (Dx - Dx_shift);
        evaluation += (-q_transition + 1) * (Dy - Dy_shift);

        // Validate [T_shift] + [D] = [T] if q_transition = 0
        const auto& x1 = Tx_shift;
        const auto& y1 = Ty_shift;
        const auto& x2 = Dx;
        const auto& y2 = Dy;
        const auto& x3 = Tx;
        const auto& y3 = Ty;
        const auto lambda_numerator = y2 - y1;
        const auto lambda_denominator = x2 - x1;
        auto x_add_check =
            (x3 + x2 + x1) * lambda_denominator * lambda_denominator - lambda_numerator * lambda_numerator;
        auto y_add_check = (y3 + y1) * lambda_denominator + (x3 - x1) * lambda_numerator;

        evaluation += (-q_transition + 1) * x_add_check;
        evaluation += (-q_transition + 1) * y_add_check;

        evals += evaluation * scaling_factor;
        // We perform q_transition checks in ecc_wnaf_relation
        // We can use the `round` column to index the wnaf slices (0 -> 1, 1 -> 3, ..., 7 -> 15) i.e. slice that indexes
        // point = round * 2 + 1
        // TODO(zac) all of the lookup algebra!!!
    }
};

} // namespace proof_system::honk::sumcheck
