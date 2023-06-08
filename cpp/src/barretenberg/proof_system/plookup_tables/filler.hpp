#pragma once

#include "types.hpp"

namespace plookup {
namespace filler_tables {

inline std::array<barretenberg::fr, 2> get_and_rotate_values_from_key(const std::array<uint64_t, 2>)
{
    return { 0x1337ULL, 0ULL };
}
inline BasicTable generate_honk_filler_table(const BasicTableId id, const size_t table_index)
{

    const size_t base = 1 << 1; // Probably has to be a power of 2
    BasicTable table;
    table.id = id;
    table.table_index = table_index;
    table.size = base * base;
    table.use_twin_keys = true;
    for (uint64_t i = 0; i < base; ++i) {
        for (uint64_t j = 0; j < base; ++j) {
            table.column_1.emplace_back(i);
            table.column_2.emplace_back(j);
            table.column_3.emplace_back(0x1337ULL + i * 3 + j * 4 + id * 0x1337);
        }
    }

    table.get_values_from_key = &get_and_rotate_values_from_key;

    table.column_1_step_size = base;
    table.column_2_step_size = base;
    table.column_3_step_size = base;

    return table;
}
inline MultiTable get_honk_filler_multitable()
{
    const MultiTableId id = HONK_FILLER_MULTI;
    const size_t number_of_elements_in_argument = 1 << 1; // Probably has to be a power of 2
    const size_t number_of_lookups = 2;
    MultiTable table(number_of_elements_in_argument,
                     number_of_elements_in_argument,
                     number_of_elements_in_argument,
                     number_of_lookups);
    table.id = id;
    table.slice_sizes.emplace_back(number_of_elements_in_argument);
    table.lookup_ids.emplace_back(HONK_FILLER_BASIC1);
    table.get_table_values.emplace_back(&get_and_rotate_values_from_key);
    table.slice_sizes.emplace_back(number_of_elements_in_argument);
    table.lookup_ids.emplace_back(HONK_FILLER_BASIC2);
    table.get_table_values.emplace_back(&get_and_rotate_values_from_key);
    return table;
}
} // namespace filler_tables
} // namespace plookup