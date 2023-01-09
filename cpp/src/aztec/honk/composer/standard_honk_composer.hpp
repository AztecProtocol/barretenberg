#include "../circuit_constructors/standard_circuit_constructor.hpp"
#include "composer_helper/composer_helper.hpp"
#include <cstddef>
#include <srs/reference_string/file_reference_string.hpp>
#include <string>
#include <transcript/manifest.hpp>
#include <proof_system/flavor/flavor.hpp>

namespace honk {
/**
 * @brief Standard Honk Composer has everything required to construct a prover and verifier, just as the legacy classes.
 *
 * @details However, it has a lot of its logic separated into subclasses and simply proxies the calls.
 *
 */
class StandardHonkComposer {
  public:
    static constexpr waffle::ComposerType type = waffle::ComposerType::STANDARD;

    static constexpr size_t UINT_LOG2_BASE = 2;
    // An instantiation of the circuit constructor that only depends on arithmetization, not  on the proof system
    StandardCircuitConstructor circuit_constructor;
    // Composer helper contains all proof-related material that is separate from circuit creation such as:
    // 1) Proving and verification keys
    // 2) CRS
    // 3) Converting variables to witness vectors/polynomials
    ComposerHelper<StandardCircuitConstructor> composer_helper;

    // Leaving it in for now just in case
    bool contains_recursive_proof = false;
    static constexpr size_t program_width = STANDARD_HONK_WIDTH;

    /**Standard methods*/

    StandardHonkComposer(const size_t size_hint = 0)
        : circuit_constructor(size_hint){};

    StandardHonkComposer(std::string const& crs_path, const size_t size_hint = 0)
        : StandardHonkComposer(
              std::unique_ptr<waffle::ReferenceStringFactory>(new waffle::FileReferenceStringFactory(crs_path)),
              size_hint){};

    StandardHonkComposer(std::shared_ptr<waffle::ReferenceStringFactory> const& crs_factory, const size_t size_hint = 0)
        : circuit_constructor(size_hint)
        , composer_helper(crs_factory)

    {}
    StandardHonkComposer(std::unique_ptr<waffle::ReferenceStringFactory>&& crs_factory, const size_t size_hint = 0)
        : circuit_constructor(size_hint)
        , composer_helper(std::move(crs_factory))

    {}

    StandardHonkComposer(std::shared_ptr<waffle::proving_key> const& p_key,
                         std::shared_ptr<waffle::verification_key> const& v_key,
                         size_t size_hint = 0)
        : circuit_constructor(size_hint)
        , composer_helper(p_key, v_key)
    {}

    StandardHonkComposer(StandardHonkComposer&& other) = default;
    StandardHonkComposer& operator=(StandardHonkComposer&& other) = default;
    ~StandardHonkComposer() {}

    /**Methods related to circuit construction
     * They simply get proxied to the circuit constructor
     */
    void assert_equal(const uint32_t a_variable_idx, const uint32_t b_variable_idx, std::string const& msg)
    {
        circuit_constructor.assert_equal(a_variable_idx, b_variable_idx, msg);
    }
    void assert_equal_constant(uint32_t const a_idx,
                               barretenberg::fr const& b,
                               std::string const& msg = "assert equal constant")
    {
        circuit_constructor.assert_equal_constant(a_idx, b, msg);
    }

    void create_add_gate(const add_triple& in) { circuit_constructor.create_add_gate(in); }
    void create_mul_gate(const mul_triple& in) { circuit_constructor.create_mul_gate(in); }
    void create_bool_gate(const uint32_t a) { circuit_constructor.create_bool_gate(a); }
    void create_poly_gate(const poly_triple& in) { circuit_constructor.create_poly_gate(in); }
    void create_big_add_gate(const add_quad& in) { circuit_constructor.create_big_add_gate(in); }
    void create_big_add_gate_with_bit_extraction(const add_quad& in)
    {
        circuit_constructor.create_big_add_gate_with_bit_extraction(in);
    }
    void create_big_mul_gate(const mul_quad& in) { circuit_constructor.create_big_mul_gate(in); }
    void create_balanced_add_gate(const add_quad& in) { circuit_constructor.create_balanced_add_gate(in); }
    void create_fixed_group_add_gate(const fixed_group_add_quad& in)
    {
        circuit_constructor.create_fixed_group_add_gate(in);
    }
    void create_fixed_group_add_gate_with_init(const fixed_group_add_quad& in, const fixed_group_init_quad& init)
    {
        circuit_constructor.create_fixed_group_add_gate_with_init(in, init);
    }
    void create_fixed_group_add_gate_final(const add_quad& in)
    {
        circuit_constructor.create_fixed_group_add_gate_final(in);
    }

