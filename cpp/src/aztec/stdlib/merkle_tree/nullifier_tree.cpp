#include "nullifier_tree.hpp"
#include "merkle_tree.hpp"
#include "hash.hpp"
#include "leveldb_store.hpp"
#include "memory_store.hpp"
#include <common/net.hpp>
#include <iostream>
#include <numeric/bitop/count_leading_zeros.hpp>
#include <numeric/bitop/keep_n_lsb.hpp>
#include <numeric/uint128/uint128.hpp>
#include <sstream>

namespace plonk {
namespace stdlib {
namespace merkle_tree {

using namespace barretenberg;

template <typename T> inline bool bit_set(T const& index, size_t i)
{
    return bool((index >> i) & 0x1);
}

template <typename Store>
NullifierTree<Store>::NullifierTree(Store& store, size_t depth, uint8_t tree_id)
    : MerkleTree<Store>(store, depth, tree_id)
{
    ASSERT(depth_ >= 1 && depth <= 256);
    zero_hashes_.resize(depth);

    // Compute the zero values at each layer.
    // Insert the zero leaf to the `leaves` and also to the tree at index 0.
    auto zero_leaf = leaf{ .value = 0, .nextIndex = 0, .nextValue = 0 };
    leaves.push_back(zero_leaf);
    auto current = hash_leaf_native(zero_leaf);
    update_element(0, current);
    for (size_t i = 0; i < depth; ++i) {
        zero_hashes_[i] = current;
        current = compress_native(current, current);
    }
}

template <typename Store>
NullifierTree<Store>::NullifierTree(NullifierTree&& other)
    : MerkleTree<Store>(std::move(other))
{}

template <typename Store> NullifierTree<Store>::~NullifierTree() {}

std::pair<size_t, bool> find_closest_leaf(std::vector<leaf> const& leaves, fr const& new_value)
{
    std::vector<uint256_t> diff;
    bool repeated = false;
    for (size_t i = 0; i < leaves.size(); i++) {
        auto leaf_value_ = uint256_t(leaves[i].value);
        auto new_value_ = uint256_t(new_value);
        if (leaf_value_ > new_value_) {
            diff.push_back(leaf_value_);
        } else if (leaf_value_ == new_value_) {
            repeated = true;
            return std::make_pair(i, repeated);
        } else {
            diff.push_back(new_value_ - leaf_value_);
        }
    }
    auto it = std::min_element(diff.begin(), diff.end());
    return std::make_pair(static_cast<size_t>(it - diff.begin()), repeated);
}

template <typename Store> fr NullifierTree<Store>::update_element(fr const& value)
{
    ASSERT(leaves.size() == size());

    // Find the leaf with the value closest and less than `value`
    size_t current;
    bool is_already_present;
    std::tie(current, is_already_present) = find_closest_leaf(leaves, value);

    leaf new_leaf = { .value = value, .nextIndex = leaves[current].nextIndex, .nextValue = leaves[current].nextValue };
    if (!is_already_present) {
        // Insert the new leaf with (nextIndex, nextValue) of the current leaf
        leaves.push_back(new_leaf);

        // Update the current leaf to point it to the new leaf
        leaves[current].nextIndex = size() + 1;
        leaves[current].nextValue = value;
    }

    // Update the old leaf in the tree
    auto old_leaf_hash = hash_leaf_native(leaves[current]);
    index_t old_leaf_index = size() - 1;
    auto r = update_element(old_leaf_index, old_leaf_hash);

    // Insert the new leaf in the tree
    auto new_leaf_hash = hash_leaf_native(new_leaf);
    index_t new_leaf_index = is_already_present ? old_leaf_index : old_leaf_index + 1;
    r = update_element(new_leaf_index, new_leaf_hash);

    return r;
}

#ifndef __wasm__
template class NullifierTree<LevelDbStore>;
#endif
template class NullifierTree<MemoryStore>;

} // namespace merkle_tree
} // namespace stdlib
} // namespace plonk