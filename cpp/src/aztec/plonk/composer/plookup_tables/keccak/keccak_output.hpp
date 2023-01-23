#pragma once

#include <numeric/bitop/pow.hpp>
#include <numeric/bitop/sparse_form.hpp>
#include <common/constexpr_utils.hpp>

#include "../types.hpp"
#include "../sparse.hpp"

namespace plookup {
namespace keccak_tables {

class KeccakOutput {

  public:
    static constexpr uint64_t BASE = 11;
    static constexpr uint64_t EFFECTIVE_BASE = 2;
    static constexpr size_t TABLE_BITS = 8;

    static constexpr uint64_t OUTPUT_NORMALIZATION_TABLE[11]{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    template <size_t i> std::pair<uint64_t, uint64_t> static update_counts(std::array<size_t, TABLE_BITS>& counts)
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
                value += (counts[j] * cumulative_base);
                normalized_value += ((OUTPUT_NORMALIZATION_TABLE[counts[j]]) << j);
                cumulative_base *= BASE;
            }

            return std::make_pair(value, normalized_value);
        }
    }

    static BasicTable generate_keccak_output_table(BasicTableId id, const size_t table_index)
    {
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
        std::pair<uint64_t, uint64_t> key_value;

        for (size_t i = 0; i < table.size; ++i) {
            table.column_1.emplace_back(key);
            table.column_2.emplace_back(value);
            table.column_3.emplace_back(barretenberg::fr(0));
            // todo fix ugly name
            key_value = update_counts<0>(counts);
            key = key_value.first;
            value = key_value.second;
        }

        table.get_values_from_key = &sparse_tables::get_sparse_normalization_values<BASE, OUTPUT_NORMALIZATION_TABLE>;

        table.column_1_step_size = barretenberg::fr(numeric::pow64(static_cast<size_t>(BASE), TABLE_BITS));
        table.column_2_step_size = barretenberg::fr(((uint64_t)1 << TABLE_BITS));
        table.column_3_step_size = barretenberg::fr(0);
        return table;
    }

    static MultiTable get_keccak_output_table(const MultiTableId id = KECCAK_FORMAT_OUTPUT)
    {
        constexpr size_t num_tables_per_multitable =
            64 / TABLE_BITS + (64 % TABLE_BITS == 0 ? 0 : 1); // 64 bits, 8 bits per entry

        uint64_t column_multiplier = numeric::pow64(BASE, TABLE_BITS);
        MultiTable table(column_multiplier, (1ULL << TABLE_BITS), 0, num_tables_per_multitable);

        table.id = id;
        for (size_t i = 0; i < num_tables_per_multitable; ++i) {
            table.slice_sizes.emplace_back(numeric::pow64(BASE, TABLE_BITS));
            table.lookup_ids.emplace_back(KECCAK_OUTPUT);
            table.get_table_values.emplace_back(
                &sparse_tables::get_sparse_normalization_values<BASE, OUTPUT_NORMALIZATION_TABLE>);
        }
        return table;
    }
};

} // namespace keccak_tables
} // namespace plookup