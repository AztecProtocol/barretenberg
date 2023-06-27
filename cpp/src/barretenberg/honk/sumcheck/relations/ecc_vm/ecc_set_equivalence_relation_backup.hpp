// #pragma once
// #include <array>
// #include <tuple>

// #include "../../polynomials/univariate.hpp"
// #include "../relation.hpp"
// #include "barretenberg/numeric/uint256/uint256.hpp"

// namespace proof_system::honk::sumcheck {

// template <typename FF, template <typename> typename TypeMuncher> class EccMsmSetEquivalenceBase {
//   public:
//     // 1 + polynomial degree of this relation
//     static constexpr size_t RELATION_LENGTH = 10;

//     using UnivariateView = TypeMuncher<FF>::template RelationType<RELATION_LENGTH>::UnivariateView;
//     using Univariate = TypeMuncher<FF>::template RelationType<RELATION_LENGTH>::Univariate;
//     using RelationParameters = proof_system::honk::sumcheck::RelationParameters<FF>;

//     /**
//      * @brief Expression for the StandardArithmetic gate.
//      * @details The relation is defined as C(extended_edges(X)...) =
//      *    (q_m * w_r * w_l) + (q_l * w_l) + (q_r * w_r) + (q_o * w_o) + q_c
//      *
//      * @param evals transformed to `evals + C(extended_edges(X)...)*scaling_factor`
//      * @param extended_edges an std::array containing the fully extended Univariate edges.
//      * @param parameters contains beta, gamma, and public_input_delta, ....
//      * @param scaling_factor optional term to scale the evaluation before adding to evals.
//      */
//     void add_edge_contribution(Univariate& evals,
//                                const auto& extended_edges,
//                                const RelationParameters& relation_params,
//                                const FF& scaling_factor) const
//     {
//         const auto& x1 = UnivariateView(extended_edges.msm_x1);
//         const auto& y1 = UnivariateView(extended_edges.msm_y1);
//         const auto& x2 = UnivariateView(extended_edges.msm_x2);
//         const auto& y2 = UnivariateView(extended_edges.msm_y2);
//         const auto& x3 = UnivariateView(extended_edges.msm_x3);
//         const auto& y3 = UnivariateView(extended_edges.msm_y3);
//         const auto& x4 = UnivariateView(extended_edges.msm_x4);
//         const auto& y4 = UnivariateView(extended_edges.msm_y4);
//         const auto& collision_inverse1 = UnivariateView(extended_edges.msm_collision_x1);
//         const auto& collision_inverse2 = UnivariateView(extended_edges.msm_collision_x2);
//         const auto& collision_inverse3 = UnivariateView(extended_edges.msm_collision_x3);
//         const auto& collision_inverse4 = UnivariateView(extended_edges.msm_collision_x4);
//         const auto& lambda1 = UnivariateView(extended_edges.msm_lambda1);
//         const auto& lambda2 = UnivariateView(extended_edges.msm_lambda2);
//         const auto& lambda3 = UnivariateView(extended_edges.msm_lambda3);
//         const auto& lambda4 = UnivariateView(extended_edges.msm_lambda4);
//         const auto& add1 = UnivariateView(extended_edges.msm_q_add1);
//         const auto& add1_shift = UnivariateView(extended_edges.msm_q_add1_shift);
//         const auto& add2 = UnivariateView(extended_edges.msm_q_add2);
//         const auto& add3 = UnivariateView(extended_edges.msm_q_add3);
//         const auto& add4 = UnivariateView(extended_edges.msm_q_add4);
//         const auto& acc_x = UnivariateView(extended_edges.msm_accumulator_x);
//         const auto& acc_y = UnivariateView(extended_edges.msm_accumulator_y);
//         const auto& acc_x_shift = UnivariateView(extended_edges.msm_accumulator_x_shift);
//         const auto& acc_y_shift = UnivariateView(extended_edges.msm_accumulator_y_shift);
//         const auto& msm_slice0 = UnivariateView(extended_edges.msm_slice1);
//         const auto& msm_slice1 = UnivariateView(extended_edges.msm_slice2);
//         const auto& msm_slice2 = UnivariateView(extended_edges.msm_slice3);
//         const auto& msm_slice3 = UnivariateView(extended_edges.msm_slice4);

//         const auto& q_msm_transition = UnivariateView(extended_edges.q_msm_transition);
//         // const auto& round_transition = UnivariateView(extended_edges.MSM_ROUND_TRANSITION);
//         const auto& msm_round = UnivariateView(extended_edges.msm_round);
//         const auto& round_shift = UnivariateView(extended_edges.msm_round_shift);

//         const auto& q_add = UnivariateView(extended_edges.msm_q_add);
//         const auto& q_add_shift = UnivariateView(extended_edges.msm_q_add_shift);
//         const auto& q_skew = UnivariateView(extended_edges.msm_q_skew);
//         const auto& q_skew_shift = UnivariateView(extended_edges.msm_q_skew_shift);
//         const auto& q_double = UnivariateView(extended_edges.msm_q_double);
//         const auto& q_double_shift = UnivariateView(extended_edges.msm_q_double_shift);

