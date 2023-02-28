#pragma once
#include <stdlib/types/types.hpp>

namespace acir_format {

struct Blake2sInput {
    uint32_t witness;
    uint32_t num_bits;

    friend bool operator==(Blake2sInput const& lhs, Blake2sInput const& rhs) = default;
};

struct Blake2sConstraint {
    std::vector<Blake2sInput> inputs;
    std::vector<uint32_t> result;

    friend bool operator==(Blake2sConstraint const& lhs, Blake2sConstraint const& rhs) = default;
};

void create_blake2s_constraints(plonk::TurboComposer& composer, const Blake2sConstraint& constraint);

template <typename B> inline void read(B& buf, Blake2sInput& constraint);

template <typename B> inline void write(B& buf, Blake2sInput const& constraint);

template <typename B> inline void read(B& buf, Blake2sConstraint& constraint);

template <typename B> inline void write(B& buf, Blake2sConstraint const& constraint);

} // namespace acir_format
