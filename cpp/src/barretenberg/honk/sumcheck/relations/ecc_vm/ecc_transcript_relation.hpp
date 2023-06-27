#pragma once
#include <array>
#include <tuple>

#include "barretenberg/honk/flavor/flavor.hpp"
#include "../../polynomials/univariate.hpp"
#include "../relation.hpp"

namespace proof_system::honk::sumcheck {

template <typename FF, template <typename> typename TypeMuncher> class ECCVMTranscriptRelationBase {
  public:
    constexpr bool scale_by_random_polynomial() { return true; }

    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 20;

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

        auto z1 = UnivariateView(extended_edges.transcript_z1);
        auto z2 = UnivariateView(extended_edges.transcript_z2);
        auto z1_zero = UnivariateView(extended_edges.transcript_z1zero);
        auto z2_zero = UnivariateView(extended_edges.transcript_z2zero);

        auto op = UnivariateView(extended_edges.transcript_op);
        auto q_add = UnivariateView(extended_edges.q_transcript_add);
        auto q_mul = UnivariateView(extended_edges.q_transcript_mul);
        auto q_mul_shift = UnivariateView(extended_edges.q_transcript_mul_shift);
        auto q_eq = UnivariateView(extended_edges.q_transcript_eq);
        // auto q_accumulate = UnivariateView(extended_edges.q_transcript_accumulate);
        // auto q_accumulate_shift = UnivariateView(extended_edges.q_transcript_accumulate_shift);
        auto q_msm_transition = UnivariateView(extended_edges.q_transcript_msm_transition);
        // auto q_msm_transition_shift =
        // UnivariateView(extended_edges.q_transcript_msm_transition_shift);
        auto msm_count = UnivariateView(extended_edges.transcript_msm_count);
        auto msm_count_shift = UnivariateView(extended_edges.transcript_msm_count_shift);

        auto pc = UnivariateView(extended_edges.transcript_pc);
        auto pc_shift = UnivariateView(extended_edges.transcript_pc_shift);

        auto transcript_accumulator_x_shift = UnivariateView(extended_edges.transcript_accumulator_x_shift);
        auto transcript_accumulator_y_shift = UnivariateView(extended_edges.transcript_accumulator_y_shift);
        auto transcript_accumulator_x = UnivariateView(extended_edges.transcript_accumulator_x);
        auto transcript_accumulator_y = UnivariateView(extended_edges.transcript_accumulator_y);
        auto transcript_msm_x = UnivariateView(extended_edges.transcript_msm_x);
        auto transcript_msm_y = UnivariateView(extended_edges.transcript_msm_y);
        auto transcript_x = UnivariateView(extended_edges.transcript_x);
        auto transcript_y = UnivariateView(extended_edges.transcript_y);
        auto is_accumulator_empty = UnivariateView(extended_edges.transcript_accumulator_empty);
        auto is_accumulator_empty_shift = UnivariateView(extended_edges.transcript_accumulator_empty_shift);
        auto q_reset_accumulator = UnivariateView(extended_edges.transcript_q_reset_accumulator);
        auto q_reset_accumulator_shift = UnivariateView(extended_edges.transcript_q_reset_accumulator);

        auto lagrange_first = UnivariateView(extended_edges.lagrange_first);

        Univariate evaluation = Univariate(0);
        // if z1zero = 0, this does not rule out z1 being zero; this produces unsatisfiable constraints when computing
        // the scalar sum However if z1zero = 1 we must require that z1 = 0. i.e. z1 * z1zero = 0;
        evaluation += z1 * z1_zero;
        evaluation += z2 * z2_zero;
        evaluation += z1_zero * (z1_zero - 1);
        evaluation += z2_zero * (z2_zero - 1);

        // // set membership components not performed here

        // validate op
        auto tmp = q_add + q_add;
        tmp += q_mul;
        tmp += tmp;
        tmp += q_eq;
        tmp += tmp;
        tmp += q_reset_accumulator;
        evaluation += (tmp - op);

        // update pc depending on if we are performing msm
        // subtract pc by number of muls
        const auto pc_delta = pc - pc_shift;
        evaluation += (pc_delta - q_mul * ((-z1_zero + 1) + (-z2_zero + 1)));

        // if q_mul and q_accumulate

        // determine if we are finishing a MSM on this row
        // MSM end states:
        // |    current row   |    next row      |
        // |    is mul + accumulate | no mul |
        // |    is mul + accumulate | is mul + accumulate |
        // n.b. can optimize this ?
        auto msm_transition = q_mul * (-q_mul_shift + 1);
        evaluation += (q_msm_transition - msm_transition);

        // wtf we do with this?
        // if msm transition...we perform a set membership write (not here)
        // if msm transition we reset msm_count
        evaluation += (q_msm_transition * msm_count_shift);

        // if not a msm transition, count updates by number of muls
        auto msm_count_delta = msm_count_shift - msm_count;
        evaluation += (-q_msm_transition + 1) * (msm_count_delta - q_mul * ((-z1_zero + 1) + (-z2_zero + 1)));

        // if msm transition and we accumulate, add output into acc.

        auto add_msm_into_accumulator = q_msm_transition * (-is_accumulator_empty + 1);
        auto x3 = transcript_accumulator_x_shift;
        auto y3 = transcript_accumulator_y_shift;
        auto x1 = transcript_accumulator_x;
        auto y1 = transcript_accumulator_y;
        auto x2 = transcript_msm_x;
        auto y2 = transcript_msm_y;
        auto tmpx = (x3 + x2 + x1) * (x2 - x1) * (x2 - x1) - (y2 - y1) * (y2 - y1);
        auto tmpy = (y3 + y1) * (x2 - x1) - (y2 - y1) * (x1 - x3);
        evaluation += tmpx * add_msm_into_accumulator;
        evaluation += tmpy * add_msm_into_accumulator;

        // if msm transition and accumulator is empty, assign accumulator
        auto assign_msm_into_accumulator = q_msm_transition * is_accumulator_empty;

        evaluation += assign_msm_into_accumulator * (x3 - x2);
        evaluation += assign_msm_into_accumulator * (y3 - y2);

        // add-accumulate
        x2 = transcript_x;
        y2 = transcript_y;

        auto add_into_accumulator = q_add * (-is_accumulator_empty + 1);
        tmpx = (x3 + x2 + x1) * (x2 - x1) * (x2 - x1) - (y2 - y1) * (y2 - y1);
        tmpy = (y3 + y1) * (x2 - x1) - (y2 - y1) * (x1 - x3);
        evaluation += tmpx * add_into_accumulator;
        evaluation += tmpy * add_into_accumulator;
        auto assign_into_accumulator = q_add * is_accumulator_empty;
        evaluation += (x3 - x2) * assign_into_accumulator;
        evaluation += (y3 - y2) * assign_into_accumulator;

        // assert that if q_mul, all of (q_add, eq, reset) are zero
        evaluation += q_mul * (q_add + q_eq + q_reset_accumulator);

        // assert that if q_add, all of (q_mul, eq, reset) are zero
        evaluation += q_add * (q_mul + q_eq + q_reset_accumulator);

        // if q_add, next is_accumulator_empty = false
        evaluation += q_add * is_accumulator_empty_shift;

        // if msm transition, next is_accumulator_empty = false
        evaluation += q_msm_transition * is_accumulator_empty_shift;

        // if q_reset, next is_accumulator_empty = true
        evaluation += q_reset_accumulator * (-is_accumulator_empty_shift + 1);

        // if NOT q_add or msm_transition or q_reset, next is_accumulator_empty = current value
        auto accumulator_state_not_modified = -(q_add + q_msm_transition + q_reset_accumulator) + 1;
        evaluation += accumulator_state_not_modified * (is_accumulator_empty_shift - is_accumulator_empty);

        // equality check
        evaluation += q_eq * (x1 - x2);
        evaluation += q_eq * (y1 - y2);

        // validate accumulator is empty on 1st row
        evaluation += lagrange_first * (-is_accumulator_empty + 1);

        // validate selectors are boolean
        evaluation += q_eq * (q_eq - 1);
        evaluation += q_add * (q_add - 1);
        evaluation += q_mul * (q_mul - 1);
        evaluation += q_reset_accumulator * (q_reset_accumulator - 1);
        evaluation += q_msm_transition * (q_msm_transition - 1);
        evaluation += is_accumulator_empty * (is_accumulator_empty - 1);
        evaluation += z1_zero * (z1_zero - 1);
        evaluation += z2_zero * (z2_zero - 1);

        // TODO(zac): validate x coordinates are not equal when adding!

        evals += evaluation * scaling_factor;
    }
};

} // namespace proof_system::honk::sumcheck
