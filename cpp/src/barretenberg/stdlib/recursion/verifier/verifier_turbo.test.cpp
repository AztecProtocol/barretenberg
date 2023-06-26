#include "verifier.hpp"
#include "program_settings.hpp"

#include "barretenberg/common/test.hpp"
#include "barretenberg/ecc/curves/bn254/fq12.hpp"
#include "barretenberg/ecc/curves/bn254/pairing.hpp"
#include "barretenberg/plonk/composer/standard_composer.hpp"
#include "barretenberg/plonk/composer/turbo_composer.hpp"
#include "barretenberg/plonk/proof_system/proving_key/serialize.hpp"
#include "barretenberg/stdlib/hash/blake3s/blake3s.hpp"
#include "barretenberg/stdlib/hash/pedersen/pedersen.hpp"
#include "barretenberg/stdlib/primitives/curves/bn254.hpp"
#include "barretenberg/transcript/transcript.hpp"

namespace proof_system::plonk::stdlib {

template <typename OuterComposer> class stdlib_verifier_turbo : public testing::Test {

    using InnerComposer = proof_system::plonk::TurboPlonkComposerHelper;
    using InnerBuilder = typename InnerComposer::CircuitConstructor;

    using OuterBuilder = typename OuterComposer::CircuitConstructor;

    using inner_curve = bn254<InnerBuilder>;
    using outer_curve = bn254<OuterBuilder>;

    using verification_key_pt = recursion::verification_key<outer_curve>;
    using recursive_settings = recursion::recursive_turbo_verifier_settings<outer_curve>;

    using inner_scalar_field_ct = inner_curve::fr_ct;
    using inner_ground_field_ct = inner_curve::fq_ct;
    using public_witness_ct = inner_curve::public_witness_ct;
    using witness_ct = inner_curve::witness_ct;
    using byte_array_ct = inner_curve::byte_array_ct;

    using inner_scalar_field = typename inner_curve::ScalarField;
    using outer_scalar_field = typename outer_curve::BaseField;
    using pairing_target_field = barretenberg::fq12;

    using ProverOfInnerCircuit = plonk::TurboProver;
    using VerifierOfInnerProof = plonk::TurboVerifier;
    using RecursiveSettings = recursive_settings;

    struct circuit_outputs {
        recursion::aggregation_state<outer_curve> aggregation_state;
        std::shared_ptr<verification_key_pt> verification_key;
    };

    static void create_inner_circuit(InnerBuilder& builder, const std::vector<inner_scalar_field>& public_inputs)
    {
        inner_scalar_field_ct a(public_witness_ct(&builder, public_inputs[0]));
        inner_scalar_field_ct b(public_witness_ct(&builder, public_inputs[1]));
        inner_scalar_field_ct c(public_witness_ct(&builder, public_inputs[2]));

        for (size_t i = 0; i < 32; ++i) {
            a = (a * b) + b + a;
            a = a.madd(b, c);
        }
        pedersen_commitment<InnerBuilder>::compress(a, b);
        byte_array_ct to_hash(&builder, "nonsense test data");
        blake3s(to_hash);

        inner_scalar_field bigfield_data = fr::random_element();
        inner_scalar_field bigfield_data_a{ bigfield_data.data[0], bigfield_data.data[1], 0, 0 };
        inner_scalar_field bigfield_data_b{ bigfield_data.data[2], bigfield_data.data[3], 0, 0 };

        inner_ground_field_ct big_a(inner_scalar_field_ct(witness_ct(&builder, bigfield_data_a.to_montgomery_form())),
                                    inner_scalar_field_ct(witness_ct(&builder, 0)));
        inner_ground_field_ct big_b(inner_scalar_field_ct(witness_ct(&builder, bigfield_data_b.to_montgomery_form())),
                                    inner_scalar_field_ct(witness_ct(&builder, 0)));

        big_a* big_b;
    };
    static void create_alternate_inner_circuit(InnerBuilder& builder,
                                               const std::vector<inner_scalar_field>& public_inputs)
    {
        inner_scalar_field_ct a(public_witness_ct(&builder, public_inputs[0]));
        inner_scalar_field_ct b(public_witness_ct(&builder, public_inputs[1]));
        inner_scalar_field_ct c(public_witness_ct(&builder, public_inputs[2]));

        for (size_t i = 0; i < 32; ++i) {
            a = (a * b) + b + a;
            a = c.madd(b, a);
        }
        pedersen_commitment<InnerBuilder>::compress(a, a);
        byte_array_ct to_hash(&builder, "different nonsense test data");
        blake3s(to_hash);

        inner_scalar_field bigfield_data = fr::random_element();
        inner_scalar_field bigfield_data_a{ bigfield_data.data[0], bigfield_data.data[1], 0, 0 };
        inner_scalar_field bigfield_data_b{ bigfield_data.data[2], bigfield_data.data[3], 0, 0 };

        inner_ground_field_ct big_a(inner_scalar_field_ct(witness_ct(&builder, bigfield_data_a.to_montgomery_form())),
                                    inner_scalar_field_ct(witness_ct(&builder, 0)));
        inner_ground_field_ct big_b(inner_scalar_field_ct(witness_ct(&builder, bigfield_data_b.to_montgomery_form())),
                                    inner_scalar_field_ct(witness_ct(&builder, 0)));
        ((big_a * big_b) + big_a) * big_b;
    }

    static circuit_outputs create_outer_circuit(InnerBuilder& inner_circuit, OuterBuilder& outer_builder)
    {
        info("Creating turbo (inner) prover...");
        ProverOfInnerCircuit prover;
        InnerComposer inner_composer;
        prover = inner_composer.create_prover(inner_circuit);

        info("Computing verification key...");
        const auto verification_key_native = inner_composer.compute_verification_key(inner_circuit);
        // Convert the verification key's elements into _circuit_ types, using the OUTER composer.
        std::shared_ptr<verification_key_pt> verification_key =
            verification_key_pt::from_witness(&outer_builder, verification_key_native);

        info("Constructing the turbo (inner) proof ...");
        plonk::proof proof_to_recursively_verify = prover.construct_proof();

        {
            // Native check is mainly for comparison vs circuit version of the verifier.
            info("Creating a native turbo (inner) verifier...");
            VerifierOfInnerProof native_verifier;
            native_verifier = inner_composer.create_verifier(inner_circuit);

            info("Verifying the turbo (inner) proof natively...");
            auto native_result = native_verifier.verify_proof(proof_to_recursively_verify);

            info("Native result: ", native_result);
        }

        transcript::Manifest recursive_manifest = InnerComposer::create_manifest(prover.key->num_public_inputs);

        auto output = recursion::verify_proof<outer_curve, RecursiveSettings>(
            &outer_builder, verification_key, recursive_manifest, proof_to_recursively_verify);

        return { output, verification_key };
    };

    static circuit_outputs create_double_outer_circuit(InnerBuilder& inner_circuit_a,
                                                       InnerBuilder& inner_circuit_b,
                                                       OuterBuilder& outer_circuit)
    {
        ProverOfInnerCircuit prover;
        InnerComposer inner_composer_a;
        prover = inner_composer_a.create_prover(inner_circuit_a);

        const auto verification_key_native = inner_composer_a.compute_verification_key(inner_circuit_a);
        std::shared_ptr<verification_key_pt> verification_key =
            verification_key_pt::from_witness(&outer_circuit, verification_key_native);

        plonk::proof proof_to_recursively_verify_a = prover.construct_proof();

        transcript::Manifest recursive_manifest = InnerComposer::create_manifest(prover.key->num_public_inputs);

        auto previous_output = recursion::verify_proof<outer_curve, RecursiveSettings>(
            &outer_circuit, verification_key, recursive_manifest, proof_to_recursively_verify_a);

        InnerComposer inner_composer_b;
        prover = inner_composer_b.create_prover(inner_circuit_b);

        const auto verification_key_b_raw = inner_composer_b.compute_verification_key(inner_circuit_b);
        std::shared_ptr<verification_key_pt> verification_key_b =
            verification_key_pt::from_witness(&outer_circuit, verification_key_b_raw);

        plonk::proof proof_to_recursively_verify_b = prover.construct_proof();

        auto output = proof_system::plonk::stdlib::recursion::verify_proof<outer_curve, RecursiveSettings>(
            &outer_circuit, verification_key_b, recursive_manifest, proof_to_recursively_verify_b, previous_output);

        verification_key_b->compress();
        verification_key->compress();
        return { output, verification_key };
    }

    // creates a cicuit that verifies either a proof from composer a, or from composer b
    static circuit_outputs create_outer_circuit_with_variable_inner_circuit(InnerBuilder& inner_circuit_a,
                                                                            InnerBuilder& inner_circuit_b,
                                                                            OuterBuilder& outer_circuit,
                                                                            const bool proof_type,
                                                                            const bool create_failing_proof = false,
                                                                            const bool use_constant_key = false)
    {
        ProverOfInnerCircuit prover_a;
        ProverOfInnerCircuit prover_b;
        InnerComposer inner_composer_a;
        InnerComposer inner_composer_b;
        prover_a = inner_composer_a.create_prover(inner_circuit_a);
        prover_b = inner_composer_b.create_prover(inner_circuit_b);

        const auto verification_key_raw_a = inner_composer_a.compute_verification_key(inner_circuit_a);
        const auto verification_key_raw_b = inner_composer_b.compute_verification_key(inner_circuit_b);

        std::shared_ptr<verification_key_pt> verification_key;
        if (use_constant_key) {
            verification_key = proof_type ? verification_key_pt::from_constants(&outer_circuit, verification_key_raw_a)
                                          : verification_key_pt::from_constants(&outer_circuit, verification_key_raw_b);

        } else {
            verification_key = proof_type ? verification_key_pt::from_witness(&outer_circuit, verification_key_raw_a)
                                          : verification_key_pt::from_witness(&outer_circuit, verification_key_raw_b);
        }

        if (!use_constant_key) {
            if (create_failing_proof) {
                verification_key->validate_key_is_in_set({ verification_key_raw_b, verification_key_raw_b });
            } else {
                verification_key->validate_key_is_in_set({ verification_key_raw_a, verification_key_raw_b });
            }
        }

        plonk::proof recursive_proof = proof_type ? prover_a.construct_proof() : prover_b.construct_proof();

        transcript::Manifest recursive_manifest = InnerComposer::create_manifest(prover_a.key->num_public_inputs);

        proof_system::plonk::stdlib::recursion::aggregation_state<outer_curve> output =
            proof_system::plonk::stdlib::recursion::verify_proof<outer_curve, RecursiveSettings>(
                &outer_circuit, verification_key, recursive_manifest, recursive_proof);

        return { output, verification_key };
    }

    // /**
    //  * @brief Check the correctness of the recursive proof public inputs
    //  *
    //  * @details Circuit constructors have no notion of SRS and any proof-related stuff except for the existence of
    //  * recursive proof-specific public inputs, so we can't check the recursive proof fully in check_circuit. So we
    //  * use this additional function to check that the recursive proof points work.
    //  *
    //  * @return boolean result
    //  */

    static bool check_recursive_proof_public_inputs(OuterBuilder& builder,
                                                    const barretenberg::pairing::miller_lines* lines)
    {
        if (builder.contains_recursive_proof && builder.recursive_proof_public_input_indices.size() == 16) {
            const auto& inputs = builder.public_inputs;
            const auto recover_fq_from_public_inputs =
                [&inputs, &builder](const size_t idx0, const size_t idx1, const size_t idx2, const size_t idx3) {
                    const uint256_t l0 = builder.get_variable(inputs[idx0]);
                    const uint256_t l1 = builder.get_variable(inputs[idx1]);
                    const uint256_t l2 = builder.get_variable(inputs[idx2]);
                    const uint256_t l3 = builder.get_variable(inputs[idx3]);

                    const uint256_t limb = l0 + (l1 << NUM_LIMB_BITS_IN_FIELD_SIMULATION) +
                                           (l2 << (NUM_LIMB_BITS_IN_FIELD_SIMULATION * 2)) +
                                           (l3 << (NUM_LIMB_BITS_IN_FIELD_SIMULATION * 3));
                    return outer_scalar_field(limb);
                };

            const auto x0 = recover_fq_from_public_inputs(builder.recursive_proof_public_input_indices[0],
                                                          builder.recursive_proof_public_input_indices[1],
                                                          builder.recursive_proof_public_input_indices[2],
                                                          builder.recursive_proof_public_input_indices[3]);
            const auto y0 = recover_fq_from_public_inputs(builder.recursive_proof_public_input_indices[4],
                                                          builder.recursive_proof_public_input_indices[5],
                                                          builder.recursive_proof_public_input_indices[6],
                                                          builder.recursive_proof_public_input_indices[7]);
            const auto x1 = recover_fq_from_public_inputs(builder.recursive_proof_public_input_indices[8],
                                                          builder.recursive_proof_public_input_indices[9],
                                                          builder.recursive_proof_public_input_indices[10],
                                                          builder.recursive_proof_public_input_indices[11]);
            const auto y1 = recover_fq_from_public_inputs(builder.recursive_proof_public_input_indices[12],
                                                          builder.recursive_proof_public_input_indices[13],
                                                          builder.recursive_proof_public_input_indices[14],
                                                          builder.recursive_proof_public_input_indices[15]);
            g1::affine_element P_affine[2]{
                { x0, y0 },
                { x1, y1 },
            };

            pairing_target_field result =
                barretenberg::pairing::reduced_ate_pairing_batch_precomputed(P_affine, lines, 2);

            return (result == pairing_target_field::one());
        }
        return true;
    }

    static void check_pairing(const circuit_outputs& circuit_output)
    {
        auto g2_lines = barretenberg::srs::get_crs_factory()->get_verifier_crs()->get_precomputed_g2_lines();
        g1::affine_element P[2];
        P[0].x = outer_scalar_field(circuit_output.aggregation_state.P0.x.get_value().lo);
        P[0].y = outer_scalar_field(circuit_output.aggregation_state.P0.y.get_value().lo);
        P[1].x = outer_scalar_field(circuit_output.aggregation_state.P1.x.get_value().lo);
        P[1].y = outer_scalar_field(circuit_output.aggregation_state.P1.y.get_value().lo);
        pairing_target_field inner_proof_result =
            barretenberg::pairing::reduced_ate_pairing_batch_precomputed(P, g2_lines, 2);
        EXPECT_EQ(inner_proof_result, pairing_target_field::one());
    }

    static void check_recursive_verification_circuit(OuterBuilder& outer_circuit, bool expected_result)
    {
        info("number of gates in recursive verification circuit = ", outer_circuit.get_num_gates());
        bool result = outer_circuit.check_circuit();
        EXPECT_EQ(result, expected_result);
        auto g2_lines = barretenberg::srs::get_crs_factory()->get_verifier_crs()->get_precomputed_g2_lines();
        EXPECT_EQ(check_recursive_proof_public_inputs(outer_circuit, g2_lines), true);
    }

  public:
    static void SetUpTestSuite() { barretenberg::srs::init_crs_factory("../srs_db/ignition"); }

    static void test_inner_circuit()
    {
        InnerBuilder builder;
        std::vector<inner_scalar_field> inputs{ inner_scalar_field::random_element(),
                                                inner_scalar_field::random_element(),
                                                inner_scalar_field::random_element() };

        create_inner_circuit(builder, inputs);

        bool result = builder.check_circuit();
        EXPECT_EQ(result, true);
    }

    static void test_recursive_proof_composition()
    {
        InnerBuilder inner_circuit;
        OuterBuilder outer_circuit;

        std::vector<inner_scalar_field> inner_public_inputs{ inner_scalar_field::random_element(),
                                                             inner_scalar_field::random_element(),
                                                             inner_scalar_field::random_element() };

        create_inner_circuit(inner_circuit, inner_public_inputs);

        auto circuit_output = create_outer_circuit(inner_circuit, outer_circuit);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[0].get_value(), inner_public_inputs[0]);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[1].get_value(), inner_public_inputs[1]);

