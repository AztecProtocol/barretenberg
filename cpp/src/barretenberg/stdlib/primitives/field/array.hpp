#pragma once
#include "field.hpp"
#include "../safe_uint/safe_uint.hpp"
#include "../bool/bool.hpp"

namespace plonk {
namespace stdlib {

/**
 * Gets the number of contiguous nonzero values of an array from the start.
 * Note: This assumes `0` always means 'not used', so be careful. As soon as we locate 0, we stop the counting.
 * If you actually want `0` to be counted, you'll need something else.
 */
template <typename Composer, size_t SIZE> field_t<Composer> array_length(std::array<field_t<Composer>, SIZE> const& arr)
{
    field_t<Composer> length = 0;
    bool_t<Composer> hit_zero = false;
    for (const auto& e : arr) {
        hit_zero |= e == 0;
        const field_t<Composer> increment = !hit_zero;
        length += increment;
    }
    return length;
};

/**
 * Note: doesn't remove the last element from the array; only returns it!
 * Note: this assumes `0` always means 'not used', so be careful. If you actually want `0` to be counted, you'll need
 * something else.
 * If it returns `0`, the array is considered 'empty'.
 */
template <typename Composer, size_t SIZE> field_t<Composer> array_pop(std::array<field_t<Composer>, SIZE> const& arr)
{
    field_t<Composer> popped_value = 0;
    bool_t<Composer> already_popped = false;
    for (size_t i = arr.size() - 1; i != (size_t)-1; i--) {
        bool_t<Composer> is_non_zero = arr[i] != 0;
        popped_value = field_t<Composer>::conditional_assign(!already_popped && is_non_zero, arr[i], popped_value);

        already_popped |= is_non_zero;
    }
    already_popped.assert_equal(true, "array_pop cannot pop from an empty array");

    return popped_value;
};

/**
 * Note: this assumes `0` always means 'not used', so be careful. If you actually want `0` to be counted, you'll need
 * something else.
 */
template <typename Composer, size_t SIZE>
void array_push(std::array<field_t<Composer>, SIZE>& arr, field_t<Composer> const& value)
{
    bool_t<Composer> already_pushed = false;
    for (size_t i = 0; i < arr.size(); ++i) {
        bool_t<Composer> is_zero = arr[i] == 0;
        arr[i] = field_t<Composer>::conditional_assign(!already_pushed && is_zero, value, arr[i]);

        already_pushed |= is_zero;
    }
    already_pushed.assert_equal(true, "array_push cannot push to a full array");
};

template <typename Composer, size_t SIZE>
inline size_t array_push(std::array<std::optional<field_t<Composer>>, SIZE>& arr, field_t<Composer> const& value)
{
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] == std::nullopt) {
            arr[i] = value;
            return i;
        }
    }
    throw_or_abort("array_push cannot push to a full array");
};

template <typename T, size_t SIZE>
inline size_t array_push(std::array<std::shared_ptr<T>, SIZE>& arr, std::shared_ptr<T> const& value)
{
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] == nullptr) {
            arr[i] = value;
            return i;
        }
    }
    throw_or_abort("array_push cannot push to a full array");
};

/**
 * Note: this assumes `0` always means 'not used', so be careful. If you actually want `0` to be counted, you'll need
 * something else.
 */
template <typename Composer, size_t SIZE>
typename plonk::stdlib::bool_t<Composer> is_array_empty(std::array<field_t<Composer>, SIZE> const& arr)
{
    bool_t<Composer> nonzero_found = false;
    for (size_t i = arr.size() - 1; i != (size_t)-1; i--) {
        bool_t<Composer> is_non_zero = arr[i] != 0;
        nonzero_found |= is_non_zero;
    }
    return !nonzero_found;
};

/**
 * Inserts the `source` array at the first zero-valued index of the `target` array.
 * Fails if the `source` array is too large vs the remaining capacity of the `target` array.
 */
template <typename Composer, size_t size_1, size_t size_2>
void push_array_to_array(std::array<field_t<Composer>, size_1> const& source,
                         std::array<field_t<Composer>, size_2>& target)
{
    // TODO: inefficient to get length this way within this function. Probably best to inline the checks that we need
    // into the below loops directly.
    field_t<Composer> target_length = array_length<Composer>(target);
    field_t<Composer> target_capacity = field_t<Composer>(target.size());
    const field_t<Composer> overflow_capacity = target_capacity + 1;

    // ASSERT(uint256_t(target_capacity.get_value()) + 1 >
    //        uint256_t(target_length.get_value()) + uint256_t(source_length.get_value()));

    field_t<Composer> j_ct = 0; // circuit-type index for the inner loop
    field_t<Composer> next_target_index = target_length;
    for (size_t i = 0; i < source.size(); ++i) {
        auto& s = source[i];

        // Triangular loop:
        for (size_t j = i; j < target.size() - source.size() + i + 1; ++j) {
            auto& t = target[j];

            bool_t<Composer> at_next_index = j_ct == next_target_index;

            t = field_t<Composer>::conditional_assign(at_next_index, s, t);

            j_ct++;
        }

        next_target_index++;

        next_target_index.assert_not_equal(overflow_capacity, "push_array_to_array target array capacity exceeded");

        j_ct = i + 1;
    }
}

} // namespace stdlib
} // namespace plonk