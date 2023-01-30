// #include "relation.hpp"
// #include <proof_system/flavor/flavor.hpp>
// #include "arithmetic_relation.hpp"
// #include "grand_product_initialization_relation.hpp"
// #include "grand_product_computation_relation.hpp"
// #include "../polynomials/univariate.hpp"
// #include "../polynomials/barycentric_data.hpp"

// #include <ecc/curves/bn254/fr.hpp>
// #include <numeric/random/engine.hpp>

// #include <gtest/gtest.h>
// using namespace honk::sumcheck;
// /**
//  * We want to test if all three relations (namely, ArithmeticRelation, GrandProductComputationRelation,
//  * GrandProductInitializationRelation) provide correct contributions by manually computing their
//  * contributions with deterministic and random inputs. The relations are supposed to work with
//  * univariates (edges) of degree one (length 2) and spit out polynomials of corresponding degrees. We have
//  * MAX_RELATION_LENGTH = 5, meaning the output of a relation can atmost be a degree 5 polynomial. Hence,
//  * we use a method compute_mock_extended_edges() which starts with degree one input polynomial (two evaluation
//  points),
//  * extends them (using barycentric formula) to six evaluation points, and stores them to an array of polynomials.
//  */
// static const size_t univariate_length = 2;
// static const size_t extension_length = 5;

// namespace honk_relation_tests {

// template <class FF> class SumcheckRelation : public testing::Test {
//   public:
//     template <size_t t> using Univariate = Univariate<FF, t>;
//     template <size_t t> using UnivariateView = UnivariateView<FF, t>;
//     static constexpr size_t FULL_RELATION_LENGH = 5;
//     static const size_t NUM_POLYNOMIALS = bonk::StandardArithmetization::NUM_POLYNOMIALS;
//     using POLYNOMIAL = bonk::StandardArithmetization::POLYNOMIAL;

//     static std::array<Univariate<extension_length>, bonk::StandardArithmetization::NUM_POLYNOMIALS>
//     compute_mock_extended_edges(Univariate<univariate_length>& input_univariate)
//     {
//         BarycentricData<FF, univariate_length, extension_length> barycentric_2_to_max =
//             BarycentricData<FF, univariate_length, extension_length>();
//         auto extended_univariate = barycentric_2_to_max.extend(input_univariate);
//         auto w_l = extended_univariate;
//         auto w_r = extended_univariate;
//         auto w_o = extended_univariate;
//         auto z_perm = extended_univariate;
//         // Note: z_perm and z_perm_shift can be any linear poly for the sake of the tests but should not be be equal
//         to
//         // each other In order to avoid a trivial computation in the case of the grand_product_computation_relation.
//         // Values here were chosen so that output univariates in tests are small positive numbers.
//         auto z_perm_shift = extended_univariate;
//         auto q_m = extended_univariate;
//         auto q_l = extended_univariate;
//         auto q_r = extended_univariate;
//         auto q_o = extended_univariate;
//         auto q_c = extended_univariate;
//         auto sigma_1 = extended_univariate;
//         auto sigma_2 = extended_univariate;
//         auto sigma_3 = extended_univariate;
//         auto id_1 = extended_univariate;
//         auto id_2 = extended_univariate;
//         auto id_3 = extended_univariate;
//         auto lagrange_first = extended_univariate;
//         auto lagrange_last = extended_univariate;

//         // Construct extended edges array in order determined by enum
//         std::array<Univariate<extension_length>, bonk::StandardArithmetization::NUM_POLYNOMIALS> extended_edges;
//         extended_edges[POLYNOMIAL::W_L] = w_l;
//         extended_edges[POLYNOMIAL::W_R] = w_r;
//         extended_edges[POLYNOMIAL::W_O] = w_o;
//         extended_edges[POLYNOMIAL::Z_PERM] = z_perm;
//         extended_edges[POLYNOMIAL::Z_PERM_SHIFT] = z_perm_shift;
//         extended_edges[POLYNOMIAL::Q_M] = q_m;
//         extended_edges[POLYNOMIAL::Q_L] = q_l;
//         extended_edges[POLYNOMIAL::Q_R] = q_r;
//         extended_edges[POLYNOMIAL::Q_O] = q_o;
//         extended_edges[POLYNOMIAL::Q_C] = q_c;
//         extended_edges[POLYNOMIAL::SIGMA_1] = sigma_1;
//         extended_edges[POLYNOMIAL::SIGMA_2] = sigma_2;
//         extended_edges[POLYNOMIAL::SIGMA_3] = sigma_3;
//         extended_edges[POLYNOMIAL::ID_1] = id_1;
//         extended_edges[POLYNOMIAL::ID_2] = id_2;
//         extended_edges[POLYNOMIAL::ID_3] = id_3;
//         extended_edges[POLYNOMIAL::LAGRANGE_FIRST] = lagrange_first;
//         extended_edges[POLYNOMIAL::LAGRANGE_LAST] = lagrange_last;

