#pragma once
#include <array>
#include <tuple>

#include "../polynomials/univariate.hpp"
#include "relation_parameters.hpp"

namespace proof_system::honk::sumcheck {

// Helper struct to allow passing an arbitrary collection of lengths to the AccumulatorTypes
template <size_t... Values> struct LengthsWrapper {};

// Forward declarations of AccumulatorTypesHelpers
template <typename FF, typename LengthsWrapper> struct UnivariateAccumulatorTypesHelper;
template <typename FF, typename LengthsWrapper> struct ValueAccumulatorTypesHelper;

// Helper to define value (FF) based accumulator types
template <typename FF, size_t... Values> struct ValueAccumulatorTypesHelper<FF, LengthsWrapper<Values...>> {
    using Accumulators = std::array<FF, sizeof...(Values)>;
    using AccumulatorViews = std::array<FF, sizeof...(Values)>;
};

// Accumulator types for values (FFs)
template <typename FF, typename Lengths> using ValueAccumulatorTypes = ValueAccumulatorTypesHelper<FF, Lengths>;

// Helper to define Univariate based accumulator types
template <typename FF, size_t... Values> struct UnivariateAccumulatorTypesHelper<FF, LengthsWrapper<Values...>> {
    using Accumulators = std::tuple<Univariate<FF, Values>...>;
    using AccumulatorViews = std::tuple<UnivariateView<FF, Values>...>;
};

// Accumulator types for Univariates
template <typename FF, typename Lengths>
using UnivariateAccumulatorTypes = UnivariateAccumulatorTypesHelper<FF, Lengths>;

} // namespace proof_system::honk::sumcheck
