#pragma once
#include <stdlib/types/types.hpp>

namespace acir_format {

struct RangeConstraint {
    uint32_t witness;
    uint32_t num_bits;

    friend bool operator==(RangeConstraint const& lhs, RangeConstraint const& rhs) = default;
};

template <typename B> inline void read(B& buf, RangeConstraint& constraint);

template <typename B> inline void write(B& buf, RangeConstraint const& constraint);

} // namespace acir_format
