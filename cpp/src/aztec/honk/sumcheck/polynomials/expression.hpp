#pragma once
#include <array>
#include <cstdint>
#include <span>
#include <ostream>
#include <common/serialize.hpp>
#include <common/assert.hpp>

namespace honk::sumcheck {

// forward declare
template <class Fr, size_t _length> class Univariate;

/**
 * @brief Baseclass for all expressions.
 *
 * @details This enables CRTP (curiously recurring template pattern),
 * which is a form of compile-time polynomorphism. If a caller accepts a `const Expr<FF, E>& expr` as argument,
 * calling expr[i] calls the sub-classe's operator[] which will return the appropriate arithmetic operation.
 * The operators +,-,* are overloaded for Expr so that a long expression essentially produces an arithmetic circuit
 * via templates.
 * For example, the type of `ExprA + ExprB` is `ExprC = ExprSum<ExprA, ExprB>`, and when it can be evaluated by
 calling
 * `ExprC[i] = ExprA[i]+ExprB[i]`. Either subexpression can itself be an expression. The result is that the full
 * expression is evaluated without having to create temporary buffers to store the results.
 *
 * @warning The type of an expression easily becomes really complicated. An expression should usually be evaluated in
 * the same line as it is defined. Since each expression only stores references to its sub-expressions, these
 references
 * will become dangling when scope of the expression is exited. This can result in hard to debug problems. The
 solution
 * is always to create a Univariate from the expressions: Univariate result = a * b - c;
 *
 *      Univariate acc = ...
 *      acc += a * b - c
 *
 * @tparam FF Finite field
 * @tparam E Type of the subclass.
 */
template <typename FF, typename E> struct Expr {
    // Returns a reference to self, but casted as the sub-class.
    E const& self() const { return static_cast<E const&>(*this); }
    // Calls the subclass' operator[] method.
    decltype(auto) operator[](size_t i) const { return self()[i]; }
};

/**
 * @brief Wrapper class for converting a Univariate into an Expr. We can then apply arithmetic operators over it to
 * construct an expression.
 */
template <class FF, size_t SIZE_> struct UnivariateExpr : public Expr<FF, UnivariateExpr<FF, SIZE_>> {

    static constexpr size_t SIZE = SIZE_;
    static constexpr size_t DEGREE = 1;

    // Store a reference to the evaluations of the original Univariate.
    std::span<const FF, SIZE> evaluations;

    // Prevent the creation from any span
    UnivariateExpr() = delete;

    // Create an UnivariateExpr from a Univariate
    template <size_t univariate_length>
        requires(SIZE <= univariate_length)
    explicit UnivariateExpr(const Univariate<FF, univariate_length>& univariate)
        : evaluations(std::span{ univariate.evaluations.data(), SIZE })
    {}

    // At the end of an expression, we just return the value
    decltype(auto) operator[](size_t i) const { return evaluations[i]; }
};

// Sum expression
template <typename FF, typename E1, typename E2>
    requires(E1::SIZE == E2::SIZE)
struct ExprSum : public Expr<FF, ExprSum<FF, E1, E2>> {
    static constexpr size_t DEGREE = std::max(E1::DEGREE, E2::DEGREE);
    static constexpr size_t SIZE = E1::SIZE;

    E1 const& expr1;
    E2 const& expr2;

    ExprSum(E1 const& expr1, E2 const& expr2)
        : expr1(expr1)
        , expr2(expr2)
    {}

    decltype(auto) operator[](size_t i) const { return expr1[i] + expr2[i]; }
};

// Difference expression
template <typename FF, typename E1, typename E2>
    requires(E1::SIZE == E2::SIZE)
struct ExprDiff : public Expr<FF, ExprDiff<FF, E1, E2>> {

    static constexpr size_t DEGREE = std::max(E1::DEGREE, E2::DEGREE);
    static constexpr size_t SIZE = E1::SIZE;

    E1 const& expr1;
    E2 const& expr2;

    ExprDiff(E1 const& expr1, E2 const& expr2)
        : expr1(expr1)
        , expr2(expr2)
    {}

    decltype(auto) operator[](size_t i) const { return expr1[i] - expr2[i]; }
};

// Multiplication expression
template <typename FF, typename E1, typename E2>
    requires(E1::SIZE == E2::SIZE)
struct ExprMul : public Expr<FF, ExprMul<FF, E1, E2>> {

    static constexpr size_t DEGREE = E1::DEGREE + E2::DEGREE;
    static constexpr size_t SIZE = E1::SIZE;

    E1 const& expr1;
    E2 const& expr2;

    ExprMul(E1 const& expr1, E2 const& expr2)
        : expr1(expr1)
        , expr2(expr2)
    {}

    decltype(auto) operator[](size_t i) const { return expr1[i] * expr2[i]; }
};

// Scale by scalar expression
template <typename FF, typename E> struct ExprScale : public Expr<FF, ExprScale<FF, E>> {

    static constexpr size_t DEGREE = E::DEGREE;
    static constexpr size_t SIZE = E::SIZE;

    E const& expr;
    FF const& scalar;

    ExprScale(E const& expr, FF const& scalar)
        : expr(expr)
        , scalar(scalar)
    {}

    decltype(auto) operator[](size_t i) const { return expr[i] * scalar; }
};

// Translate by scalar expression
template <typename FF, typename E> struct ExprTranslate : public Expr<FF, ExprTranslate<FF, E>> {

    static constexpr size_t DEGREE = E::DEGREE;
    static constexpr size_t SIZE = E::SIZE;

    E const& expr;
    FF const& scalar;

    ExprTranslate(E const& expr, FF const& scalar)
        : expr(expr)
        , scalar(scalar)
    {}

    decltype(auto) operator[](size_t i) const { return expr[i] + scalar; }
};

/**
 * Overload all the arithmetic operators. Instead of actually performing computation, these are essentially
 constructors
 * for each arithmetic expression type. We need to cast the expressions to their subclasses, which we can do with the
 * `self()` operator.
 */

template <typename FF, typename E1, typename E2>
ExprSum<FF, E1, E2> operator+(Expr<FF, E1> const& expr1, Expr<FF, E2> const& expr2)
{
    return ExprSum<FF, E1, E2>(expr1.self(), expr2.self());
}

template <typename FF, typename E1, typename E2>
ExprDiff<FF, E1, E2> operator-(Expr<FF, E1> const& expr1, Expr<FF, E2> const& expr2)
{
    return ExprDiff<FF, E1, E2>(expr1.self(), expr2.self());
}

template <typename FF, typename E1, typename E2>
ExprMul<FF, E1, E2> operator*(Expr<FF, E1> const& expr1, Expr<FF, E2> const& expr2)
{
    return ExprMul<FF, E1, E2>(expr1.self(), expr2.self());
}

template <typename FF, typename E> ExprScale<FF, E> operator*(Expr<FF, E> const& expr, FF const& scalar)
{
    return ExprScale<FF, E>(expr.self(), scalar);
}

template <typename FF, typename E> ExprScale<FF, E> operator*(FF const& scalar, Expr<FF, E> const& expr)
{
    return ExprScale<FF, E>(expr.self(), scalar);
}

template <typename FF, typename E> ExprTranslate<FF, E> operator+(Expr<FF, E> const& expr, FF const& scalar)
{
    return ExprTranslate<FF, E>(expr.self(), scalar);
}

template <typename FF, typename E> ExprTranslate<FF, E> operator+(FF const& scalar, Expr<FF, E> const& expr)
{
    return ExprTranslate<FF, E>(expr.self(), scalar);
}

} // namespace honk::sumcheck
