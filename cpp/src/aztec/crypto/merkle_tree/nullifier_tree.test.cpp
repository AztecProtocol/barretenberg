#include "leveldb_store.hpp"
#include "memory_store.hpp"
#include "memory_tree.hpp"
#include "nullifier_tree.hpp"
#include <common/streams.hpp>
#include <common/test.hpp>
#include <numeric/random/engine.hpp>

using namespace barretenberg;
using namespace crypto::merkle_tree;

namespace {
auto& engine = numeric::random::get_debug_engine();
auto& random_engine = numeric::random::get_engine();
} // namespace

static std::vector<fr> VALUES = []() {
    std::vector<fr> values(1024);
    for (size_t i = 0; i < 1024; ++i) {
        values[i] = fr(random_engine.get_random_uint8());
    }
    return values;
}();

TEST(crypto_nullifier_tree, test_nullifier_basic)
{
    MemoryStore store;
    auto db = NullifierTree(store, 256);

    // We assume that the first leaf is already filled with (0, 0, 0).
    EXPECT_EQ(db.size(), 1ULL);

    // Add a new non-zero leaf at index 1.
    db.update_element(30);
    EXPECT_EQ(db.size(), 2ULL);

    // Add third.
    db.update_element(10);
    EXPECT_EQ(db.size(), 3ULL);

    // Add forth.
    db.update_element(20);
    EXPECT_EQ(db.size(), 4ULL);

    // Add forth but with same value.
    db.update_element(20);
    EXPECT_EQ(db.size(), 4ULL);
}