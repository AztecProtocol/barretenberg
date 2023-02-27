#pragma once
#include <common/serialize.hpp>
#include <plonk/composer/composer_base.hpp>
#include <plonk/composer/turbo_composer.hpp>

namespace waffle {

struct LogicConstraint {
    uint32_t a;
    uint32_t b;
    uint32_t result;
    uint32_t num_bits;
    uint32_t is_xor_gate;
};

void create_logic_gate(TurboComposer& composer,
                       const uint32_t a,
                       const uint32_t b,
                       const uint32_t result,
                       const size_t num_bits,
                       const bool is_xor_gate)
{
    auto accumulators = composer.create_logic_constraint(a, b, num_bits, is_xor_gate);
    composer.copy_from_to(accumulators.out.back(), result);
}
void xor_gate(TurboComposer& composer, const uint32_t a, const uint32_t b, const uint32_t result, const size_t num_bits)
{
    create_logic_gate(composer, a, b, result, num_bits, true);
}
void and_gate(TurboComposer& composer, const uint32_t a, const uint32_t b, const uint32_t result, const size_t num_bits)
{
    create_logic_gate(composer, a, b, result, num_bits, false);
}

template <typename B> inline void read(B& buf, LogicConstraint& constraint)
{
    using serialize::read;
    read(buf, constraint.a);
    read(buf, constraint.b);
    read(buf, constraint.result);
    read(buf, constraint.num_bits);
    read(buf, constraint.is_xor_gate);
}

template <typename B> inline void write(B& buf, LogicConstraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.a);
    write(buf, constraint.b);
    write(buf, constraint.result);
    write(buf, constraint.num_bits);
    write(buf, constraint.is_xor_gate);
}

inline bool operator==(LogicConstraint const& lhs, LogicConstraint const& rhs)
{
    // clang-format off
    return
        lhs.a == rhs.a &&
        lhs.b == rhs.b &&
        lhs.result == rhs.result &&
        lhs.num_bits == rhs.num_bits &&
        lhs.is_xor_gate == rhs.is_xor_gate;
    // clang-format on
}
} // namespace waffle
