#include "relation.hpp"
#include <proof_system/flavor/flavor.hpp>
#include "arithmetic_relation.hpp"
#include "grand_product_initialization_relation.hpp"
#include "grand_product_computation_relation.hpp"
#include "../polynomials/univariate.hpp"

#include <ecc/curves/bn254/fr.hpp>
#include <numeric/random/engine.hpp>

#include <gtest/gtest.h>

using namespace honk::sumcheck;

namespace honk_relation_tests {

template <class FF> class SumcheckRelation : public testing::Test {
  public:
    template <size_t t> using Univariate = Univariate<FF, t>;
    template <size_t t> using UnivariateView = UnivariateView<FF, t>;
    static constexpr size_t FULL_RELATION_LENGH = 5;
    static const size_t NUM_POLYNOMIALS = bonk::StandardArithmetization::NUM_POLYNOMIALS;
    using POLYNOMIAL = bonk::StandardArithmetization::POLYNOMIAL;

    // TODO(luke): may want to make this more flexible/genericzs
    static std::array<Univariate<FULL_RELATION_LENGH>, NUM_POLYNOMIALS> compute_mock_extended_edges()
    {
        // TODO(Cody): build from Univariate<2>'s?
        // evaluation form, i.e. w_l(0) = 1, w_l(1) = 2,.. The poly is x+1.
        auto w_l = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto w_r = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto w_o = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto z_perm = Univariate<5>({ 1, 2, 3, 4, 5 });
        // Note: z_perm and z_perm_shift can be any linear poly for the sake of the tests but should not be be equal to
        // each other In order to avoid a trivial computation in the case of the grand_product_computation_relation.
        // Values here were chosen so that output univariates in tests are small positive numbers.
        auto z_perm_shift = Univariate<5>({ 0, 1, 2, 3, 4 }); // this is not real shifted data
        auto q_m = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto q_l = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto q_r = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto q_o = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto q_c = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto sigma_1 = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto sigma_2 = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto sigma_3 = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto id_1 = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto id_2 = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto id_3 = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto lagrange_first = Univariate<5>({ 1, 2, 3, 4, 5 });
        auto lagrange_last = Univariate<5>({ 1, 2, 3, 4, 5 });

        // Construct extended edges array in order determined by enum
        std::array<Univariate<5>, bonk::StandardArithmetization::NUM_POLYNOMIALS> extended_edges;
        extended_edges[POLYNOMIAL::W_L] = w_l;
        extended_edges[POLYNOMIAL::W_R] = w_r;
        extended_edges[POLYNOMIAL::W_O] = w_o;
        extended_edges[POLYNOMIAL::Z_PERM] = z_perm;
        extended_edges[POLYNOMIAL::Z_PERM_SHIFT] = z_perm_shift;
        extended_edges[POLYNOMIAL::Q_M] = q_m;
        extended_edges[POLYNOMIAL::Q_L] = q_l;
        extended_edges[POLYNOMIAL::Q_R] = q_r;
        extended_edges[POLYNOMIAL::Q_O] = q_o;
        extended_edges[POLYNOMIAL::Q_C] = q_c;
        extended_edges[POLYNOMIAL::SIGMA_1] = sigma_1;
        extended_edges[POLYNOMIAL::SIGMA_2] = sigma_2;
        extended_edges[POLYNOMIAL::SIGMA_3] = sigma_3;
        extended_edges[POLYNOMIAL::ID_1] = id_1;
        extended_edges[POLYNOMIAL::ID_2] = id_2;
        extended_edges[POLYNOMIAL::ID_3] = id_3;
        extended_edges[POLYNOMIAL::LAGRANGE_FIRST] = lagrange_first;
        extended_edges[POLYNOMIAL::LAGRANGE_LAST] = lagrange_last;

        return extended_edges;
    }

    /**
     * @brief Returns randomly sampled parameters to feed to the relations.
     *
     * @return RelationParameters<FF>
     */
    RelationParameters<FF> compute_mock_relation_parameters()
    {
        return { .zeta = FF::random_element(),
                 .alpha = FF::random_element(),
                 .beta = FF::random_element(),
                 .gamma = FF::random_element(),
                 .public_input_delta = FF::random_element() };
    }

    /**
     * @brief Compute the evaluation of a `relation` in different ways, comparing it to the provided `expected_evals`
     *
     * @details Check both `add_full_relation_value_contribution` and `add_edge_contribution` by comparing the result to
     * the `expected_evals` computed by the caller.
     * Ensures that the relations compute the same result as the expression given in the tests.
     *
     * @param expected_evals Relation evaluation computed by the caller.
     * @param relation being tested
     * @param extended_edges
     * @param relation_parameters
     */
    static void validate_evaluations(const Univariate<FULL_RELATION_LENGH>& expected_evals,
                                     const auto relation,
                                     const std::array<Univariate<FULL_RELATION_LENGH>, NUM_POLYNOMIALS>& extended_edges,
                                     const RelationParameters<FF>& relation_parameters)
    {

        // Compute the expression index-by-index
        Univariate<FULL_RELATION_LENGH> expected_evals_index{ 0 };
        for (size_t i = 0; i < FULL_RELATION_LENGH; ++i) {
            // Get an array of the same size as `extended_edges` with only the i-th element of each extended edge.
            std::array evals_i = transposed_univariate_array_at(extended_edges, i);
            // Evaluate the relation
            relation.add_full_relation_value_contribution(
                expected_evals_index.value_at(i), evals_i, relation_parameters);
        }
        EXPECT_EQ(expected_evals, expected_evals_index);

        // Compute the expression using the class, that converts the extended edges to UnivariateView
        auto expected_evals_view = Univariate<relation.RELATION_LENGTH>(0);
        relation.add_edge_contribution(expected_evals_view, extended_edges, relation_parameters, 1);

        // Tiny hack to reduce `expected_evals` to be of size `relation.RELATION_LENGTH`
        Univariate<relation.RELATION_LENGTH> expected_evals_restricted{ UnivariateView<relation.RELATION_LENGTH>(
            expected_evals) };
        EXPECT_EQ(expected_evals_restricted, expected_evals_view);
    };
};

using FieldTypes = testing::Types<barretenberg::fr>;
TYPED_TEST_SUITE(SumcheckRelation, FieldTypes);

#define SUMCHECK_RELATION_TYPE_ALIASES using FF = TypeParam;

TYPED_TEST(SumcheckRelation, ArithmeticRelation)
{
    SUMCHECK_RELATION_TYPE_ALIASES
    using MULTIVARIATE = bonk::StandardArithmetization::POLYNOMIAL;

    const auto extended_edges = TestFixture::compute_mock_extended_edges();
    const auto relation_parameters = TestFixture::compute_mock_relation_parameters();

    auto relation = ArithmeticRelation<FF>();
    // Manually compute the expected edge contribution
    const auto& w_l = extended_edges[MULTIVARIATE::W_L];
    const auto& w_r = extended_edges[MULTIVARIATE::W_R];
    const auto& w_o = extended_edges[MULTIVARIATE::W_O];
    const auto& q_m = extended_edges[MULTIVARIATE::Q_M];
    const auto& q_l = extended_edges[MULTIVARIATE::Q_L];
    const auto& q_r = extended_edges[MULTIVARIATE::Q_R];
    const auto& q_o = extended_edges[MULTIVARIATE::Q_O];
    const auto& q_c = extended_edges[MULTIVARIATE::Q_C];

    // We first compute the evaluations using UnivariateViews, with the provided hard-coded formula.
    // Ensure that expression changes are detected.
    // expected_evals, length 4, extends to { { 5, 22, 57, 116, 205} };
    auto expected_evals = (q_m * w_r * w_l) + (q_r * w_r) + (q_l * w_l) + (q_o * w_o) + (q_c);
    this->validate_evaluations(expected_evals, relation, extended_edges, relation_parameters);
};

TYPED_TEST(SumcheckRelation, GrandProductComputationRelation)
{
    SUMCHECK_RELATION_TYPE_ALIASES
    using MULTIVARIATE = bonk::StandardArithmetization::POLYNOMIAL;

    const auto extended_edges = TestFixture::compute_mock_extended_edges();
    const auto relation_parameters = TestFixture::compute_mock_relation_parameters();

    auto relation = GrandProductComputationRelation<FF>();

    const auto& beta = relation_parameters.beta;
    const auto& gamma = relation_parameters.gamma;
    const auto& public_input_delta = relation_parameters.public_input_delta;
    // TODO(luke): Write a test that illustrates the following?
    // Note: the below z_perm_shift = X^2 will fail because it results in a relation of degree 2*1*1*1 = 5 which
    // cannot be represented by 5 points. Therefore when we do the calculation then barycentrically extend, we are
    // effectively exprapolating a 4th degree polynomial instead of the correct 5th degree poly
    // auto z_perm_shift = Univariate<FF, 5>({ 1, 4, 9, 16, 25 }); // X^2

    // Manually compute the expected edge contribution
    const auto& w_1 = extended_edges[MULTIVARIATE::W_L];
    const auto& w_2 = extended_edges[MULTIVARIATE::W_R];
    const auto& w_3 = extended_edges[MULTIVARIATE::W_O];
    const auto& sigma_1 = extended_edges[MULTIVARIATE::SIGMA_1];
    const auto& sigma_2 = extended_edges[MULTIVARIATE::SIGMA_2];
    const auto& sigma_3 = extended_edges[MULTIVARIATE::SIGMA_3];
    const auto& id_1 = extended_edges[MULTIVARIATE::ID_1];
    const auto& id_2 = extended_edges[MULTIVARIATE::ID_1];
    const auto& id_3 = extended_edges[MULTIVARIATE::ID_1];
    const auto& z_perm = extended_edges[MULTIVARIATE::Z_PERM];
    const auto& z_perm_shift = extended_edges[MULTIVARIATE::Z_PERM_SHIFT];
    const auto& lagrange_first = extended_edges[MULTIVARIATE::LAGRANGE_FIRST];
    const auto& lagrange_last = extended_edges[MULTIVARIATE::LAGRANGE_LAST];

    // We first compute the evaluations using UnivariateViews, with the provided hard-coded formula.
    // Ensure that expression changes are detected.
    // expected_evals in the below step { { 27, 250, 1029, 2916, 6655 } } - { { 27, 125, 343, 729, 1331 } }
    auto expected_evals = (z_perm + lagrange_first) * (w_1 + id_1 * beta + gamma) * (w_2 + id_2 * beta + gamma) *
                              (w_3 + id_3 * beta + gamma) -
                          (z_perm_shift + lagrange_last * public_input_delta) * (w_1 + sigma_1 * beta + gamma) *
                              (w_2 + sigma_2 * beta + gamma) * (w_3 + sigma_3 * beta + gamma);

    this->validate_evaluations(expected_evals, relation, extended_edges, relation_parameters);
};

TYPED_TEST(SumcheckRelation, GrandProductInitializationRelation)
{
    SUMCHECK_RELATION_TYPE_ALIASES
    using MULTIVARIATE = bonk::StandardArithmetization::POLYNOMIAL;

    const auto extended_edges = TestFixture::compute_mock_extended_edges();
    const auto relation_parameters = TestFixture::compute_mock_relation_parameters();
    auto relation = GrandProductInitializationRelation<FF>();

    // Manually compute the expected edge contribution

    const auto& z_perm_shift = extended_edges[MULTIVARIATE::Z_PERM_SHIFT];
    const auto& lagrange_last = extended_edges[MULTIVARIATE::LAGRANGE_LAST];
    // We first compute the evaluations using UnivariateViews, with the provided hard-coded formula.
    // Ensure that expression changes are detected.
    // expected_evals, lenght 3 (coeff form = x^2 + x), extends to { { 0, 2, 6, 12, 20 } }
    auto expected_evals = z_perm_shift * lagrange_last;

    this->validate_evaluations(expected_evals, relation, extended_edges, relation_parameters);
};

} // namespace honk_relation_tests