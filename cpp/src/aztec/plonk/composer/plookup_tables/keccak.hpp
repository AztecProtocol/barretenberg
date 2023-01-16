#pragma once

#include <crypto/aes128/aes128.hpp>
#include <numeric/bitop/rotate.hpp>
#include <numeric/bitop/sparse_form.hpp>
#include <numeric/bitop/pow.hpp>

#include "types.hpp"
#include "sparse.hpp"

namespace plookup {
namespace keccak_tables {

static constexpr uint64_t theta_normalization_table[11]{
    0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
};

static constexpr uint64_t rho_normalization_table[3]{
    0,
    1,
    0,
};

static constexpr uint64_t chi_normalization_table[5]{
    0, // 1 + 2a - b + c => a xor (~b & c)
    0, 1, 1, 0,
};

static constexpr uint64_t output_normalization_table[2]{ 0, 1 };

inline BasicTable generate_theta_renormalization_table(BasicTableId id, const size_t table_index)
{
    return sparse_tables::generate_sparse_renormalization_table<11, 5, theta_normalization_table, 11>(id, table_index);
}

inline BasicTable generate_rho_renormalization_table(BasicTableId id, const size_t table_index)
{
    return sparse_tables::generate_sparse_renormalization_table<11, 11, rho_normalization_table, 3>(id, table_index);
}

inline BasicTable generate_keccak_output_table(BasicTableId id, const size_t table_index)
{
    return sparse_tables::generate_sparse_normalization_table_new<11, 8, output_normalization_table, 2>(id,
                                                                                                        table_index);
}

inline MultiTable get_keccak_input_table(const MultiTableId id = KECCAK_FORMAT_INPUT)
{
    const size_t num_entries = 8;

    MultiTable table(1 << 8, uint256_t(11).pow(8), 0, num_entries);

    table.id = id;
    for (size_t i = 0; i < num_entries; ++i) {
        table.slice_sizes.emplace_back(1 << 8);
        table.lookup_ids.emplace_back(KECCAK_SPARSE_MAP);
        table.get_table_values.emplace_back(&sparse_tables::get_sparse_table_with_rotation_values<11, 0>);
    }
    return table;
}

inline MultiTable get_theta_output_table(const MultiTableId id = KECCAK_THETA_OUTPUT)
{
    constexpr size_t base = 11;
    constexpr size_t num_bits_per_table = 5;
    constexpr size_t num_tables_per_multitable = 13; // 64 bits, 5 bits per entry

    uint64_t column_multiplier = numeric::pow64(base, num_bits_per_table);
    MultiTable table(column_multiplier, column_multiplier, 0, num_tables_per_multitable);

    table.id = id;
    for (size_t i = 0; i < num_tables_per_multitable; ++i) {
        table.slice_sizes.emplace_back(numeric::pow64(base, num_bits_per_table));
        table.lookup_ids.emplace_back(KECCAK_THETA);
        table.get_table_values.emplace_back(
            &sparse_tables::get_sparse_renormalization_values<base, theta_normalization_table>);
    }
    return table;
}

inline MultiTable get_rho_output_table(const MultiTableId id = KECCAK_RHO_OUTPUT)
{
    constexpr size_t base = 11;
    constexpr size_t num_bits_per_table = 11;       // values only range from 0-2 per 'bit', not 0-11
    constexpr size_t num_tables_per_multitable = 6; // 64 bits, 11 bits per entry

    uint64_t column_multiplier = numeric::pow64(base, num_bits_per_table);
    MultiTable table(column_multiplier, column_multiplier, 0, num_tables_per_multitable);

    table.id = id;
    for (size_t i = 0; i < num_tables_per_multitable; ++i) {
        table.slice_sizes.emplace_back(numeric::pow64(base, num_bits_per_table));
        table.lookup_ids.emplace_back(KECCAK_RHO);
        table.get_table_values.emplace_back(
            &sparse_tables::get_sparse_renormalization_values<base, rho_normalization_table>);
    }
    return table;
}

template <size_t base, const uint64_t* base_table>
inline std::array<barretenberg::fr, 2> get_chi_renormalization_values(const std::array<uint64_t, 2> key)
{
    uint64_t accumulator = 0;
    uint64_t input = key[0];
    uint64_t base_shift = 1;
    while (input > 0) {
        uint64_t slice = input % base;
        uint64_t bit = base_table[static_cast<size_t>(slice)];
        accumulator += (bit * base_shift);
        input -= slice;
        input /= base;
        base_shift *= base;
    }
    return { barretenberg::fr(accumulator), barretenberg::fr(accumulator % 2) };
}

inline MultiTable get_chi_output_table(const MultiTableId id = KECCAK_CHI_OUTPUT)
{
    constexpr size_t base = 11;
    constexpr size_t num_bits_per_table = 8;        // values only range from 0-4 per 'bit', not 0-11
    constexpr size_t num_tables_per_multitable = 8; // 64 bits, 8 bits per entry

    uint64_t column_multiplier = numeric::pow64(base, num_bits_per_table);
    MultiTable table(column_multiplier, column_multiplier, 0, num_tables_per_multitable);

    table.id = id;
    for (size_t i = 0; i < num_tables_per_multitable; ++i) {
        table.slice_sizes.emplace_back(numeric::pow64(base, num_bits_per_table));
        table.lookup_ids.emplace_back(KECCAK_CHI);
        table.get_table_values.emplace_back(&get_chi_renormalization_values<base, chi_normalization_table>);
    }
    return table;
}

inline MultiTable get_keccak_output_table(const MultiTableId id = KECCAK_FORMAT_OUTPUT)
{
    constexpr size_t base = 11;
    constexpr size_t num_bits_per_table = 8;        // values only range from 0-1 per 'bit', not 0-11
    constexpr size_t num_tables_per_multitable = 8; // 64 bits, 8 bits per entry

    uint64_t column_multiplier = numeric::pow64(base, num_bits_per_table);
    MultiTable table(column_multiplier, column_multiplier, 0, num_tables_per_multitable);

    table.id = id;
    for (size_t i = 0; i < num_tables_per_multitable; ++i) {
        table.slice_sizes.emplace_back(numeric::pow64(base, num_bits_per_table));
        table.lookup_ids.emplace_back(KECCAK_OUTPUT);
        table.get_table_values.emplace_back(
            &sparse_tables::get_sparse_normalization_values<base, output_normalization_table>);
    }
    return table;
}

inline BasicTable generate_chi_renormalization_table(BasicTableId id, const size_t table_index)
{
    constexpr size_t base = 11;
    constexpr size_t num_bits = 8;

    constexpr size_t max_base_value_plus_one = 5;
    // max_base_value_plus_one sometimes may not equal base iff this is an intermediate lookup table
    // (e.g. keccak, we have base11 values that need to be normalized where the actual values-per-base only range from
    // [0, 1, 2])
    BasicTable table;
    table.id = id;
    table.table_index = table_index;
    table.use_twin_keys = false;
    table.size = numeric::pow64(static_cast<uint64_t>(max_base_value_plus_one), num_bits);

    std::array<size_t, num_bits> counts{};

    uint64_t key = 0;
    uint64_t value = 0;
    std::pair<uint64_t, uint64_t> key_value;
    for (size_t i = 0; i < num_bits; ++i) {
        for (size_t j = 0; j < max_base_value_plus_one; ++j) {
            table.column_1.emplace_back(value);
            table.column_2.emplace_back(key);
            table.column_3.emplace_back(value % 2);
            key_value = sparse_tables::update_counts<0, num_bits, base, max_base_value_plus_one>(counts);
            key_value.first = key;
            key_value.second = value;
        }
    }

    table.get_values_from_key = &get_chi_renormalization_values<max_base_value_plus_one, chi_normalization_table>;

    table.column_1_step_size = barretenberg::fr(table.size);
    table.column_2_step_size = barretenberg::fr(table.size);
    table.column_3_step_size = barretenberg::fr(0);
    return table;
}

} // namespace keccak_tables
} // namespace plookup