    void fix_witness(const uint32_t witness_index, const barretenberg::fr& witness_value)
    {
        circuit_constructor.fix_witness(witness_index, witness_value);
    }

    std::vector<uint32_t> decompose_into_base4_accumulators(const uint32_t witness_index,

                                                            const size_t num_bits,
                                                            std::string const& msg = "create_range_constraint")
    {
        return circuit_constructor.decompose_into_base4_accumulators(witness_index, num_bits, msg);
    }

    void create_range_constraint(const uint32_t variable_index,
                                 const size_t num_bits,
                                 std::string const& msg = "create_range_constraint")
    {
        circuit_constructor.create_range_constraint(variable_index, num_bits, msg);
    }

    accumulator_triple create_logic_constraint(const uint32_t a,
                                               const uint32_t b,
                                               const size_t num_bits,
                                               bool is_xor_gate)
    {
        return circuit_constructor.create_logic_constraint(a, b, num_bits, is_xor_gate);
    }

    accumulator_triple create_and_constraint(const uint32_t a, const uint32_t b, const size_t num_bits)
    {
        return circuit_constructor.create_and_constraint(a, b, num_bits);
    }

    accumulator_triple create_xor_constraint(const uint32_t a, const uint32_t b, const size_t num_bits)
    {
        return circuit_constructor.create_xor_constraint(a, b, num_bits);
    }
    uint32_t add_variable(const barretenberg::fr& in) { return circuit_constructor.add_variable(in); }

    uint32_t add_public_variable(const barretenberg::fr& in) { return circuit_constructor.add_public_variable(in); }

    uint32_t put_constant_variable(const barretenberg::fr& variable)
    {
        return circuit_constructor.put_constant_variable(variable);
    }

    size_t get_num_constant_gates() const { return circuit_constructor.get_num_constant_gates(); }

    bool check_circuit() { return circuit_constructor.check_circuit(); }

