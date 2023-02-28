#pragma once
#include <stdlib/types/types.hpp>

namespace acir_format {

struct FixedBaseScalarMul {
    uint32_t scalar;
    uint32_t pub_key_x;
    uint32_t pub_key_y;

    friend bool operator==(FixedBaseScalarMul const& lhs, FixedBaseScalarMul const& rhs) = default;
};

void create_fixed_base_constraint(plonk::TurboComposer& composer, const FixedBaseScalarMul& input);

template <typename B> inline void read(B& buf, FixedBaseScalarMul& constraint);

template <typename B> inline void write(B& buf, FixedBaseScalarMul const& constraint);

} // namespace acir_format
