#pragma once
#include <array>
#include <cstdint>
#include <span>
#include <ostream>
#include <common/serialize.hpp>
#include <common/assert.hpp>

namespace honk::sumcheck {

template <typename FF, typename E> struct Expr {
    FF operator[](size_t i) const { return static_cast<E const&>(*this)[i]; }
    constexpr size_t degree() const { return static_cast<E const&>(*this).degree(); }
    constexpr size_t size() const { return static_cast<E const&>(*this).size(); }
};

template <typename FF, size_t N> struct Poly : public Expr<FF, Poly<FF, N>> {

    std::array<FF, N> evals;

    Poly() = default;

    Poly(FF a)
    {
        for (size_t i = 0; i < N; ++i) {
            evals[i] = a;
        }
    }

    template <typename P> Poly(Expr<FF, P> const& p)
    {
        for (size_t i = 0; i < N; ++i) {
            evals[i] = p[i];
        }
    }

    FF& operator[](size_t i) { return evals[i]; }
    FF operator[](size_t i) const { return evals[i]; }
    constexpr size_t degree() const { return 1; }
    constexpr size_t size() const { return N; }
};

template <typename FF, typename P1, typename P2> struct PolySum : public Expr<FF, PolySum<FF, P1, P2>> {
    P1 const& p1;
    P2 const& p2;

    PolySum(P1 const& p1, P2 const& p2)
        : p1(p1)
        , p2(p2)
    {}

    FF operator[](size_t i) const { return p1[i] + p2[i]; }

    constexpr size_t degree() const { return std::max(p1.degree(), p2.degree()); }

    constexpr size_t size() const { return std::min(p1.size(), p2.size()); }
};

template <typename FF, typename P1, typename P2>
PolySum<FF, P1, P2> operator+(Expr<FF, P1> const& p1, Expr<FF, P2> const& p2)
{
    return PolySum<FF, P1, P2>(static_cast<const P1&>(p1), static_cast<const P2&>(p2));
}

template <typename FF, typename P1, typename P2> struct PolyDiff : public Expr<FF, PolyDiff<FF, P1, P2>> {
    P1 const& p1;
    P2 const& p2;

    PolyDiff(P1 const& p1, P2 const& p2)
        : p1(p1)
        , p2(p2)
    {}

    FF operator[](size_t i) const { return p1[i] - p2[i]; }

    constexpr size_t degree() const { return std::max(p1.degree(), p2.degree()); }

    constexpr size_t size() const { return std::min(p1.size(), p2.size()); }
};

template <typename FF, typename P1, typename P2>
PolyDiff<FF, P1, P2> operator-(Expr<FF, P1> const& p1, Expr<FF, P2> const& p2)
{
    return PolyDiff<FF, P1, P2>(static_cast<const P1&>(p1), static_cast<const P2&>(p2));
}
template <typename FF, typename P1, typename P2> struct PolyMul : public Expr<FF, PolyMul<FF, P1, P2>> {
    P1 const& p1;
    P2 const& p2;

    PolyMul(P1 const& p1, P2 const& p2)
        : p1(p1)
        , p2(p2)
    {}

    FF operator[](size_t i) const { return p1[i] * p2[i]; }

    constexpr size_t degree() const { return p1.degree() + p2.degree(); }
    constexpr size_t size() const { return std::min(p1.size(), p2.size()); }
};

template <typename FF, typename P1, typename P2>
PolyMul<FF, P1, P2> operator*(Expr<FF, P1> const& p1, Expr<FF, P2> const& p2)
{
    return PolyMul<FF, P1, P2>(static_cast<const P1&>(p1), static_cast<const P2&>(p2));
}

template <typename FF, typename P> struct PolyScale : public Expr<FF, PolyScale<FF, P>> {
    P const& p;
    FF const& scalar;

    PolyScale(P const& p, FF const& scalar)
        : p(p)
        , scalar(scalar)
    {}

    FF operator[](size_t i) const { return p[i] * scalar; }

    constexpr size_t degree() const { return p.degree(); }
    constexpr size_t size() const { return p.size(); }
};

template <typename FF, typename P> PolyScale<FF, P> operator*(Expr<FF, P> const& p, FF const& scalar)
{
    return PolyScale<FF, P>(static_cast<const P&>(p), static_cast<const FF&>(scalar));
}
template <typename FF, typename P> PolyScale<FF, P> operator*(FF const& scalar, Expr<FF, P> const& p)
{
    return PolyScale<FF, P>(static_cast<const P&>(p), static_cast<const FF&>(scalar));
}

template <class Fr, size_t view_length> class UnivariateView;

// TODO(Cody): This violates Rule of Five
template <class Fr, size_t _length> class Univariate {
  public:
    static constexpr size_t LENGTH = _length;

    std::array<Fr, _length> evaluations;

    Univariate() = default;

    explicit Univariate(std::array<Fr, _length> evaluations)
        : evaluations(evaluations)
    {}

    // Construct Univariate from scalar
    explicit Univariate(Fr value)
        : evaluations{}
    {
        for (size_t i = 0; i < _length; ++i) {
            evaluations[i] = value;
        }
    }
    // Construct Univariate from UnivariateView
    explicit Univariate(UnivariateView<Fr, _length> in)
        : evaluations{}
    {
        for (size_t i = 0; i < in.evaluations.size(); ++i) {
            evaluations[i] = in.evaluations[i];
        }
    }

    Fr& value_at(size_t i) { return evaluations[i]; };

    // Write the Univariate evaluations to a buffer
    std::vector<uint8_t> to_buffer() const { return ::to_buffer(evaluations); }

    // Static method for creating a Univariate from a buffer
    // IMPROVEMENT: Could be made to identically match equivalent methods in e.g. field.hpp. Currently bypasses
    // unnecessary ::from_buffer call
    static Univariate serialize_from_buffer(uint8_t const* buffer)
    {
        Univariate result;
        std::read(buffer, result.evaluations);
        return result;
    }

    bool operator==(const Univariate& other) const = default;

    Univariate& operator+=(const Univariate& other)
    {
        for (size_t i = 0; i < _length; ++i) {
            evaluations[i] += other.evaluations[i];
        }
        return *this;
    }
    Univariate& operator-=(const Univariate& other)
    {
        for (size_t i = 0; i < _length; ++i) {
            evaluations[i] -= other.evaluations[i];
        }
        return *this;
    }
    Univariate& operator*=(const Univariate& other)
    {
        for (size_t i = 0; i < _length; ++i) {
            evaluations[i] *= other.evaluations[i];
        }
        return *this;
    }
    Univariate operator+(const Univariate& other) const
    {
        Univariate res(*this);
        res += other;
        return res;
    }

    Univariate operator-(const Univariate& other) const
    {
        Univariate res(*this);
        res -= other;
        return res;
    }
    Univariate operator*(const Univariate& other) const
    {
        Univariate res(*this);
        res *= other;
        return res;
    }

    // Operations between Univariate and scalar
    Univariate& operator+=(const Fr& scalar)
    {
        for (auto& eval : evaluations) {
            eval += scalar;
        }
        return *this;
    }

    Univariate& operator-=(const Fr& scalar)
    {
        for (auto& eval : evaluations) {
            eval -= scalar;
        }
        return *this;
    }
    Univariate& operator*=(const Fr& scalar)
    {
        for (auto& eval : evaluations) {
            eval *= scalar;
        }
        return *this;
    }

    Univariate operator+(const Fr& scalar)
    {
        Univariate res(*this);
        res += scalar;
        return res;
    }

    Univariate operator-(const Fr& scalar)
    {
        Univariate res(*this);
        res -= scalar;
        return res;
    }

    Univariate operator*(const Fr& scalar)
    {
        Univariate res(*this);
        res *= scalar;
        return res;
    }

    // Operations between Univariate and UnivariateView
    Univariate& operator+=(const UnivariateView<Fr, _length>& view)
    {
        for (size_t i = 0; i < _length; ++i) {
            evaluations[i] += view.evaluations[i];
        }
        return *this;
    }

    Univariate& operator-=(const UnivariateView<Fr, _length>& view)
    {
        for (size_t i = 0; i < _length; ++i) {
            evaluations[i] -= view.evaluations[i];
        }
        return *this;
    }

    Univariate& operator*=(const UnivariateView<Fr, _length>& view)
    {
        for (size_t i = 0; i < _length; ++i) {
            evaluations[i] *= view.evaluations[i];
        }
        return *this;
    }

    Univariate operator+(const UnivariateView<Fr, _length>& view)
    {
        Univariate res(*this);
        res += view;
        return res;
    }

    Univariate operator-(const UnivariateView<Fr, _length>& view)
    {
        Univariate res(*this);
        res -= view;
        return res;
    }

    Univariate operator*(const UnivariateView<Fr, _length>& view)
    {
        Univariate res(*this);
        res *= view;
        return res;
    }

    // Output is immediately parsable as a list of integers by Python.
    friend std::ostream& operator<<(std::ostream& os, const Univariate& u)
    {
        os << "[";
        os << u.evaluations[0] << "," << std::endl;
        for (size_t i = 1; i < u.evaluations.size(); i++) {
            os << " " << u.evaluations[i];
            if (i + 1 < u.evaluations.size()) {
                os << "," << std::endl;
            } else {
                os << "]";
            };
        }
        return os;
    }
};

template <class Fr, size_t view_length> class UnivariateView {
  public:
    std::span<Fr, view_length> evaluations;

    UnivariateView() = default;

    Fr& value_at(size_t i) { return evaluations[i]; };

    template <size_t full_length>
    explicit UnivariateView(Univariate<Fr, full_length>& univariate_in)
        : evaluations(std::span<Fr>(univariate_in.evaluations.begin(), view_length)){};

    Univariate<Fr, view_length> operator+(const UnivariateView& other) const
    {
        Univariate<Fr, view_length> res(*this);
        res += other;
        return res;
    }

    Univariate<Fr, view_length> operator-(const UnivariateView& other) const
    {
        Univariate<Fr, view_length> res(*this);
        res -= other;
        return res;
    }

    Univariate<Fr, view_length> operator*(const UnivariateView& other) const
    {
        Univariate<Fr, view_length> res(*this);
        res *= other;
        return res;
    }

    Univariate<Fr, view_length> operator*(const Univariate<Fr, view_length>& other) const
    {
        Univariate<Fr, view_length> res(*this);
        res *= other;
        return res;
    }

    Univariate<Fr, view_length> operator+(const Univariate<Fr, view_length>& other) const
    {
        Univariate<Fr, view_length> res(*this);
        res += other;
        return res;
    }

    Univariate<Fr, view_length> operator+(const Fr& other) const
    {
        Univariate<Fr, view_length> res(*this);
        res += other;
        return res;
    }

    Univariate<Fr, view_length> operator-(const Fr& other) const
    {
        Univariate<Fr, view_length> res(*this);
        res -= other;
        return res;
    }

    Univariate<Fr, view_length> operator*(const Fr& other) const
    {
        Univariate<Fr, view_length> res(*this);
        res *= other;
        return res;
    }

    // Output is immediately parsable as a list of integers by Python.
    friend std::ostream& operator<<(std::ostream& os, const UnivariateView& u)
    {
        os << "[";
        os << u.evaluations[0] << "," << std::endl;
        for (size_t i = 1; i < u.evaluations.size(); i++) {
            os << " " << u.evaluations[i];
            if (i + 1 < u.evaluations.size()) {
                os << "," << std::endl;
            } else {
                os << "]";
            };
        }
        return os;
    }
};
} // namespace honk::sumcheck
