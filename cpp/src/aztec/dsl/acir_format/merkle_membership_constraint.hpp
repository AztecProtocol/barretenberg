#pragma once
#include <stdlib/types/types.hpp>

namespace acir_format {

struct MerkleMembershipConstraint {
    std::vector<uint32_t> hash_path; // Vector of pairs of hashpaths. eg indices 0,1 denotes the pair (0,1)
    uint32_t root;                   // Single field element -- field_t
    uint32_t leaf;                   // Single field element -- field_t
    uint32_t result;                 // Single field element -- bool_t
    uint32_t index;

    friend bool operator==(MerkleMembershipConstraint const& lhs, MerkleMembershipConstraint const& rhs) = default;
};

void create_merkle_check_membership_constraint(plonk::TurboComposer& composer, const MerkleMembershipConstraint& input);

template <typename B> inline void read(B& buf, MerkleMembershipConstraint& constraint);

template <typename B> inline void write(B& buf, MerkleMembershipConstraint const& constraint);

} // namespace acir_format
