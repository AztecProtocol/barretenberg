#pragma once
#include "../hash.hpp"
#include "../merkle_tree.hpp"
#include "nullifier_leaf.hpp"

namespace proof_system::plonk {
namespace stdlib {
namespace merkle_tree {

using namespace barretenberg;

template <typename Store> class NullifierTree : public MerkleTree<Store> {
  public:
    typedef uint256_t index_t;

    NullifierTree(Store& store, size_t depth, uint8_t tree_id = 0);
    NullifierTree(NullifierTree const& other) = delete;
    NullifierTree(NullifierTree&& other);
    ~NullifierTree();

    using MerkleTree<Store>::get_hash_path;
    using MerkleTree<Store>::root;
    using MerkleTree<Store>::size;
    using MerkleTree<Store>::depth;

    fr update_element(fr const& value);

  protected:
    using MerkleTree<Store>::update_element;
    using MerkleTree<Store>::get_element;
    using MerkleTree<Store>::compute_zero_path_hash;

    // const std::vector<barretenberg::fr>& get_hashes() { return hashes_; }
    const WrappedNullifierLeaf get_leaf(size_t index)
    {
        // return (index < leaves_.size()) ? leaves_[index] : WrappedNullifierLeaf::zero();
        return (index < leaves.size()) ? leaves[index] : WrappedNullifierLeaf::zero();
    }
    const std::vector<WrappedNullifierLeaf>& get_leaves() { return leaves; }

    using MerkleTree<Store>::store_;
    using MerkleTree<Store>::zero_hashes_;
    using MerkleTree<Store>::depth_;
    using MerkleTree<Store>::tree_id_;
    std::vector<WrappedNullifierLeaf> leaves;
};

extern template class NullifierTree<MemoryStore>;

} // namespace merkle_tree
} // namespace stdlib
} // namespace proof_system::plonk