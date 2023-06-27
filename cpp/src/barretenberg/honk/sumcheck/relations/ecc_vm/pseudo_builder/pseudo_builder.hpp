#pragma once

#include "../ecc_vm_types.hpp"
#include "./eccvm_builder_types.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/honk/flavor/ecc_vm.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"
#include "barretenberg/honk/flavor/ecc_vm.hpp"

#include "./msm_builder.hpp"
#include "./transcript_builder.hpp"
#include "./precomputed_tables_builder.hpp"

namespace proof_system::honk::sumcheck {

class ECCVMBuilder {
  public:
    static constexpr size_t NUM_SCALAR_BITS = eccvm::NUM_SCALAR_BITS;
    static constexpr size_t WNAF_SLICE_BITS = eccvm::WNAF_SLICE_BITS;
    static constexpr size_t NUM_WNAF_SLICES = eccvm::NUM_WNAF_SLICES;
    static constexpr uint64_t WNAF_MASK = eccvm::WNAF_MASK;
    static constexpr size_t POINT_TABLE_SIZE = eccvm::POINT_TABLE_SIZE;
    static constexpr size_t WNAF_SLICES_PER_ROW = eccvm::WNAF_SLICES_PER_ROW;
    static constexpr size_t ADDITIONS_PER_ROW = eccvm::ADDITIONS_PER_ROW;

    using Flavor = proof_system::honk::flavor::ECCVM;
    using RawPolynomials = typename Flavor::FoldedPolynomials;
    static constexpr size_t NUM_POLYNOMIALS = Flavor::NUM_ALL_ENTITIES;

    using MSM = eccvm::MSM;
    using VMOperation = eccvm::VMOperation;
    std::vector<VMOperation> vm_operations;
    using ScalarMul = eccvm::ScalarMul;

    uint32_t get_number_of_muls()
    {
        uint32_t num_muls = 0;
        for (auto& op : vm_operations) {
            if (op.mul) {
                if (op.z1 != 0) {
                    num_muls++;
                }
                if (op.z2 != 0) {
                    num_muls++;
                }
            }
        }
        return num_muls;
    }

    std::vector<MSM> get_msms()
    {
        const uint32_t num_muls = get_number_of_muls();
        /**
         * For input point [P], return { -15[P], -13[P], ..., -[P], [P], ..., 13[P], 15[P] }
         */
        const auto compute_precomputed_table = [](const grumpkin::g1::affine_element& base_point) {
            const auto d2 = grumpkin::g1::element(base_point).dbl();
            std::array<grumpkin::g1::affine_element, POINT_TABLE_SIZE> table;
            table[POINT_TABLE_SIZE / 2] = base_point;
            for (size_t i = 1; i < POINT_TABLE_SIZE / 2; ++i) {
                table[i + POINT_TABLE_SIZE / 2] = grumpkin::g1::element(table[i + POINT_TABLE_SIZE / 2 - 1]) + d2;
            }
            for (size_t i = 0; i < POINT_TABLE_SIZE / 2; ++i) {
                table[i] = -table[POINT_TABLE_SIZE - 1 - i];
            }
            return table;
        };
        const auto compute_wnaf_slices = [](uint256_t scalar) {
            std::array<int, NUM_WNAF_SLICES> output;
            int previous_slice = 0;
            for (size_t i = 0; i < NUM_WNAF_SLICES; ++i) {
                // slice the scalar into 4-bit chunks, starting with the least significant bits
                uint64_t raw_slice = static_cast<uint64_t>(scalar) & WNAF_MASK;

                bool is_even = ((raw_slice & 1ULL) == 0ULL);

                int wnaf_slice = static_cast<int>(raw_slice);

                if (i == 0 && is_even) {
                    // if least significant slice is even, we add 1 to create an odd value && set 'skew' to true
                    wnaf_slice += 1;
                } else if (is_even) {
                    // for other slices, if it's even, we add 1 to the slice value
                    // and subtract 16 from the previous slice to preserve the total scalar sum
                    static constexpr int borrow_constant = static_cast<int>(1ULL << WNAF_SLICE_BITS);
                    previous_slice -= borrow_constant;
                    wnaf_slice += 1;
                }

                if (i > 0) {
                    const size_t idx = i - 1;
                    output[NUM_WNAF_SLICES - idx - 1] = previous_slice;
                }
                previous_slice = wnaf_slice;

                // downshift raw_slice by 4 bits
                scalar = scalar >> WNAF_SLICE_BITS;
            }

            ASSERT(scalar == 0);

            output[0] = previous_slice;

            return output;
        };
        std::vector<MSM> msms;
        std::vector<ScalarMul> active_msm;

        // We start pc at `num_muls` and decrement for each mul processed.
        // This gives us two desired properties:
        // 1: the value of pc at the 1st row = number of muls (easy to check)
        // 2: the value of pc for the final mul = 1
        // The latter point is valuable as it means that we can add empty rows (where pc = 0) and still satisfy our
        // sumcheck relations that involve pc (if we did the other way around, starting at 1 and ending at num_muls, we
        // create a discontinuity in pc values between the last transcript row and the following empty row)
        uint32_t pc = num_muls;

        const auto process_mul = [&active_msm, &pc, &compute_wnaf_slices, &compute_precomputed_table](
                                     const auto& scalar, const auto& base_point) {
            if (scalar != 0) {
                active_msm.push_back(ScalarMul{
                    .pc = pc,
                    .scalar = scalar,
                    .base_point = base_point,
                    .wnaf_slices = compute_wnaf_slices(scalar),
                    .wnaf_skew = (scalar & 1) == 0,
                    .precomputed_table = compute_precomputed_table(base_point),
                });
                pc--;
            }
        };

        for (auto& op : vm_operations) {
            if (op.mul) {
                process_mul(op.z1, op.base_point);
                process_mul(op.z2,
                            grumpkin::g1::affine_element{ op.base_point.x * grumpkin::fq::cube_root_of_unity(),
                                                          -op.base_point.y });

            } else {
                if (active_msm.size() > 0) {
                    msms.push_back(active_msm);
                    active_msm = {};
                }
            }
        }
        if (active_msm.size() > 0) {
            msms.push_back(active_msm);
        }

        ASSERT(pc == 0);
        return msms;
    }