//         const auto& msm_size_shift = UnivariateView(extended_edges.msm_size_of_msm_shift);

//         const auto& msm_pc = UnivariateView(extended_edges.msm_pc);
//         const auto& count_shift = UnivariateView(extended_edges.msm_count_shift);

//         std::array<UnivariateView, 8> s{
//             UnivariateView(extended_edges.table_s1), UnivariateView(extended_edges.table_s2),
//             UnivariateView(extended_edges.table_s3), UnivariateView(extended_edges.table_s4),
//             UnivariateView(extended_edges.table_s5), UnivariateView(extended_edges.table_s6),
//             UnivariateView(extended_edges.table_s7), UnivariateView(extended_edges.table_s8),
//         };
//         // write wnaf slices

//         const auto& gamma = relation_params.gamma;
//         const auto& eta = relation_params.eta;
//         const auto& eta_sqr = relation_params.eta_sqr;
//         const auto& eta_cube = relation_params.eta_cube;

//         const auto& table_pc = UnivariateView(extended_edges.table_pc);
//         auto table_round = UnivariateView(extended_edges.table_round);

//         auto table_round2 = table_round + table_round;
//         auto table_round4 = table_round2 + table_round2;

//         auto wnaf_slice0 = s[0] + s[0] + s[1];
//         auto wnaf_slice1 = s[2] + s[2] + s[3];
//         auto wnaf_slice2 = s[4] + s[4] + s[5];
//         auto wnaf_slice3 = s[6] + s[6] + s[7];

//         // todo can optimize
//         auto wnaf_slice_input0 = gamma + wnaf_slice0 + table_pc * eta + table_round4 * eta_sqr;
//         auto wnaf_slice_input1 = gamma + wnaf_slice1 + table_pc * eta + (table_round4 + 1) * eta_sqr;
//         auto wnaf_slice_input2 = gamma + wnaf_slice2 + table_pc * eta + (table_round4 + 2) * eta_sqr;
//         auto wnaf_slice_input3 = gamma + wnaf_slice3 + table_pc * eta + (table_round4 + 3) * eta_sqr;

//         auto wnaf_slice_output0 = add1 * (gamma + msm_slice0 + msm_pc * eta + msm_round * eta_sqr) + (-add1 + 1);
//         auto wnaf_slice_output1 = add2 * (gamma + msm_slice1 + (msm_pc + 1) * eta + msm_round * eta_sqr) + (-add2 +
//         1); auto wnaf_slice_output2 = add3 * (gamma + msm_slice2 + (msm_pc + 2) * eta + msm_round * eta_sqr) + (-add3
//         + 1); auto wnaf_slice_output3 = add4 * (gamma + msm_slice3 + (msm_pc + 3) * eta + msm_round * eta_sqr) +
//         (-add4 + 1);

//         // numerator = degree 5 (needs missing selector)
//         auto wnaf_slice_numerator = wnaf_slice_input0 * wnaf_slice_input1 * wnaf_slice_input2 * wnaf_slice_input3;

//         // denominator = degree 8
//         auto wnaf_slice_denominator = wnaf_slice_output0 * wnaf_slice_output1 * wnaf_slice_output2 *
//         wnaf_slice_input3;

//         // update grand product
//         const auto& table_x = UnivariateView(extended_edges.table_tx);
//         const auto& table_y = UnivariateView(extended_edges.table_ty);
//         auto wnaf_scalar_sum = UnivariateView(extended_edges.table_scalar_sum);
//         auto table_skew = UnivariateView(extended_edges.table_skew);
//         static constexpr FF negative_inverse_seven = FF(-7).invert();
//         auto adjusted_skew = table_skew * negative_inverse_seven;

//         const auto convert_to_wnaf = [](const UnivariateView& s0, const UnivariateView& s1) {
//             auto t = s0 + s0;
//             t += t;
//             t += s1;

//             auto naf = t + t - 15;
//             return naf;
//         };

//         const auto w0 = convert_to_wnaf(s[0], s[1]);
//         const auto w1 = convert_to_wnaf(s[2], s[3]);
//         const auto w2 = convert_to_wnaf(s[4], s[5]);
//         const auto w3 = convert_to_wnaf(s[6], s[7]);
//         auto row_slice = w0;
//         row_slice += row_slice;
//         row_slice += row_slice;
//         row_slice += row_slice;
//         row_slice += row_slice;
//         row_slice += w1;
//         row_slice += row_slice;
//         row_slice += row_slice;
//         row_slice += row_slice;
//         row_slice += row_slice;
//         row_slice += w2;
//         row_slice += row_slice;
//         row_slice += row_slice;
//         row_slice += row_slice;
//         row_slice += row_slice;
//         row_slice += w3;

