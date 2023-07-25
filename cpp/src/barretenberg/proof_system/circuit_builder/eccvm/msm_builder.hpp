#pragma once

#include <cstddef>

#include "./eccvm_builder_types.hpp"

namespace proof_system {

template <typename Flavor> class ECCVMMSMMBuilder {
  public:
    using CycleGroup = typename Flavor::CycleGroup;
    using FF = typename Flavor::FF;
    using Element = typename CycleGroup::element;
    using AffineElement = typename CycleGroup::affine_element;

    static constexpr size_t ADDITIONS_PER_ROW = proof_system_eccvm::ADDITIONS_PER_ROW;
    static constexpr size_t NUM_SCALAR_BITS = proof_system_eccvm::NUM_SCALAR_BITS;
    static constexpr size_t WNAF_SLICE_BITS = proof_system_eccvm::WNAF_SLICE_BITS;

    struct MSMState {
        uint32_t pc = 0;
        uint32_t msm_size = 0;
        uint32_t msm_count = 0;
        uint32_t msm_round = 0;
        bool q_msm_transition = false;
        bool q_add = false;
        bool q_double = false;
        bool q_skew = false;

        struct AddState {
            bool add = false;
            int slice = 0;
            AffineElement point{ 0, 0 };
            FF lambda = 0;
            FF collision_inverse = 0;
        };
        std::array<AddState, 4> add_state{ AddState{ false, 0, { 0, 0 }, 0, 0 },
                                           AddState{ false, 0, { 0, 0 }, 0, 0 },
                                           AddState{ false, 0, { 0, 0 }, 0, 0 },
                                           AddState{ false, 0, { 0, 0 }, 0, 0 } };
        FF accumulator_x = 0;
        FF accumulator_y = 0;
    };

    static std::vector<MSMState> compute_msm_state(const std::vector<proof_system_eccvm::MSM<CycleGroup>>& msms,
                                                   std::array<std::vector<size_t>, 2>& point_table_read_counts,
                                                   const uint32_t total_number_of_muls)
    {
        // when we define our point lookup table, we have 2 write columns and 4 read columns
        // when we perform a read on a given row, we need to increment the read count on the respective write column by
        // 1 we can define the following struture: 1st write column = positive 2nd write column = negative the row
        // number is a function of pc and slice value row = pc_delta * rows_per_point_table + some function of the slice
        // value pc_delta = total_number_of_muls - pc std::vector<std::array<size_t, > point_table_read_counts;
        const size_t table_rows = static_cast<const size_t>(total_number_of_muls) * 8;
        point_table_read_counts[0].reserve(table_rows);
        point_table_read_counts[1].reserve(table_rows);
        for (size_t i = 0; i < table_rows; ++i) {
            point_table_read_counts[0].emplace_back(0);
            point_table_read_counts[1].emplace_back(0);
        }
        const auto update_read_counts = [&](const size_t pc, const int slice) {
            // When we compute our wnaf/point tables, we start with the point with the largest pc value.
            // i.e. if we are reading a slice for point with a point counter value `pc`,
            // its position in the wnaf/point table (relative to other points) will be `total_number_of_muls - pc`
            const size_t pc_delta = total_number_of_muls - pc;
            const size_t pc_offset = pc_delta * 8;
            bool slice_negative = slice < 0;
            const int slice_row = (slice + 15) / 2;

            const size_t column_index = slice_negative ? 1 : 0;

            if (slice_negative) {
                point_table_read_counts[column_index][pc_offset + static_cast<size_t>(slice_row)]++;
            } else {
                // 8 maps to 7
                // 15 maps to 0

                // 15 - x
                point_table_read_counts[column_index][pc_offset + 15 - static_cast<size_t>(slice_row)]++;
            }
            // slice : row
            // -15 : 0
            // -13 : 1
            // -11 : 2
            // -9 : 3
            // -7 : 4
            // -5 : 5
            // -3 : 6
            // -1 : 7
            // 1 : 8
            // 3 : 9
            // 5 : 10
            // 7 : 11
            // 9 : 12
            // 11 : 13
            // 13 : 14
            // 15 : 15
        };
        std::vector<MSMState> msm_state;
        // start with empty row (shiftable polynomials must have 0 as first coefficient)
        msm_state.emplace_back(MSMState{});
        uint32_t pc = total_number_of_muls;
        AffineElement accumulator = CycleGroup::affine_point_at_infinity;

        for (const auto& msm : msms) {
            const size_t msm_size = msm.size();

            const size_t rows_per_round = (msm_size / ADDITIONS_PER_ROW) + (msm_size % ADDITIONS_PER_ROW != 0 ? 1 : 0);
            static constexpr size_t num_rounds = NUM_SCALAR_BITS / WNAF_SLICE_BITS;

            const auto add_points = [](auto& P1, auto& P2, auto& lambda, auto& collision_inverse, bool predicate) {
                lambda = predicate ? (P2.y - P1.y) / (P2.x - P1.x) : 0;
                collision_inverse = predicate ? (P2.x - P1.x).invert() : 0;
                auto x3 = predicate ? lambda * lambda - (P2.x + P1.x) : P1.x;
                auto y3 = predicate ? lambda * (P1.x - x3) - P1.y : P1.y;
                return AffineElement(x3, y3);
            };
            for (size_t j = 0; j < num_rounds; ++j) {
                for (size_t k = 0; k < rows_per_round; ++k) {
                    MSMState row;
                    const size_t points_per_row =
                        (k + 1) * ADDITIONS_PER_ROW > msm_size ? msm_size % ADDITIONS_PER_ROW : ADDITIONS_PER_ROW;
                    const size_t idx = k * ADDITIONS_PER_ROW;
                    row.q_msm_transition = (j == 0) && (k == 0);

                    AffineElement acc(accumulator);
                    Element acc_expected = accumulator;
                    for (size_t m = 0; m < 4; ++m) {
                        auto& add_state = row.add_state[m];
                        add_state.add = points_per_row > m;
                        int slice = add_state.add ? msm[idx + m].wnaf_slices[j] : 0;
                        add_state.slice = add_state.add ? (slice + 15) / 2 : 0;
                        add_state.point = add_state.add
                                              ? msm[idx + m].precomputed_table[static_cast<size_t>(add_state.slice)]
                                              : AffineElement{ 0, 0 };
                        bool add_predicate = (m == 0 ? (j != 0 || k != 0) : add_state.add);

                        auto& p1 = (m == 0) ? add_state.point : acc;
                        auto& p2 = (m == 0) ? acc : add_state.point;

                        acc_expected = add_predicate ? (acc_expected + add_state.point) : Element(p1);
                        if (add_state.add) {
                            update_read_counts(pc - idx - m, slice);
                        }
                        acc = add_points(p1, p2, add_state.lambda, add_state.collision_inverse, add_predicate);
                        ASSERT(acc == AffineElement(acc_expected));
                    }
                    row.q_add = true;
                    row.q_double = false;
                    row.q_skew = false;
                    row.msm_round = static_cast<uint32_t>(j);
                    row.msm_size = static_cast<uint32_t>(msm_size);
                    row.msm_count = static_cast<uint32_t>(idx);
                    row.accumulator_x = accumulator.is_point_at_infinity() ? 0 : accumulator.x;
                    row.accumulator_y = accumulator.is_point_at_infinity() ? 0 : accumulator.y;
                    row.pc = pc;
                    accumulator = acc;
                    msm_state.push_back(row);
                }
                if (j < num_rounds - 1) {
                    MSMState row;
                    row.q_msm_transition = false;
                    row.msm_round = static_cast<uint32_t>(j + 1);
                    row.msm_size = static_cast<uint32_t>(msm_size);
                    row.msm_count = static_cast<uint32_t>(0);
                    row.q_add = false;
                    row.q_double = true;
                    row.q_skew = false;

                    auto dx = accumulator.x;
                    auto dy = accumulator.y;
                    for (size_t m = 0; m < 4; ++m) {
                        auto& add_state = row.add_state[m];
                        add_state.add = false;
                        add_state.slice = 0;
                        add_state.point = { 0, 0 };
                        add_state.collision_inverse = 0;
                        add_state.lambda = ((dx + dx + dx) * dx) / (dy + dy);
                        auto x3 = add_state.lambda.sqr() - dx - dx;
                        dy = add_state.lambda * (dx - x3) - dy;
                        dx = x3;
                    }

                    row.accumulator_x = accumulator.is_point_at_infinity() ? 0 : accumulator.x;
                    row.accumulator_y = accumulator.is_point_at_infinity() ? 0 : accumulator.y;
                    accumulator = Element(accumulator).dbl().dbl().dbl().dbl();
                    row.pc = pc;
                    msm_state.push_back(row);
                } else {
                    for (size_t k = 0; k < rows_per_round; ++k) {
                        MSMState row;

                        const size_t points_per_row =
                            (k + 1) * ADDITIONS_PER_ROW > msm_size ? msm_size % ADDITIONS_PER_ROW : ADDITIONS_PER_ROW;
                        const size_t idx = k * ADDITIONS_PER_ROW;
                        row.q_msm_transition = false;

                        AffineElement acc(accumulator);
                        Element acc_expected = accumulator;

                        for (size_t m = 0; m < 4; ++m) {
                            auto& add_state = row.add_state[m];
                            add_state.add = points_per_row > m;
                            add_state.slice = add_state.add ? msm[idx + m].wnaf_skew ? 7 : 0 : 0;

                            add_state.point = add_state.add
                                                  ? msm[idx + m].precomputed_table[static_cast<size_t>(add_state.slice)]
                                                  : AffineElement{ 0, 0 };
                            bool add_predicate = add_state.add ? msm[idx + m].wnaf_skew : false;
                            if (add_state.add) {
                                update_read_counts(pc - idx - m, msm[idx + m].wnaf_skew ? -1 : -15);
                            }
                            acc = add_points(
                                acc, add_state.point, add_state.lambda, add_state.collision_inverse, add_predicate);
                            acc_expected = add_predicate ? (acc_expected + add_state.point) : acc_expected;
                            ASSERT(acc == AffineElement(acc_expected));
                        }
                        row.q_add = false;
                        row.q_double = false;
                        row.q_skew = true;
                        row.msm_round = static_cast<uint32_t>(j + 1);
                        row.msm_size = static_cast<uint32_t>(msm_size);
                        row.msm_count = static_cast<uint32_t>(idx);

                        row.accumulator_x = accumulator.is_point_at_infinity() ? 0 : accumulator.x;
                        row.accumulator_y = accumulator.is_point_at_infinity() ? 0 : accumulator.y;

                        row.pc = pc;
                        accumulator = acc;
                        msm_state.emplace_back(row);
                    }
                }
            }
            pc -= static_cast<uint32_t>(msm_size);
            // Validate our computed accumulator matches the real MSM result!
            Element expected = CycleGroup::point_at_infinity;
            for (size_t i = 0; i < msm.size(); ++i) {
                expected += (Element(msm[i].base_point) * msm[i].scalar);
            }
            // Validate the accumulator is correct!
            ASSERT(accumulator == AffineElement(expected));
        }

        MSMState final_row;
        final_row.pc = pc;
        final_row.q_msm_transition = true;
        final_row.accumulator_x = accumulator.is_point_at_infinity() ? 0 : accumulator.x;
        final_row.accumulator_y = accumulator.is_point_at_infinity() ? 0 : accumulator.y;
        final_row.msm_size = 0;
        final_row.msm_count = 0;
        final_row.q_add = false;
        final_row.q_double = false;
        final_row.q_skew = false;
        final_row.add_state = { typename MSMState::AddState{ false, 0, AffineElement{ 0, 0 }, 0, 0 },
                                typename MSMState::AddState{ false, 0, AffineElement{ 0, 0 }, 0, 0 },
                                typename MSMState::AddState{ false, 0, AffineElement{ 0, 0 }, 0, 0 },
                                typename MSMState::AddState{ false, 0, AffineElement{ 0, 0 }, 0, 0 } };

        msm_state.emplace_back(final_row);
        return msm_state;
    }
};
} // namespace proof_system