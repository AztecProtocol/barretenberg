#pragma once

#include <numeric/bitop/pow.hpp>
#include <numeric/bitop/sparse_form.hpp>
#include <common/constexpr_utils.hpp>
#include "../types.hpp"

namespace plookup {
namespace keccak_tables {

class KeccakInput {

  public:
    static constexpr uint64_t BASE = 11;
    static constexpr size_t TABLE_BITS = 8;

    static std::array<barretenberg::fr, 2> get_keccak_input_values(const std::array<uint64_t, 2> key)
    {
        const uint256_t t0 = numeric::map_into_sparse_form<BASE>(key[0]);

        constexpr size_t msb_shift = (64 % TABLE_BITS == 0) ? TABLE_BITS - 1 : (64 % TABLE_BITS) - 1;
        const uint256_t t1 = key[0] >> msb_shift;
        return { barretenberg::fr(t0), barretenberg::fr(t1) };
    }

    static BasicTable generate_keccak_input_table(BasicTableId id, const size_t table_index)
    {
        BasicTable table;
        table.id = id;
        table.table_index = table_index;
        table.size = (1U << TABLE_BITS);
        table.use_twin_keys = false;
        constexpr size_t msb_shift = (64 % TABLE_BITS == 0) ? TABLE_BITS - 1 : (64 % TABLE_BITS) - 1;

        for (uint64_t i = 0; i < table.size; ++i) {
            const uint64_t source = i;
            const auto target = numeric::map_into_sparse_form<BASE>(source);
            table.column_1.emplace_back(barretenberg::fr(source));
            table.column_2.emplace_back(barretenberg::fr(target));
            table.column_3.emplace_back(barretenberg::fr(source >> msb_shift));
        }

        table.get_values_from_key = &get_keccak_input_values;

        uint256_t sparse_step_size = 1;
        for (size_t i = 0; i < TABLE_BITS; ++i) {
            sparse_step_size *= BASE;
        }
        table.column_1_step_size = barretenberg::fr((1 << TABLE_BITS));
        table.column_2_step_size = barretenberg::fr(sparse_step_size);
        table.column_3_step_size = barretenberg::fr(sparse_step_size);

        return table;
    }

    static MultiTable get_keccak_input_table(const MultiTableId id = KECCAK_FORMAT_INPUT)
    {
        const size_t num_entries = 8;

        MultiTable table(1 << 8, uint256_t(11).pow(8), 0, num_entries);

        table.id = id;
        for (size_t i = 0; i < num_entries; ++i) {
            table.slice_sizes.emplace_back(1 << 8);
            table.lookup_ids.emplace_back(KECCAK_INPUT);
            table.get_table_values.emplace_back(&get_keccak_input_values);
        }
        return table;
    }
};

} // namespace keccak_tables
} // namespace plookup