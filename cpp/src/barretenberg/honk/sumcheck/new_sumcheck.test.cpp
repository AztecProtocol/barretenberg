#include "sumcheck.hpp"
#include "barretenberg/honk/transcript/transcript.hpp"
#include "barretenberg/honk/flavor/standard.hpp"
#include "barretenberg/transcript/transcript_wrappers.hpp"
#include "relations/arithmetic_relation.hpp"
#include "relations/permutation_relation.hpp"
#include "relations/lookup_relation.hpp"
#include "relations/ultra_arithmetic_relation.hpp"
#include "relations/gen_perm_sort_relation.hpp"
#include "relations/elliptic_relation.hpp"
#include "relations/auxiliary_relation.hpp"
#include "barretenberg/transcript/manifest.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include <gtest/internal/gtest-internal.h>
#include "barretenberg/numeric/random/engine.hpp"
#include "barretenberg/honk/composer/standard_honk_composer.hpp"
#include "barretenberg/honk/composer/ultra_honk_composer.hpp"

#include <initializer_list>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <sys/types.h>
#include <vector>

using namespace proof_system::honk;
using namespace proof_system::honk::sumcheck;
using Flavor = honk::flavor::Standard;
using FF = typename Flavor::FF;
using ProverPolynomials = typename Flavor::ProverPolynomials;

