#include "ecc_msm_relation.hpp"
#include "barretenberg/honk/sumcheck/relations/relation_parameters.hpp"
#include "barretenberg/honk/sumcheck/relations/relation_definitions_fwd.hpp"
#include "barretenberg/honk/flavor/ecc_vm.hpp"

namespace proof_system::honk::sumcheck {

template <typename FF>
template <typename AccumulatorTypes>
ECCVMSetRelationBase<FF>::template Accumulator<AccumulatorTypes> ECCVMSetRelationBase<
    FF>::compute_permutation_numerator(const auto& extended_edges,
                                       const RelationParameters<FF>& relation_params,
                                       const size_t index)
{
    using Accumulator = typename std::tuple_element<0, typename AccumulatorTypes::Accumulators>::type;

    const auto& table_round = get_view<FF, AccumulatorTypes>(extended_edges.table_round, index);
    const auto table_round2 = table_round + table_round;
    const auto table_round4 = table_round2 + table_round2;

    const auto& gamma = relation_params.gamma;
    const auto& eta = relation_params.eta;
    const auto& eta_sqr = relation_params.eta_sqr;
    const auto& eta_cube = relation_params.eta_cube;
    const auto& table_pc = get_view<FF, AccumulatorTypes>(extended_edges.table_pc, index);
    const auto& q_wnaf = get_view<FF, AccumulatorTypes>(extended_edges.q_wnaf, index);

    /**
     * @brief First term: tuple of (pc, round, wnaf_slice), computed when slicing scalar multipliers into slices,
     *        as part of ECCVMWnafRelation.
     *        If q_wnaf = 1, tuple entry = (wnaf-slice + point-counter * eta + msm-round * eta_sqr).
     *                       There are 4 tuple entires per row.
     */
    Accumulator numerator(1); // degree-0
    {
        const auto& s0 = get_view<FF, AccumulatorTypes>(extended_edges.table_s1, index);
        const auto& s1 = get_view<FF, AccumulatorTypes>(extended_edges.table_s2, index);

        auto wnaf_slice = s0 + s0;
        wnaf_slice += wnaf_slice;
        wnaf_slice += s1;

        // todo can optimize
        const auto wnaf_slice_input0 = wnaf_slice + gamma + table_pc * eta + table_round4 * eta_sqr;
        numerator *= wnaf_slice_input0; // degree-1
    }
    {
        const auto& s0 = get_view<FF, AccumulatorTypes>(extended_edges.table_s3, index);
        const auto& s1 = get_view<FF, AccumulatorTypes>(extended_edges.table_s4, index);

        auto wnaf_slice = s0 + s0;
        wnaf_slice += wnaf_slice;
        wnaf_slice += s1;

        // todo can optimize
        const auto wnaf_slice_input1 = wnaf_slice + gamma + table_pc * eta + (table_round4 + 1) * eta_sqr;
        numerator *= wnaf_slice_input1; // degree-2
    }
    {
        const auto& s0 = get_view<FF, AccumulatorTypes>(extended_edges.table_s5, index);
        const auto& s1 = get_view<FF, AccumulatorTypes>(extended_edges.table_s6, index);

        auto wnaf_slice = s0 + s0;
        wnaf_slice += wnaf_slice;
        wnaf_slice += s1;

        // todo can optimize
        const auto wnaf_slice_input2 = wnaf_slice + gamma + table_pc * eta + (table_round4 + 2) * eta_sqr;
        numerator *= wnaf_slice_input2; // degree-3
    }
    {
        const auto& s0 = get_view<FF, AccumulatorTypes>(extended_edges.table_s7, index);
        const auto& s1 = get_view<FF, AccumulatorTypes>(extended_edges.table_s8, index);

        auto wnaf_slice = s0 + s0;
        wnaf_slice += wnaf_slice;
        wnaf_slice += s1;
        // todo can optimize
        const auto wnaf_slice_input3 = wnaf_slice + gamma + table_pc * eta + (table_round4 + 3) * eta_sqr;
        numerator *= wnaf_slice_input3; // degree-4
    }
    {
        // skew product if relevant
        const auto& skew = get_view<FF, AccumulatorTypes>(extended_edges.table_skew, index);
        const auto& table_point_transition =
            get_view<FF, AccumulatorTypes>(extended_edges.table_point_transition, index);
        const auto skew_input =
            table_point_transition * (skew + gamma + table_pc * eta + (table_round4 + 4) * eta_sqr) +
            (-table_point_transition + 1);
        numerator *= skew_input; // degree-5
    }
    {
        const auto& permutation_offset = relation_params.permutation_offset;
        numerator *= q_wnaf * (-permutation_offset + 1) + permutation_offset; // degree-7
    }

    /**
     * @brief Second term: tuple of (point-counter, P.x, P.y, scalar-multiplier), used in ECCVMWnafRelation and
     * ECCVMPointTableRelation. ECCVMWnafRelation validates the sum of the wnaf slices associated with point-counter
     * equals scalar-multiplier. ECCVMPointTableRelation computes a table of muliples of [P]: { -15[P], -13[P], ...,
     * 15[P] }. We need to validate that scalar-multiplier and [P] = (P.x, P.y) come from MUL opcodes in the transcript
     * columns.
     */
    {
        const auto& table_x = get_view<FF, AccumulatorTypes>(extended_edges.table_tx, index);
        const auto& table_y = get_view<FF, AccumulatorTypes>(extended_edges.table_ty, index);

        const auto& table_skew = get_view<FF, AccumulatorTypes>(extended_edges.table_skew, index);
        static constexpr FF negative_inverse_seven = FF(-7).invert();
        auto adjusted_skew = table_skew * negative_inverse_seven;

        const auto& wnaf_scalar_sum = get_view<FF, AccumulatorTypes>(extended_edges.table_scalar_sum, index);
        const auto w0 =
            convert_to_wnaf<AccumulatorTypes>(get_view<FF, AccumulatorTypes>(extended_edges.table_s1, index),
                                              get_view<FF, AccumulatorTypes>(extended_edges.table_s2, index));
        const auto w1 =
            convert_to_wnaf<AccumulatorTypes>(get_view<FF, AccumulatorTypes>(extended_edges.table_s3, index),
                                              get_view<FF, AccumulatorTypes>(extended_edges.table_s4, index));
        const auto w2 =
            convert_to_wnaf<AccumulatorTypes>(get_view<FF, AccumulatorTypes>(extended_edges.table_s5, index),
                                              get_view<FF, AccumulatorTypes>(extended_edges.table_s6, index));
        const auto w3 =
            convert_to_wnaf<AccumulatorTypes>(get_view<FF, AccumulatorTypes>(extended_edges.table_s7, index),
                                              get_view<FF, AccumulatorTypes>(extended_edges.table_s8, index));

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

        auto table_point_transition = get_view<FF, AccumulatorTypes>(extended_edges.table_point_transition, index);

        auto point_table_init_read = (table_pc + table_x * eta + table_y * eta_sqr + scalar_sum_full * eta_cube);
        point_table_init_read =
            table_point_transition * (point_table_init_read + gamma) + (-table_point_transition + 1);

        numerator *= point_table_init_read; // degree-9
    }
    /**
     * @brief Third term: tuple of (point-counter, P.x, P.y, msm-size) from ECCVMMSMRelation.
     *        (P.x, P.y) is the output of a multi-scalar-multiplication evaluated in ECCVMMSMRelation.
     *        We need to validate that the same values (P.x, P.y) are present in the Transcript columns and describe a
     *        multi-scalar multiplication of size `msm-size`, starting at `point-counter`.
     *
     *        If q_msm_transition_shift = 1, this indicates the current row is the last row of a multiscalar
     * multiplication evaluation. The output of the MSM will be present on `(msm_accumulator_x_shift,
     * msm_accumulator_y_shift)`. The values of `msm_accumulator_x_shift, msm_accumulator_y_shift, msm_pc,
     * msm_size_of_msm` must match up with equivalent values `transcript_msm_output_x, transcript_msm_output_y,
     * transcript_pc, transcript_msm_count` present in the Transcript columns
     */
    {
        const auto& lagrange_first = get_view<FF, AccumulatorTypes>(extended_edges.lagrange_first, index);
        const auto& partial_q_msm_transition_shift =
            get_view<FF, AccumulatorTypes>(extended_edges.q_msm_transition_shift, index);
        const auto q_msm_transition_shift = (-lagrange_first + 1) * partial_q_msm_transition_shift;
        const auto& msm_pc_shift = get_view<FF, AccumulatorTypes>(extended_edges.msm_pc_shift, index);

        const auto& msm_x_shift = get_view<FF, AccumulatorTypes>(extended_edges.msm_accumulator_x_shift, index);
        const auto& msm_y_shift = get_view<FF, AccumulatorTypes>(extended_edges.msm_accumulator_y_shift, index);
        const auto& msm_size = get_view<FF, AccumulatorTypes>(extended_edges.msm_size_of_msm, index);

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
        numerator *= msm_result_write; // degree-11
    }
    return numerator;
}

template <typename FF>
template <typename AccumulatorTypes>
ECCVMSetRelationBase<FF>::template Accumulator<AccumulatorTypes> ECCVMSetRelationBase<
    FF>::compute_permutation_denominator(const auto& extended_edges,
                                         const RelationParameters<FF>& relation_params,
                                         const size_t index)
{
    using Accumulator = typename std::tuple_element<0, typename AccumulatorTypes::Accumulators>::type;

    // TODO(@zac-williamson). The degree of this contribution is 17! makes overall relation degree 19.
    // Can optimise by refining the algebra, once we have a stable base to iterate off of.
    const auto& gamma = relation_params.gamma;
    const auto& eta = relation_params.eta;
    const auto& eta_sqr = relation_params.eta_sqr;
    const auto& eta_cube = relation_params.eta_cube;
    const auto& msm_pc = get_view<FF, AccumulatorTypes>(extended_edges.msm_pc, index);
    const auto& msm_count = get_view<FF, AccumulatorTypes>(extended_edges.msm_count, index);
    const auto& msm_round = get_view<FF, AccumulatorTypes>(extended_edges.msm_round, index);

    /**
     * @brief First term: tuple of (pc, round, wnaf_slice), used to determine which points we extract from lookup tables
     * when evaluaing MSMs in ECCVMMsmRelation.
     * These values must be equivalent to the values computed in the 1st term of `compute_permutation_numerator`
     */
    Accumulator denominator(1); // degree-0
    {
        const auto& add1 = get_view<FF, AccumulatorTypes>(extended_edges.msm_q_add1, index);
        const auto& msm_slice1 = get_view<FF, AccumulatorTypes>(extended_edges.msm_slice1, index);

        auto wnaf_slice_output1 =
            add1 * (msm_slice1 + gamma + (msm_pc - msm_count) * eta + msm_round * eta_sqr) + (-add1 + 1);
        denominator *= wnaf_slice_output1; // degree-2
    }
    {
        const auto& add2 = get_view<FF, AccumulatorTypes>(extended_edges.msm_q_add2, index);
        const auto& msm_slice2 = get_view<FF, AccumulatorTypes>(extended_edges.msm_slice2, index);

        auto wnaf_slice_output2 =
            add2 * (msm_slice2 + gamma + (msm_pc - msm_count - 1) * eta + msm_round * eta_sqr) + (-add2 + 1);
        denominator *= wnaf_slice_output2; // degree-4
    }
    {
        const auto& add3 = get_view<FF, AccumulatorTypes>(extended_edges.msm_q_add3, index);
        const auto& msm_slice3 = get_view<FF, AccumulatorTypes>(extended_edges.msm_slice3, index);

        auto wnaf_slice_output3 =
            add3 * (msm_slice3 + gamma + (msm_pc - msm_count - 2) * eta + msm_round * eta_sqr) + (-add3 + 1);
        denominator *= wnaf_slice_output3; // degree-6
    }
    {
        const auto& add4 = get_view<FF, AccumulatorTypes>(extended_edges.msm_q_add4, index);
        const auto& msm_slice4 = get_view<FF, AccumulatorTypes>(extended_edges.msm_slice4, index);
        auto wnaf_slice_output4 =
            add4 * (msm_slice4 + gamma + (msm_pc - msm_count - 3) * eta + msm_round * eta_sqr) + (-add4 + 1);
        denominator *= wnaf_slice_output4; // degree-8
    }

    /**
     * @brief Second term: tuple of (transcript_pc, transcript_x, transcript_y, z1) OR (transcript_pc, \lambda *
     * transcript_x, -transcript_y, z2) for each scalar multiplication in ECCVMTranscriptRelation columns. (the latter
     * term uses the curve endomorphism: \lambda = cube root of unity). These values must be equivalent to the second
     * term values in `compute_permutation_numerator`
     */
    {
        const auto& transcript_pc = get_view<FF, AccumulatorTypes>(extended_edges.transcript_pc, index);

        auto transcript_x = get_view<FF, AccumulatorTypes>(extended_edges.transcript_x, index);
        auto transcript_y = get_view<FF, AccumulatorTypes>(extended_edges.transcript_y, index);
        auto z1 = get_view<FF, AccumulatorTypes>(extended_edges.transcript_z1, index);
        auto z2 = get_view<FF, AccumulatorTypes>(extended_edges.transcript_z2, index);
        auto z1_zero = get_view<FF, AccumulatorTypes>(extended_edges.transcript_z1zero, index);
        auto z2_zero = get_view<FF, AccumulatorTypes>(extended_edges.transcript_z2zero, index);
        auto q_transcript_mul = get_view<FF, AccumulatorTypes>(extended_edges.q_transcript_mul, index);

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
        // point_table_init_write = degree 2

        auto point_table_init_write =
            q_transcript_mul * transcript_input1 * transcript_input2 + (-q_transcript_mul + 1);
        denominator *= point_table_init_write; // degree-13

        // auto point_table_init_write_1 = q_transcript_mul * transcript_input1 + (-q_transcript_mul + 1);
        // denominator *= point_table_init_write_1; // degree-11

        // auto point_table_init_write_2 = q_transcript_mul * transcript_input2 + (-q_transcript_mul + 1);
        // denominator *= point_table_init_write_2; // degree-14
    }
    /**
     * @brief Third term: tuple of (point-counter, P.x, P.y, msm-size) from ECCVMTranscriptRelation.
     *        (P.x, P.y) is the *claimed* output of a multi-scalar-multiplication evaluated in ECCVMMSMRelation.
     *        We need to validate that the msm output produced in ECCVMMSMRelation is equivalent to the output present
     * in `transcript_msm_output_x, transcript_msm_output_y`, for a given multi-scalar multiplication starting at
     * `transcript_pc` and has size `transcript_msm_count`
     */
    {
        auto transcript_pc_shift = get_view<FF, AccumulatorTypes>(extended_edges.transcript_pc_shift, index);
        auto transcript_msm_x = get_view<FF, AccumulatorTypes>(extended_edges.transcript_msm_x, index);
        auto transcript_msm_y = get_view<FF, AccumulatorTypes>(extended_edges.transcript_msm_y, index);
        auto q_transcript_msm_transition =
            get_view<FF, AccumulatorTypes>(extended_edges.q_transcript_msm_transition, index);
        auto transcript_msm_count = get_view<FF, AccumulatorTypes>(extended_edges.transcript_msm_count, index);
        auto z1_zero = get_view<FF, AccumulatorTypes>(extended_edges.transcript_z1zero, index);
        auto z2_zero = get_view<FF, AccumulatorTypes>(extended_edges.transcript_z2zero, index);
        auto q_transcript_mul = get_view<FF, AccumulatorTypes>(extended_edges.q_transcript_mul, index);

        auto full_msm_count = transcript_msm_count + q_transcript_mul * ((-z1_zero + 1) + (-z2_zero + 1));
        //      auto count_test = transcript_msm_count
        // msm_result_read = degree 2
        auto msm_result_read =
            transcript_pc_shift + transcript_msm_x * eta + transcript_msm_y * eta_sqr + full_msm_count * eta_cube;

        msm_result_read = q_transcript_msm_transition * (msm_result_read + gamma) + (-q_transcript_msm_transition + 1);
        denominator *= msm_result_read; // degree-17
    }
    return denominator;
}

/**
 * @brief Expression for the StandardArithmetic gate.
 * @details The relation is defined as C(extended_edges(X)...) =
 *    (q_m * w_r * w_l) + (q_l * w_l) + (q_r * w_r) + (q_o * w_o) + q_c
 *
 * @param evals transformed to `evals + C(extended_edges(X)...)*scaling_factor`
 * @param extended_edges an std::array containing the fully extended Accumulator edges.
 * @param parameters contains beta, gamma, and public_input_delta, ....
 * @param scaling_factor optional term to scale the evaluation before adding to evals.
 */
template <typename FF>
template <typename AccumulatorTypes>
void ECCVMSetRelationBase<FF>::add_edge_contribution_impl(typename AccumulatorTypes::Accumulators& accumulator,
                                                          const auto& extended_edges,
                                                          const RelationParameters<FF>& relation_params,
                                                          const FF& scaling_factor) const
{
    using View = typename std::tuple_element<0, typename AccumulatorTypes::AccumulatorViews>::type;
    using Accumulator = typename std::tuple_element<0, typename AccumulatorTypes::Accumulators>::type;

    // degree-11
    Accumulator numerator_evaluation =
        compute_permutation_numerator<AccumulatorTypes>(extended_edges, relation_params, 0);

    // degree-17
    Accumulator denominator_evaluation =
        compute_permutation_denominator<AccumulatorTypes>(extended_edges, relation_params, 0);

    const auto& lagrange_first = View(extended_edges.lagrange_first);
    const auto& lagrange_last = View(extended_edges.lagrange_last);

    const auto& z_perm = View(extended_edges.z_perm);
    const auto& z_perm_shift = View(extended_edges.z_perm_shift);

    // degree-18
    std::get<0>(accumulator) +=
        ((z_perm + lagrange_first) * numerator_evaluation - (z_perm_shift + lagrange_last) * denominator_evaluation) *
        scaling_factor;

    // TODO(zac) ADD BOUNDARY CHECKS WOLOLOLOLOLO
}

template class ECCVMSetRelationBase<barretenberg::fr>;
DEFINE_SUMCHECK_RELATION_CLASS(ECCVMSetRelationBase, flavor::ECCVM);
DEFINE_SUMCHECK_RELATION_CLASS(ECCVMSetRelationBase, flavor::ECCVMGrumpkin);
DEFINE_SUMCHECK_PERMUTATION_CLASS(ECCVMSetRelationBase, flavor::ECCVM);
DEFINE_SUMCHECK_PERMUTATION_CLASS(ECCVMSetRelationBase, flavor::ECCVMGrumpkin);

} // namespace proof_system::honk::sumcheck
