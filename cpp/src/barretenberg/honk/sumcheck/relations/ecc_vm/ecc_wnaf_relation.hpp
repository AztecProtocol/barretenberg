#pragma once
#include <array>
#include <tuple>

#include "barretenberg/honk/flavor/flavor.hpp"
#include "../../polynomials/univariate.hpp"
#include "../relation.hpp"
#include "barretenberg/numeric/uint256/uint256.hpp"

namespace proof_system::honk::sumcheck {

template <typename FF, template <typename> typename TypeMuncher> class ECCVMWnafRelationBase {
  public:
    constexpr bool scale_by_random_polynomial() { return true; }

    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 5;

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
        auto scalar_sum = UnivariateView(extended_edges.table_scalar_sum);
        auto scalar_sum_new = UnivariateView(extended_edges.table_scalar_sum_shift);
        auto q_transition = UnivariateView(extended_edges.table_point_transition);
        auto round = UnivariateView(extended_edges.table_round);
        auto round_shift = UnivariateView(extended_edges.table_round_shift);
        auto pc = UnivariateView(extended_edges.table_pc);
        auto pc_shift = UnivariateView(extended_edges.table_pc_shift);
        auto q_wnaf = UnivariateView(extended_edges.q_wnaf);

        const auto& table_skew = UnivariateView(extended_edges.table_skew);

        std::array<UnivariateView, 8> slices{
            UnivariateView(extended_edges.table_s1), UnivariateView(extended_edges.table_s2),
            UnivariateView(extended_edges.table_s3), UnivariateView(extended_edges.table_s4),
            UnivariateView(extended_edges.table_s5), UnivariateView(extended_edges.table_s6),
            UnivariateView(extended_edges.table_s7), UnivariateView(extended_edges.table_s8),
        };

        Univariate evaluation = Univariate(0);
        const auto check_slice = [&evaluation](const UnivariateView& s) {
            evaluation += s * (s - 1) * (s - 2) * (s - 3);
        };

        const auto convert_to_wnaf = [](const UnivariateView& s0, const UnivariateView& s1) {
            auto t = s0 + s0;
            t += t;
            t += s1;

            auto naf = t + t - 15;
            return naf;
        };

        for (const auto& slice : slices) {
            check_slice(slice);
        }

        const auto w0 = convert_to_wnaf(slices[0], slices[1]);
        const auto w1 = convert_to_wnaf(slices[2], slices[3]);
        const auto w2 = convert_to_wnaf(slices[4], slices[5]);
        const auto w3 = convert_to_wnaf(slices[6], slices[7]);
        auto row_slice = w0;
        row_slice += row_slice;
        row_slice += row_slice;
        row_slice += row_slice;
        row_slice += row_slice;
        row_slice += w1;
        row_slice += row_slice;
        row_slice += row_slice;
        row_slice += row_slice;
        row_slice += row_slice;
        row_slice += w2;
        row_slice += row_slice;
        row_slice += row_slice;
        row_slice += row_slice;
        row_slice += row_slice;
        row_slice += w3;

        auto sum_delta = scalar_sum + scalar_sum;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += sum_delta;
        sum_delta += row_slice;

        const auto check_sum = scalar_sum_new - sum_delta;

        // todo multiply evals by q_wnaf at end
        // if not transitioning between scalars, check scalar sum correctly updated
        evaluation += q_wnaf * (-q_transition + 1) * check_sum;

        // if transitioning between scalars, check scalar sum on next row is zero
        evaluation += q_wnaf * scalar_sum_new * q_transition;

        // if transitioning between scalars, assert that round == 7 and next round == 0
        evaluation += q_wnaf * q_transition * (round - 7);
        evaluation += q_wnaf * q_transition * round_shift;

        // if not transitioning, assert that round increments by 1
        evaluation += q_wnaf * (-q_transition + 1) * (round_shift - round - 1);

        // if not transitioning, assert that pc does not change
        evaluation += q_wnaf * (-q_transition + 1) * (pc_shift - pc);

        // if transitioning, assert that pc decrements by 1
        evaluation += q_wnaf * q_transition * (pc - pc_shift - 1);

        // validate skew is 0 or 7
        evaluation += q_wnaf * (table_skew * (table_skew - 7));

        evals += evaluation * scaling_factor;
    }
};

} // namespace proof_system::honk::sumcheck