namespace test_sumcheck_new {

TEST(NewSumcheck, Standard)
{
    using Flavor = honk::flavor::Standard;
    using FF = typename Flavor::FF;
    using ProverPolynomials = typename Flavor::ProverPolynomials;

    // Create a composer and a dummy circuit with a few gates
    auto composer = StandardHonkComposer();
    fr a = fr::one();
    // Using the public variable to check that public_input_delta is computed and added to the relation correctly
    uint32_t a_idx = composer.add_public_variable(a);
    fr b = fr::one();
    fr c = a + b;
    fr d = a + c;
    uint32_t b_idx = composer.add_variable(b);
    uint32_t c_idx = composer.add_variable(c);
    uint32_t d_idx = composer.add_variable(d);
    for (size_t i = 0; i < 16; i++) {
        composer.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
        composer.create_add_gate({ d_idx, c_idx, a_idx, fr::one(), fr::neg_one(), fr::neg_one(), fr::zero() });
    }
    // Create a prover (it will compute proving key and witness)
    auto prover = composer.create_prover();

    // Generate beta and gamma
    fr beta = fr::random_element();
    fr gamma = fr::random_element();

    // Compute public input delta
    const auto public_inputs = composer.circuit_constructor.get_public_inputs();
    auto public_input_delta =
        honk::compute_public_input_delta<FF>(public_inputs, beta, gamma, prover.key->circuit_size);

    sumcheck::RelationParameters<FF> relation_parameters{
        .beta = beta,
        .gamma = gamma,
        .public_input_delta = public_input_delta,
    };

    // Compute grand product polynomial
    polynomial z_permutation = prover_library::compute_permutation_grand_product<Flavor>(prover.key, beta, gamma);

    ProverPolynomials prover_polynomials;

    prover_polynomials.w_l = prover.key->w_l;
    prover_polynomials.w_r = prover.key->w_r;
    prover_polynomials.w_o = prover.key->w_o;
    prover_polynomials.z_perm = z_permutation;
    prover_polynomials.z_perm_shift = z_permutation.shifted();
    prover_polynomials.q_m = prover.key->q_m;
    prover_polynomials.q_l = prover.key->q_l;
    prover_polynomials.q_r = prover.key->q_r;
    prover_polynomials.q_o = prover.key->q_o;
    prover_polynomials.q_c = prover.key->q_c;
    prover_polynomials.sigma_1 = prover.key->sigma_1;
    prover_polynomials.sigma_2 = prover.key->sigma_2;
    prover_polynomials.sigma_3 = prover.key->sigma_3;
    prover_polynomials.id_1 = prover.key->id_1;
    prover_polynomials.id_2 = prover.key->id_2;
    prover_polynomials.id_3 = prover.key->id_3;
    prover_polynomials.lagrange_first = prover.key->lagrange_first;
    prover_polynomials.lagrange_last = prover.key->lagrange_last;

    auto prover_transcript = ProverTranscript<FF>::init_empty();

    auto sumcheck_prover = Sumcheck<Flavor, ProverTranscript<FF>, ArithmeticRelation, PermutationRelation>(
        prover.key->circuit_size, prover_transcript);

    auto prover_output = sumcheck_prover.execute_prover(prover_polynomials, relation_parameters);

    auto verifier_transcript = VerifierTranscript<FF>::init_empty(prover_transcript);

    auto sumcheck_verifier = Sumcheck<Flavor, VerifierTranscript<FF>, ArithmeticRelation, PermutationRelation>(
        prover.key->circuit_size, verifier_transcript);

    std::optional verifier_output = sumcheck_verifier.execute_verifier(relation_parameters);

    ASSERT_TRUE(verifier_output.has_value());
}

TEST(NewSumcheck, Ultra)
{
    using Flavor = honk::flavor::Ultra;
    using FF = typename Flavor::FF;
    using ProverPolynomials = typename Flavor::ProverPolynomials;

    // Create a composer and a dummy circuit with a few gates
    auto composer = UltraHonkComposer();
    fr a = fr::one();

    // Add some basic add gates
    uint32_t a_idx = composer.add_variable(a);
    fr b = fr::one();
    fr c = a + b;
    fr d = a + c;
    uint32_t b_idx = composer.add_variable(b);
    uint32_t c_idx = composer.add_variable(c);
    uint32_t d_idx = composer.add_variable(d);
    for (size_t i = 0; i < 16; i++) {
        composer.create_add_gate({ a_idx, b_idx, c_idx, 1, 1, -1, 0 });
        composer.create_add_gate({ d_idx, c_idx, a_idx, 1, -1, -1, 0 });
    }

    // Add a big add gate with use of next row to test q_arith = 2
    fr e = a + b + c + d;
    uint32_t e_idx = composer.add_variable(e);

    uint32_t zero_idx = composer.get_zero_idx();
    composer.create_big_add_gate({ a_idx, b_idx, c_idx, d_idx, -1, -1, -1, -1, 0 }, true); // use next row
    composer.create_big_add_gate({ zero_idx, zero_idx, zero_idx, e_idx, 0, 0, 0, 0, 0 }, false);

    // Add some lookup gates (related to pedersen hashing)
    barretenberg::fr pedersen_input_value = fr::random_element();
    const fr input_hi = uint256_t(pedersen_input_value).slice(126, 256);
    const fr input_lo = uint256_t(pedersen_input_value).slice(0, 126);
    const auto input_hi_index = composer.add_variable(input_hi);
    const auto input_lo_index = composer.add_variable(input_lo);

    const auto sequence_data_hi = plookup::get_lookup_accumulators(plookup::MultiTableId::PEDERSEN_LEFT_HI, input_hi);
    const auto sequence_data_lo = plookup::get_lookup_accumulators(plookup::MultiTableId::PEDERSEN_LEFT_LO, input_lo);

    composer.create_gates_from_plookup_accumulators(
        plookup::MultiTableId::PEDERSEN_LEFT_HI, sequence_data_hi, input_hi_index);
    composer.create_gates_from_plookup_accumulators(
        plookup::MultiTableId::PEDERSEN_LEFT_LO, sequence_data_lo, input_lo_index);

    // Add a sort gate (simply checks that consecutive inputs have a difference of < 4)
    a_idx = composer.add_variable(FF(0));
    b_idx = composer.add_variable(FF(1));
    c_idx = composer.add_variable(FF(2));
    d_idx = composer.add_variable(FF(3));
    composer.create_sort_constraint({ a_idx, b_idx, c_idx, d_idx });

    // Add an elliptic curve addition gate
    grumpkin::g1::affine_element p1 = crypto::generators::get_generator_data({ 0, 0 }).generator;
    grumpkin::g1::affine_element p2 = crypto::generators::get_generator_data({ 0, 1 }).generator;

    grumpkin::fq beta_scalar = grumpkin::fq::cube_root_of_unity();
    grumpkin::g1::affine_element p2_endo = p2;
    p2_endo.x *= beta_scalar;

    grumpkin::g1::affine_element p3(grumpkin::g1::element(p1) - grumpkin::g1::element(p2_endo));

    uint32_t x1 = composer.add_variable(p1.x);
    uint32_t y1 = composer.add_variable(p1.y);
    uint32_t x2 = composer.add_variable(p2.x);
    uint32_t y2 = composer.add_variable(p2.y);
    uint32_t x3 = composer.add_variable(p3.x);
    uint32_t y3 = composer.add_variable(p3.y);

    ecc_add_gate gate{ x1, y1, x2, y2, x3, y3, beta_scalar, -1 };
    composer.create_ecc_add_gate(gate);

    // Add some RAM gates
    uint32_t ram_values[8]{
        composer.add_variable(fr::random_element()), composer.add_variable(fr::random_element()),
        composer.add_variable(fr::random_element()), composer.add_variable(fr::random_element()),
        composer.add_variable(fr::random_element()), composer.add_variable(fr::random_element()),
        composer.add_variable(fr::random_element()), composer.add_variable(fr::random_element()),
    };

    size_t ram_id = composer.create_RAM_array(8);

    for (size_t i = 0; i < 8; ++i) {
        composer.init_RAM_element(ram_id, i, ram_values[i]);
    }

    a_idx = composer.read_RAM_array(ram_id, composer.add_variable(5));
    EXPECT_EQ(a_idx != ram_values[5], true);

    b_idx = composer.read_RAM_array(ram_id, composer.add_variable(4));
    c_idx = composer.read_RAM_array(ram_id, composer.add_variable(1));

    composer.write_RAM_array(ram_id, composer.add_variable(4), composer.add_variable(500));
    d_idx = composer.read_RAM_array(ram_id, composer.add_variable(4));

    EXPECT_EQ(composer.get_variable(d_idx), 500);

    // ensure these vars get used in another arithmetic gate
    const auto e_value = composer.get_variable(a_idx) + composer.get_variable(b_idx) + composer.get_variable(c_idx) +
                         composer.get_variable(d_idx);
    e_idx = composer.add_variable(e_value);

    composer.create_big_add_gate({ a_idx, b_idx, c_idx, d_idx, -1, -1, -1, -1, 0 }, true);
    composer.create_big_add_gate(
        {
            composer.get_zero_idx(),
            composer.get_zero_idx(),
            composer.get_zero_idx(),
            e_idx,
            0,
            0,
            0,
            0,
            0,
        },
        false);

    // Create a prover (it will compute proving key and witness)
    auto prover = composer.create_prover();

    // Generate eta, beta and gamma
    fr eta = fr::random_element();
    fr beta = fr::random_element();
    fr gamma = fr::random_element();

    // Compute public input delta
    const auto public_inputs = composer.circuit_constructor.get_public_inputs();
    auto public_input_delta =
        honk::compute_public_input_delta<FF>(public_inputs, beta, gamma, prover.key->circuit_size);
    auto lookup_grand_product_delta =
        honk::compute_lookup_grand_product_delta<FF>(beta, gamma, prover.key->circuit_size);

    sumcheck::RelationParameters<FF> relation_parameters{
        .eta = eta,
        .beta = beta,
        .gamma = gamma,
        .public_input_delta = public_input_delta,
        .lookup_grand_product_delta = lookup_grand_product_delta,
    };

    // Compute sorted witness-table accumulator
    prover.key->sorted_accum = prover_library::compute_sorted_list_accumulator<Flavor>(prover.key, eta);

    // Add RAM/ROM memory records to wire four
    prover_library::add_plookup_memory_records_to_wire_4<Flavor>(prover.key, eta);

    // Compute grand product polynomial
    prover.key->z_perm = prover_library::compute_permutation_grand_product<Flavor>(prover.key, beta, gamma);

    // Compute lookup grand product polynomial
    prover.key->z_lookup = prover_library::compute_lookup_grand_product<Flavor>(prover.key, eta, beta, gamma);

    ProverPolynomials prover_polynomials;

    prover_polynomials.w_l = prover.key->w_l;
    prover_polynomials.w_r = prover.key->w_r;
    prover_polynomials.w_o = prover.key->w_o;
    prover_polynomials.w_4 = prover.key->w_4;
    prover_polynomials.w_l_shift = prover.key->w_l.shifted();
    prover_polynomials.w_r_shift = prover.key->w_r.shifted();
    prover_polynomials.w_o_shift = prover.key->w_o.shifted();
    prover_polynomials.w_4_shift = prover.key->w_4.shifted();
    prover_polynomials.sorted_accum = prover.key->sorted_accum;
    prover_polynomials.sorted_accum_shift = prover.key->sorted_accum.shifted();
    prover_polynomials.table_1 = prover.key->table_1;
    prover_polynomials.table_2 = prover.key->table_2;
    prover_polynomials.table_3 = prover.key->table_3;
    prover_polynomials.table_4 = prover.key->table_4;
    prover_polynomials.table_1_shift = prover.key->table_1.shifted();
    prover_polynomials.table_2_shift = prover.key->table_2.shifted();
    prover_polynomials.table_3_shift = prover.key->table_3.shifted();
    prover_polynomials.table_4_shift = prover.key->table_4.shifted();
    prover_polynomials.z_perm = prover.key->z_perm;
    prover_polynomials.z_perm_shift = prover.key->z_perm.shifted();
    prover_polynomials.z_lookup = prover.key->z_lookup;
    prover_polynomials.z_lookup_shift = prover.key->z_lookup.shifted();
    prover_polynomials.q_m = prover.key->q_m;
    prover_polynomials.q_l = prover.key->q_l;
    prover_polynomials.q_r = prover.key->q_r;
    prover_polynomials.q_o = prover.key->q_o;
    prover_polynomials.q_c = prover.key->q_c;
    prover_polynomials.q_4 = prover.key->q_4;
    prover_polynomials.q_arith = prover.key->q_arith;
    prover_polynomials.q_sort = prover.key->q_sort;
    prover_polynomials.q_elliptic = prover.key->q_elliptic;
    prover_polynomials.q_aux = prover.key->q_aux;
    prover_polynomials.q_lookup = prover.key->q_lookup;
    prover_polynomials.sigma_1 = prover.key->sigma_1;
    prover_polynomials.sigma_2 = prover.key->sigma_2;
    prover_polynomials.sigma_3 = prover.key->sigma_3;
    prover_polynomials.sigma_4 = prover.key->sigma_4;
    prover_polynomials.id_1 = prover.key->id_1;
    prover_polynomials.id_2 = prover.key->id_2;
    prover_polynomials.id_3 = prover.key->id_3;
    prover_polynomials.id_4 = prover.key->id_4;
    prover_polynomials.lagrange_first = prover.key->lagrange_first;
    prover_polynomials.lagrange_last = prover.key->lagrange_last;

    auto prover_transcript = ProverTranscript<FF>::init_empty();

    auto sumcheck_prover = Sumcheck<Flavor,
                                    ProverTranscript<FF>,
                                    UltraArithmeticRelation,
                                    UltraPermutationRelation,
                                    LookupRelation,
                                    GenPermSortRelation,
                                    EllipticRelation,
                                    AuxiliaryRelation>(prover.key->circuit_size, prover_transcript);

    auto prover_output = sumcheck_prover.execute_prover(prover_polynomials, relation_parameters);

    auto verifier_transcript = VerifierTranscript<FF>::init_empty(prover_transcript);

    auto sumcheck_verifier = Sumcheck<Flavor,
                                      VerifierTranscript<FF>,
                                      UltraArithmeticRelation,
                                      UltraPermutationRelation,
                                      LookupRelation,
                                      GenPermSortRelation,
                                      EllipticRelation,
                                      AuxiliaryRelation>(prover.key->circuit_size, verifier_transcript);

    std::optional verifier_output = sumcheck_verifier.execute_verifier(relation_parameters);

    ASSERT_TRUE(verifier_output.has_value());
}

// WORKTODO: move these utility tests to SumcheckRound tests
TEST(NewSumcheck, TupleOfTuplesOfUnivariates)
{
    using Flavor = honk::flavor::Standard;
    using FF = typename Flavor::FF;

    // Define three linear univariates of different sizes
    Univariate<FF, 3> univariate_1({ 1, 2, 3 });
    Univariate<FF, 2> univariate_2({ 2, 4 });
    Univariate<FF, 5> univariate_3({ 3, 4, 5, 6, 7 });
    const size_t MAX_LENGTH = 5;

    // Instantiate some barycentric extension utility classes
    auto barycentric_util_1 = BarycentricData<FF, 3, MAX_LENGTH>();
    auto barycentric_util_2 = BarycentricData<FF, 2, MAX_LENGTH>();
    auto barycentric_util_3 = BarycentricData<FF, 5, MAX_LENGTH>();

    // Construct a tuple of tuples of the form { {univariate_1}, {univariate_2, univariate_3} }
    auto tuple_of_tuples = std::make_tuple(std::make_tuple(univariate_1), std::make_tuple(univariate_2, univariate_3));

    // Use scale_univariate_accumulators to scale by challenge powers
    FF challenge = 5;
    FF running_challenge = 1;
    SumcheckRound<Flavor, ArithmeticRelation>::scale_univariates(tuple_of_tuples, challenge, running_challenge);

    // Use extend_and_batch_univariates to extend to MAX_LENGTH then accumulate
    auto result = Univariate<FF, MAX_LENGTH>();
    SumcheckRound<Flavor, ArithmeticRelation>::extend_and_batch_univariates(tuple_of_tuples, result);

    // Repeat the batching process manually
    auto result_expected = barycentric_util_1.extend(univariate_1) * 1 +
                           barycentric_util_2.extend(univariate_2) * challenge +
                           barycentric_util_3.extend(univariate_3) * challenge * challenge;

    // Compare final batched univarites
    EXPECT_EQ(result, result_expected);

    // Reinitialize univariate accumulators to zero
    SumcheckRound<Flavor, ArithmeticRelation>::zero_univariates(tuple_of_tuples);

    // Check that reinitialization was successful
    Univariate<FF, 3> expected_1({ 0, 0, 0 });
    Univariate<FF, 2> expected_2({ 0, 0 });
    Univariate<FF, 5> expected_3({ 0, 0, 0, 0, 0 });
    EXPECT_EQ(std::get<0>(std::get<0>(tuple_of_tuples)), expected_1);
    EXPECT_EQ(std::get<0>(std::get<1>(tuple_of_tuples)), expected_2);
    EXPECT_EQ(std::get<1>(std::get<1>(tuple_of_tuples)), expected_3);
}

TEST(NewSumcheck, TuplesOfEvaluationArrays)
{
    using Flavor = honk::flavor::Standard;
    using FF = typename Flavor::FF;

    // Define two arrays of arbitrary elements
    std::array<FF, 1> evaluations_1 = { 4 };
    std::array<FF, 2> evaluations_2 = { 6, 2 };

    // Construct a tuple
    auto tuple_of_arrays = std::make_tuple(evaluations_1, evaluations_2);

    // Use scale_and_batch_elements to scale by challenge powers
    FF challenge = 5;
    FF running_challenge = 1;
    FF result = 0;
    SumcheckRound<Flavor, ArithmeticRelation>::scale_and_batch_elements(
        tuple_of_arrays, challenge, running_challenge, result);

    // Repeat the batching process manually
    auto result_expected =
        evaluations_1[0] * 1 + evaluations_2[0] * challenge + evaluations_2[1] * challenge * challenge;

    // Compare batched result
    EXPECT_EQ(result, result_expected);

    // Reinitialize univariate accumulators to zero
    SumcheckRound<Flavor, ArithmeticRelation>::zero_elements(tuple_of_arrays);

    EXPECT_EQ(std::get<0>(tuple_of_arrays)[0], 0);
    EXPECT_EQ(std::get<1>(tuple_of_arrays)[0], 0);
    EXPECT_EQ(std::get<1>(tuple_of_arrays)[1], 0);
}

} // namespace test_sumcheck_new
