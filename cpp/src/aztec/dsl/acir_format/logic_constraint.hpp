#pragma once
#include <stdlib/types/types.hpp>

namespace acir_format {

struct LogicConstraint {
    uint32_t a;
    uint32_t b;
    uint32_t result;
    uint32_t num_bits;
    uint32_t is_xor_gate;

    friend bool operator==(LogicConstraint const& lhs, LogicConstraint const& rhs) = default;
};

void create_logic_gate(
    TurboComposer& composer, uint32_t a, uint32_t b, uint32_t result, size_t num_bits, bool is_xor_gate);

void xor_gate(TurboComposer& composer, uint32_t a, uint32_t b, uint32_t result, size_t num_bits);

void and_gate(TurboComposer& composer, uint32_t a, uint32_t b, uint32_t result, size_t num_bits);

template <typename B> inline void read(B& buf, LogicConstraint& constraint);

template <typename B> inline void write(B& buf, LogicConstraint const& constraint);

} // namespace acir_format
