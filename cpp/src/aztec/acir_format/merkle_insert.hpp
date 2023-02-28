#include <stdlib/types/types.hpp>

namespace acir_format {

struct MerkleInsertConstraint {
    std::vector<uint32_t> hash_path; // Vector of pairs of hashpaths. eg indices 0,1 denotes the pair (0,1)
    uint32_t old_root;               // Single field element -- field_t
    uint32_t new_root;               // Single field element -- field_t -- the new root
    uint32_t old_leaf;               // Single field element -- field_t
    uint32_t new_leaf;               // Single field element -- field_t
    uint32_t index;

    friend bool operator==(MerkleInsertConstraint const& lhs, MerkleInsertConstraint const& rhs) = default;
};

void create_merkle_insert_constraint(plonk::TurboComposer& composer, const MerkleInsertConstraint& input);

template <typename B> inline void read(B& buf, MerkleInsertConstraint& constraint);

template <typename B> inline void write(B& buf, MerkleInsertConstraint const& constraint);

} // namespace acir_format
