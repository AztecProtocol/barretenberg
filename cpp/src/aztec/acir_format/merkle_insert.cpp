#include "merkle_insert.hpp"

using namespace plonk::stdlib::types;
using namespace plonk::stdlib::merkle_tree;
using namespace barretenberg;
using namespace plonk;

namespace acir_format {

void create_merkle_insert_constraint(plonk::TurboComposer& composer, const MerkleInsertConstraint& input)
{
    /// Convert leaves from a witness index into a byte array.
    field_ct old_leaf = field_ct::from_witness_index(&composer, input.old_leaf);
    // byte_array old_leaf_arr(old_leaf);
    field_ct new_leaf = field_ct::from_witness_index(&composer, input.new_leaf);
    // byte_array new_leaf_arr(new_leaf);

    /// Convert index from a witness index into a byte array
    field_ct index_field = field_ct::from_witness_index(&composer, input.index);
    auto index_arr_bits = index_field.decompose_into_bits();
    // byte_array index_arr(index_field);

    /// Convert roots into field_ct
    field_ct old_root = field_ct::from_witness_index(&composer, input.old_root);
    field_ct new_root = field_ct::from_witness_index(&composer, input.new_root);

    /// We are given the HashPath as a Vec<fr>
    /// We want to first convert it into a Vec<(fr, fr)> then cast this to hash_path
    /// struct which requires the method create_witness_hashpath
    hash_path<plonk::TurboComposer> hash_path;

    for (size_t i = 0; i < input.hash_path.size(); i = i + 2) {
        field_ct left = field_ct::from_witness_index(&composer, input.hash_path[i]);
        field_ct right = field_ct::from_witness_index(&composer, input.hash_path[i + 1]);
        hash_path.push_back(std::make_pair(left, right));
    }

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

} // namespace acir_format
