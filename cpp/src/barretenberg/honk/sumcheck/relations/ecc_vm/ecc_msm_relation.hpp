#pragma once
#include <array>
#include <tuple>

#include "../../polynomials/univariate.hpp"
#include "../relation.hpp"
#include "barretenberg/numeric/uint256/uint256.hpp"

namespace proof_system::honk::sumcheck {

template <typename FF, template <typename> typename TypeMuncher> class ECCMSMRelationBase {
  public:
    constexpr bool scale_by_random_polynomial() { return true; }

    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 10;

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
        const auto& x1 = UnivariateView(extended_edges.msm_x1);
        const auto& y1 = UnivariateView(extended_edges.msm_y1);
        const auto& x2 = UnivariateView(extended_edges.msm_x2);
        const auto& y2 = UnivariateView(extended_edges.msm_y2);
        const auto& x3 = UnivariateView(extended_edges.msm_x3);
        const auto& y3 = UnivariateView(extended_edges.msm_y3);
        const auto& x4 = UnivariateView(extended_edges.msm_x4);
        const auto& y4 = UnivariateView(extended_edges.msm_y4);
        const auto& collision_inverse1 = UnivariateView(extended_edges.msm_collision_x1);
        const auto& collision_inverse2 = UnivariateView(extended_edges.msm_collision_x2);
        const auto& collision_inverse3 = UnivariateView(extended_edges.msm_collision_x3);
        const auto& collision_inverse4 = UnivariateView(extended_edges.msm_collision_x4);
        const auto& lambda1 = UnivariateView(extended_edges.msm_lambda1);
        const auto& lambda2 = UnivariateView(extended_edges.msm_lambda2);
        const auto& lambda3 = UnivariateView(extended_edges.msm_lambda3);
        const auto& lambda4 = UnivariateView(extended_edges.msm_lambda4);
        const auto& add1 = UnivariateView(extended_edges.msm_q_add1);
        const auto& add1_shift = UnivariateView(extended_edges.msm_q_add1_shift);
        const auto& add2 = UnivariateView(extended_edges.msm_q_add2);
        const auto& add3 = UnivariateView(extended_edges.msm_q_add3);
        const auto& add4 = UnivariateView(extended_edges.msm_q_add4);
        const auto& acc_x = UnivariateView(extended_edges.msm_accumulator_x);
        const auto& acc_y = UnivariateView(extended_edges.msm_accumulator_y);
        const auto& acc_x_shift = UnivariateView(extended_edges.msm_accumulator_x_shift);
        const auto& acc_y_shift = UnivariateView(extended_edges.msm_accumulator_y_shift);
        const auto& slice1 = UnivariateView(extended_edges.msm_slice1);
        const auto& slice2 = UnivariateView(extended_edges.msm_slice2);
        const auto& slice3 = UnivariateView(extended_edges.msm_slice3);
        const auto& slice4 = UnivariateView(extended_edges.msm_slice4);

        const auto& q_msm_transition = UnivariateView(extended_edges.q_msm_transition);
        const auto& q_msm_transition_shift = UnivariateView(extended_edges.q_msm_transition_shift);
        // const auto& round_transition = UnivariateView(extended_edges.MSM_ROUND_TRANSITION);
        const auto& round = UnivariateView(extended_edges.msm_round);
        const auto& round_shift = UnivariateView(extended_edges.msm_round_shift);

        const auto& q_add = UnivariateView(extended_edges.msm_q_add);
        const auto& q_add_shift = UnivariateView(extended_edges.msm_q_add_shift);
        const auto& q_skew = UnivariateView(extended_edges.msm_q_skew);
        const auto& q_skew_shift = UnivariateView(extended_edges.msm_q_skew_shift);
        const auto& q_double = UnivariateView(extended_edges.msm_q_double);
        const auto& q_double_shift = UnivariateView(extended_edges.msm_q_double_shift);

        const auto& msm_size = UnivariateView(extended_edges.msm_size_of_msm);
        const auto& msm_size_shift = UnivariateView(extended_edges.msm_size_of_msm_shift);

        const auto& pc = UnivariateView(extended_edges.msm_pc);
        const auto& pc_shift = UnivariateView(extended_edges.msm_pc_shift);
        const auto& count = UnivariateView(extended_edges.msm_count);
        const auto& count_shift = UnivariateView(extended_edges.msm_count_shift);

        // evals += ((q_add * q_add) - q_add) * scaling_factor;
        // return;
        /**
         * @brief Addition relation
         *
         * If selector = 0, lambda = 0
         * Degree of x_out, y_out = max degree of x_a/x_b + 1
         * 4 Iterations will produce an output degree of 6
         */
        auto add = [&](auto& xb,
                       auto& yb,
                       auto& xa,
                       auto& ya,
                       auto& lambda,
                       auto& selector,
                       auto& relation,
                       auto& collision_relation) {
            // (L * (xb - xa) - yb - ya) * s = 0
            // L * (1 - s) = 0
            // (combine) (L * (xb - xa - 1) - yb - ya) * s + L = 0
            relation += selector * (lambda * (xb - xa - 1) - (yb - ya)) + lambda;
            collision_relation += selector * (xb - xa);
            // x3 = L.L + (-xb - xa) * q + (1 - q) xa
            auto x_out = lambda * lambda + (-xb - xa - xa) * selector + xa;

            // y3 = L . (xa - x3) - ya * q + (1 - q) ya
            auto y_out = lambda * (xa - x_out) + (-ya - ya) * selector + ya;
            return std::array<Univariate, 2>{ x_out, y_out };
        };

        Univariate evaluation = Univariate(0);
        // ADD operations (if row represents ADD round, not SKEW or DOUBLE)
        Univariate add_relation = Univariate(0);
        Univariate x1_collision_relation = Univariate(0);
        Univariate x2_collision_relation = Univariate(0);
        Univariate x3_collision_relation = Univariate(0);
        Univariate x4_collision_relation = Univariate(0);

        auto add_into_accumulator = -q_msm_transition + 1;
        auto [x_t1, y_t1] =
            add(acc_x, acc_y, x1, y1, lambda1, add_into_accumulator, add_relation, x1_collision_relation);
        auto [x_t2, y_t2] = add(x2, y2, x_t1, y_t1, lambda2, add2, add_relation, x2_collision_relation);
        auto [x_t3, y_t3] = add(x3, y3, x_t2, y_t2, lambda3, add3, add_relation, x3_collision_relation);
        auto [x_t4, y_t4] = add(x4, y4, x_t3, y_t3, lambda4, add4, add_relation, x4_collision_relation);

        // Validate accumulator output matches ADD output if q_add = 1
        // (this is a degree-6 relation)
        evaluation += q_add * (acc_x_shift - x_t4);
        evaluation += q_add * (acc_y_shift - y_t4);
        evaluation += q_add * add_relation;

        // SKEW operations
        // When computing x * [P], if x is even we must subtract [P] from accumulator
        // (this is because our windowed non-adjacent-form can only represent odd numbers)
        // Round 32 represents "skew" round.
        // If scalar slice == 7, we add into accumulator (point_table[7] maps to -[P])
        // If scalar slice == 0, we do not add into accumulator
        // i.e. for the skew round we can use the slice values as our "selector" when doing conditional point adds
        Univariate skew_relation = Univariate(0);
        static constexpr FF inverse_seven = FF(7).invert();
        auto skew1_select = slice1 * inverse_seven;
        auto skew2_select = slice2 * inverse_seven;
        auto skew3_select = slice3 * inverse_seven;
        auto skew4_select = slice4 * inverse_seven;
        Univariate x1_skew_collision_relation = Univariate(0);
        Univariate x2_skew_collision_relation = Univariate(0);
        Univariate x3_skew_collision_relation = Univariate(0);
        Univariate x4_skew_collision_relation = Univariate(0);

        // add skew points iff row is a SKEW row AND slice = 7 (point_table[7] maps to -[P])
        // N.B. while it would be nice to have one `add` relation for both ADD and SKEW rounds,
        // this would increase degree of sumcheck identity vs evaluating them separately.
        // This is because, for add rounds, the result of adding [P1], [Acc] is [P1 + Acc] or [P1]
        //             but for skew rounds, the result of adding [P1], [Acc] is [P1 + Acc] or [Acc]
        auto [x_s1, y_s1] = add(x1, y1, acc_x, acc_y, lambda1, skew1_select, skew_relation, x1_skew_collision_relation);
        auto [x_s2, y_s2] = add(x2, y2, x_s1, y_s1, lambda2, skew2_select, skew_relation, x2_skew_collision_relation);
        auto [x_s3, y_s3] = add(x3, y3, x_s2, y_s2, lambda3, skew3_select, skew_relation, x3_skew_collision_relation);
        auto [x_s4, y_s4] = add(x4, y4, x_s3, y_s3, lambda4, skew4_select, skew_relation, x4_skew_collision_relation);

        // Validate accumulator output matches SKEW output if q_skew = 1
        // (this is a degree-6 relation)
        evaluation += q_skew * (acc_x_shift - x_s4);
        evaluation += q_skew * (acc_y_shift - y_s4);
        evaluation += q_skew * skew_relation;

        // Check x-coordinates do not collide if row is an ADD row or a SKEW row
        // if either q_add or q_skew = 1, an inverse should exist for each computed relation
        // Step 1: construct boolean selectors that describe whether we added a point at the current row
        const auto add_first_point = add_into_accumulator * q_add + q_skew * skew1_select;
        const auto add_second_point = add2 * q_add + q_skew * skew2_select;
        const auto add_third_point = add3 * q_add + q_skew * skew3_select;
        const auto add_fourth_point = add4 * q_add + q_skew * skew4_select;
        // Step 2: construct the delta between x-coordinates for each point add (depending on if row is ADD or SKEW)
        const auto x1_delta = x1_skew_collision_relation * q_skew + x1_collision_relation * q_add;
        const auto x2_delta = x2_skew_collision_relation * q_skew + x2_collision_relation * q_add;
        const auto x3_delta = x3_skew_collision_relation * q_skew + x3_collision_relation * q_add;
        const auto x4_delta = x4_skew_collision_relation * q_skew + x4_collision_relation * q_add;
        // Step 3: x_delta * inverse - 1 = 0 if we performed a point addition (else x_delta * inverse = 0)
        evaluation += x1_delta * collision_inverse1 - add_first_point;
        evaluation += x2_delta * collision_inverse2 - add_second_point;
        evaluation += x3_delta * collision_inverse3 - add_third_point;
        evaluation += x4_delta * collision_inverse4 - add_fourth_point;

        /**
         * @brief doubles a point.
         *
         * Degree of x_out = 2
         * Degree of y_out = 3
         * Degree of relation = 4
         */
        auto dbl = [&](auto& x, auto& y, auto& lambda, auto& relation) {
            auto two_x = x + x;
            relation += lambda * (y + y) - (two_x + x) * x;
            auto x_out = lambda * lambda - two_x;
            auto y_out = lambda * (x - x_out) - y;
            return std::array<Univariate, 2>{ x_out, y_out };
        };

        // VALIDATE ACCUMULATOR OUTPUT IF WE ARE DOUBLING ACCUMULATOR THIS ROW
        Univariate double_relation = Univariate(0);
        auto [x_d1, y_d1] = dbl(acc_x, acc_y, lambda1, double_relation);
        auto [x_d2, y_d2] = dbl(x_d1, y_d1, lambda2, double_relation);
        auto [x_d3, y_d3] = dbl(x_d2, y_d2, lambda3, double_relation);
        auto [x_d4, y_d4] = dbl(x_d3, y_d3, lambda4, double_relation);
        evaluation += q_double * (acc_x_shift - x_d4);
        evaluation += q_double * (acc_y_shift - y_d4);
        evaluation += q_double * double_relation;

        // TODO(zac): what about first add which always must be true if row is add?
        // Do we need to enforce?

        // If add_i = 0, slice_i = 0
        // When add_i = 0, force slice_i to ALSO be 0
        evaluation += (-add1 + 1) * slice1;
        evaluation += (-add2 + 1) * slice2;
        evaluation += (-add3 + 1) * slice3;
        evaluation += (-add4 + 1) * slice4;

        // only one of q_skew, q_double, q_add can be nonzero
        evaluation += (q_add * q_double + q_add * q_skew + q_double * q_skew);
        // std::cout << "I " << evaluation << std::endl;

        // We look up wnaf slices by mapping round + pc -> slice
        // We use an exact set membership check to validate that
        // wnafs written in wnaf_relation == wnafs read in msm relation
        // We use `add1/add2/add3/add4` to flag whether we are performing a wnaf read op
        // We can set these to be Prover-defined as the set membership check implicitly ensures that the correct reads
        // have occurred.
        // if q_msm_transition = 0, round_shift - round = 0 or 1
        const auto round_delta = round_shift - round;

        // ROUND TRANSITION LOGIC (when round does not change)
        // If q_msm_transition = 0 (next row) then round_delta = 0 or 1
        const auto round_transition = round_delta * (-q_msm_transition_shift + 1);
        evaluation += round_transition * (round_delta - 1);

        // ROUND TRANSITION LOGIC (when round DOES change)
        // round_transition describes whether we are transitioning between rounds of an MSM
        // If round_transition = 1, the next row is either a double (if round != 31) or we are adding skew (if round ==
        // 31) round_transition * skew * (round - 31) = 0 (if round tx and skew, round == 31) round_transition * (skew +
        // double - 1) = 0 (if round tx, skew XOR double = 1) i.e. if round tx and round != 31, double = 1
        evaluation += round_transition * q_skew_shift * (round - 31);
        evaluation += round_transition * (q_skew_shift + q_double_shift - 1);

        // if no double or no skew, round_delta = 0
        evaluation += round_transition * (-q_double_shift + 1) * (-q_skew_shift + 1);

        // if double, next double != 1
        evaluation += q_double * q_double_shift;

        // if double, next add = 1
        evaluation += q_double * (-q_add_shift + 1);

        // updating count
        // if q_msm_transition = 0 and round_transition = 0, count_shift = count + add1 + add2 + add3 + add4
        // todo: we need this?
        evaluation +=
            (-q_msm_transition_shift + 1) * (-round_delta + 1) * (count_shift - count - add1 - add2 - add3 - add4);

        // if q_msm_transition = 0 and round_transition = 1, count_shift = 0
        evaluation += (-q_msm_transition_shift + 1) * round_delta * count_shift;

        // if q_msm_transition = 1, count_shift = 0
        evaluation += q_msm_transition_shift * count_shift;

        // if q_msm_transition = 1, pc = pc_shift + msm_size
        evaluation += q_msm_transition_shift * (msm_size + pc_shift - pc);

        // Addition continuity checks
        // We want to RULE OUT the following scenarios:
        // Case 1: add2 = 1, add1 = 0
        // Case 2: add3 = 1, add2 = 0
        // Case 3: add4 = 1, add3 = 0
        // These checks ensure that the current row does not skip points (for both ADD and SKEW ops)
        // This is part of a wider set of checks we use to ensure that all point data is used in the assigned
        // multiscalar multiplication operation.
        // (and not in a different MSM operation)
        evaluation += add2 * (-add1 + 1);
        evaluation += add3 * (-add2 + 1);
        evaluation += add4 * (-add3 + 1);

        // Final continuity check.
        // If an addition spans two rows, we need to make sure that the following scenario is RULED OUT:
        //   add4 = 0 on the CURRENT row, add1 = 1 on the NEXT row
        // We must apply the above for the two cases:
        // Case 1: q_add = 1 on the CURRENT row, q_add = 1 on the NEXT row
        // Case 2: q_skew = 1 on the CURRENT row, q_skew = 1 on the NEXT row
        // (i.e. if q_skew = 1, q_add_shift = 1 this implies an MSM transition so we skip this continuity check)
        evaluation += (q_add * q_add_shift + q_skew * q_skew_shift) * (-add4 + 1) * add1_shift;

        // if q_msm_transition_shift = 0, msm_size does not change
        evaluation += (-q_msm_transition_shift + 1) * (msm_size_shift - msm_size);

        evals += (evaluation * scaling_factor);

        // remaining checks
        // when transition occurs, perform set membership lookup on (accumulator / pc / msm_size)
        // perform set membership lookups on add_i * (pc / round / slice_i)
        // perform lookups on (pc / slice_i / x / y)
    }
};

} // namespace proof_system::honk::sumcheck
