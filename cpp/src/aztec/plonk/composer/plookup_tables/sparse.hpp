#pragma once

#include "./types.hpp"

#include <crypto/aes128/aes128.hpp>
#include <numeric/bitop/rotate.hpp>
#include <numeric/bitop/sparse_form.hpp>
#include <numeric/bitop/pow.hpp>

namespace plookup {
namespace sparse_tables {

template <uint64_t base, uint64_t num_rotated_bits>
inline std::array<barretenberg::fr, 2> get_sparse_table_with_rotation_values(const std::array<uint64_t, 2> key)
{
    const auto t0 = numeric::map_into_sparse_form<base>(key[0]);
    barretenberg::fr t1;
    if constexpr (num_rotated_bits > 0) {
        t1 = numeric::map_into_sparse_form<base>(numeric::rotate32((uint32_t)key[0], num_rotated_bits));
    } else {
        t1 = t0;
    }
    return { barretenberg::fr(t0), barretenberg::fr(t1) };
}

template <uint64_t base, uint64_t bits_per_slice, uint64_t num_rotated_bits>
inline BasicTable generate_sparse_table_with_rotation(BasicTableId id, const size_t table_index)
{
    BasicTable table;
    table.id = id;
    table.table_index = table_index;
    table.size = (1U << bits_per_slice);
    table.use_twin_keys = false;

    for (uint64_t i = 0; i < table.size; ++i) {
        const uint64_t source = i;
        const auto target = numeric::map_into_sparse_form<base>(source);
        table.column_1.emplace_back(barretenberg::fr(source));
        table.column_2.emplace_back(barretenberg::fr(target));

        if constexpr (num_rotated_bits > 0) {
            const auto rotated =
                numeric::map_into_sparse_form<base>(numeric::rotate32((uint32_t)source, num_rotated_bits));
            table.column_3.emplace_back(barretenberg::fr(rotated));
        } else {
            table.column_3.emplace_back(barretenberg::fr(target));
        }
    }

    table.get_values_from_key = &get_sparse_table_with_rotation_values<base, num_rotated_bits>;

    uint256_t sparse_step_size = 1;
    for (size_t i = 0; i < bits_per_slice; ++i) {
        sparse_step_size *= base;
    }
    // ?????? why is column1 automatically 2^11?
    //     table.column_1_step_size = barretenberg::fr((1 << 11));
    table.column_1_step_size = barretenberg::fr((1 << bits_per_slice));
    table.column_2_step_size = barretenberg::fr(sparse_step_size);
    table.column_3_step_size = barretenberg::fr(sparse_step_size);

    return table;
}

template <size_t base, const uint64_t* base_table>
inline std::array<barretenberg::fr, 2> get_sparse_normalization_values(const std::array<uint64_t, 2> key)
{
    uint64_t accumulator = 0;
    uint64_t input = key[0];
    uint64_t count = 0;
    while (input > 0) {
        uint64_t slice = input % base;
        uint64_t bit = base_table[static_cast<size_t>(slice)];
        accumulator += (bit << count);
        input -= slice;
        input /= base;
        ++count;
    }
    return { barretenberg::fr(accumulator), barretenberg::fr(0) };
}

template <size_t base, const uint64_t* base_table>
inline std::array<barretenberg::fr, 2> get_sparse_renormalization_values(const std::array<uint64_t, 2> key)
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
    return { barretenberg::fr(accumulator), barretenberg::fr(0) };
}

template <size_t base, uint64_t num_bits, const uint64_t* base_table>
inline BasicTable generate_sparse_normalization_table(BasicTableId id, const size_t table_index)
{
    /**
     * If t = 7*((e >>> 6) + (e >>> 11) + (e >>> 25)) + e + 2f + 3g
     * we can create a mapping between the 28 distinct values, and the result of
     * (e >>> 6) ^ (e >>> 11) ^ (e >>> 25) + e + 2f + 3g
     */

    BasicTable table;
    table.id = id;
    table.table_index = table_index;
    table.use_twin_keys = false;
    table.size = numeric::pow64(static_cast<uint64_t>(base), num_bits);

    numeric::sparse_int<base, num_bits> accumulator(0);
    numeric::sparse_int<base, num_bits> to_add(1);
    for (size_t i = 0; i < table.size; ++i) {
        const auto& limbs = accumulator.get_limbs();
        uint64_t key = 0;
        for (size_t j = 0; j < num_bits; ++j) {
            const size_t table_idx = static_cast<size_t>(limbs[j]);
            key += ((base_table[table_idx]) << static_cast<uint64_t>(j));
        }

        table.column_1.emplace_back(accumulator.get_sparse_value());
        table.column_2.emplace_back(key);
        table.column_3.emplace_back(barretenberg::fr(0));
        accumulator += to_add;
    }

    table.get_values_from_key = &get_sparse_normalization_values<base, base_table>;

    table.column_1_step_size = barretenberg::fr(table.size);
    table.column_2_step_size = barretenberg::fr(((uint64_t)1 << num_bits));
    table.column_3_step_size = barretenberg::fr(0);
    return table;
}

template <size_t i, size_t num_bits, uint64_t base_value, uint64_t max_base_value>
std::pair<uint64_t, uint64_t> update_counts(std::array<size_t, num_bits>& counts)
{
    ASSERT(i < num_bits);
    if constexpr (i >= num_bits) {
        // TODO use concepts or template metaprogramming to put this condition in method declaration
        return std::make_pair(0, 0);
    } else {
        if (counts[i] == max_base_value) {
            counts[i] = 0;
            return update_counts<i + 1, num_bits, base_value, max_base_value>(counts);
        } else {
            counts[i] += 1;
        }

        uint64_t value = 0;
        uint64_t normalized_value = 0;
        uint64_t cumulative_base = 1;
        for (size_t j = 0; j <= i; ++j) {
            value += counts[j] * cumulative_base;
            normalized_value += (counts[j] % 2) * cumulative_base;
            cumulative_base *= base_value;
        }
        return std::make_pair(value, normalized_value);
    }
}

template <size_t i, size_t num_bits, uint64_t base_value, uint64_t max_base_value>
std::pair<uint64_t, uint64_t> update_counts_value_in_binary_basis(std::array<size_t, num_bits>& counts)
{
    ASSERT(i < num_bits);
    if constexpr (i >= num_bits) {
        // TODO use concepts or template metaprogramming to put this condition in method declaration
        return std::make_pair(0, 0);
    } else {
        if (counts[i] == max_base_value) {
            counts[i] = 0;
            return update_counts_value_in_binary_basis<i + 1, num_bits, base_value, max_base_value>(counts);
        } else {
            counts[i] += 1;
        }

        uint64_t value = 0;
        uint64_t normalized_value = 0;
        uint64_t cumulative_base = 1;
        for (size_t j = 0; j <= i; ++j) {
            value += counts[j] * cumulative_base;
            normalized_value += (counts[j] % 2) * cumulative_base;
            cumulative_base *= 2;
        }
        return std::make_pair(value, normalized_value);
    }
}

template <size_t base, uint64_t num_bits, const uint64_t* base_table, uint64_t max_base_value_plus_one = base>
inline BasicTable generate_sparse_renormalization_table(BasicTableId id, const size_t table_index)
{
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
            table.column_3.emplace_back(barretenberg::fr(0));
            key_value = update_counts<0, num_bits, base, max_base_value_plus_one>(counts);
            key_value.first = key;
            key_value.second = value;
        }
    }

    table.get_values_from_key = &get_sparse_renormalization_values<max_base_value_plus_one, base_table>;

    table.column_1_step_size = barretenberg::fr(table.size);
    table.column_2_step_size = barretenberg::fr(table.size);
    table.column_3_step_size = barretenberg::fr(0);
    return table;
}

// todo replace older method if this works
template <size_t base, uint64_t num_bits, const uint64_t* base_table, uint64_t max_base_value_plus_one = base>
inline BasicTable generate_sparse_normalization_table_new(BasicTableId id, const size_t table_index)
{
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
            table.column_3.emplace_back(barretenberg::fr(0));
            // todo fix ugly name
            key_value = update_counts_value_in_binary_basis<0, num_bits, base, max_base_value_plus_one>(counts);
            key_value.first = key;
            key_value.second = value;
        }
    }

    table.get_values_from_key = &get_sparse_normalization_values<max_base_value_plus_one, base_table>;

    table.column_1_step_size = barretenberg::fr(table.size);
    table.column_2_step_size = barretenberg::fr(((uint64_t)1 << num_bits));
    table.column_3_step_size = barretenberg::fr(0);
    return table;
}
} // namespace sparse_tables
} // namespace plookup