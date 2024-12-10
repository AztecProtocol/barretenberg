// AUTOGENERATED FILE
#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "barretenberg/common/ref_vector.hpp"
#include "columns.hpp"

namespace bb::avm {

template <typename FF_> struct AvmFullRow {
    using FF = FF_;

    FF AVM_ALL_ENTITIES;

    RefVector<const FF> as_vector() const;
    static std::vector<std::string> names();
    static constexpr size_t SIZE = 764;

    // Risky but oh so efficient.
    FF& get_column(ColumnAndShifts col)
    {
        static_assert(sizeof(*this) == sizeof(FF) * static_cast<size_t>(ColumnAndShifts::NUM_COLUMNS));
        return reinterpret_cast<FF*>(this)[static_cast<size_t>(col)];
    }

    const FF& get_column(ColumnAndShifts col) const
    {
        static_assert(sizeof(*this) == sizeof(FF) * static_cast<size_t>(ColumnAndShifts::NUM_COLUMNS));
        return reinterpret_cast<FF*>(this)[static_cast<size_t>(col)];
    }
};

template <typename FF> std::ostream& operator<<(std::ostream& os, AvmFullRow<FF> const& row);

} // namespace bb::avm

namespace bb {

// Expose this in the bb namespace. For compatibility with the old witgen.
using avm::AvmFullRow;

} // namespace bb