#include "field.hpp"
#include <gtest/gtest.h>
#include "barretenberg/common/msgpack_impl.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"

TEST(field_msgpack, basic)
{
    barretenberg::fr field {1ull, 2ull, 3ull, 4ull};
    barretenberg::fr result {0, 0, 0, 0};
    msgpack::sbuffer buffer;
    msgpack::pack(buffer, field);
    msgpack::unpack(buffer.data(), buffer.size()).get().convert(result);
    EXPECT_EQ(field, result);
}