    // // num_sumcheck_rounds = 1 if using quotient polynomials, otherwise = number of sumcheck rounds
    template <size_t num_sumcheck_rounds = 1>
    static transcript::Manifest create_unrolled_manifest(const size_t num_public_inputs)
    {
        constexpr size_t g1_size = 64;
        (void)g1_size;
        constexpr size_t fr_size = 32;
        const size_t public_input_size = fr_size * num_public_inputs;
        /*  A RoundManifest describes data that will be put in or extracted from a transcript.
            Here we have (1 + 7 + num_sumcheck_rounds)-many RoundManifests. */

        std::vector<transcript::Manifest::RoundManifest> manifest_rounds;
        // Round 0
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest(
            { { .name = "circuit_size", .num_bytes = 4, .derived_by_verifier = true },
              { .name = "public_input_size", .num_bytes = 4, .derived_by_verifier = true } },
            /* challenge_name = */ "init",
            /* num_challenges_in = */ 1));
        // Round 1
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest({ /* this is a noop */ },
                                                                         /* challenge_name = */ "eta",
                                                                         /* num_challenges_in = */ 0));
        // Round 2
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest(
            {
                { .name = "public_inputs", .num_bytes = public_input_size, .derived_by_verifier = false },
                { .name = "W_1", .num_bytes = g1_size, .derived_by_verifier = false },
                { .name = "W_2", .num_bytes = g1_size, .derived_by_verifier = false },
                { .name = "W_3", .num_bytes = g1_size, .derived_by_verifier = false },
            },
            /* challenge_name = */ "beta",
            /* num_challenges_in = */ 2) // also produce "gamma"
        );

        // Round 3
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest(
            { { .name = "Z_PERM", .num_bytes = g1_size, .derived_by_verifier = false } },
            /* challenge_name = */ "alpha",
            /* num_challenges_in = */ 1));

        // Rounds 3 + 1, ... 3 + num_sumcheck_rounds
        for (size_t i = 0; i < num_sumcheck_rounds; i++) {
            auto label = std::to_string(num_sumcheck_rounds - i);
            manifest_rounds.emplace_back(
                transcript::Manifest::RoundManifest({ { .name = "uni_" + label,
                                                        .num_bytes = fr_size * honk::StandardHonk::MAX_RELATION_LENGTH,
                                                        .derived_by_verifier = false } },
                                                    /* challenge_name = */ "u_" + label,
                                                    /* num_challenges_in = */ 1));
        }

        // Rounds 4 + num_sumcheck_rounds
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest(
            {
                { .name = "w_1", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 0 },
                { .name = "w_2", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 1 },
                { .name = "w_3", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 2 },
                { .name = "sigma_1", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 3 },
                { .name = "sigma_2", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 4 },
                { .name = "sigma_3", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 5 },
                { .name = "q_1", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 6 },
                { .name = "q_2", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 7 },
                { .name = "q_3", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 8 },
                { .name = "q_m", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 9 },
                { .name = "q_c", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 10 },
                { .name = "z_perm", .num_bytes = fr_size, .derived_by_verifier = false, .challenge_map_index = 11 },
                { .name = "z_perm_omega",
                  .num_bytes = fr_size,
                  .derived_by_verifier = false,
                  .challenge_map_index = -1 },
            },
            /* challenge_name = */ "rho",
            /* num_challenges_in = */ waffle::STANDARD_UNROLLED_MANIFEST_SIZE - 1, /* TODO(Cody): this is bad. */
            /* map_challenges_in = */ true));

        // Rounds 5 + num_sumcheck_rounds
        std::vector<transcript::Manifest::ManifestEntry> fold_commitment_entries;
        for (size_t i = 0; i + 1 < num_sumcheck_rounds; i++) {
            fold_commitment_entries.emplace_back(transcript::Manifest::ManifestEntry(
                { .name = "FOLD_" + std::to_string(i), .num_bytes = g1_size, .derived_by_verifier = false }));
        };
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest(fold_commitment_entries,
                                                                         /* challenge_name = */ "r",
                                                                         /* num_challenges_in */ 1));

        // Rounds 6 + num_sumcheck_rounds
        std::vector<transcript::Manifest::ManifestEntry> gemini_evaluation_entries;
        for (size_t i = 0; i < num_sumcheck_rounds; i++) {
            gemini_evaluation_entries.emplace_back(transcript::Manifest::ManifestEntry(
                { .name = "a_" + std::to_string(i), .num_bytes = fr_size, .derived_by_verifier = false }));
        };
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest(gemini_evaluation_entries,
                                                                         /* challenge_name = */ "nu",
                                                                         /* num_challenges_in */ 1));

        // Rounds 7 + num_sumcheck_rounds
        manifest_rounds.emplace_back(
            transcript::Manifest::RoundManifest({ { .name = "Q", .num_bytes = g1_size, .derived_by_verifier = false } },
                                                /* challenge_name = */ "z",
                                                /* num_challenges_in */ 1));

        // Rounds 8 + num_sumcheck_rounds
        manifest_rounds.emplace_back(
            transcript::Manifest::RoundManifest({ { .name = "Q", .num_bytes = g1_size, .derived_by_verifier = false } },
                                                /* challenge_name = */ "separator",
                                                /* num_challenges_in */ 1));
        auto output = transcript::Manifest(manifest_rounds);

        return output;
    }

    /**Proof and verification-related methods*/

    std::shared_ptr<waffle::proving_key> compute_proving_key()
    {
        return composer_helper.compute_proving_key(circuit_constructor);
    }

    std::shared_ptr<waffle::verification_key> compute_verification_key()
    {
        return composer_helper.compute_verification_key(circuit_constructor);
    }
    void compute_witness() { composer_helper.compute_witness(circuit_constructor); };
    waffle::Verifier create_verifier() { return composer_helper.create_verifier(circuit_constructor); }
    /**
     * Preprocess the circuit. Delegates to create_prover.
     *
     * @return A new initialized prover.
     */

    // Prover preprocess() { return composer_helper.create_prover(circuit_constructor); };
    // Prover create_prover() { return composer_helper.create_prover(circuit_constructor); };
    // UnrolledVerifier create_unrolled_verifier()
    waffle::UnrolledVerifier create_unrolled_verifier()
    {
        return composer_helper.create_unrolled_verifier(circuit_constructor);
    }
    StandardUnrolledProver create_unrolled_prover() { return composer_helper.create_unrolled_prover(circuit_constructor); };
}; // namespace waffle
} // namespace honk