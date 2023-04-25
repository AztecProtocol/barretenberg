#include "field.hpp"
#include <gtest/gtest.h>
#include "barretenberg/common/msgpack_impl.hpp"

TEST(field_msgpack, basic)
{
    barretenberg::field field(1ull, 2ull, 3ull, 4ull);
    barretenberg::field result(0, 0, 0, 0);
    msgpack::sbuffer buffer;
    msgpack::pack(buffer, field);
    msgpack::unpack(buffer.data(), buffer.size()).get().convert(result);
    EXPECT_EQ(result, expected);
}