//         return extended_edges;
//     }

//     /**
//      * @brief Returns randomly sampled parameters to feed to the relations.
//      *
//      * @return RelationParameters<FF>
//      */
//     RelationParameters<FF> compute_mock_relation_parameters()
//     {
//         return { .zeta = FF::random_element(),
//                  .alpha = FF::random_element(),
//                  .beta = FF::random_element(),
//                  .gamma = FF::random_element(),
//                  .public_input_delta = FF::random_element() };
//     }

//     /**
//      * @brief Given an array of Univariates, create a new array containing only the i-th evaluations
//      * of all the univariates.
//      *
//      * @note Not really optimized, mainly used for testing that the relations evaluate to the same value when
//      * evaluated as Univariates, Expressions, or index-by-index
//      * @todo Maybe this is more helpful as part of a `check_logic` function.
//      *
//      * @tparam NUM_UNIVARIATES number of univariates in the input array (deduced from `univariates`)
//      * @tparam univariate_length number of evaluations (deduced from `univariates`)
//      * @param univariates array of Univariates
//      * @param i index of the evaluations we want to take from each univariate
//      * @return std::array<FF, NUM_UNIVARIATES> such that result[j] = univariates[j].value_at(i)
//      */
//     template <std::size_t NUM_UNIVARIATES, size_t univariate_length>
//     static std::array<FF, NUM_UNIVARIATES> transposed_univariate_array_at(
//         const std::array<Univariate<univariate_length>, NUM_UNIVARIATES>& univariates, size_t i)
//     {
//         ASSERT(i < univariate_length);
//         std::array<FF, NUM_UNIVARIATES> result;
//         for (size_t j = 0; j < NUM_UNIVARIATES; ++j) {
//             result[j] = univariates[j].value_at(i);
//         }
//         return result;
//     };

//     /**
//      * @brief Compute the evaluation of a `relation` in different ways, comparing it to the provided `expected_evals`
//      *
//      * @details Check both `add_full_relation_value_contribution` and `add_edge_contribution` by comparing the result
//      to
//      * the `expected_evals` computed by the caller.
//      * Ensures that the relations compute the same result as the expression given in the tests.
//      *
//      * @param expected_evals Relation evaluation computed by the caller.
//      * @param relation being tested
//      * @param extended_edges
//      * @param relation_parameters
//      */
//     static void validate_evaluations(const Univariate<FULL_RELATION_LENGH>& expected_evals,
//                                      const auto relation,
//                                      const std::array<Univariate<FULL_RELATION_LENGH>, NUM_POLYNOMIALS>&
//                                      extended_edges, const RelationParameters<FF>& relation_parameters)
//     {

//         // Compute the expression index-by-index
//         Univariate<FULL_RELATION_LENGH> expected_evals_index{ 0 };
//         for (size_t i = 0; i < FULL_RELATION_LENGH; ++i) {
//             // Get an array of the same size as `extended_edges` with only the i-th element of each extended edge.
//             std::array evals_i = transposed_univariate_array_at(extended_edges, i);
//             // Evaluate the relation
//             relation.add_full_relation_value_contribution(
//                 expected_evals_index.value_at(i), evals_i, relation_parameters);
//         }
//         EXPECT_EQ(expected_evals, expected_evals_index);

//         // Compute the expression using the class, that converts the extended edges to UnivariateView
//         auto expected_evals_view = Univariate<relation.RELATION_LENGTH>(0);
//         relation.add_edge_contribution(expected_evals_view, extended_edges, relation_parameters, 1);

