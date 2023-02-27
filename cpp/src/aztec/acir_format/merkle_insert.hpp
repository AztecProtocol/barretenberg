#include <common/serialize.hpp>
#include <common/assert.hpp>
#include <plonk/composer/composer_base.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include <stdlib/merkle_tree/membership.hpp>
#include <stdlib/types/types.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>
#include <stdlib/primitives/byte_array/byte_array.hpp>

using namespace plonk::stdlib::types;
using namespace plonk::stdlib::merkle_tree;
typedef plonk::stdlib::field_t<plonk::TurboComposer> field_t;
typedef plonk::stdlib::byte_array<plonk::TurboComposer> byte_array;

using namespace barretenberg;
using namespace plonk;

struct MerkleInsertConstraint {
    std::vector<uint32_t> hash_path; // Vector of pairs of hashpaths. eg indices 0,1 denotes the pair (0,1)
    uint32_t old_root;               // Single field element -- field_t
    uint32_t new_root;               // Single field element -- field_t -- the new root
    uint32_t old_leaf;               // Single field element -- field_t
    uint32_t new_leaf;               // Single field element -- field_t
    uint32_t index;
};

void create_merkle_insert_constraint(plonk::TurboComposer& composer, const MerkleInsertConstraint& input)
{
    /// Convert leaves from a witness index into a byte array.
    field_t old_leaf = field_t::from_witness_index(&composer, input.old_leaf);
    // byte_array old_leaf_arr(old_leaf);
    field_t new_leaf = field_t::from_witness_index(&composer, input.new_leaf);
    // byte_array new_leaf_arr(new_leaf);

    /// Convert index from a witness index into a byte array
    field_t index_field = field_t::from_witness_index(&composer, input.index);
    auto index_arr_bits = index_field.decompose_into_bits();
    // byte_array index_arr(index_field);

    /// Convert roots into field_t
    field_t old_root = field_t::from_witness_index(&composer, input.old_root);
    field_t new_root = field_t::from_witness_index(&composer, input.new_root);

    /// We are given the HashPath as a Vec<fr>
    /// We want to first convert it into a Vec<(fr, fr)> then cast this to hash_path
    /// struct which requires the method create_witness_hashpath
    hash_path<plonk::TurboComposer> hash_path;

    for (size_t i = 0; i < input.hash_path.size(); i = i + 2) {
        field_t left = field_t::from_witness_index(&composer, input.hash_path[i]);
        field_t right = field_t::from_witness_index(&composer, input.hash_path[i + 1]);
        hash_path.push_back(std::make_pair(left, right));
    }

    // void update_membership(field_t<Composer> const& new_root,
    //                        field_t<Composer> const& new_value,
    //                        field_t<Composer> const& old_root,
    //                        hash_path<Composer> const& old_hashes,
    //                        field_t<Composer> const& old_value,
    //                        bit_vector<Composer> const& index,
    //                        std::string const& msg = "update_membership")

    update_membership(new_root, new_leaf, old_root, hash_path, old_leaf, index_arr_bits);
}

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

inline bool operator==(MerkleInsertConstraint const& lhs, MerkleInsertConstraint const& rhs)
{
    // clang-format off
    return
        lhs.old_root == rhs.old_root &&
        lhs.new_root == rhs.new_root &&
        lhs.old_leaf == rhs.old_leaf &&
        lhs.new_leaf == rhs.new_leaf &&
        lhs.index == rhs.index &&
        lhs.hash_path == rhs.hash_path;
    // clang-format on
}
