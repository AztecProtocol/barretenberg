// AUTOGENERATED FILE
#pragma once

#include "columns.hpp"

namespace bb::avm2 {

template <typename FF_> struct AvmFullRow {
    using FF = FF_;

    FF AVM2_ALL_ENTITIES;

    // Risky but oh so efficient.
    FF& get_column(ColumnAndShifts col)
    {
        static_assert(sizeof(*this) == sizeof(FF) * static_cast<size_t>(ColumnAndShifts::SENTINEL_DO_NOT_USE));
        return reinterpret_cast<FF*>(this)[static_cast<size_t>(col)];
    }

    const FF& get_column(ColumnAndShifts col) const
    {
        static_assert(sizeof(*this) == sizeof(FF) * static_cast<size_t>(ColumnAndShifts::SENTINEL_DO_NOT_USE));
        return reinterpret_cast<const FF*>(this)[static_cast<size_t>(col)];
    }
};

} // namespace bb::avm2

namespace bb {

// Expose this in the bb namespace. For compatibility with the old witgen.
using avm2::AvmFullRow;

} // namespace bb