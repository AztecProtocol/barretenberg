#include <stdlib/types/types.hpp>

namespace acir_format {

struct HashToFieldInput {
    uint32_t witness;
    uint32_t num_bits;

    friend bool operator==(HashToFieldInput const& lhs, HashToFieldInput const& rhs) = default;
};

struct HashToFieldConstraint {
    std::vector<HashToFieldInput> inputs;
    uint32_t result;

    friend bool operator==(HashToFieldConstraint const& lhs, HashToFieldConstraint const& rhs) = default;
};

void create_hash_to_field_constraints(plonk::TurboComposer& composer, const HashToFieldConstraint constraint);

template <typename B> inline void read(B& buf, HashToFieldInput& constraint);

template <typename B> inline void write(B& buf, HashToFieldInput const& constraint);

template <typename B> inline void read(B& buf, HashToFieldConstraint& constraint);

template <typename B> inline void write(B& buf, HashToFieldConstraint const& constraint);

} // namespace acir_format
