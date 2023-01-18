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

static constexpr uint64_t output_normalization_table[11]{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

inline BasicTable generate_theta_renormalization_table(BasicTableId id, const size_t table_index)
{
    return sparse_tables::generate_sparse_renormalization_table<11, 5, theta_normalization_table, 11>(id, table_index);
}

inline BasicTable generate_keccak_output_table(BasicTableId id, const size_t table_index)
{
    return sparse_tables::generate_sparse_normalization_table_keccak<11, 8, output_normalization_table, 2>(id,
                                                                                                           table_index);
}

template <uint64_t base, size_t bits_per_slice>
inline std::array<barretenberg::fr, 2> get_keccak_input_values(const std::array<uint64_t, 2> key)
{
    const uint256_t t0 = numeric::map_into_sparse_form<base>(key[0]);

    constexpr size_t msb_shift = (64 % bits_per_slice == 0) ? bits_per_slice - 1 : (64 % bits_per_slice) - 1;
    const uint256_t t1 = key[0] >> msb_shift;
    return { barretenberg::fr(t0), barretenberg::fr(t1) };
}

inline BasicTable generate_keccak_input_table(BasicTableId id, const size_t table_index)
{
    static constexpr uint64_t base = 11;
    static constexpr uint64_t bits_per_slice = 8;
    BasicTable table;
    table.id = id;
    table.table_index = table_index;
    table.size = (1U << bits_per_slice);
    table.use_twin_keys = false;
    constexpr size_t msb_shift = (64 % bits_per_slice == 0) ? bits_per_slice - 1 : (64 % bits_per_slice) - 1;

    for (uint64_t i = 0; i < table.size; ++i) {
        const uint64_t source = i;
        const auto target = numeric::map_into_sparse_form<base>(source);
        table.column_1.emplace_back(barretenberg::fr(source));
        table.column_2.emplace_back(barretenberg::fr(target));
        table.column_3.emplace_back(barretenberg::fr(source >> msb_shift));
    }

    table.get_values_from_key = &get_keccak_input_values<base, bits_per_slice>;

    uint256_t sparse_step_size = 1;
    for (size_t i = 0; i < bits_per_slice; ++i) {
        sparse_step_size *= base;
    }
    table.column_1_step_size = barretenberg::fr((1 << bits_per_slice));
    table.column_2_step_size = barretenberg::fr(sparse_step_size);
    table.column_3_step_size = barretenberg::fr(sparse_step_size);

    return table;
}

inline MultiTable get_keccak_input_table(const MultiTableId id = KECCAK_FORMAT_INPUT)
{
    const size_t num_entries = 8;

    MultiTable table(1 << 8, uint256_t(11).pow(8), 0, num_entries);

    table.id = id;
    for (size_t i = 0; i < num_entries; ++i) {
        table.slice_sizes.emplace_back(1 << 8);
        table.lookup_ids.emplace_back(KECCAK_INPUT);
        table.get_table_values.emplace_back(&get_keccak_input_values<11, 8>);
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

template <size_t base, size_t num_bits, const uint64_t* base_table>
inline std::array<barretenberg::fr, 2> get_keccak_renormalization_values(const std::array<uint64_t, 2> key)
{
    uint64_t accumulator = 0;
    uint64_t input = key[0];
    uint64_t base_shift = 1;
    constexpr uint64_t divisor_exponent = (64 % num_bits == 0) ? (num_bits - 1) : (64 % num_bits) - 1;
    constexpr uint64_t divisor = numeric::pow64(base, divisor_exponent);
    while (input > 0) {
        uint64_t slice = input % base;
        uint64_t bit = base_table[static_cast<size_t>(slice)];
        accumulator += (bit * base_shift);
        input -= slice;
        input /= base;
        base_shift *= base;
    }

    return { barretenberg::fr(accumulator), barretenberg::fr(accumulator / divisor) };
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
            &get_keccak_renormalization_values<base, num_bits_per_table, rho_normalization_table>);
    }
    return table;
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
        table.get_table_values.emplace_back(
            &get_keccak_renormalization_values<base, num_bits_per_table, chi_normalization_table>);
    }
    return table;
}

inline MultiTable get_keccak_output_table(const MultiTableId id = KECCAK_FORMAT_OUTPUT)
{
    constexpr size_t base = 11;
    constexpr size_t num_bits_per_table = 8;        // values only range from 0-1 per 'bit', not 0-11
    constexpr size_t num_tables_per_multitable = 8; // 64 bits, 8 bits per entry

    uint64_t column_multiplier = numeric::pow64(base, num_bits_per_table);
    MultiTable table(column_multiplier, (1ULL << num_bits_per_table), 0, num_tables_per_multitable);

    table.id = id;
    for (size_t i = 0; i < num_tables_per_multitable; ++i) {
        table.slice_sizes.emplace_back(numeric::pow64(base, num_bits_per_table));
        table.lookup_ids.emplace_back(KECCAK_OUTPUT);
        table.get_table_values.emplace_back(
            &sparse_tables::get_sparse_normalization_values<base, output_normalization_table>);
    }
    return table;
}

template <size_t num_bits, size_t max_base_value_plus_one, const uint64_t* base_table>
inline BasicTable generate_keccak_renormalization_table(BasicTableId id, const size_t table_index)
{
    constexpr size_t base = 11;
    // constexpr size_t num_bits = 8;

    // constexpr size_t max_base_value_plus_one = 5;
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

    // want the most significant bit of the 64-bit integer, when this table is used on the most significant slice
    constexpr uint64_t divisor_exponent = (64 % num_bits == 0) ? (num_bits - 1) : (64 % num_bits) - 1;
    constexpr uint64_t divisor = numeric::pow64(static_cast<uint64_t>(base), divisor_exponent);
    std::pair<uint64_t, uint64_t> key_value;
    for (size_t i = 0; i < table.size; ++i) {
        table.column_1.emplace_back(key);
        table.column_2.emplace_back(value);
        // column_3 = most significant bit of key. Used to rotate left by 1 bit

        table.column_3.emplace_back(value / divisor);
        key_value = sparse_tables::update_counts<0, num_bits, base, max_base_value_plus_one, base_table>(counts);
        key = key_value.first;
        value = key_value.second;
    }

    table.get_values_from_key = &get_keccak_renormalization_values<base, num_bits, base_table>;

    uint64_t step_size = numeric::pow64(static_cast<uint64_t>(base), num_bits);
    table.column_1_step_size = barretenberg::fr(step_size);
    table.column_2_step_size = barretenberg::fr(step_size);
    table.column_3_step_size = barretenberg::fr(0);
    return table;
}

inline BasicTable generate_chi_renormalization_table(BasicTableId id, const size_t table_index)
{
    return generate_keccak_renormalization_table<8, 5, chi_normalization_table>(id, table_index);
}

inline BasicTable generate_rho_renormalization_table(BasicTableId id, const size_t table_index)
{
    return generate_keccak_renormalization_table<11, 3, rho_normalization_table>(id, table_index);
}

} // namespace keccak_tables
} // namespace plookup