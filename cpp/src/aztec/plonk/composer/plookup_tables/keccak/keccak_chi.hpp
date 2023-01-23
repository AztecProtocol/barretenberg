#pragma once

#include <numeric/bitop/pow.hpp>
#include <common/constexpr_utils.hpp>
#include "../types.hpp"

namespace plookup {
namespace keccak_tables {

class Chi {
  public:
    static constexpr uint64_t CHI_NORMALIZATION_TABLE[5]{
        0, // 1 + 2a - b + c => a xor (~b & c)
        0, 1, 1, 0,
    };

    static constexpr uint64_t BASE = 11;
    static constexpr uint64_t EFFECTIVE_BASE = 5;
    static constexpr uint64_t TABLE_BITS = 6;

    static std::array<barretenberg::fr, 2> get_chi_renormalization_values(const std::array<uint64_t, 2> key)
    {
        uint64_t accumulator = 0;
        uint64_t input = key[0];
        uint64_t base_shift = 1;
        constexpr uint64_t divisor_exponent = (64 % TABLE_BITS == 0) ? (TABLE_BITS - 1) : (64 % TABLE_BITS) - 1;
        constexpr uint64_t divisor = numeric::pow64(BASE, divisor_exponent);
        while (input > 0) {
            uint64_t slice = input % BASE;
            uint64_t bit = CHI_NORMALIZATION_TABLE[static_cast<size_t>(slice)];
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
                normalized_value += (CHI_NORMALIZATION_TABLE[counts[j]]) * cumulative_base;
                cumulative_base *= BASE;
            }
            return std::make_pair(value, normalized_value);
        }
    }

    static BasicTable generate_chi_renormalization_table(BasicTableId id, const size_t table_index)
    {
        // constexpr size_t num_bits = 8;

        // constexpr size_t max_base_value_plus_one = 5;
        // max_base_value_plus_one sometimes may not equal base iff this is an intermediate lookup table
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
        constexpr uint64_t divisor_exponent = (64 % TABLE_BITS == 0) ? (TABLE_BITS - 1) : (64 % TABLE_BITS) - 1;
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

        table.get_values_from_key = &get_chi_renormalization_values;

        uint64_t step_size = numeric::pow64(static_cast<uint64_t>(BASE), TABLE_BITS);
        table.column_1_step_size = barretenberg::fr(step_size);
        table.column_2_step_size = barretenberg::fr(step_size);
        table.column_3_step_size = barretenberg::fr(0);
        return table;
    }

    static MultiTable get_chi_output_table(const MultiTableId id = KECCAK_CHI_OUTPUT)
    {
        constexpr size_t num_tables_per_multitable =
            (64 / TABLE_BITS) + (64 % TABLE_BITS == 0 ? 0 : 1); // 64 bits, 8 bits per entry

        uint64_t column_multiplier = numeric::pow64(BASE, TABLE_BITS);
        MultiTable table(column_multiplier, column_multiplier, 0, num_tables_per_multitable);

        table.id = id;
        for (size_t i = 0; i < num_tables_per_multitable; ++i) {
            table.slice_sizes.emplace_back(numeric::pow64(BASE, TABLE_BITS));
            table.lookup_ids.emplace_back(KECCAK_CHI);
            table.get_table_values.emplace_back(&get_chi_renormalization_values);
        }
        return table;
    }
};
} // namespace keccak_tables
} // namespace plookup