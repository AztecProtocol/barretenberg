#include "nullifier_memory_tree.hpp"
#include "hash.hpp"

namespace crypto {
namespace merkle_tree {

IndexedMerkleTree::IndexedMerkleTree(size_t depth)
    : depth_(depth)
{
    ASSERT(depth_ >= 1 && depth <= 32);
    total_size_ = 1UL << depth_;
    hashes_.resize(total_size_ * 2 - 2);

    // Build the entire tree.
    leaf zero_leaf = { 0, 0, 0 };
    leaves_.push_back(zero_leaf);
    auto current = zero_leaf.hash();
    update_element_internal(0, current);
    size_t layer_size = total_size_;
    for (size_t offset = 0; offset < hashes_.size(); offset += layer_size, layer_size /= 2) {
        for (size_t i = 0; i < layer_size; ++i) {
            hashes_[offset + i] = current;
        }
        current = compress_native(current, current);
    }

    root_ = current;
}

fr_hash_path IndexedMerkleTree::get_hash_path(size_t index)
{
    fr_hash_path path(depth_);
    size_t offset = 0;
    size_t layer_size = total_size_;
    for (size_t i = 0; i < depth_; ++i) {
        index -= index & 0x1;
        path[i] = std::make_pair(hashes_[offset + index], hashes_[offset + index + 1]);
        offset += layer_size;
        layer_size >>= 1;
        index >>= 1;
    }
    return path;
}

fr IndexedMerkleTree::update_element_internal(size_t index, fr const& value)
{
    size_t offset = 0;
    size_t layer_size = total_size_;
    fr current = value;
    for (size_t i = 0; i < depth_; ++i) {
        hashes_[offset + index] = current;
        index &= (~0ULL) - 1;
        current = compress_native(hashes_[offset + index], hashes_[offset + index + 1]);
        offset += layer_size;
        layer_size >>= 1;
        index >>= 1;
    }
    root_ = current;
    return root_;
}

fr IndexedMerkleTree::update_element(fr const& value)
{
    // Find the leaf with the value closest and less than `value`
    size_t current;
    bool is_already_present;
    std::tie(current, is_already_present) = find_closest_leaf(leaves_, value);

    leaf new_leaf = { .value = value,
                      .nextIndex = leaves_[current].nextIndex,
                      .nextValue = leaves_[current].nextValue };
    if (!is_already_present) {
        // Update the current leaf to point it to the new leaf
        leaves_[current].nextIndex = leaves_.size();
        leaves_[current].nextValue = value;

        // Insert the new leaf with (nextIndex, nextValue) of the current leaf
        leaves_.push_back(new_leaf);
    }

    // Update the old leaf in the tree
    auto old_leaf_hash = leaves_[current].hash();
    size_t old_leaf_index = current;
    auto root = update_element_internal(old_leaf_index, old_leaf_hash);

    // Insert the new leaf in the tree
    auto new_leaf_hash = new_leaf.hash();
    size_t new_leaf_index = is_already_present ? old_leaf_index : leaves_.size() - 1;
    root = update_element_internal(new_leaf_index, new_leaf_hash);

    return root;
}

} // namespace merkle_tree
} // namespace crypto