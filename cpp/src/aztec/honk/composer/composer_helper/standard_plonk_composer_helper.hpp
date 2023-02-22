#pragma once

#include <srs/reference_string/file_reference_string.hpp>
#include <proof_system/proving_key/proving_key.hpp>
#include <plonk/proof_system/prover/prover.hpp>
#include <plonk/proof_system/verifier/verifier.hpp>
#include <honk/circuit_constructors/standard_circuit_constructor.hpp>
#include <honk/pcs/commitment_key.hpp>
#include <proof_system/verification_key/verification_key.hpp>
#include <plonk/proof_system/verifier/verifier.hpp>
#include <proof_system/composer/composer_base.hpp>
#include "composer_helper_lib.hpp"
#include "permutation_helper.hpp"

#include <utility>

namespace honk {
// TODO(Kesha): change initializations to specify this parameter
// Cody: What does this mean?
template <typename CircuitConstructor> class StandardPlonkComposerHelper {
  public:
    static constexpr size_t NUM_RANDOMIZED_GATES = 2; // equal to the number of multilinear evaluations leaked
    static constexpr size_t program_width = CircuitConstructor::program_width;
    std::shared_ptr<waffle::proving_key> circuit_proving_key;
    std::shared_ptr<waffle::verification_key> circuit_verification_key;
    // TODO(kesha): we need to put this into the commitment key, so that the composer doesn't have to handle srs at all
    std::shared_ptr<waffle::ReferenceStringFactory> crs_factory_;

    std::vector<uint32_t> recursive_proof_public_input_indices;
    bool contains_recursive_proof = false;
    bool computed_witness = false;

    StandardPlonkComposerHelper()
        : StandardPlonkComposerHelper(std::shared_ptr<waffle::ReferenceStringFactory>(
              new waffle::FileReferenceStringFactory("../srs_db/ignition")))
    {}
    StandardPlonkComposerHelper(std::shared_ptr<waffle::ReferenceStringFactory> crs_factory)
        : crs_factory_(std::move(crs_factory))
    {}

    StandardPlonkComposerHelper(std::unique_ptr<waffle::ReferenceStringFactory>&& crs_factory)
        : crs_factory_(std::move(crs_factory))
    {}
    StandardPlonkComposerHelper(std::shared_ptr<waffle::proving_key> p_key,
                                std::shared_ptr<waffle::verification_key> v_key)
        : circuit_proving_key(std::move(p_key))
        , circuit_verification_key(std::move(v_key))
    {}
    StandardPlonkComposerHelper(StandardPlonkComposerHelper&& other) noexcept = default;
    StandardPlonkComposerHelper(const StandardPlonkComposerHelper& other) = delete;
    StandardPlonkComposerHelper& operator=(StandardPlonkComposerHelper&& other) noexcept = default;
    StandardPlonkComposerHelper& operator=(const StandardPlonkComposerHelper& other) = delete;
    ~StandardPlonkComposerHelper() = default;

    inline std::vector<SelectorProperties> standard_selector_properties()
    {
        std::vector<SelectorProperties> result{
            { "q_m", false }, { "q_c", false }, { "q_1", false }, { "q_2", false }, { "q_3", false },
        };
        return result;
    }
    void add_recursive_proof(CircuitConstructor& circuit_constructor,
                             const std::vector<uint32_t>& proof_output_witness_indices)
    {

        if (contains_recursive_proof) {
            circuit_constructor.failure("added recursive proof when one already exists");
        }
        contains_recursive_proof = true;

        for (const auto& idx : proof_output_witness_indices) {
            circuit_constructor.set_public_input(idx);
            recursive_proof_public_input_indices.push_back((uint32_t)(circuit_constructor.public_inputs.size() - 1));
        }
    }
    std::shared_ptr<waffle::proving_key> compute_proving_key(const CircuitConstructor& circuit_constructor);
    std::shared_ptr<waffle::verification_key> compute_verification_key(const CircuitConstructor& circuit_constructor);

    void compute_witness(const CircuitConstructor& circuit_constructor) { compute_witness_base(circuit_constructor); }

    waffle::Verifier create_verifier(const CircuitConstructor& circuit_constructor);
    /**
     * Preprocess the circuit. Delegates to create_prover.
     *
     * @return A new initialized prover.
     */
    waffle::Prover preprocess(const CircuitConstructor& circuit_constructor)
    {
        return create_prover(circuit_constructor);
    };
    waffle::Prover create_prover(const CircuitConstructor& circuit_constructor);

    waffle::UnrolledVerifier create_unrolled_verifier(const CircuitConstructor& circuit_constructor);

    template <typename Flavor>
    waffle::UnrolledProver create_unrolled_prover(const CircuitConstructor& circuit_constructor);
    // TODO(Adrian): Seems error prone to provide the number of randomized gates
    // Cody: Where should this go? In the flavor (or whatever that becomes)?
    std::shared_ptr<waffle::proving_key> compute_proving_key_base(
        const CircuitConstructor& circuit_constructor,
        const size_t minimum_ciricut_size = 0,
        const size_t num_randomized_gates = NUM_RANDOMIZED_GATES);
    // This needs to be static as it may be used only to compute the selector commitments.

    static std::shared_ptr<waffle::verification_key> compute_verification_key_base(
        std::shared_ptr<waffle::proving_key> const& proving_key,
        std::shared_ptr<waffle::VerifierReferenceString> const& vrs);

    void compute_witness_base(const CircuitConstructor& circuit_constructor, const size_t minimum_circuit_size = 0);
    /**
     * Create a manifest, which specifies proof rounds, elements and who supplies them.
     *
     * @param num_public_inputs The number of public inputs.
     *
     * @return Constructed manifest.
     * */
    static transcript::Manifest create_manifest(const size_t num_public_inputs)
    {
        // add public inputs....
        constexpr size_t g1_size = 64;
        constexpr size_t fr_size = 32;
        const size_t public_input_size = fr_size * num_public_inputs;
        const transcript::Manifest output = transcript::Manifest(

            { transcript::Manifest::RoundManifest(
                  { { "circuit_size", 4, true }, { "public_input_size", 4, true } }, "init", 1),

              transcript::Manifest::RoundManifest({}, "eta", 0),

              transcript::Manifest::RoundManifest(
                  {
                      { "public_inputs", public_input_size, false },
                      { "W_1", g1_size, false },
                      { "W_2", g1_size, false },
                      { "W_3", g1_size, false },
                  },
                  "beta",
                  2),
              transcript::Manifest::RoundManifest({ { "Z_PERM", g1_size, false } }, "alpha", 1),
              transcript::Manifest::RoundManifest(
                  { { "T_1", g1_size, false }, { "T_2", g1_size, false }, { "T_3", g1_size, false } }, "z", 1),

              transcript::Manifest::RoundManifest(
                  {
                      { "w_1", fr_size, false, 0 },
                      { "w_2", fr_size, false, 1 },
                      { "w_3", fr_size, false, 2 },
                      { "sigma_1", fr_size, false, 3 },
                      { "sigma_2", fr_size, false, 4 },
                      { "z_perm_omega", fr_size, false, -1 },
                  },
                  "nu",
                  waffle::STANDARD_UNROLLED_MANIFEST_SIZE - 6,
                  true),

              transcript::Manifest::RoundManifest(
                  { { "PI_Z", g1_size, false }, { "PI_Z_OMEGA", g1_size, false } }, "separator", 1) });
        return output;
    }

    static transcript::Manifest create_unrolled_manifest(const size_t num_public_inputs)
    {
        constexpr size_t g1_size = 64;
        constexpr size_t fr_size = 32;
        const size_t public_input_size = fr_size * num_public_inputs;
        /*  A RoundManifest describes data that will be put in or extracted from a transcript.
            Here we have 7 RoundManifests. */
        const transcript::Manifest output = transcript::Manifest(
            { // clang-format off

              // Round 0
              transcript::Manifest::RoundManifest(
                { 
                  { .name = "circuit_size",      .num_bytes = 4, .derived_by_verifier = true },
                  { .name = "public_input_size", .num_bytes = 4, .derived_by_verifier = true }
                },
                /* challenge_name = */ "init",
                /* num_challenges_in = */ 1),

              // Round 1
              transcript::Manifest::RoundManifest(
                {}, 
                /* challenge_name = */ "eta", 
                /* num_challenges_in = */ 0),

              // Round 2
              transcript::Manifest::RoundManifest(
                {
                    { .name = "public_inputs", .num_bytes = public_input_size, .derived_by_verifier = false },
                    { .name = "W_1",           .num_bytes = g1_size,           .derived_by_verifier = false },
                    { .name = "W_2",           .num_bytes = g1_size,           .derived_by_verifier = false },
                    { .name = "W_3",           .num_bytes = g1_size,           .derived_by_verifier = false },
                },
                /* challenge_name = */ "beta",
                /* num_challenges_in = */ 2),

              // Round 3
              transcript::Manifest::RoundManifest(
                { { .name = "Z_PERM", .num_bytes = g1_size, .derived_by_verifier = false } }, 
                /* challenge_name = */ "alpha",
                /* num_challenges_in = */ 1),

              // Round 4
              transcript::Manifest::RoundManifest(
                { { .name = "T_1", .num_bytes = g1_size, .derived_by_verifier = false },
                  { .name = "T_2", .num_bytes = g1_size, .derived_by_verifier = false },
                  { .name = "T_3", .num_bytes = g1_size, .derived_by_verifier = false } },
                /* challenge_name = */ "z",
                /* num_challenges_in = */ 1),

              // Round 5
              transcript::Manifest::RoundManifest(
                {
                    { .name = "t",            .num_bytes = fr_size, .derived_by_verifier = true,  .challenge_map_index = -1 },
                    { .name = "w_1",          .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 0 },
                    { .name = "w_2",          .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 1 },
                    { .name = "w_3",          .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 2 },
                    { .name = "sigma_1",      .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 3 },
                    { .name = "sigma_2",      .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 4 },
                    { .name = "sigma_3",      .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 5 },
                    { .name = "q_1",          .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 6 },
                    { .name = "q_2",          .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 7 },
                    { .name = "q_3",          .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 8 },
                    { .name = "q_m",          .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 9 },
                    { .name = "q_c",          .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 10 },
                    { .name = "z_perm",       .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 11 },
                    { .name = "z_perm_omega", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = -1 },
                },
                /* challenge_name = */ "nu",
                /* num_challenges_in = */ waffle::STANDARD_UNROLLED_MANIFEST_SIZE,
                /* map_challenges_in = */ true),

              // Round 6
              transcript::Manifest::RoundManifest(
                { { .name = "PI_Z",       .num_bytes = g1_size, .derived_by_verifier = false },
                  { .name = "PI_Z_OMEGA", .num_bytes = g1_size, .derived_by_verifier = false } },
                /* challenge_name = */ "separator",
                /* num_challenges_in = */ 1) }

            // clang-format off
    );
        return output;
    }
};

} // namespace honk