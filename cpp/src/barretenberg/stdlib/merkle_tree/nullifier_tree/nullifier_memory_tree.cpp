#include "nullifier_memory_tree.hpp"
#include "../hash.hpp"

namespace proof_system::plonk {
namespace stdlib {
namespace merkle_tree {

NullifierMemoryTree::NullifierMemoryTree(size_t depth)
    : MemoryTree(depth)
{
    ASSERT(depth_ >= 1 && depth <= 32);
    total_size_ = 1UL << depth_;
    hashes_.resize(total_size_ * 2 - 2);

    // Build the entire tree and fill with 0 hashes.
    auto current = WrappedNullifierLeaf::zero().hash();
    size_t layer_size = total_size_;
    for (size_t offset = 0; offset < hashes_.size(); offset += layer_size, layer_size /= 2) {
        for (size_t i = 0; i < layer_size; ++i) {
            hashes_[offset + i] = current;
        }
        current = hash_pair_native(current, current);
    }

    // Insert the initial leaf at index 0
    auto initial_leaf = WrappedNullifierLeaf(nullifier_leaf{ .value = 0, .nextIndex = 0, .nextValue = 0 });
    leaves_.push_back(initial_leaf);
    root_ = update_element(0, initial_leaf.hash());
}

fr NullifierMemoryTree::update_element(fr const& value)
{
    // Find the leaf with the value closest and less than `value`

    // If value is 0 we simply append 0 a null NullifierLeaf to the tree
    if (value == 0) {
        auto zero_leaf = WrappedNullifierLeaf::zero();
        leaves_.push_back(zero_leaf);
        return update_element(leaves_.size() - 1, zero_leaf.hash());
    }

    size_t current;
    bool is_already_present;
    std::tie(current, is_already_present) = find_closest_leaf(leaves_, value);

    nullifier_leaf current_leaf = leaves_[current].unwrap();
    nullifier_leaf new_leaf = { .value = value,
                                .nextIndex = current_leaf.nextIndex,
                                .nextValue = current_leaf.nextValue };

    if (!is_already_present) {
        // Update the current leaf to point it to the new leaf
        current_leaf.nextIndex = leaves_.size();
        current_leaf.nextValue = value;

        leaves_[current].set(current_leaf);

        // Insert the new leaf with (nextIndex, nextValue) of the current leaf
        leaves_.push_back(new_leaf);
    }

    // Update the old leaf in the tree
    auto old_leaf_hash = current_leaf.hash();
    size_t old_leaf_index = current;
    auto root = update_element(old_leaf_index, old_leaf_hash);

    // Insert the new leaf in the tree
    auto new_leaf_hash = new_leaf.hash();
    size_t new_leaf_index = is_already_present ? old_leaf_index : leaves_.size() - 1;
    root = update_element(new_leaf_index, new_leaf_hash);

    return root;
}

// Check for a larger value in an array
bool check_has_less_than(std::vector<fr> const& values, fr const& value)
{
    // Must perform comparisons on integers
    auto const value_as_uint = uint256_t(value);
    for (auto const& v : values) {
        if (uint256_t(v) < value_as_uint) {
            return true;
        }
    }
    return false;
}

// handle synthetic membership assertions
LowLeafWitnessData NullifierMemoryTree::batch_insert(std::vector<fr> const& values)
{
    // Start insertion index
    size_t const start_insertion_index = this->size();

    // Low nullifiers
    auto values_size = values.size();
    std::vector<nullifier_leaf> low_nullifiers(values_size);
    std::vector<nullifier_leaf> pending_insertion_tree(values_size);

    // Low nullifier sibling paths
    std::vector<std::vector<fr>> sibling_paths(values_size);

    // Low nullifier indexes
    std::vector<uint32_t> low_nullifier_indexes(values_size);

    // Keep track of the currently touched nodes while updating
    std::map<size_t, std::vector<fr>> touched_nodes;

    // Keep track of 0 values
    std::vector<fr> const empty_sp(depth_, 0);
    auto const empty_leaf = nullifier_leaf::empty();
    uint32_t const empty_index = 0;

    // Find the leaf with the value closest and less than `value` for each value
    for (size_t i = 0; i < values.size(); ++i) {

        auto new_value = values[i];
        auto insertion_index = start_insertion_index + i;

        size_t current = 0;
        bool is_already_present = false;
        std::tie(current, is_already_present) = find_closest_leaf(leaves_, new_value);

        // If the inserted value is 0, then we ignore and provide a dummy low nullifier
        if (new_value == 0) {
            sibling_paths[i] = empty_sp;
            low_nullifier_indexes[i] = empty_index;
            low_nullifiers[i] = empty_leaf;
            pending_insertion_tree[i] = empty_leaf;
            continue;
        }

        // If the low_nullifier node has been touched this sub tree insertion, we provide a dummy sibling path
        // It will be up to the circuit to check if the included node is valid vs the other nodes that have been
        // inserted before it If it has not been touched, we provide a sibling path then update the nodes pointers
        auto prev_nodes = touched_nodes.find(current);

        bool has_less_than = false;
        if (prev_nodes != touched_nodes.end()) {
            has_less_than = check_has_less_than(prev_nodes->second, new_value);
        }
        // If there is a lower value in the tree, we need to check the current low nullifiers for one that can be used
        if (has_less_than) {
            for (size_t j = 0; j < pending_insertion_tree.size(); ++j) {

                nullifier_leaf& pending = pending_insertion_tree[j];
                // Skip checking empty values
                if (pending.is_empty()) {
                    continue;
                }

                if (uint256_t(pending.value) < uint256_t(new_value) &&
                    (uint256_t(pending.nextValue) > uint256_t(new_value) || pending.nextValue == fr::zero())) {

                    // Add a new pending low nullifier for this value
                    nullifier_leaf const current_low_leaf = { .value = new_value,
                                                              .nextIndex = pending_insertion_tree[j].nextIndex,
                                                              .nextValue = pending_insertion_tree[j].nextValue };

                    pending_insertion_tree[i] = current_low_leaf;

                    // Update the pending low nullifier to point at the new value
                    pending.nextValue = new_value;
                    pending.nextIndex = insertion_index;

                    break;
                }
            }

            // add empty low nullifier
            sibling_paths[i] = empty_sp;
            low_nullifier_indexes[i] = empty_index;
            low_nullifiers[i] = empty_leaf;
        } else {
            // Update the touched mapping
            if (prev_nodes == touched_nodes.end()) {
                std::vector<fr> const new_touched_values = { new_value };
                touched_nodes[current] = new_touched_values;
            } else {
                prev_nodes->second.push_back(new_value);
            }

            nullifier_leaf const low_nullifier = leaves_[current].unwrap();
            std::vector<fr> const sibling_path = this->get_sibling_path(current);

            sibling_paths[i] = sibling_path;
            low_nullifier_indexes[i] = static_cast<uint32_t>(current);
            low_nullifiers[i] = low_nullifier;

            // TODO(SEAN): rename this and new leaf
            nullifier_leaf insertion_leaf = { .value = new_value,
                                              .nextIndex = low_nullifier.nextIndex,
                                              .nextValue = low_nullifier.nextValue };
            pending_insertion_tree[i] = insertion_leaf;

            // Update the current low nullifier
            nullifier_leaf const new_leaf = { .value = low_nullifier.value,
                                              .nextIndex = insertion_index,
                                              .nextValue = new_value };

            // Update the old leaf in the tree
            // update old value in tree
            update_element_in_place(current, new_leaf);
        }
    }

    // resize leaves array
    this->leaves_.resize(this->leaves_.size() + pending_insertion_tree.size());
    for (size_t i = 0; i < pending_insertion_tree.size(); ++i) {
        nullifier_leaf const pending = pending_insertion_tree[i];

        // Update the old leaf in the tree
        // update old value in tree
        update_element_in_place(start_insertion_index + i, pending);
    }

    // Return tuple of low nullifiers and sibling paths
    return std::make_tuple(low_nullifiers, sibling_paths, low_nullifier_indexes);
}

// Update the value of a leaf in place
fr NullifierMemoryTree::update_element_in_place(size_t index, const nullifier_leaf& leaf)
{
    this->leaves_[index].set(leaf);
    return update_element(index, leaf.hash());
}

} // namespace merkle_tree
} // namespace stdlib
} // namespace proof_system::plonk