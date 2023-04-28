#pragma once
#include <array>
#include <concepts>
#include <vector>
#include "barretenberg/srs/reference_string/reference_string.hpp"
#include "barretenberg/polynomials/evaluation_domain.hpp"
#include "barretenberg/proof_system/types/composer_type.hpp"

// WORKTODO: Names of these classes
// WORKTODO: Selectors should come from arithmetization.
// WORKTODO: Define special member functions in reasonable way and untangle the bad consequences elsewhere (e.g., in
// WORKTODO: privacy
// SumcheckOutput)
// TODO(Cody): Handle types.
/**
 * WORKTODO: Outline what's going on here, explain the data model and how to think about these classes.
 *
 */
namespace proof_system::honk::flavor {

/**
 * @brief Base data class, a wrapper for std::array, from which every flavor class ultimately derives.
 * @details Ideally we could derive from std::array, but that is unsafe because std::array's destructor is non-virtual.
 *
 * @tparam T The underlying data type stored in the array
 * @tparam HandleType The type that will be used to
 * @tparam NUM_ENTITIES The size of the underlying array.
 */
template <typename DataType, typename HandleType, size_t NUM_ENTITIES> class Data_ {
  public:
    using ArrayType = std::array<DataType, NUM_ENTITIES>;
    ArrayType _data;

    virtual ~Data_() = default;

    DataType& operator[](size_t idx) { return _data[idx]; };
    typename ArrayType::iterator begin() { return _data.begin(); };
    typename ArrayType::iterator end() { return _data.end(); };

    constexpr size_t size() { return NUM_ENTITIES; };
};

template <typename DataType, typename HandleType, size_t NUM_PRECOMPUTED_ENTITIES>
class PrecomputedData_ : public Data_<DataType, HandleType, NUM_PRECOMPUTED_ENTITIES> {
  public:
    size_t circuit_size;
    size_t log_circuit_size;
    size_t num_public_inputs;
    ComposerType composer_type; // TODO(#392)

    virtual std::vector<HandleType> get_selectors() = 0;
    virtual std::vector<HandleType> get_sigma_polynomials() = 0;
    virtual std::vector<HandleType> get_id_polynomials() = 0;
};

// TODO(Cody): Made this public derivation so that I could iterate through
// the selectors
template <typename PrecomputedData, typename FF> class ProvingKey_ : public PrecomputedData {
  public:
    bool contains_recursive_proof;
    std::vector<uint32_t> recursive_proof_public_input_indices;
    std::shared_ptr<ProverReferenceString> crs;
    barretenberg::EvaluationDomain<FF> evaluation_domain;
};

/**
 * @brief Collect all entities (really, views of these) from the protocol in one place.
 * @details No need for a distinction between storage and view classes here.
 *
 * @tparam PrecomputedData
 */
template <typename PrecomputedData> class VerificationKey_ : public PrecomputedData {
  public:
    std::shared_ptr<VerifierReferenceString> vrs;
};

template <typename DataType, typename HandleType, size_t NUM_ALL_ENTITIES>
class AllData_ : public Data_<DataType, DataType, NUM_ALL_ENTITIES> {
  public:
    virtual std::vector<HandleType> get_unshifted() = 0;
    virtual std::vector<HandleType> get_to_be_shifted() = 0;
    virtual std::vector<HandleType> get_shifted() = 0;

    // TODO(Cody): Look for a better solution?
    std::vector<HandleType> get_unshifted_then_shifted()
    {
        std::vector<HandleType> result{ get_unshifted() };
        std::vector<HandleType> shifted{ get_shifted() };
        result.insert(result.end(), shifted.begin(), shifted.end());
        return result;
    };
};

} // namespace proof_system::honk::flavor

// Forward declare honk flavors
namespace proof_system::honk::flavor {
class Standard;
class Ultra;
} // namespace proof_system::honk::flavor

// Forward declare plonk flavors
namespace proof_system::plonk::flavor {
class Standard;
class Turbo;
class Ultra;
} // namespace proof_system::plonk::flavor

// Establish concepts for testing flavor attributes
namespace proof_system {
/**
 * @brief Test whether a type T lies in a list of types ...U.
 *
 * @tparam T The type being tested
 * @tparam U A parameter pack of types being checked against T.
 */
template <typename T, typename... U> concept IsAnyOf = (std::same_as<T, U> || ...);

template <typename T>
concept IsPlonkFlavor = IsAnyOf<T, plonk::flavor::Standard, plonk::flavor::Turbo, plonk::flavor::Ultra>;

template <typename T> concept IsHonkFlavor = IsAnyOf<T, honk::flavor::Standard, honk::flavor::Ultra>;

} // namespace proof_system
