#pragma once

#include <numeric/bitop/pow.hpp>
#include <common/constexpr_utils.hpp>
#include "../types.hpp"

namespace plookup {
namespace keccak_tables {

template <size_t TABLE_BITS = 0, size_t LANE_INDEX = 0> class Rho {

  public:
    static constexpr uint64_t BASE = 11;
    static constexpr uint64_t EFFECTIVE_BASE = 3;
    static constexpr size_t MAXIMUM_MULTITABLE_BITS = 8;

    // Rotation offsets, y vertically, x horizontally: r[y * 5 + x]
    static constexpr std::array<size_t, 25> ROTATIONS = {
        0, 1, 62, 28, 27, 36, 44, 6, 55, 20, 3, 10, 43, 25, 39, 41, 45, 15, 21, 8, 18, 2, 61, 56, 14,
    };

    static constexpr uint64_t RHO_NORMALIZATION_TABLE[3]{
        0,
        1,
        0,
    };

    static std::array<barretenberg::fr, 2> get_rho_renormalization_values(const std::array<uint64_t, 2> key)
    {
        uint64_t accumulator = 0;
        uint64_t input = key[0];
        uint64_t base_shift = 1;
        constexpr uint64_t divisor_exponent = TABLE_BITS - 1;
        constexpr uint64_t divisor = numeric::pow64(BASE, divisor_exponent);
        while (input > 0) {
            uint64_t slice = input % BASE;
            uint64_t bit = RHO_NORMALIZATION_TABLE[static_cast<size_t>(slice)];
            accumulator += (bit * base_shift);
            input -= slice;
            input /= BASE;
            base_shift *= BASE;
        }

        return { barretenberg::fr(accumulator), barretenberg::fr(accumulator / divisor) };
    }

    template <size_t i> static std::pair<uint64_t, uint64_t> update_counts(std::array<size_t, TABLE_BITS>& counts)
    {
        ASSERT(i <= TABLE_BITS);
        if constexpr (i >= TABLE_BITS) {
            // TODO use concepts or template metaprogramming to put this condition in method declaration
            return std::make_pair(0, 0);
        } else {
            if (counts[i] == EFFECTIVE_BASE - 1) {
                counts[i] = 0;
                return update_counts<i + 1>(counts);
            } else {
                counts[i] += 1;
            }

            uint64_t value = 0;
            uint64_t normalized_value = 0;
            uint64_t cumulative_base = 1;
            for (size_t j = 0; j < TABLE_BITS; ++j) {
                value += counts[j] * cumulative_base;
                normalized_value += (RHO_NORMALIZATION_TABLE[counts[j]]) * cumulative_base;
                cumulative_base *= BASE;
            }
            return std::make_pair(value, normalized_value);
        }
    }

    static BasicTable generate_rho_renormalization_table(BasicTableId id, const size_t table_index)
    {

        // constexpr size_t EFFECTIVE_BASE = 5;
        // EFFECTIVE_BASE sometimes may not equal base iff this is an intermediate lookup table
        // (e.g. keccak, we have base11 values that need to be normalized where the actual values-per-base only range
        // from [0, 1, 2])
        BasicTable table;
        table.id = id;
        table.table_index = table_index;
        table.use_twin_keys = false;
        table.size = numeric::pow64(static_cast<uint64_t>(EFFECTIVE_BASE), TABLE_BITS);

        std::array<size_t, TABLE_BITS> counts{};

        uint64_t key = 0;
        uint64_t value = 0;

        // want the most significant bit of the 64-bit integer, when this table is used on the most significant slice
        constexpr uint64_t divisor_exponent = TABLE_BITS - 1;
        constexpr uint64_t divisor = numeric::pow64(static_cast<uint64_t>(BASE), divisor_exponent);
        std::pair<uint64_t, uint64_t> key_value;

        for (size_t i = 0; i < table.size; ++i) {
            table.column_1.emplace_back(key);
            table.column_2.emplace_back(value);
            // column_3 = most significant bit of key. Used to rotate left by 1 bit

            table.column_3.emplace_back(value / divisor);
            key_value = update_counts<0>(counts);
            key = key_value.first;
            value = key_value.second;
        }

        table.get_values_from_key = &get_rho_renormalization_values;

        uint64_t step_size = numeric::pow64(static_cast<uint64_t>(BASE), TABLE_BITS);
        table.column_1_step_size = barretenberg::fr(step_size);
        table.column_2_step_size = barretenberg::fr(step_size);
        table.column_3_step_size = barretenberg::fr(0);
        return table;
    }

    static MultiTable get_rho_output_table(const MultiTableId id = KECCAK_NORMALIZE_AND_ROTATE)
    {
        constexpr size_t left_bits = ROTATIONS[LANE_INDEX];
        constexpr size_t right_bits = 64 - ROTATIONS[LANE_INDEX];
        constexpr size_t num_left_tables =
            left_bits / MAXIMUM_MULTITABLE_BITS + (left_bits % MAXIMUM_MULTITABLE_BITS > 0 ? 1 : 0);
        constexpr size_t num_right_tables =
            right_bits / MAXIMUM_MULTITABLE_BITS + (right_bits % MAXIMUM_MULTITABLE_BITS > 0 ? 1 : 0);

        MultiTable table;
        table.id = id;

        // size_t num_bits_processed = 0;
        table.column_1_step_sizes.push_back(1);
        table.column_2_step_sizes.push_back(1);
        table.column_3_step_sizes.push_back(1);
        barretenberg::constexpr_for<0, num_right_tables, 1>([&]<size_t i> {
            constexpr size_t num_bits_processed = (i * MAXIMUM_MULTITABLE_BITS);
            constexpr size_t bit_slice = (num_bits_processed + MAXIMUM_MULTITABLE_BITS > right_bits)
                                             ? right_bits % MAXIMUM_MULTITABLE_BITS
                                             : MAXIMUM_MULTITABLE_BITS;

            // if (i == num_right_tables - 1) {
            //     if (num_left_tables == 0) {
            //         table.column_1_step_sizes.push_back(uint256_t(11).pow(bit_slice));
            //         table.column_2_step_sizes.push_back(uint256_t(11).pow(bit_slice));
            //         table.column_3_step_sizes.push_back(0);
            //     } else {
            //         table.column_1_step_sizes.push_back(uint256_t(11).pow(bit_slice));
            //         table.column_2_step_sizes.push_back(0);
            //         table.column_3_step_sizes.push_back(0);
            //     }

            // } else {
            if (i == num_right_tables - 1) {
                table.column_1_step_sizes.push_back(uint256_t(11).pow(bit_slice));
                table.column_2_step_sizes.push_back(0);
                table.column_3_step_sizes.push_back(0);
            } else {
                table.column_1_step_sizes.push_back(uint256_t(11).pow(bit_slice));
                table.column_2_step_sizes.push_back(uint256_t(11).pow(bit_slice));
                table.column_3_step_sizes.push_back(0);
            }

            table.slice_sizes.push_back(numeric::pow64(11, bit_slice));
            table.get_table_values.emplace_back(&get_rho_renormalization_values);
            table.lookup_ids.push_back((BasicTableId)((size_t)KECCAK_RHO_1 + (bit_slice - 1)));
        });
        barretenberg::constexpr_for<0, num_left_tables, 1>([&]<size_t i> {
            constexpr size_t num_bits_processed = (i * MAXIMUM_MULTITABLE_BITS);

            constexpr size_t bit_slice = (num_bits_processed + MAXIMUM_MULTITABLE_BITS > left_bits)
                                             ? left_bits % MAXIMUM_MULTITABLE_BITS
                                             : MAXIMUM_MULTITABLE_BITS;

            if (i != num_left_tables - 1) {
                table.column_1_step_sizes.push_back(uint256_t(BASE).pow(bit_slice));
                table.column_2_step_sizes.push_back(uint256_t(BASE).pow(bit_slice));
                table.column_3_step_sizes.push_back(0);
            }

            table.slice_sizes.push_back(numeric::pow64(BASE, bit_slice));
            table.get_table_values.emplace_back(&get_rho_renormalization_values);
            table.lookup_ids.push_back((BasicTableId)((size_t)KECCAK_RHO_1 + (bit_slice - 1)));
        });

        return table;
    }
};

} // namespace keccak_tables
} // namespace plookup