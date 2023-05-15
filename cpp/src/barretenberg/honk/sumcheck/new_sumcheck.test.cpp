#include "sumcheck.hpp"
#include "barretenberg/honk/transcript/transcript.hpp"
#include "barretenberg/honk/flavor/standard.hpp"
#include "barretenberg/transcript/transcript_wrappers.hpp"
#include "relations/arithmetic_relation.hpp"
#include "relations/permutation_relation.hpp"
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
    // using Flavor = honk::flavor::Ultra;
    // using FF = typename Flavor::FF;
    // using ProverPolynomials = typename Flavor::ProverPolynomials;

    // // Create a composer and a dummy circuit with a few gates
    // auto composer = UltraHonkComposer();
    // fr a = fr::one();
    // // Using the public variable to check that public_input_delta is computed and added to the relation correctly
    // uint32_t a_idx = composer.add_variable(a);
    // fr b = fr::one();
    // fr c = a + b;
    // fr d = a + c;
    // uint32_t b_idx = composer.add_variable(b);
    // uint32_t c_idx = composer.add_variable(c);
    // uint32_t d_idx = composer.add_variable(d);
    // for (size_t i = 0; i < 16; i++) {
    //     composer.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    //     composer.create_add_gate({ d_idx, c_idx, a_idx, fr::one(), fr::neg_one(), fr::neg_one(), fr::zero() });
    // }
    // // Create a prover (it will compute proving key and witness)
    // auto prover = composer.create_prover();

    // // Generate beta and gamma
    // fr beta = fr::random_element();
    // fr gamma = fr::random_element();

    // // Compute public input delta
    // const auto public_inputs = composer.circuit_constructor.get_public_inputs();
    // auto public_input_delta =
    //     honk::compute_public_input_delta<FF>(public_inputs, beta, gamma, prover.key->circuit_size);

    // sumcheck::RelationParameters<FF> relation_parameters{
    //     .beta = beta,
    //     .gamma = gamma,
    //     .public_input_delta = public_input_delta,
    // };

    // // Compute grand product polynomial
    // polynomial z_permutation = prover_library::compute_permutation_grand_product<Flavor>(prover.key, beta, gamma);

    // ProverPolynomials prover_polynomials;

    // prover_polynomials.w_l = prover.key->w_l;
    // prover_polynomials.w_r = prover.key->w_r;
    // prover_polynomials.w_o = prover.key->w_o;
    // prover_polynomials.w_4 = prover.key->w_4;
    // prover_polynomials.w_l_shift = prover.key->w_l.shifted();
    // prover_polynomials.w_r_shift = prover.key->w_r.shifted();
    // prover_polynomials.w_o_shift = prover.key->w_o.shifted();
    // prover_polynomials.w_4_shift = prover.key->w_4.shifted();
    // prover_polynomials.sorted_1 = prover.key->sorted_1;
    // prover_polynomials.sorted_2 = prover.key->sorted_2;
    // prover_polynomials.sorted_3 = prover.key->sorted_3;
    // prover_polynomials.sorted_4 = prover.key->sorted_4;
    // prover_polynomials.sorted_accum = sorted_list_accumulator;
    // prover_polynomials.sorted_accum_shift = sorted_list_accumulator.shifted();
    // prover_polynomials.table_1 = prover.key->table_1;
    // prover_polynomials.table_2 = prover.key->table_2;
    // prover_polynomials.table_3 = prover.key->table_3;
    // prover_polynomials.table_4 = prover.key->table_4;
    // prover_polynomials.table_1_shift = prover.key->table_1.shifted();
    // prover_polynomials.table_2_shift = prover.key->table_2.shifted();
    // prover_polynomials.table_3_shift = prover.key->table_3.shifted();
    // prover_polynomials.table_4_shift = prover.key->table_4.shifted();
    // prover_polynomials.z_perm = z_permutation;
    // prover_polynomials.z_perm_shift = z_permutation.shifted();
    // prover_polynomials.z_lookup = z_lookup;
    // prover_polynomials.z_lookup_shift = z_lookup.shifted();
    // prover_polynomials.q_m = prover.key->q_m;
    // prover_polynomials.q_l = prover.key->q_l;
    // prover_polynomials.q_r = prover.key->q_r;
    // prover_polynomials.q_o = prover.key->q_o;
    // prover_polynomials.q_c = prover.key->q_c;
    // prover_polynomials.q_4 = prover.key->q_4;
    // prover_polynomials.q_arith = prover.key->q_arith;
    // prover_polynomials.q_sort = prover.key->q_sort;
    // prover_polynomials.q_elliptic = prover.key->q_elliptic;
    // prover_polynomials.q_aux = prover.key->q_aux;
    // prover_polynomials.q_lookup = prover.key->q_lookup;
    // prover_polynomials.sigma_1 = prover.key->sigma_1;
    // prover_polynomials.sigma_2 = prover.key->sigma_2;
    // prover_polynomials.sigma_3 = prover.key->sigma_3;
    // prover_polynomials.sigma_4 = prover.key->sigma_4;
    // prover_polynomials.id_1 = prover.key->id_1;
    // prover_polynomials.id_2 = prover.key->id_2;
    // prover_polynomials.id_3 = prover.key->id_3;
    // prover_polynomials.id_4 = prover.key->id_4;
    // prover_polynomials.lagrange_first = prover.key->lagrange_first;
    // prover_polynomials.lagrange_last = prover.key->lagrange_last;

    // auto prover_transcript = ProverTranscript<FF>::init_empty();

    // auto sumcheck_prover = Sumcheck<Flavor, ProverTranscript<FF>, ArithmeticRelation, PermutationRelation>(
    //     prover.key->circuit_size, prover_transcript);

    // auto prover_output = sumcheck_prover.execute_prover(prover_polynomials, relation_parameters);

    // auto verifier_transcript = VerifierTranscript<FF>::init_empty(prover_transcript);

    // auto sumcheck_verifier = Sumcheck<Flavor, VerifierTranscript<FF>, ArithmeticRelation, PermutationRelation>(
    //     prover.key->circuit_size, verifier_transcript);

    // std::optional verifier_output = sumcheck_verifier.execute_verifier(relation_parameters);

    // ASSERT_TRUE(verifier_output.has_value());
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
