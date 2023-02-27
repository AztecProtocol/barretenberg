#pragma once
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

struct MerkleMembershipConstraint {
    std::vector<uint32_t> hash_path; // Vector of pairs of hashpaths. eg indices 0,1 denotes the pair (0,1)
    uint32_t root;                   // Single field element -- field_t
    uint32_t leaf;                   // Single field element -- field_t
    uint32_t result;                 // Single field element -- bool_t
    uint32_t index;

    friend bool operator==(MerkleMembershipConstraint const& lhs, MerkleMembershipConstraint const& rhs) = default;
};

void create_merkle_check_membership_constraint(plonk::TurboComposer& composer, const MerkleMembershipConstraint& input)
{
    /// Convert value from a witness index into a field element.
    /// This is the hash of the message. In Barretenberg, this would be input.value = hash_value(message)
    field_t leaf = field_t::from_witness_index(&composer, input.leaf);

    /// Convert index from a witness index into a byte array
    field_t index_field = field_t::from_witness_index(&composer, input.index);
    auto index_bits = index_field.decompose_into_bits();

    /// Convert root into a field_t
    field_t root = field_t::from_witness_index(&composer, input.root);

    /// We are given the HashPath as a Vec<fr>
    /// We want to first convert it into a Vec<(fr, fr)> then cast this to hash_path
    /// struct which requires the method create_witness_hashpath
    hash_path<plonk::TurboComposer> hash_path;

    // In Noir we accept a hash path that only contains one hash per tree level
    // It is ok to reuse the leaf as it will be overridden in check_subtree_membership when computing the current root
    // at each tree level
    for (size_t i = 0; i < input.hash_path.size(); i++) {
        if (index_bits[i].get_value() == false) {
            field_t left = leaf;
            field_t right = field_t::from_witness_index(&composer, input.hash_path[i]);
            hash_path.push_back(std::make_pair(left, right));
        } else {
            field_t left = field_t::from_witness_index(&composer, input.hash_path[i]);
            field_t right = leaf;
            hash_path.push_back(std::make_pair(left, right));
        }
    }

    auto exists = check_subtree_membership(root, hash_path, leaf, index_bits, 0);
    composer.assert_equal_constant(exists.witness_index, fr::one());
}

template <typename B> inline void read(B& buf, MerkleMembershipConstraint& constraint)
{
    using serialize::read;
    read(buf, constraint.hash_path);
    read(buf, constraint.root);
    read(buf, constraint.leaf);
    read(buf, constraint.result);
    read(buf, constraint.index);
}

template <typename B> inline void write(B& buf, MerkleMembershipConstraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.hash_path);
    write(buf, constraint.root);
    write(buf, constraint.leaf);
    write(buf, constraint.result);
    write(buf, constraint.index);
}
