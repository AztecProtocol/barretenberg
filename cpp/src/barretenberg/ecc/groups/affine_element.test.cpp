#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"
#include "barretenberg/ecc/curves/secp256k1/secp256k1.hpp"
#include "barretenberg/ecc/curves/secp256r1/secp256r1.hpp"
#include "barretenberg/common/test.hpp"
#include <fstream>
#include "barretenberg/common/serialize.hpp"

namespace test_affine_element {
template <typename G1> class test_affine_element : public testing::Test {
    using element = typename G1::element;
    using affine_element = typename G1::affine_element;

  public:
    static void test_read_write_buffer()
    {
        // a generic point
        {
            affine_element P = affine_element(element::random_element());
            affine_element Q;
            affine_element R;

            std::vector<uint8_t> v(65); // extra byte to allow a bad read
            uint8_t* ptr = v.data();
            affine_element::serialize_to_buffer(P, ptr);

            // bad read
            Q = affine_element::serialize_from_buffer(ptr + 1);
            ASSERT_FALSE(Q.on_curve() && !Q.is_point_at_infinity());
            ASSERT_FALSE(P == Q);

            // good read
            R = affine_element::serialize_from_buffer(ptr);
            ASSERT_TRUE(R.on_curve());
            ASSERT_TRUE(P == R);
        }

        // point at infinity
        {
            affine_element P = affine_element(element::random_element());
            P.self_set_infinity();
            affine_element R;

            std::vector<uint8_t> v(64);
            uint8_t* ptr = v.data();
            affine_element::serialize_to_buffer(P, ptr);

            R = affine_element::serialize_from_buffer(ptr);
            ASSERT_TRUE(R.is_point_at_infinity());
            ASSERT_TRUE(P == R);
        }
    }

    static void test_point_compression()
    {
        for (size_t i = 0; i < 10; i++) {
            affine_element P = affine_element(element::random_element());
            uint256_t compressed = P.compress();
            affine_element Q = affine_element::from_compressed(compressed);
            EXPECT_EQ(P, Q);
        }
    }

    // Regression test to ensure that the point at infinity is not equal to its coordinate-wise reduction, which may lie
    // on the curve, depending on the y-coordinate.
    // TODO: add corresponding typed test class
    static void test_infinity_regression()
    {
        affine_element P;
        P.self_set_infinity();
        affine_element R(0, P.y);
        ASSERT_FALSE(P == R);
    }
};

typedef testing::Types<barretenberg::g1, grumpkin::g1, secp256k1::g1, secp256r1::g1> TestTypes;

TYPED_TEST_SUITE(test_affine_element, TestTypes);

TYPED_TEST(test_affine_element, read_write_buffer)
{
    TestFixture::test_read_write_buffer();
}

TYPED_TEST(test_affine_element, point_compression)
{
    if constexpr (TypeParam::Fq::modulus.data[3] >= 0x4000000000000000ULL) {
        GTEST_SKIP();
    } else {
        TestFixture::test_point_compression();
    }
}

// Regression test to ensure that the point at infinity is not equal to its coordinate-wise reduction, which may lie
// on the curve, depending on the y-coordinate.
TEST(affine_element, infinity_ordering_regression)
{
    secp256k1::g1::affine_element P(0, 1), Q(0, 1);

    P.self_set_infinity();
    EXPECT_NE(P < Q, Q < P);
}

TEST(affine_element, get_coordinate_limbs)
{
    secp256k1::fq x(uint256_t{ 0x0000000000000001, 0xa00000000000001a, 0xb00000000000011b, 0xc00000000000111c });
    secp256k1::fq y(uint256_t{ 0x0000000000000002, 0xd00000000000002d, 0xe00000000000022e, 0xf00000000000222f });
    secp256k1::g1::affine_element P(x, y);

    std::vector<barretenberg::fr> limbs = P.get_coordinate_limbs();
    EXPECT_EQ(uint256_t(limbs[0]), uint256_t(0x0000000000000001, 0xa, 0, 0));
    EXPECT_EQ(uint256_t(limbs[1]), uint256_t(0xba00000000000001, 0x1, 0, 0));
    EXPECT_EQ(uint256_t(limbs[2]), uint256_t(0x1cb0000000000001, 0x1, 0, 0));
    EXPECT_EQ(uint256_t(limbs[3]), uint256_t(0x000c000000000001, 0x0, 0, 0));
    EXPECT_EQ(uint256_t(limbs[4]), uint256_t(0x0000000000000002, 0xd, 0, 0));
    EXPECT_EQ(uint256_t(limbs[5]), uint256_t(0xed00000000000002, 0x2, 0, 0));
    EXPECT_EQ(uint256_t(limbs[6]), uint256_t(0x2fe0000000000002, 0x2, 0, 0));
    EXPECT_EQ(uint256_t(limbs[7]), uint256_t(0x000f000000000002, 0x0, 0, 0));
}
} // namespace test_affine_element