//         // Tiny hack to reduce `expected_evals` to be of size `relation.RELATION_LENGTH`
//         Univariate<relation.RELATION_LENGTH> expected_evals_restricted{ UnivariateView<relation.RELATION_LENGTH>(
//             expected_evals) };
//         EXPECT_EQ(expected_evals_restricted, expected_evals_view);
//     };
// };
// using FieldTypes = testing::Types<barretenberg::fr>;
// TYPED_TEST_SUITE(SumcheckRelation, FieldTypes);

// #define SUMCHECK_RELATION_TYPE_ALIASES using FF = TypeParam;

// TYPED_TEST(SumcheckRelation, ArithmeticRelation)
// {
//     SUMCHECK_RELATION_TYPE_ALIASES
//     auto run_test = [](bool is_random_input) {
//         const auto relation_parameters = TestFixture::compute_mock_relation_parameters();
//         using MULTIVARIATE = bonk::StandardArithmetization::POLYNOMIAL;
//         std::array<Univariate<FF, extension_length>, bonk::StandardArithmetization::NUM_POLYNOMIALS> extended_edges;
//         if (!is_random_input) {
//             // evaluation form, i.e. input_univariate(0) = 1, input_univariate(1) = 2,.. The poly is x+1.
//             auto input_univariate = Univariate<FF, univariate_length>({ 1, 2 });
//             extended_edges = TestFixture::compute_mock_extended_edges(input_univariate);
//         } else {
//             // input_univariate a random poly of degree one
//             auto input_univariate = Univariate<FF, univariate_length>({ FF::random_element(), FF::random_element()
//             }); extended_edges = TestFixture::compute_mock_extended_edges(input_univariate);
//         };
//         auto relation = ArithmeticRelation<FF>();
//         // Manually compute the expected edge contribution
//         const auto& w_l = extended_edges[MULTIVARIATE::W_L];
//         const auto& w_r = extended_edges[MULTIVARIATE::W_R];
//         const auto& w_o = extended_edges[MULTIVARIATE::W_O];
//         const auto& q_m = extended_edges[MULTIVARIATE::Q_M];
//         const auto& q_l = extended_edges[MULTIVARIATE::Q_L];
//         const auto& q_r = extended_edges[MULTIVARIATE::Q_R];
//         const auto& q_o = extended_edges[MULTIVARIATE::Q_O];
//         const auto& q_c = extended_edges[MULTIVARIATE::Q_C];

//         // We first compute the evaluations using UnivariateViews, with the provided hard-coded formula.
//         // Ensure that expression changes are detected.
//         // expected_evals, length 4, extends to { { 5, 22, 57, 116, 205} };
//         auto expected_evals = (q_m * w_r * w_l) + (q_r * w_r) + (q_l * w_l) + (q_o * w_o) + (q_c);
//         validate_evaluations(expected_evals, relation, extended_edges, relation_parameters);
//     };
//     run_test(/* is_random_input=*/true);
//     run_test(/* is_random_input=*/false);
// };

// TYPED_TEST(SumcheckRelation, GrandProductComputationRelation)
// {
//     SUMCHECK_RELATION_TYPE_ALIASES
//     auto run_test = [](bool is_random_input) {
//         const auto relation_parameters = TestFixture::compute_mock_relation_parameters();

//         auto relation = GrandProductComputationRelation<FF>();
//         // Manually compute the expected edge contribution
//         using MULTIVARIATE = bonk::StandardArithmetization::POLYNOMIAL;
//         std::array<Univariate<FF, extension_length>, bonk::StandardArithmetization::NUM_POLYNOMIALS> extended_edges;
//         if (!is_random_input) {
//             // evaluation form, i.e. input_univariate(0) = 1, input_univariate(1) = 2,.. The poly is x+1.
//             auto input_univariate = Univariate<FF, univariate_length>({ 1, 2 });
//             const auto extended_edges = TestFixture::compute_mock_extended_edges(input_univariate);
//         } else {
//             // input_univariate a random poly of degree one
//             auto input_univariate = Univariate<FF, univariate_length>({ FF::random_element(), FF::random_element()
//             }); const auto extended_edges = TestFixture::compute_mock_extended_edges(input_univariate);
//         };
//         const auto& beta = relation_parameters.beta;
//         const auto& gamma = relation_parameters.gamma;
//         const auto& public_input_delta = relation_parameters.public_input_delta;
//         // TODO(luke): Write a test that illustrates the following?
//         // Note: the below z_perm_shift = X^2 will fail because it results in a relation of degree 2*1*1*1 = 5 which
//         // cannot be represented by 5 points. Therefore when we do the calculation then barycentrically extend, we
//         are
//         // effectively exprapolating a 4th degree polynomial instead of the correct 5th degree poly
//         // auto z_perm_shift = Univariate<FF, 5>({ 1, 4, 9, 16, 25 }); // X^2

