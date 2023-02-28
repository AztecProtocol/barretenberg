#include <stdlib/types/types.hpp>

namespace acir_format {

struct Sha256Input {
    uint32_t witness;
    uint32_t num_bits;

    friend bool operator==(Sha256Input const& lhs, Sha256Input const& rhs) = default;
};

struct Sha256Constraint {
    std::vector<Sha256Input> inputs;
    std::vector<uint32_t> result;

    friend bool operator==(Sha256Constraint const& lhs, Sha256Constraint const& rhs) = default;
};

// This function does not work (properly) because the stdlib:sha256 function is not working correctly for 512 bits
// pair<witness_index, bits>
void create_sha256_constraints(plonk::TurboComposer& composer, const Sha256Constraint& constraint);

template <typename B> inline void read(B& buf, Sha256Input& constraint);

template <typename B> inline void write(B& buf, Sha256Input const& constraint);

template <typename B> inline void read(B& buf, Sha256Constraint& constraint);

template <typename B> inline void write(B& buf, Sha256Constraint const& constraint);

} // namespace acir_format
