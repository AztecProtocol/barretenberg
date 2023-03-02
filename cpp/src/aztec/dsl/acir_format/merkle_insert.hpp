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

template <typename B> inline void read(B& buf, MerkleInsertConstraint& constraint)
{
    using serialize::read;
    read(buf, constraint.hash_path);
    read(buf, constraint.old_root);
    read(buf, constraint.new_root);
    read(buf, constraint.old_leaf);
    read(buf, constraint.new_leaf);
    read(buf, constraint.index);
}

template <typename B> inline void write(B& buf, MerkleInsertConstraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.hash_path);
    write(buf, constraint.old_root);
    write(buf, constraint.new_root);
    write(buf, constraint.old_leaf);
    write(buf, constraint.new_leaf);
    write(buf, constraint.index);
}
} // namespace acir_format