//         // Manually compute the expected edge contribution
//         const auto& w_1 = extended_edges[MULTIVARIATE::W_L];
//         const auto& w_2 = extended_edges[MULTIVARIATE::W_R];
//         const auto& w_3 = extended_edges[MULTIVARIATE::W_O];
//         const auto& sigma_1 = extended_edges[MULTIVARIATE::SIGMA_1];
//         const auto& sigma_2 = extended_edges[MULTIVARIATE::SIGMA_2];
//         const auto& sigma_3 = extended_edges[MULTIVARIATE::SIGMA_3];
//         const auto& id_1 = extended_edges[MULTIVARIATE::ID_1];
//         const auto& id_2 = extended_edges[MULTIVARIATE::ID_1];
//         const auto& id_3 = extended_edges[MULTIVARIATE::ID_1];
//         const auto& z_perm = extended_edges[MULTIVARIATE::Z_PERM];
//         const auto& z_perm_shift = extended_edges[MULTIVARIATE::Z_PERM_SHIFT];
//         const auto& lagrange_first = extended_edges[MULTIVARIATE::LAGRANGE_FIRST];
//         const auto& lagrange_last = extended_edges[MULTIVARIATE::LAGRANGE_LAST];

//         // We first compute the evaluations using UnivariateViews, with the provided hard-coded formula.
//         // Ensure that expression changes are detected.
//         // expected_evals in the below step { { 27, 250, 1029, 2916, 6655 } } - { { 27, 125, 343, 729, 1331 } }
//         auto expected_evals = (z_perm + lagrange_first) * (w_1 + id_1 * beta + gamma) * (w_2 + id_2 * beta + gamma) *
//                                   (w_3 + id_3 * beta + gamma) -
//                               (z_perm_shift + lagrange_last * public_input_delta) * (w_1 + sigma_1 * beta + gamma) *
//                                   (w_2 + sigma_2 * beta + gamma) * (w_3 + sigma_3 * beta + gamma);

//         validate_evaluations(expected_evals, relation, extended_edges, relation_parameters);
//     };
//     run_test(/* is_random_input=*/true);
//     run_test(/* is_random_input=*/false);
// };

// // TYPED_TEST(SumcheckRelation, GrandProductInitializationRelation)
// // {
// //     SUMCHECK_RELATION_TYPE_ALIASES
// //     auto run_test = [](bool is_random_input) {
// //         auto relation = GrandProductInitializationRelation<FF>();
// //         // Manually compute the expected edge contribution
// //         using MULTIVARIATE = bonk::StandardArithmetization::POLYNOMIAL;
// //         const auto relation_parameters = TestFixture::compute_mock_relation_parameters();
// //         if (!is_random_input) {
// //             // evaluation form, i.e. input_univariate(0) = 1, input_univariate(1) = 2,.. The poly is x+1.
// //             auto input_univariate = Univariate<FF, univariate_length>({ 1, 2 });
// //             const auto extended_edges = TestFixture::compute_mock_extended_edges(input_univariate);
// //         } else {
// //             // input_univariate a random poly of degree one
// //             auto input_univariate = Univariate<FF, univariate_length>({ FF::random_element(), FF::random_element()
// });
// //             const auto extended_edges = TestFixture::compute_mock_extended_edges(input_univariate);
// //         };

// //         const auto& z_perm_shift = extended_edges[MULTIVARIATE::Z_PERM_SHIFT];
// //         const auto& lagrange_last = extended_edges[MULTIVARIATE::LAGRANGE_LAST];
// //         // We first compute the evaluations using UnivariateViews, with the provided hard-coded formula.
// //         // Ensure that expression changes are detected.
// //         // expected_evals, lenght 3 (coeff form = x^2 + x), extends to { { 0, 2, 6, 12, 20 } }
// //         auto expected_evals = z_perm_shift * lagrange_last;

// //         validate_evaluations(expected_evals, relation, extended_edges, relation_parameters);
// //     };
// //     run_test(/* is_random_input=*/true);
// //     run_test(/* is_random_input=*/false);
// // };

// } // namespace honk_relation_tests