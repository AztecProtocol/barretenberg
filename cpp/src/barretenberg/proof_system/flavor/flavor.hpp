/**
 * @file flavor.hpp
 * @brief Base class templates for structures that contain data related to the fundamental polynomials of a Honk
 * variant (a "flavor").
 *
 * @details #Motivation
 * We choose the framework set out in these classes for several reasons.
 * For one, it allows for a large amount of the information of Honk flavor to be read at a glance in a single file.
 *
 * The primary motivation, however, is to reduce the sort loose coupling that is a significant source of complexity in
 * the original plonk code. There, we find many similarly-named entities defined in a wide variety of places (to name
 * some: selector_properties; FooSelectors; PolynomialIndex; the labels given to the polynomial store; the commitment
 * label; inconsistent terminology and notation around these), and it can be difficult to discover or remember the
 * relationships between these. We aim to standardize these, to enfore identical and informative naming, and to prevent
 * the developer having to think very much about the ordering of protocol entities in disparate places.
 *
 * Another motivation is iterate on the polynomial manifest, which is nice in its compatness, but which feels needlessly
 * manual and low-level. In the past, this contained even more boolean parameters, making it quite hard to parse.
 * Looping over the polynomial manifest means extracting a globally-defined "FOO_MANIFEST_SIZE" (the use of "manifest"
 * here is distinct from the manifests in the transcript) and then looping over a C-style array, and manually parsing
 * the various tags. We greatly enrich this structure by using basic C++ OOP functionality. Rather than recording the
 * polynomial source in an enum, we simply group polynomial handles using getter functions in our new class. We get code
 * that is more compact, more legible, and which is safer because it admits ranged `for` loops.
 *
 * Another motivation is proper and clear specification of Honk variants. The flavors are meant to be explicit and
 * easily comparable. The various settings template parameters and things like the ComposerType enum became overloaded
 * in time, and continue to be a point of accumulation for tech debt. We aim to remedy some of this by putting proving
 * system information in the flavor, and circuit construction information in the arithmetization (or larger circuit
 * constructor class).
 *
 * @details #Data model
 * All of the flavor classes derive from a single Entitives_ template, which simply wraps a std::array (we would
 * inherit, but this is unsafe as std::array has a non-virtual destructor). The developer should think of every flavor
 * class as being:
 *  - A std::array<DataType, N> instance.
 *  - An informative name for each entry of that array that is fixed at compile time.
 *  - Some classic metadata like we'd see in plonk (e.g., a circuit size, a refernce string, an evaluation domain).
 *  - A collection of getters that record subsets of the array that are of interest in the Honk variant.
 *
 * Each getter returns a container of HandleType's, where a HandleType is something that is inexpensive to create and
 * lets one view and mutate a DataType instance. The primary example here is that std::span is the handle type chosen
 * for barrtenberg::Polynomial.
 *
 * @details #Some Notes
 *
 * @note It would be ideal to codify more structure in this file and to have it imposed on the actual flavors, but our
 * inheritance model is complicated as it is, and we saw no reasonable way to fix this.
 *
 * @note One asymmetry to note is in the use of the term "key". It is worthwhile to distinguish betwen prover/verifier
 * circuit data, and "keys" that consist of such data augmented with witness data (whether, raw, blinded, or polynomial
 * commitments). Currently the proving key contains witness data, while the verification key does not.
 * TODO(Cody): It would be nice to resolve this but it's not essential.
 *
 * @note The VerifierCommitments classes are not 'tight' in the sense that that the underlying array contains(a few)
 * empty slots. This is a conscious choice to limit complexity at the expense of explicitness. Note that there is very
 * little memory cost here since the DataType size in that case is small.
 *
 * @todo TODO(#394): Folded polynomials should use polynomial class.
 * @todo TODO(#395): Getters should return arrays?
 * @todo TODO(#396): Access specifiers?
 * @todo TODO(#397): Use more handle types?
 * @todo TODO(#398): Selectors should come from arithmetization.
 */

#pragma once
#include <array>
#include <concepts>
#include <vector>
#include "barretenberg/srs/reference_string/reference_string.hpp"
#include "barretenberg/polynomials/evaluation_domain.hpp"
#include "barretenberg/proof_system/types/composer_type.hpp"

namespace proof_system::honk::flavor {

/**
 * @brief Base data class, a wrapper for std::array, from which every flavor class ultimately derives.
 * @details Ideally we could derive from std::array, but that is unsafe because std::array's destructor is
 * non-virtual.
 *
 * @tparam T The underlying data type stored in the array
 * @tparam HandleType The type that will be used to
 * @tparam NUM_ENTITIES The size of the underlying array.
 */
template <typename DataType, typename HandleType, size_t NUM_ENTITIES> class Entities_ {
  public:
    using ArrayType = std::array<DataType, NUM_ENTITIES>;
    ArrayType _data;

    virtual ~Entities_() = default;

    DataType& operator[](size_t idx) { return _data[idx]; };
    typename ArrayType::iterator begin() { return _data.begin(); };
    typename ArrayType::iterator end() { return _data.end(); };

    constexpr size_t size() { return NUM_ENTITIES; };
};

template <typename DataType, typename HandleType, size_t NUM_PRECOMPUTED_ENTITIES>
class PrecomputedEntities_ : public Entities_<DataType, HandleType, NUM_PRECOMPUTED_ENTITIES> {
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
template <typename PrecomputedEntities, typename FF> class ProvingKey_ : public PrecomputedEntities {
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
 * @tparam PrecomputedEntities
 */
template <typename PrecomputedEntities> class VerificationKey_ : public PrecomputedEntities {
  public:
    std::shared_ptr<VerifierReferenceString> vrs;
};

template <typename DataType, typename HandleType, size_t NUM_ALL_ENTITIES>
class AllEntities_ : public Entities_<DataType, DataType, NUM_ALL_ENTITIES> {
  public:
    virtual std::vector<HandleType> get_wires() = 0;
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
