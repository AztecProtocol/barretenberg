#pragma once

#include "barretenberg/srs/reference_string/file_reference_string.hpp"
#include "barretenberg/proof_system/proving_key/proving_key.hpp"
#include "barretenberg/plonk/proof_system/prover/prover.hpp"
#include "barretenberg/plonk/proof_system/verifier/verifier.hpp"
#include "barretenberg/proof_system/circuit_constructors/standard_circuit_constructor.hpp"
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/proof_system/verification_key/verification_key.hpp"
#include "barretenberg/plonk/proof_system/verifier/verifier.hpp"
#include "barretenberg/proof_system/composer/composer_base.hpp"
#include "barretenberg/proof_system/composer/composer_helper_lib.hpp"
#include "barretenberg/proof_system/composer/permutation_helper.hpp"

#include <utility>

namespace bonk {
// TODO(Kesha): change initializations to specify this parameter
// Cody: What does this mean?
template <typename CircuitConstructor> class UltraPlonkComposerHelper {
  public:
    static constexpr size_t NUM_RANDOMIZED_GATES = 2; // equal to the number of multilinear evaluations leaked
    static constexpr size_t program_width = CircuitConstructor::program_width;
    std::shared_ptr<bonk::proving_key> circuit_proving_key;
    std::shared_ptr<bonk::verification_key> circuit_verification_key;
    // TODO(#218)(kesha): we need to put this into the commitment key, so that the composer doesn't have to handle srs
    // at all
    std::shared_ptr<bonk::ReferenceStringFactory> crs_factory_;

    std::vector<uint32_t> recursive_proof_public_input_indices;
    bool contains_recursive_proof = false;
    bool computed_witness = false;
    bool circuit_finalised = false; // TODO(luke): Does this belong here or does this break our separation..

    UltraPlonkComposerHelper()
        : UltraPlonkComposerHelper("../srs_db/ignition")
    {}

    UltraPlonkComposerHelper(std::string const& crs_path)
        : UltraPlonkComposerHelper(std::unique_ptr<ReferenceStringFactory>(new FileReferenceStringFactory(crs_path))){};

    UltraPlonkComposerHelper(std::shared_ptr<ReferenceStringFactory> const crs_factory)
        : crs_factory_(std::move(crs_factory))
    {}

    UltraPlonkComposerHelper(std::shared_ptr<proving_key> p_key, std::shared_ptr<verification_key> v_key)
        : circuit_proving_key(std::move(p_key))
        , circuit_verification_key(std::move(v_key))
    {}

    UltraPlonkComposerHelper(UltraPlonkComposerHelper&& other) = default;
    UltraPlonkComposerHelper& operator=(UltraPlonkComposerHelper&& other) = default;
    ~UltraPlonkComposerHelper() {}

    std::vector<bonk::SelectorProperties> ultra_selector_properties()
    {
        // When reading and writing the proving key from a buffer we must precompute the Lagrange form of certain
        // selector polynomials. In order to avoid a new selector type and definitions in the polynomial manifest, we
        // can instead store the Lagrange forms of all the selector polynomials.
        //
        // This workaround increases the memory footprint of the prover, and is a possible place of improvement in the
        // future. Below is the previous state showing where the Lagrange form is necessary for a selector:
        //     { "q_m", true },         { "q_c", true },    { "q_1", true },        { "q_2", true },
        //     { "q_3", true },         { "q_4", false },   { "q_arith", false },   { "q_sort", false },
        //     { "q_elliptic", false }, { "q_aux", false }, { "table_type", true },
        std::vector<bonk::SelectorProperties> result{
            { "q_m", true },        { "q_c", true },   { "q_1", true },        { "q_2", true },
            { "q_3", true },        { "q_4", true },   { "q_arith", true },    { "q_sort", true },
            { "q_elliptic", true }, { "q_aux", true }, { "table_type", true },
        };
        return result;
    }

    std::shared_ptr<bonk::proving_key> compute_proving_key(const CircuitConstructor& circuit_constructor);
    std::shared_ptr<bonk::verification_key> compute_verification_key(const CircuitConstructor& circuit_constructor);
};

} // namespace bonk