    std::vector<ScalarMul> get_flattened_scalar_muls(const std::vector<MSM>& msms)
    {
        std::vector<ScalarMul> result;
        for (auto& msm : msms) {
            for (auto& mul : msm) {
                result.push_back(mul);
            }
        }
        return result;
    }

    void add_accumulate(const grumpkin::g1::affine_element& to_add)
    {
        vm_operations.emplace_back(VMOperation{
            .add = true,
            .mul = false,
            .eq = false,
            .reset = false,
            .base_point = to_add,
            .z1 = 0,
            .z2 = 0,
            .mul_scalar_full = 0,
        });
    }

    void mul_accumulate(const grumpkin::g1::affine_element& to_mul, const grumpkin::fr& scalar)
    {
        grumpkin::fr z1 = 0;
        grumpkin::fr z2 = 0;
        auto converted = scalar.from_montgomery_form();
        grumpkin::fr::split_into_endomorphism_scalars(converted, z1, z2);
        z1 = z1.to_montgomery_form();
        z2 = z2.to_montgomery_form();
        vm_operations.emplace_back(VMOperation{
            .add = false,
            .mul = true,
            .eq = false,
            .reset = false,
            .base_point = to_mul,
            .z1 = z1,
            .z2 = z2,
            .mul_scalar_full = scalar,
        });
    }
    void eq(const grumpkin::g1::affine_element& expected)
    {
        vm_operations.emplace_back(VMOperation{
            .add = false,
            .mul = false,
            .eq = true,
            .reset = true,
            .base_point = expected,
            .z1 = 0,
            .z2 = 0,
            .mul_scalar_full = 0,
        });
    }

    void empty_row()
    {
        vm_operations.emplace_back(VMOperation{
            .add = false,
            .mul = false,
            .eq = false,
            .reset = false,
            .base_point = grumpkin::g1::affine_point_at_infinity,
            .z1 = 0,
            .z2 = 0,
            .mul_scalar_full = 0,
        });
    }

