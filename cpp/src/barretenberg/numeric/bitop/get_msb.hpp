// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace bb::numeric {

// from http://supertech.csail.mit.edu/papers/debruijn.pdf
constexpr inline uint32_t get_msb32(const uint32_t in)
{
    constexpr std::array<uint8_t, 32> MultiplyDeBruijnBitPosition{ 0,  9,  1,  10, 13, 21, 2,  29, 11, 14, 16,
                                                                   18, 22, 25, 3,  30, 8,  12, 20, 28, 15, 17,
                                                                   24, 7,  19, 27, 23, 6,  26, 5,  4,  31 };

    uint32_t v = in | (in >> 1);
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;

    return MultiplyDeBruijnBitPosition[static_cast<uint32_t>(v * static_cast<uint32_t>(0x07C4ACDD)) >>
                                       static_cast<uint32_t>(27)];
}

constexpr inline uint64_t get_msb64(const uint64_t in)
{
    constexpr std::array<uint8_t, 64> de_bruijn_sequence{ 0,  47, 1,  56, 48, 27, 2,  60, 57, 49, 41, 37, 28,
                                                          16, 3,  61, 54, 58, 35, 52, 50, 42, 21, 44, 38, 32,
                                                          29, 23, 17, 11, 4,  62, 46, 55, 26, 59, 40, 36, 15,
                                                          53, 34, 51, 20, 43, 31, 22, 10, 45, 25, 39, 14, 33,
                                                          19, 30, 9,  24, 13, 18, 8,  12, 7,  6,  5,  63 };

    uint64_t t = in | (in >> 1);
    t |= t >> 2;
    t |= t >> 4;
    t |= t >> 8;
    t |= t >> 16;
    t |= t >> 32;
    return static_cast<uint64_t>(de_bruijn_sequence[(t * 0x03F79D71B4CB0A89ULL) >> 58ULL]);
};

template <typename T> constexpr inline T get_msb(const T in)
{
    return (sizeof(T) <= 4) ? get_msb32(in) : get_msb64(in);
}

template <typename T> constexpr inline T round_up_power_2(const T in)
{
    auto lower_bound = T(1) << get_msb(in);
    return (lower_bound == in || lower_bound == 1) ? in : lower_bound * 2;
}

} // namespace bb::numeric