        circuit_output.aggregation_state.assign_object_to_proof_outputs();
        EXPECT_EQ(outer_circuit.failed(), false);

        check_pairing(circuit_output);
        check_recursive_verification_circuit(outer_circuit, true);
    }

    static void test_double_verification()
    {
        InnerBuilder inner_circuit_a;
        InnerBuilder inner_circuit_b;

        OuterBuilder mid_circuit_a;
        OuterBuilder mid_circuit_b;

        OuterBuilder outer_circuit;

        std::vector<inner_scalar_field> inner_inputs{ inner_scalar_field::random_element(),
                                                      inner_scalar_field::random_element(),
                                                      inner_scalar_field::random_element() };

        create_inner_circuit(inner_circuit_a, inner_inputs);
        create_inner_circuit(inner_circuit_b, inner_inputs);

        auto circuit_output_a = create_outer_circuit(inner_circuit_a, mid_circuit_a);

        uint256_t a0 = circuit_output_a.aggregation_state.P0.x.binary_basis_limbs[1].element.get_value();
        uint256_t a1 = circuit_output_a.aggregation_state.P0.y.binary_basis_limbs[1].element.get_value();
        uint256_t a2 = circuit_output_a.aggregation_state.P1.x.binary_basis_limbs[1].element.get_value();
        uint256_t a3 = circuit_output_a.aggregation_state.P1.y.binary_basis_limbs[1].element.get_value();

        ASSERT(a0.get_msb() <= 68);
        ASSERT(a1.get_msb() <= 68);
        ASSERT(a2.get_msb() <= 68);
        ASSERT(a3.get_msb() <= 68);

        circuit_output_a.aggregation_state.assign_object_to_proof_outputs();

        auto circuit_output_b = create_outer_circuit(inner_circuit_b, mid_circuit_b);

        circuit_output_b.aggregation_state.assign_object_to_proof_outputs();

        auto circuit_output = create_double_outer_circuit(mid_circuit_a, mid_circuit_b, outer_circuit);
        circuit_output.aggregation_state.assign_object_to_proof_outputs();
        EXPECT_EQ(circuit_output_a.aggregation_state.public_inputs[0].get_value(), inner_inputs[0]);
        EXPECT_EQ(circuit_output_a.aggregation_state.public_inputs[1].get_value(), inner_inputs[1]);

        check_pairing(circuit_output);
        check_recursive_verification_circuit(outer_circuit, true);
    }

    // verifies a proof of a circuit that verifies one of two proofs. Test 'a' uses a proof over the first of the
    // two variable circuits
    static void test_recursive_proof_composition_with_variable_verification_key_a()
    {
        InnerBuilder inner_circuit_a;
        InnerBuilder inner_circuit_b;
        OuterBuilder outer_circuit;

        std::vector<inner_scalar_field> inner_inputs_a{ inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element() };

        std::vector<inner_scalar_field> inner_inputs_b{ inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element() };

        create_inner_circuit(inner_circuit_a, inner_inputs_a);
        create_alternate_inner_circuit(inner_circuit_b, inner_inputs_b);

        auto circuit_output =
            create_outer_circuit_with_variable_inner_circuit(inner_circuit_a, inner_circuit_b, outer_circuit, true);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[0].get_value(), inner_inputs_a[0]);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[1].get_value(), inner_inputs_a[1]);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[2].get_value(), inner_inputs_a[2]);

        check_pairing(circuit_output);
        check_recursive_verification_circuit(outer_circuit, true);
    }

    // verifies a proof of a circuit that verifies one of two proofs. Test 'b' uses a proof over the second of the
    // two variable circuits
    static void test_recursive_proof_composition_with_variable_verification_key_b()
    {
        InnerBuilder inner_circuit_a;
        InnerBuilder inner_circuit_b;
        OuterBuilder outer_circuit;

        std::vector<inner_scalar_field> inner_inputs_a{ inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element() };

        std::vector<inner_scalar_field> inner_inputs_b{ inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element() };

        create_inner_circuit(inner_circuit_a, inner_inputs_a);
        create_alternate_inner_circuit(inner_circuit_b, inner_inputs_b);

        auto circuit_output =
            create_outer_circuit_with_variable_inner_circuit(inner_circuit_a, inner_circuit_b, outer_circuit, false);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[0].get_value(), inner_inputs_b[0]);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[1].get_value(), inner_inputs_b[1]);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[2].get_value(), inner_inputs_b[2]);

        check_pairing(circuit_output);
        check_recursive_verification_circuit(outer_circuit, true);
    }

    static void test_recursive_proof_composition_with_variable_verification_key_failure_case()
    {
        InnerBuilder inner_circuit_a;
        InnerBuilder inner_circuit_b;
        OuterBuilder outer_circuit;

        std::vector<inner_scalar_field> inner_inputs_a{ inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element() };

        std::vector<inner_scalar_field> inner_inputs_b{ inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element() };

        create_inner_circuit(inner_circuit_a, inner_inputs_a);
        create_alternate_inner_circuit(inner_circuit_b, inner_inputs_b);

        auto circuit_output = create_outer_circuit_with_variable_inner_circuit(
            inner_circuit_a, inner_circuit_b, outer_circuit, true, true);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[0].get_value(), inner_inputs_a[0]);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[1].get_value(), inner_inputs_a[1]);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[2].get_value(), inner_inputs_a[2]);

        check_pairing(circuit_output);
        check_recursive_verification_circuit(outer_circuit, false);
    }

    static void test_recursive_proof_composition_with_constant_verification_key()
    {
        InnerBuilder inner_circuit_a;
        InnerBuilder inner_circuit_b;
        OuterBuilder outer_circuit;

        std::vector<inner_scalar_field> inner_inputs_a{ inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element() };

        std::vector<inner_scalar_field> inner_inputs_b{ inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element(),
                                                        inner_scalar_field::random_element() };

        create_inner_circuit(inner_circuit_a, inner_inputs_a);
        create_alternate_inner_circuit(inner_circuit_b, inner_inputs_b);

        auto circuit_output = create_outer_circuit_with_variable_inner_circuit(
            inner_circuit_a, inner_circuit_b, outer_circuit, true, false, true);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[0].get_value(), inner_inputs_a[0]);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[1].get_value(), inner_inputs_a[1]);
        EXPECT_EQ(circuit_output.aggregation_state.public_inputs[2].get_value(), inner_inputs_a[2]);

        check_pairing(circuit_output);
        check_recursive_verification_circuit(outer_circuit, true);
    }
};