//         auto scalar_sum_full = wnaf_scalar_sum + wnaf_scalar_sum;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += scalar_sum_full;
//         scalar_sum_full += row_slice + adjusted_skew;

//         auto table_point_transition = UnivariateView(extended_edges.table_point_transition);

//         auto point_table_init_read = table_pc + table_x * eta + table_y * eta_sqr + scalar_sum_full * eta_cube;

//         // point_table_init_write = degree 2
//         point_table_init_read =
//             table_point_transition * (gamma + point_table_init_read) + (-table_point_transition + 1);

//         const auto& transcript_pc = UnivariateView(extended_edges.transcript_pc);
//         const auto& transcript_pc_shift = UnivariateView(extended_edges.transcript_pc_shift);

//         auto transcript_x = UnivariateView(extended_edges.transcript_x);
//         auto transcript_y = UnivariateView(extended_edges.transcript_y);
//         auto z1 = UnivariateView(extended_edges.transcript_z1);
//         auto z2 = UnivariateView(extended_edges.transcript_z2);
//         auto z1_zero = UnivariateView(extended_edges.transcript_z1zero);
//         auto z2_zero = UnivariateView(extended_edges.transcript_z2zero);
//         auto q_transcript_mul = UnivariateView(extended_edges.q_transcript_mul);

//         auto lookup_first = (-z1_zero + 1);
//         auto lookup_second = (-z2_zero + 1);
//         FF endomorphism_base_field_shift =
//             123; // TODO: cube root of unity in the curve field. changes depending on FF...

//         auto transcript_input1 = transcript_pc + transcript_x * eta + transcript_y * eta_sqr + z1 * eta_cube;
//         auto transcript_input2 =
//             transcript_pc + transcript_x * endomorphism_base_field_shift * eta - transcript_y * eta_sqr + z2 *
//             eta_cube;

//         // | q_mul | z2_zero | z1_zero | lookup                 |
//         // | ----- | ------- | ------- | ---------------------- |
//         // | 0     | -       | -       | 1                      |
//         // | 1     | 0       | 1       | X + gamma              |
//         // | 1     | 1       | 0       | Y + gamma              |
//         // | 1     | 1       | 1       | (X + gamma)(Y + gamma) |
//         transcript_input1 = (gamma + transcript_input1) * lookup_first + (-lookup_first + 1);
//         transcript_input2 = (gamma + transcript_input2) * lookup_second + (-lookup_second + 1);
//         // point_table_init_write = degree 5
//         auto point_table_init_write =
//             q_transcript_mul * transcript_input1 * transcript_input2 + (-q_transcript_mul + 1);

//         auto transcript_msm_x = UnivariateView(extended_edges.transcript_msm_x);
//         auto transcript_msm_y = UnivariateView(extended_edges.transcript_msm_y);
//         auto q_transcript_msm_transition = UnivariateView(extended_edges.q_transcript_msm_transition);
//         auto transcript_msm_count = UnivariateView(extended_edges.transcript_msm_count);

//         // msm_result_read = degree 2
//         auto msm_result_read =
//             transcript_pc_shift + transcript_msm_x * eta + transcript_msm_y * eta_sqr + transcript_msm_count *
//             eta_cube;
//         msm_result_read = q_transcript_msm_transition * (gamma + msm_result_read) + (-q_transcript_msm_transition +
//         1);

//         const auto& q_msm_transition_shift = UnivariateView(extended_edges.q_msm_transition_shift);
//         const auto& msm_pc_shift = UnivariateView(extended_edges.msm_pc_shift);

//         const auto& msm_x = UnivariateView(extended_edges.msm_accumulator_x_shift);
//         const auto& msm_y = UnivariateView(extended_edges.msm_accumulator_y_shift);
//         const auto& msm_size = UnivariateView(extended_edges.msm_size_of_msm);

//         // q_msm_transition = 1 when a row BEGINS a new msm
//         //
//         // row msm tx  acc.x acc.y pc  msm_size
//         // i   0       no    no    no  yes
//         // i+1 1       yes   yes   yes no
//         //
//         // at row i we are at the final row of the current msm
//         // at row i the value of `msm_size` = size of current msm
//         // at row i + 1 we have the final accumulated value of the msm computation
//         // at row i + 1 we have updated `pc` to be `(pc at start of msm) + msm_count`
//         // at row i + 1 q_msm_transtiion = 1

//         auto msm_result_write = msm_pc_shift + msm_x * eta + msm_y * eta_sqr + msm_size * eta_cube;

//         // msm_result_write = degree 2
//         msm_result_write = q_msm_transition_shift * (gamma + msm_result_write) + (-q_msm_transition_shift + 1);

//         // overall degree = 12
//         // 13 w. flim flam
//         // (8, 5)
//         // (2, 5)
//         // (2, 2)
//     }
// };

// } // namespace proof_system::honk::sumcheck
