#include <gtest/gtest.h>
#include "barretenberg/proof_system/op_queue/ecc_op_queue.hpp"

namespace proof_system::test_flavor {
TEST(ECCOpQueueTest, Basic)
{
    ECCOpQueue op_queue;
    op_queue.add_accumulate(grumpkin::g1::affine_one);
    EXPECT_EQ(op_queue._data[0].base_point, grumpkin::g1::affine_one);
    op_queue.empty_row();
    EXPECT_EQ(op_queue._data[1].add, false);
}

} // namespace proof_system::test_flavor
