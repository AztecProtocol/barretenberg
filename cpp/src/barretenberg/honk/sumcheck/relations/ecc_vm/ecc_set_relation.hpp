#pragma once
#include <array>
#include <tuple>

#include "../../polynomials/univariate.hpp"
#include "../relation.hpp"
#include "barretenberg/numeric/uint256/uint256.hpp"
#include <barretenberg/common/constexpr_utils.hpp>

namespace proof_system::honk::sumcheck {

template <typename FF, template <typename> typename TypeMuncher> class ECCVMSetRelationBase {
  public:
    constexpr bool scale_by_random_polynomial() { return true; }

    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 20;

    using UnivariateView = TypeMuncher<FF>::template RelationType<RELATION_LENGTH>::UnivariateView;
    using Univariate = TypeMuncher<FF>::template RelationType<RELATION_LENGTH>::Univariate;
    using RelationParameters = proof_system::honk::sumcheck::RelationParameters<FF>;

    // numerator degree = 8
    // denominator degree = 9
    static constexpr size_t NUMERATOR_TERMS = 8;
    static constexpr size_t DENOMINATOR_TERMS = 7;

    static const UnivariateView get_element_view(const auto& container, const size_t index = 0)
    {
        if constexpr (std::is_same<typeof(container), const std::vector<FF>>::value) {
            return container[index];
        } else {
            return UnivariateView(container);
        }
    }

    static Univariate convert_to_wnaf(const auto& s0, const auto& s1)
    {
        auto t = s0 + s0;
        t += t;
        t += s1;

        auto naf = t + t - 15;
        return naf;
    }

    template <size_t numerator_index>
    static Univariate compute_numerator_term(const auto& extended_edges,
                                             const RelationParameters& relation_params,
                                             const size_t index = 0)
    {
        static_assert(numerator_index < NUMERATOR_TERMS);

        const auto& table_round = get_element_view(extended_edges.table_round, index);
        const auto table_round2 = table_round + table_round;
        const auto table_round4 = table_round2 + table_round2;

        const auto& gamma = relation_params.gamma;
        const auto& eta = relation_params.eta;
        const auto& eta_sqr = relation_params.eta_sqr;
        const auto& eta_cube = relation_params.eta_cube;
        const auto& table_pc = get_element_view(extended_edges.table_pc, index);
        const auto& q_wnaf = get_element_view(extended_edges.q_wnaf, index);

        if constexpr (numerator_index == 0) {
            const auto& s0 = get_element_view(extended_edges.table_s1, index);
            const auto& s1 = get_element_view(extended_edges.table_s2, index);

            auto wnaf_slice = s0 + s0;
            wnaf_slice += wnaf_slice;
            wnaf_slice += s1;

            // todo can optimize
            const auto wnaf_slice_input0 = wnaf_slice + gamma + table_pc * eta + table_round4 * eta_sqr;
            return wnaf_slice_input0;
        } else if constexpr (numerator_index == 1) {
            const auto& s0 = get_element_view(extended_edges.table_s3, index);
            const auto& s1 = get_element_view(extended_edges.table_s4, index);

            auto wnaf_slice = s0 + s0;
            wnaf_slice += wnaf_slice;
            wnaf_slice += s1;

            // todo can optimize
            const auto wnaf_slice_input1 = wnaf_slice + gamma + table_pc * eta + (table_round4 + 1) * eta_sqr;
            return wnaf_slice_input1;
        } else if constexpr (numerator_index == 2) {
            const auto& s0 = get_element_view(extended_edges.table_s5, index);
            const auto& s1 = get_element_view(extended_edges.table_s6, index);

            auto wnaf_slice = s0 + s0;
            wnaf_slice += wnaf_slice;
            wnaf_slice += s1;

            // todo can optimize
            const auto wnaf_slice_input2 = wnaf_slice + gamma + table_pc * eta + (table_round4 + 2) * eta_sqr;
            return wnaf_slice_input2;
        } else if constexpr (numerator_index == 3) {
            const auto& s0 = get_element_view(extended_edges.table_s7, index);
            const auto& s1 = get_element_view(extended_edges.table_s8, index);

            auto wnaf_slice = s0 + s0;
            wnaf_slice += wnaf_slice;
            wnaf_slice += s1;
            // todo can optimize
            const auto wnaf_slice_input3 = wnaf_slice + gamma + table_pc * eta + (table_round4 + 3) * eta_sqr;
            return wnaf_slice_input3;
        } else if constexpr (numerator_index == 4) {
            // skew product if relevant
            const auto& skew = get_element_view(extended_edges.table_skew, index);
            const auto& table_point_transition = get_element_view(extended_edges.table_point_transition, index);
            const auto skew_input =
                table_point_transition * (skew + gamma + table_pc * eta + (table_round4 + 4) * eta_sqr) +
                (-table_point_transition + 1);
            return skew_input;
        } else if constexpr (numerator_index == 5) {

            const auto& permutation_offset = relation_params.permutation_offset;
            return q_wnaf * (-permutation_offset + 1) + permutation_offset;
            return q_wnaf * (-permutation_offset + 1) + permutation_offset;

        } else if constexpr (numerator_index == 6) {
            const auto& table_x = get_element_view(extended_edges.table_tx, index);
            const auto& table_y = get_element_view(extended_edges.table_ty, index);

            const auto& table_skew = get_element_view(extended_edges.table_skew, index);
            static constexpr FF negative_inverse_seven = FF(-7).invert();
            auto adjusted_skew = table_skew * negative_inverse_seven;

            const auto& wnaf_scalar_sum = get_element_view(extended_edges.table_scalar_sum, index);
            const auto w0 = convert_to_wnaf(get_element_view(extended_edges.table_s1, index),
                                            get_element_view(extended_edges.table_s2, index));
            const auto w1 = convert_to_wnaf(get_element_view(extended_edges.table_s3, index),
                                            get_element_view(extended_edges.table_s4, index));
            const auto w2 = convert_to_wnaf(get_element_view(extended_edges.table_s5, index),
                                            get_element_view(extended_edges.table_s6, index));
            const auto w3 = convert_to_wnaf(get_element_view(extended_edges.table_s7, index),
                                            get_element_view(extended_edges.table_s8, index));

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

            auto scalar_sum_full = wnaf_scalar_sum + wnaf_scalar_sum;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += scalar_sum_full;
            scalar_sum_full += row_slice + adjusted_skew;

            auto table_point_transition = get_element_view(extended_edges.table_point_transition, index);

            auto point_table_init_read = (table_pc + table_x * eta + table_y * eta_sqr + scalar_sum_full * eta_cube);
            point_table_init_read =
                table_point_transition * (point_table_init_read + gamma) + (-table_point_transition + 1);

            return point_table_init_read;
        } else if constexpr (numerator_index == 7) {
            const auto& q_msm_transition_shift = get_element_view(extended_edges.q_msm_transition_shift, index);
            const auto& msm_pc_shift = get_element_view(extended_edges.msm_pc_shift, index);

            const auto& msm_x_shift = get_element_view(extended_edges.msm_accumulator_x_shift, index);
            const auto& msm_y_shift = get_element_view(extended_edges.msm_accumulator_y_shift, index);
            const auto& msm_size = get_element_view(extended_edges.msm_size_of_msm, index);

            // q_msm_transition = 1 when a row BEGINS a new msm
            //
            // row msm tx  acc.x acc.y pc  msm_size
            // i   0       no    no    no  yes
            // i+1 1       yes   yes   yes no
            //
            // at row i we are at the final row of the current msm
            // at row i the value of `msm_size` = size of current msm
            // at row i + 1 we have the final accumulated value of the msm computation
            // at row i + 1 we have updated `pc` to be `(pc at start of msm) + msm_count`
            // at row i + 1 q_msm_transtiion = 1

            auto msm_result_write = msm_pc_shift + msm_x_shift * eta + msm_y_shift * eta_sqr + msm_size * eta_cube;

            // msm_result_write = degree 2
            msm_result_write = q_msm_transition_shift * (msm_result_write + gamma) + (-q_msm_transition_shift + 1);
            return msm_result_write;
        }
        return Univariate(1);
    }

    template <size_t denominator_index>
    static Univariate compute_denominator_term(const auto& extended_edges,
                                               const RelationParameters& relation_params,
                                               const size_t index = 0)
    {

        static_assert(denominator_index < DENOMINATOR_TERMS);
        const auto& gamma = relation_params.gamma;
        const auto& eta = relation_params.eta;
        const auto& eta_sqr = relation_params.eta_sqr;
        const auto& eta_cube = relation_params.eta_cube;
        const auto& msm_pc = get_element_view(extended_edges.msm_pc, index);
        const auto& msm_count = get_element_view(extended_edges.msm_count, index);
        const auto& msm_round = get_element_view(extended_edges.msm_round, index);
        const auto& msm_size_of_msm = get_element_view(extended_edges.msm_size_of_msm, index);

        if constexpr (denominator_index == 0) {
            const auto& add1 = get_element_view(extended_edges.msm_q_add1, index);
            const auto& msm_slice1 = get_element_view(extended_edges.msm_slice1, index);

            auto wnaf_slice_output1 =
                add1 * (msm_slice1 + gamma + (msm_pc - msm_count) * eta + msm_round * eta_sqr) + (-add1 + 1);
            return wnaf_slice_output1;

        } else if constexpr (denominator_index == 1) {
            const auto& add2 = get_element_view(extended_edges.msm_q_add2, index);
            const auto& msm_slice2 = get_element_view(extended_edges.msm_slice2, index);

            auto wnaf_slice_output2 =
                add2 * (msm_slice2 + gamma + (msm_pc - msm_count - 1) * eta + msm_round * eta_sqr) + (-add2 + 1);
            return wnaf_slice_output2;

        } else if constexpr (denominator_index == 2) {
            const auto& add3 = get_element_view(extended_edges.msm_q_add3, index);
            const auto& msm_slice3 = get_element_view(extended_edges.msm_slice3, index);

            auto wnaf_slice_output3 =
                add3 * (msm_slice3 + gamma + (msm_pc - msm_count - 2) * eta + msm_round * eta_sqr) + (-add3 + 1);
            return wnaf_slice_output3;

        } else if constexpr (denominator_index == 3) {
            const auto& add4 = get_element_view(extended_edges.msm_q_add4, index);
            const auto& msm_slice4 = get_element_view(extended_edges.msm_slice4, index);
            auto wnaf_slice_output4 =
                add4 * (msm_slice4 + gamma + (msm_pc - msm_count - 3) * eta + msm_round * eta_sqr) + (-add4 + 1);
            return wnaf_slice_output4;
        } else if constexpr (denominator_index == 4 || denominator_index == 5) {
            const auto& transcript_pc = get_element_view(extended_edges.transcript_pc, index);
            const auto& transcript_pc_shift = get_element_view(extended_edges.transcript_pc_shift, index);

            auto transcript_x = get_element_view(extended_edges.transcript_x, index);
            auto transcript_y = get_element_view(extended_edges.transcript_y, index);
            auto z1 = get_element_view(extended_edges.transcript_z1, index);
            auto z2 = get_element_view(extended_edges.transcript_z2, index);
            auto z1_zero = get_element_view(extended_edges.transcript_z1zero, index);
            auto z2_zero = get_element_view(extended_edges.transcript_z2zero, index);
            auto q_transcript_mul = get_element_view(extended_edges.q_transcript_mul, index);

            auto lookup_first = (-z1_zero + 1);
            auto lookup_second = (-z2_zero + 1);
            FF endomorphism_base_field_shift = FF::cube_root_of_unity();

            auto transcript_input1 = transcript_pc + transcript_x * eta + transcript_y * eta_sqr + z1 * eta_cube;
            auto transcript_input2 = (transcript_pc - 1) + transcript_x * endomorphism_base_field_shift * eta -
                                     transcript_y * eta_sqr + z2 * eta_cube;

            // | q_mul | z2_zero | z1_zero | lookup                 |
            // | ----- | ------- | ------- | ---------------------- |
            // | 0     | -       | -       | 1                      |
            // | 1     | 0       | 1       | X + gamma              |
            // | 1     | 1       | 0       | Y + gamma              |
            // | 1     | 1       | 1       | (X + gamma)(Y + gamma) |
            transcript_input1 = (transcript_input1 + gamma) * lookup_first + (-lookup_first + 1);
            transcript_input2 = (transcript_input2 + gamma) * lookup_second + (-lookup_second + 1);
            // point_table_init_write = degree 5
            if (denominator_index == 4) {
                auto point_table_init_write = q_transcript_mul * transcript_input1 + (-q_transcript_mul + 1);
                return point_table_init_write;
            }
            auto point_table_init_write = q_transcript_mul * transcript_input2 + (-q_transcript_mul + 1);
            return point_table_init_write;
        } else if constexpr (denominator_index == 6) {
            auto transcript_pc_shift = get_element_view(extended_edges.transcript_pc_shift, index);
            auto transcript_msm_x = get_element_view(extended_edges.transcript_msm_x, index);
            auto transcript_msm_y = get_element_view(extended_edges.transcript_msm_y, index);
            auto q_transcript_msm_transition = get_element_view(extended_edges.q_transcript_msm_transition, index);
            auto transcript_msm_count = get_element_view(extended_edges.transcript_msm_count, index);
            auto z1_zero = get_element_view(extended_edges.transcript_z1zero, index);
            auto z2_zero = get_element_view(extended_edges.transcript_z2zero, index);
            auto q_transcript_mul = get_element_view(extended_edges.q_transcript_mul, index);

            auto full_msm_count = transcript_msm_count + q_transcript_mul * ((-z1_zero + 1) + (-z2_zero + 1));
            //      auto count_test = transcript_msm_count
            // msm_result_read = degree 2
            auto msm_result_read =
                transcript_pc_shift + transcript_msm_x * eta + transcript_msm_y * eta_sqr + full_msm_count * eta_cube;

            msm_result_read =
                q_transcript_msm_transition * (msm_result_read + gamma) + (-q_transcript_msm_transition + 1);
            return msm_result_read;
        }
        return Univariate(1);
    }

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
                               const RelationParameters& relation_params,
                               const FF& scaling_factor) const
    {

        Univariate numerator_evaluation = Univariate(1);
        barretenberg::constexpr_for<0, NUMERATOR_TERMS, 1>([&]<size_t numerator_index>() {
            numerator_evaluation *= compute_numerator_term<numerator_index>(extended_edges, relation_params, 0);
        });

        Univariate denominator_evaluation = Univariate(1);
        barretenberg::constexpr_for<0, DENOMINATOR_TERMS, 1>([&]<size_t denominator_index>() {
            denominator_evaluation *= compute_denominator_term<denominator_index>(extended_edges, relation_params, 0);
        });

        const auto& lagrange_first = UnivariateView(extended_edges.lagrange_first);
        const auto& lagrange_last = UnivariateView(extended_edges.lagrange_last);

        const auto& z_perm = UnivariateView(extended_edges.z_perm);
        const auto& z_perm_shift = UnivariateView(extended_edges.z_perm_shift);

        evals += ((z_perm + lagrange_first) * numerator_evaluation -
                  (z_perm_shift + lagrange_last) * denominator_evaluation) *
                 scaling_factor;
    }
};

} // namespace proof_system::honk::sumcheck
