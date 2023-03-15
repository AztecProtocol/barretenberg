#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/honk/sumcheck/sumcheck.hpp"

#include <gtest/gtest.h>

namespace test_sumcheck_polynomials {

using namespace honk::sumcheck;

template <class FF> class MultivariatesTests : public testing::Test {};

using FieldTypes = testing::Types<barretenberg::fr>;
TYPED_TEST_SUITE(MultivariatesTests, FieldTypes);

#define MULTIVARIATES_TESTS_TYPE_ALIASES using FF = TypeParam;

/*
 * We represent a bivariate f0 as f0(X0, X1). The indexing starts from 0 to match with the round number in sumcheck.
 * The idea is variable X0 (lsb) will be folded at round 2 (the first sumcheck round),
 * then the variable X1 (msb) will be folded at round 1 (the last rond in this case). Pictorially we have,
 *          v10 ------ v11
 *           |          |
 *   X0(lsb) |          |
 *           |  X1(msb) |
 *          v00 ------ v01
 * f0(X0, X1) = v00 * (1-X0) * (1-X1)
 *            + v10 *   X0   * (1-X1)
 *            + v01 * (1-X0) *   X1
 *            + v11 *   X0   *   X1.
 *
 * To effectively represent folding we write,
 * f0(X0, X1) = [v00 * (1-X0) + v10 * X0] * (1-X1)
 *            + [v01 * (1-X0) + v11 * X0] *   X1.
 *
 * After folding at round 0 (round challenge u0), we have,
 * f0(u0,X1) = (v00 * (1-u0) + v10 * u0) * (1-X1)
 *           + (v01 * (1-u0) + v11 * u0) *   X1.
 *
 * After folding at round 1 (round challenge u1), we have,
 * f0(u0,u1) = (v00 * (1-u0) + v10 * u0) * (1-u1)
 *           + (v01 * (1-u0) + v11 * u0) *   u1.
 */
TYPED_TEST(MultivariatesTests, FoldTwoRoundsSpecial)
{
    MULTIVARIATES_TESTS_TYPE_ALIASES

    FF v00 = 0;
    FF v10 = 1;
    FF v01 = 0;
    FF v11 = 0;

    const std::array<FF, 4> f0 = { v00, v10, v01, v11 };
    std::vector<FF> f0_evaluated;

    FF round_challenge_0 = { 0x6c7301b49d85a46c, 0x44311531e39c64f6, 0xb13d66d8d6c1a24c, 0x04410c360230a295 };
    round_challenge_0.self_to_montgomery_form();
    FF expected_lo = v00 * (FF(1) - round_challenge_0) + v10 * round_challenge_0;
    FF expected_hi = v01 * (FF(1) - round_challenge_0) + v11 * round_challenge_0;

    partially_evaluate(f0_evaluated, std::span<const FF>{ f0 }, round_challenge_0);

    EXPECT_EQ(f0_evaluated[0], round_challenge_0);
    EXPECT_EQ(f0_evaluated[1], FF(0));

    FF round_challenge_1 = 2;
    FF expected_val = expected_lo * (FF(1) - round_challenge_1) + expected_hi * round_challenge_1;

    partially_evaluate(f0_evaluated, std::span<const FF>{ f0_evaluated }, round_challenge_1);
    EXPECT_EQ(f0_evaluated[0], expected_val);
}

TYPED_TEST(MultivariatesTests, FoldTwoRoundsGeneric)
{
    MULTIVARIATES_TESTS_TYPE_ALIASES

    FF v00 = FF::random_element();
    FF v10 = FF::random_element();
    FF v01 = FF::random_element();
    FF v11 = FF::random_element();

    std::array<FF, 4> f0 = { v00, v10, v01, v11 };
    std::vector<FF> f0_evaluated;

    FF round_challenge_0 = FF::random_element();
    FF expected_lo = v00 * (FF(1) - round_challenge_0) + v10 * round_challenge_0;
    FF expected_hi = v01 * (FF(1) - round_challenge_0) + v11 * round_challenge_0;

    partially_evaluate(f0_evaluated, std::span<const FF>{ f0 }, round_challenge_0);

    EXPECT_EQ(f0_evaluated[0], expected_lo);
    EXPECT_EQ(f0_evaluated[1], expected_hi);

    FF round_challenge_1 = FF::random_element();
    FF expected_val = expected_lo * (FF(1) - round_challenge_1) + expected_hi * round_challenge_1;
    partially_evaluate(f0_evaluated, std::span<const FF>{ f0_evaluated }, round_challenge_1);
    EXPECT_EQ(f0_evaluated[0], expected_val);
}

/*
 * Similarly for a trivariate polynomial f0(X0, X1, X2), we have
 * f0(X0, X1, X2) = v000 * (1-X0) * (1-X1) * (1-X2)
 *                + v100 *   X0   * (1-X1) * (1-X2)
 *                + v010 * (1-X0) *   X1   * (1-X2)
 *                + v110 *   X0   *   X1   * (1-X2)
 *                + v001 * (1-X0) * (1-X1) *   X2
 *                + v101 *   X0   * (1-X1) *   X2
 *                + v011 * (1-X0) *   X1   *   X2
 *                + v111 *   X0   *   X1   *   X2.
 * After round 0 (round challenge u0), we have
 *  f0(u0, X1, X2) = [v000 * (1-u0) + v100 * u0] * (1-X1) * (1-X2)
 *                 + [v010 * (1-u0) + v110 * u0] *   X1   * (1-X2)
 *                 + [v001 * (1-u0) + v101 * u0] * (1-X1) *   X2
 *                 + [v011 * (1-u0) + v111 * u0] *   X1   *   X2.
 * After round 1 (round challenge u1), we have
 * f0(u0, u1, X2) = [(v000 * (1-u0) + v100 * u0) * (1-u1) + (v010 * (1-u0) + v110 * u0) * u1] * (1-X2)
 *                + [(v001 * (1-u0) + v101 * u0) * (1-u1) + (v011 * (1-u0) + v111 * u0) * u1] *   X2.
 * After round 2 (round challenge u2), we have
 * f0(u0, u1, u2) = [(v000 * (1-u0) + v100 * u0) * (1-u1) + (v010 * (1-u0) + v110 * u0) * u1] * (1-u2)
 *                + [(v001 * (1-u0) + v101 * u0) * (1-u1) + (v011 * (1-u0) + v111 * u0) * u1] *   u2.
 */
TYPED_TEST(MultivariatesTests, FoldThreeRoundsSpecial)
{
    MULTIVARIATES_TESTS_TYPE_ALIASES

    FF v000 = 1;
    FF v100 = 2;
    FF v010 = 3;
    FF v110 = 4;
    FF v001 = 5;
    FF v101 = 6;
    FF v011 = 7;
    FF v111 = 8;

    std::array<FF, 8> f0 = { v000, v100, v010, v110, v001, v101, v011, v111 };
    std::vector<FF> f0_evaluated;

    FF round_challenge_0 = 1;
    FF expected_q1 = v000 * (FF(1) - round_challenge_0) + v100 * round_challenge_0; // 2
    FF expected_q2 = v010 * (FF(1) - round_challenge_0) + v110 * round_challenge_0; // 4
    FF expected_q3 = v001 * (FF(1) - round_challenge_0) + v101 * round_challenge_0; // 6
    FF expected_q4 = v011 * (FF(1) - round_challenge_0) + v111 * round_challenge_0; // 8

    partially_evaluate(f0_evaluated, std::span<const FF>{ f0 }, round_challenge_0);

    EXPECT_EQ(f0_evaluated[0], expected_q1);
    EXPECT_EQ(f0_evaluated[1], expected_q2);
    EXPECT_EQ(f0_evaluated[2], expected_q3);
    EXPECT_EQ(f0_evaluated[3], expected_q4);

    FF round_challenge_1 = 2;
    FF expected_lo = expected_q1 * (FF(1) - round_challenge_1) + expected_q2 * round_challenge_1; // 6
    FF expected_hi = expected_q3 * (FF(1) - round_challenge_1) + expected_q4 * round_challenge_1; // 10

    partially_evaluate(f0_evaluated, std::span<const FF>{ f0_evaluated }, round_challenge_1);
    EXPECT_EQ(f0_evaluated[0], expected_lo);
    EXPECT_EQ(f0_evaluated[1], expected_hi);

    FF round_challenge_2 = 3;
    FF expected_val = expected_lo * (FF(1) - round_challenge_2) + expected_hi * round_challenge_2; // 18
    partially_evaluate(f0_evaluated, std::span<const FF>{ f0_evaluated }, round_challenge_2);
    EXPECT_EQ(f0_evaluated[0], expected_val);
}

TYPED_TEST(MultivariatesTests, FoldThreeRoundsGeneric)
{
    MULTIVARIATES_TESTS_TYPE_ALIASES

    FF v000 = FF::random_element();
    FF v100 = FF::random_element();
    FF v010 = FF::random_element();
    FF v110 = FF::random_element();
    FF v001 = FF::random_element();
    FF v101 = FF::random_element();
    FF v011 = FF::random_element();
    FF v111 = FF::random_element();

    std::array<FF, 8> f0 = { v000, v100, v010, v110, v001, v101, v011, v111 };
    std::vector<FF> f0_evaluated;

    FF round_challenge_0 = FF::random_element();
    FF expected_q1 = v000 * (FF(1) - round_challenge_0) + v100 * round_challenge_0;
    FF expected_q2 = v010 * (FF(1) - round_challenge_0) + v110 * round_challenge_0;
    FF expected_q3 = v001 * (FF(1) - round_challenge_0) + v101 * round_challenge_0;
    FF expected_q4 = v011 * (FF(1) - round_challenge_0) + v111 * round_challenge_0;

    partially_evaluate(f0_evaluated, std::span<const FF>{ f0 }, round_challenge_0);

    EXPECT_EQ(f0_evaluated[0], expected_q1);
    EXPECT_EQ(f0_evaluated[1], expected_q2);
    EXPECT_EQ(f0_evaluated[2], expected_q3);
    EXPECT_EQ(f0_evaluated[3], expected_q4);

    FF round_challenge_1 = FF::random_element();
    FF expected_lo = expected_q1 * (FF(1) - round_challenge_1) + expected_q2 * round_challenge_1;
    FF expected_hi = expected_q3 * (FF(1) - round_challenge_1) + expected_q4 * round_challenge_1;

    partially_evaluate(f0_evaluated, std::span<const FF>{ f0_evaluated }, round_challenge_1);
    EXPECT_EQ(f0_evaluated[0], expected_lo);
    EXPECT_EQ(f0_evaluated[1], expected_hi);

    FF round_challenge_2 = FF::random_element();
    FF expected_val = expected_lo * (FF(1) - round_challenge_2) + expected_hi * round_challenge_2;
    partially_evaluate(f0_evaluated, std::span<const FF>{ f0_evaluated }, round_challenge_2);
    EXPECT_EQ(f0_evaluated[0], expected_val);
}
} // namespace test_sumcheck_polynomials