    RawPolynomials compute_full_polynomials()
    {
        const auto msms = get_msms();
        const auto flattened_muls = get_flattened_scalar_muls(msms);

        std::array<std::vector<size_t>, 2> point_table_read_counts;
        const auto transcript_state =
            ECCVMTranscriptBuilder::compute_transcript_state(vm_operations, get_number_of_muls());
        const auto precompute_table_state = ECCVMPrecomputedTblesBuilder::compute_precompute_state(flattened_muls);
        const auto msm_state = ECCVMMSMMBuilder::compute_msm_state(msms, point_table_read_counts, get_number_of_muls());

        const size_t msm_size = msm_state.size();
        const size_t transcript_size = transcript_state.size();
        const size_t precompute_table_size = precompute_table_state.size();

        const size_t num_rows = std::max(precompute_table_size, std::max(msm_size, transcript_size));

        const size_t num_rows_log2 = numeric::get_msb64(num_rows);
        size_t num_rows_pow2 = 1UL << (num_rows_log2 + (1UL << num_rows_log2 == num_rows ? 0 : 1));

        RawPolynomials rows;
        for (size_t j = 0; j < NUM_POLYNOMIALS; ++j) {
            rows[j].reserve(num_rows_pow2);
            for (size_t i = 0; i < num_rows_pow2; ++i) {
                rows[j].push_back(0);
            }
        }

        rows.lagrange_first[0] = 1;
        rows.lagrange_last[rows.lagrange_last.size() - 1] = 1;

        for (size_t i = 0; i < point_table_read_counts[0].size(); ++i) {
            rows.lookup_read_counts_0[i] = point_table_read_counts[0][i];
            rows.lookup_read_counts_1[i] = point_table_read_counts[1][i];
        }
        for (size_t i = 0; i < transcript_state.size(); ++i) {
            rows.transcript_accumulator_empty[i] = transcript_state[i].accumulator_empty;
            rows.q_transcript_add[i] = transcript_state[i].q_add;
            rows.q_transcript_mul[i] = transcript_state[i].q_mul;
            rows.q_transcript_eq[i] = transcript_state[i].q_eq;
            rows.transcript_q_reset_accumulator[i] = transcript_state[i].q_reset_accumulator;
            rows.q_transcript_msm_transition[i] = transcript_state[i].q_msm_transition;
            rows.transcript_pc[i] = transcript_state[i].pc;
            rows.transcript_msm_count[i] = transcript_state[i].msm_count;
            rows.transcript_x[i] = transcript_state[i].base_x;
            rows.transcript_y[i] = transcript_state[i].base_y;
            rows.transcript_z1[i] = transcript_state[i].z1;
            rows.transcript_z2[i] = transcript_state[i].z2;
            rows.transcript_z1zero[i] = transcript_state[i].z1_zero;
            rows.transcript_z2zero[i] = transcript_state[i].z2_zero;
            rows.transcript_op[i] = transcript_state[i].opcode;
            rows.transcript_accumulator_x[i] = transcript_state[i].accumulator_x;
            rows.transcript_accumulator_y[i] = transcript_state[i].accumulator_y;
            rows.transcript_msm_x[i] = transcript_state[i].msm_output_x;
            rows.transcript_msm_y[i] = transcript_state[i].msm_output_y;
        }

        for (size_t i = 0; i < precompute_table_state.size(); ++i) {
            rows.q_wnaf[i] = 1; // todo document, derive etc etc
            rows.table_pc[i] = precompute_table_state[i].pc;
            rows.table_point_transition[i] = static_cast<uint64_t>(precompute_table_state[i].point_transition);
            // rows.table_point_transition_shift = static_cast<uint64_t>(table_state[i].point_transition);
            rows.table_round[i] = precompute_table_state[i].round;
            rows.table_scalar_sum[i] = precompute_table_state[i].scalar_sum;

            rows.table_s1[i] = precompute_table_state[i].s1;
            rows.table_s2[i] = precompute_table_state[i].s2;
            rows.table_s3[i] = precompute_table_state[i].s3;
            rows.table_s4[i] = precompute_table_state[i].s4;
            rows.table_s5[i] = precompute_table_state[i].s5;
            rows.table_s6[i] = precompute_table_state[i].s6;
            rows.table_s7[i] = precompute_table_state[i].s7;
            rows.table_s8[i] = precompute_table_state[i].s8;
            rows.table_skew[i] = precompute_table_state[i].skew ? 7 : 0; // static_cast<uint64_t>(table_state[i].skew);

            rows.table_dx[i] = precompute_table_state[i].precompute_double.x;
            rows.table_dy[i] = precompute_table_state[i].precompute_double.y;
            rows.table_tx[i] = precompute_table_state[i].precompute_accumulator.x;
            rows.table_ty[i] = precompute_table_state[i].precompute_accumulator.y;
        }

        for (size_t i = 0; i < msm_state.size(); ++i) {
            rows.q_msm_transition[i] = static_cast<int>(msm_state[i].q_msm_transition);
            rows.msm_q_add[i] = static_cast<int>(msm_state[i].q_add);
            rows.msm_q_double[i] = static_cast<int>(msm_state[i].q_double);
            rows.msm_q_skew[i] = static_cast<int>(msm_state[i].q_skew);
            rows.msm_accumulator_x[i] = msm_state[i].accumulator_x;
            rows.msm_accumulator_y[i] = msm_state[i].accumulator_y;
            rows.msm_pc[i] = msm_state[i].pc;
            rows.msm_size_of_msm[i] = msm_state[i].msm_size;
            rows.msm_count[i] = msm_state[i].msm_count;
            rows.msm_round[i] = msm_state[i].msm_round;
            rows.msm_q_add1[i] = static_cast<int>(msm_state[i].add_state[0].add);
            rows.msm_q_add2[i] = static_cast<int>(msm_state[i].add_state[1].add);
            rows.msm_q_add3[i] = static_cast<int>(msm_state[i].add_state[2].add);
            rows.msm_q_add4[i] = static_cast<int>(msm_state[i].add_state[3].add);
            rows.msm_x1[i] = msm_state[i].add_state[0].point.x;
            rows.msm_y1[i] = msm_state[i].add_state[0].point.y;
            rows.msm_x2[i] = msm_state[i].add_state[1].point.x;
            rows.msm_y2[i] = msm_state[i].add_state[1].point.y;
            rows.msm_x3[i] = msm_state[i].add_state[2].point.x;
            rows.msm_y3[i] = msm_state[i].add_state[2].point.y;
            rows.msm_x4[i] = msm_state[i].add_state[3].point.x;
            rows.msm_y4[i] = msm_state[i].add_state[3].point.y;
            rows.msm_collision_x1[i] = msm_state[i].add_state[0].collision_inverse;
            rows.msm_collision_x2[i] = msm_state[i].add_state[1].collision_inverse;
            rows.msm_collision_x3[i] = msm_state[i].add_state[2].collision_inverse;
            rows.msm_collision_x4[i] = msm_state[i].add_state[3].collision_inverse;
            rows.msm_lambda1[i] = msm_state[i].add_state[0].lambda;
            rows.msm_lambda2[i] = msm_state[i].add_state[1].lambda;
            rows.msm_lambda3[i] = msm_state[i].add_state[2].lambda;
            rows.msm_lambda4[i] = msm_state[i].add_state[3].lambda;
            rows.msm_slice1[i] = msm_state[i].add_state[0].slice;
            rows.msm_slice2[i] = msm_state[i].add_state[1].slice;
            rows.msm_slice3[i] = msm_state[i].add_state[2].slice;
            rows.msm_slice4[i] = msm_state[i].add_state[3].slice;
        }

        for (size_t i = 0; i < num_rows_pow2 - 1; ++i) {
            // shifts. todo handle using flavor methods
            rows.q_transcript_mul_shift[i] = rows.q_transcript_mul[i + 1];
            rows.q_transcript_accumulate_shift[i] = rows.q_transcript_accumulate[i + 1];
            rows.transcript_msm_count_shift[i] = rows.transcript_msm_count[i + 1];
            rows.transcript_accumulator_x_shift[i] = rows.transcript_accumulator_x[i + 1];
            rows.transcript_accumulator_y_shift[i] = rows.transcript_accumulator_y[i + 1];
            rows.table_scalar_sum_shift[i] = rows.table_scalar_sum[i + 1];
            rows.table_dx_shift[i] = rows.table_dx[i + 1];
            rows.table_dy_shift[i] = rows.table_dy[i + 1];
            rows.table_tx_shift[i] = rows.table_tx[i + 1];
            rows.table_ty_shift[i] = rows.table_ty[i + 1];
            rows.q_msm_transition_shift[i] = rows.q_msm_transition[i + 1];
            rows.msm_q_add_shift[i] = rows.msm_q_add[i + 1];
            rows.msm_q_double_shift[i] = rows.msm_q_double[i + 1];
            rows.msm_q_skew_shift[i] = rows.msm_q_skew[i + 1];
            rows.msm_accumulator_x_shift[i] = rows.msm_accumulator_x[i + 1];
            rows.msm_accumulator_y_shift[i] = rows.msm_accumulator_y[i + 1];
            rows.msm_size_of_msm_shift[i] = rows.msm_size_of_msm[i + 1];
            rows.msm_count_shift[i] = rows.msm_count[i + 1];
            rows.msm_round_shift[i] = rows.msm_round[i + 1];
            rows.msm_q_add1_shift[i] = rows.msm_q_add1[i + 1];
            rows.msm_pc_shift[i] = rows.msm_pc[i + 1];
            rows.table_pc_shift[i] = rows.table_pc[i + 1];
            rows.transcript_pc_shift[i] = rows.transcript_pc[i + 1];
            rows.table_round_shift[i] = rows.table_round[i + 1];
            rows.transcript_accumulator_empty_shift[i] = rows.transcript_accumulator_empty[i + 1];
            rows.q_wnaf_shift[i] = rows.q_wnaf[i + 1];
        }
        return rows;
    }
};
} // namespace proof_system::honk::sumcheck