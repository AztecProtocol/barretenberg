#pragma once

#include <numeric/bitop/pow.hpp>
#include <common/constexpr_utils.hpp>
#include "../types.hpp"

namespace plookup {
namespace keccak_tables {

class Theta {
  public:
    static constexpr size_t TABLE_BITS = 4;
    static constexpr uint64_t BASE = 11;

    static constexpr uint64_t THETA_NORMALIZATION_TABLE[11]{
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
    };

    template <size_t i> static std::pair<uint64_t, uint64_t> update_counts(std::array<size_t, TABLE_BITS>& counts)
    {
        ASSERT(i <= TABLE_BITS);
        if constexpr (i >= TABLE_BITS) {
            // TODO use concepts or template metaprogramming to put this condition in method declaration
            return std::make_pair(0, 0);
        } else {
            if (counts[i] == BASE - 1) {
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
                normalized_value += (THETA_NORMALIZATION_TABLE[counts[j]]) * cumulative_base;
                cumulative_base *= BASE;
            }
            return std::make_pair(value, normalized_value);
        }
    }

    static std::array<barretenberg::fr, 2> get_theta_renormalization_values(const std::array<uint64_t, 2> key)
    {
        uint64_t accumulator = 0;
        uint64_t input = key[0];
        uint64_t base_shift = 1;
        while (input > 0) {
            uint64_t slice = input % BASE;
            uint64_t bit = THETA_NORMALIZATION_TABLE[static_cast<size_t>(slice)];
            accumulator += (bit * base_shift);
            input -= slice;
            input /= BASE;
            base_shift *= BASE;
        }
        return { barretenberg::fr(accumulator), barretenberg::fr(0) };
    }

    static BasicTable generate_theta_renormalization_table(BasicTableId id, const size_t table_index)
    {
        static constexpr uint64_t base = 11;
        static constexpr uint64_t max_base_value_plus_one = 11;
        static constexpr size_t num_bits = 4;
        // max_base_value_plus_one sometimes may not equal base iff this is an intermediate lookup table
        // (e.g. keccak, we have base11 values that need to be normalized where the actual values-per-base only range
        // from [0, 1, 2])
        BasicTable table;
        table.id = id;
        table.table_index = table_index;
        table.use_twin_keys = false;
        table.size = numeric::pow64(static_cast<uint64_t>(max_base_value_plus_one), num_bits);

        std::array<size_t, num_bits> counts{};

        uint64_t key = 0;
        uint64_t value = 0;
        std::pair<uint64_t, uint64_t> key_value;

        for (size_t i = 0; i < table.size; ++i) {
            table.column_1.emplace_back(key);
            table.column_2.emplace_back(value);
            table.column_3.emplace_back(barretenberg::fr(0));
            key_value = update_counts<0>(counts);
            key = key_value.first;
            value = key_value.second;
        }

        table.get_values_from_key = &get_theta_renormalization_values;

        uint64_t step_size = numeric::pow64(static_cast<uint64_t>(base), num_bits);
        table.column_1_step_size = barretenberg::fr(step_size);
        table.column_2_step_size = barretenberg::fr(step_size);
        table.column_3_step_size = barretenberg::fr(0);
        return table;
    }

    static MultiTable get_theta_output_table(const MultiTableId id = KECCAK_THETA_OUTPUT)
    {
        constexpr size_t base = 11;
        constexpr size_t num_bits_per_table = 4;
        constexpr size_t num_tables_per_multitable =
            (64 / num_bits_per_table) + (64 % num_bits_per_table == 0 ? 0 : 1); // 64 bits, 5 bits per entry

        uint64_t column_multiplier = numeric::pow64(base, num_bits_per_table);
        MultiTable table(column_multiplier, column_multiplier, 0, num_tables_per_multitable);

        table.id = id;
        for (size_t i = 0; i < num_tables_per_multitable; ++i) {
            table.slice_sizes.emplace_back(numeric::pow64(base, num_bits_per_table));
            table.lookup_ids.emplace_back(KECCAK_THETA);
            table.get_table_values.emplace_back(&get_theta_renormalization_values);
        }
        return table;
    }
};
} // namespace keccak_tables
} // namespace plookup