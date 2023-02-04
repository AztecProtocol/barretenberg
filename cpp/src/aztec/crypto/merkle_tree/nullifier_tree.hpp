#pragma once
#include "hash.hpp"
#include "merkle_tree.hpp"

namespace crypto {
namespace merkle_tree {

using namespace barretenberg;

struct leaf {
    fr value;
    uint256_t nextIndex;
    fr nextValue;
};

inline std::ostream& operator<<(std::ostream& os, leaf const& input)
{
    os << "value   = " << input.value << "\nnextIdx = " << input.nextIndex << "\nnextVal = " << input.nextValue;
    return os;
}

inline void read(uint8_t const*& it, leaf& input)
{
    using serialize::read;
    read(it, input.value);
    read(it, input.nextIndex);
    read(it, input.nextValue);
}

inline void write(std::vector<uint8_t>& buf, leaf const& input)
{
    using serialize::write;
    write(buf, input.value);
    write(buf, input.nextIndex);
    write(buf, input.nextValue);
}

inline barretenberg::fr hash_leaf_native(leaf const& input_leaf)
{
    return crypto::pedersen::compress_native({ input_leaf.value, input_leaf.nextIndex });
}

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

  private:
    using MerkleTree<Store>::update_element;
    using MerkleTree<Store>::get_element;
    using MerkleTree<Store>::compute_zero_path_hash;

  private:
    using MerkleTree<Store>::store_;
    using MerkleTree<Store>::zero_hashes_;
    using MerkleTree<Store>::depth_;
    using MerkleTree<Store>::tree_id_;
    std::vector<leaf> leaves;
};

extern template class NullifierTree<LevelDbStore>;
extern template class NullifierTree<MemoryStore>;

typedef NullifierTree<LevelDbStore> LevelDbNullifierTree;

} // namespace merkle_tree
} // namespace crypto