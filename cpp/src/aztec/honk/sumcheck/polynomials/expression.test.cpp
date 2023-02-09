#include "honk/sumcheck/polynomials/expression.hpp"
#include "univariate.hpp"
#include <ecc/curves/bn254/fr.hpp>

#include <gtest/gtest.h>

using namespace honk::sumcheck;
namespace test_univariate {

TEST(ExpressionTest, Expr)
{
    using FF = barretenberg::fr;
    using Univariate = Univariate<FF, 4>;
    using Expr = UnivariateExpr<FF, 4>;

    const FF s0{ 0 };
    const FF s1{ 1 };
    const FF s2{ 2 };
    const FF s3{ 3 };

    const Univariate p0{ 0 };
    const Univariate p1{ 1 };
    const Univariate p2{ 2 };
    const Univariate p3{ 3 };
    const Univariate p6{ 6 };

    const Expr e0{ p0 }; // E0
    const Expr e1{ p1 }; // E1
    const Expr e2{ p2 }; // E2
    const Expr e3{ p3 }; // E2

    EXPECT_EQ(Univariate(e1 + e2), p3);      // ExprSum<E1,E2>
    EXPECT_EQ(Univariate(e2 - e1), p1);      // ExprDiff<E2,E1>
    EXPECT_EQ(Univariate(e1 * e2), p2);      // ExprMul<E1,E2>
    EXPECT_EQ(Univariate(e2 + s1), p3);      // ExprTranslate<E2>
    EXPECT_EQ(Univariate(s2 + e1), p3);      // ExprTranslate<E1>
    EXPECT_EQ(Univariate(e2 * s1), p2);      // ExprScale<E2>
    EXPECT_EQ(Univariate(s2 * e1), p2);      // ExprScale<E1>
    EXPECT_EQ(Univariate(e1 * e3 - e2), p1); // ExprDiff<ExprMul<E1,E3>, E2>

    EXPECT_EQ(Univariate((e1 * e2) * e3), p6); // ExprMul<ExprMul<E1,E2>, E3>
    EXPECT_EQ(Univariate(e1 * (e2 * e3)), p6); // ExprMul<E1, ExprMul<E2,E3>>
    // Below is not valid since e1^4 would require that the size of Univariate be 5
    // EXPECT_EQ(Univariate(e1 * e1 * e1 * e1), p1);
}

} // namespace test_univariate
