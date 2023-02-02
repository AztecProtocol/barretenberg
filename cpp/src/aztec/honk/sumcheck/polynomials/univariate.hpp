#pragma once
#include <array>
#include <cstdint>
#include <span>
#include <ostream>
#include <common/serialize.hpp>
#include <common/assert.hpp>

namespace honk::sumcheck {

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
    Univariate(const Univariate& other) = default;
    Univariate(Univariate&& other) noexcept = default;

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
    const Fr& value_at(size_t i) const { return evaluations[i]; };

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

    // Operations between Univariate and other Univariate
    Univariate& operator=(const Univariate& other)
    {
        evaluations = other.evaluations;
        return *this;
    }

    Univariate& operator=(Univariate&& other) noexcept
    {
        evaluations = std::move(other.evaluations);
        return *this;
    }

    bool operator==(const Univariate& other) const = default;

    Univariate operator+=(const Univariate& other)
    {
        for (size_t i = 0; i < _length; ++i) {
            evaluations[i] += other.evaluations[i];
        }
        return *this;
    }
    Univariate operator-=(const Univariate& other)
    {
        for (size_t i = 0; i < _length; ++i) {
            evaluations[i] -= other.evaluations[i];
        }
        return *this;
    }
    Univariate operator*=(const Univariate& other)
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
    Univariate operator+=(const Fr& scalar)
    {
        for (auto& eval : evaluations) {
            eval += scalar;
        }
        return *this;
    }

    Univariate operator-=(const Fr& scalar)
    {
        for (auto& eval : evaluations) {
            eval -= scalar;
        }
        return *this;
    }
    Univariate operator*=(const Fr& scalar)
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
    Univariate operator+=(const UnivariateView<Fr, _length>& view)
    {
        for (size_t i = 0; i < _length; ++i) {
            evaluations[i] += view.evaluations[i];
        }
        return *this;
    }

    Univariate operator-=(const UnivariateView<Fr, _length>& view)
    {
        for (size_t i = 0; i < _length; ++i) {
            evaluations[i] -= view.evaluations[i];
        }
        return *this;
    }

    Univariate operator*=(const UnivariateView<Fr, _length>& view)
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
    std::span<const Fr, view_length> evaluations;

    UnivariateView() = default;

    const Fr& value_at(size_t i) const { return evaluations[i]; };

    template <size_t full_length>
    explicit UnivariateView(const Univariate<Fr, full_length>& univariate_in)
        : evaluations(std::span<const Fr>(univariate_in.evaluations.data(), view_length)){};

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

/**
 * @brief Helper method for `array_to_array`
 * @details The output array is created by unpacking the index_sequence of the same length as `elements`
 */
template <typename T, typename U, std::size_t N, std::size_t... Is>
std::array<T, N> array_to_array_aux(const std::array<U, N>& elements, std::index_sequence<Is...>)
{
    return { { T{ elements[Is] }... } };
};

/**
 * @brief Given an std::array<U,N>, returns an std::array<T,N>, by calling the (explicit) constructor T(U).
 *
 * @details https://stackoverflow.com/a/32175958
 *
 * @tparam T Output type
 * @tparam U Input type (deduced from `elements`)
 * @tparam N Common array size (deduced from `elements`)
 * @param elements array to be converted
 * @return std::array<T, N>
 */
template <typename T, typename U, std::size_t N> std::array<T, N> array_to_array(const std::array<U, N>& elements)
{
    // Calls the aux method that uses the index sequence to unpack all values in `elements`
    return array_to_array_aux<T, U, N>(elements, std::make_index_sequence<N>());
};

/**
 * @brief Given an array of Univariates, create a new array containing only the i-th evaluations
 * of all the univariates.
 *
 * @note Not really optimized, mainly used for testing that the relations evaluate to the same value when
 * evaluated as Univariates, Expressions, or index-by-index
 *
 * @tparam FF field (deduced from `univariates`)
 * @tparam NUM_UNIVARIATES number of univariates in the input array (deduced from `univariates`)
 * @tparam univariate_length number of evaluations (deduced from `univariates`)
 * @param univariates array of Univariates
 * @param i index of the evaluations we want to take from each univariate
 * @return std::array<FF, NUM_UNIVARIATES> such that result[j] = univariates[j].value_at(i)
 */
template <typename FF, std::size_t NUM_UNIVARIATES, size_t univariate_length>
std::array<FF, NUM_UNIVARIATES> transposed_univariate_array_at(
    const std::array<Univariate<FF, univariate_length>, NUM_UNIVARIATES>& univariates, size_t i)
{
    ASSERT(i < univariate_length);
    std::array<FF, NUM_UNIVARIATES> result;
    for (size_t j = 0; j < NUM_UNIVARIATES; ++j) {
        result[j] = univariates[j].value_at(i);
    }
    return result;
};

} // namespace honk::sumcheck