typedef testing::Types<plonk::StandardPlonkComposerHelper, plonk::TurboPlonkComposerHelper> OuterCircuitTypes;

TYPED_TEST_SUITE(stdlib_verifier_turbo, OuterCircuitTypes);

HEAVY_TYPED_TEST(stdlib_verifier_turbo, test_inner_circuit)
{
    TestFixture::test_inner_circuit();
}

HEAVY_TYPED_TEST(stdlib_verifier_turbo, recursive_proof_composition)
{
    TestFixture::test_recursive_proof_composition();
};

HEAVY_TYPED_TEST(stdlib_verifier_turbo, double_verification)
{
    if constexpr (std::same_as<TypeParam, TurboPlonkComposerHelper>) {
        TestFixture::test_double_verification();
    } else {
        // Test doesn't compile.
        GTEST_SKIP();
    }
};

HEAVY_TYPED_TEST(stdlib_verifier_turbo, recursive_proof_composition_with_variable_verification_key_a)
{
    TestFixture::test_recursive_proof_composition_with_variable_verification_key_a();
}

HEAVY_TYPED_TEST(stdlib_verifier_turbo, recursive_proof_composition_with_variable_verification_key_b)
{
    TestFixture::test_recursive_proof_composition_with_variable_verification_key_b();
}

HEAVY_TYPED_TEST(stdlib_verifier_turbo, recursive_proof_composition_var_verif_key_fail)
{
    TestFixture::test_recursive_proof_composition_with_variable_verification_key_failure_case();
}

HEAVY_TYPED_TEST(stdlib_verifier_turbo, recursive_proof_composition_const_verif_key)
{
    TestFixture::test_recursive_proof_composition_with_constant_verification_key();
}

} // namespace proof_system::plonk::stdlib