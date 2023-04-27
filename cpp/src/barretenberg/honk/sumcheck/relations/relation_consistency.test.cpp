#include "barretenberg/honk/sumcheck/relations/lookup_grand_product_relation.hpp"
#include "barretenberg/honk/sumcheck/relations/ultra_arithmetic_relation.hpp"
#include "barretenberg/honk/sumcheck/relations/ultra_arithmetic_relation_secondary.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"
#include "relation.hpp"
#include "arithmetic_relation.hpp"
#include "grand_product_initialization_relation.hpp"
#include "grand_product_computation_relation.hpp"
#include "../polynomials/univariate.hpp"
#include "../polynomials/barycentric_data.hpp"

#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/numeric/random/engine.hpp"

#include <cstddef>
#include <gtest/gtest.h>
using namespace proof_system::honk::sumcheck;
/**
 * We want to test if all three relations (namely, ArithmeticRelation, GrandProductComputationRelation,
 * GrandProductInitializationRelation) provide correct contributions by manually computing their
 * contributions with deterministic and random inputs. The relations are supposed to work with
 * univariates (edges) of degree one (length 2) and spit out polynomials of corresponding degrees. We have
 * MAX_RELATION_LENGTH = 5, meaning the output of a relation can atmost be a degree 5 polynomial. Hence,
 * we use a method compute_mock_extended_edges() which starts with degree one input polynomial (two evaluation
 points),
 * extends them (using barycentric formula) to six evaluation points, and stores them to an array of polynomials.
 */
static const size_t INPUT_UNIVARIATE_LENGTH = 2;

namespace proof_system::honk_relation_tests {

class StandardRelationConsistency : public testing::Test {
  public:
    using Flavor = honk::flavor::Standard;
    using FF = typename Flavor::FF;
    using PurportedEvaluations = typename Flavor::PurportedEvaluations;
    // TODO(#390): Move MAX_RELATION_LENGTH into Flavor and simplify this.

    template <size_t t> using ExtendedEdges = typename Flavor::template ExtendedEdges<t>;

    // TODO(#225)(Adrian): Accept FULL_RELATION_LENGTH as a template parameter for this function only, so that the test
    // can decide to which degree the polynomials must be extended. Possible accept an existing list of "edges" and
    // extend them to the degree.
    template <size_t FULL_RELATION_LENGTH, size_t NUM_POLYNOMIALS>
    static void compute_mock_extended_edges(
        ExtendedEdges<FULL_RELATION_LENGTH>& extended_edges,
        std::array<Univariate<FF, INPUT_UNIVARIATE_LENGTH>, NUM_POLYNOMIALS>& input_edges)
    {
        BarycentricData<FF, INPUT_UNIVARIATE_LENGTH, FULL_RELATION_LENGTH> barycentric_2_to_max =
            BarycentricData<FF, INPUT_UNIVARIATE_LENGTH, FULL_RELATION_LENGTH>();
        for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
            extended_edges[i] = barycentric_2_to_max.extend(input_edges[i]);
        }
    }

    /**
     * @brief Returns randomly sampled parameters to feed to the relations.
     *
     * @return RelationParameters<FF>
     */
    RelationParameters<FF> compute_mock_relation_parameters()
    {
        return { .eta = FF::random_element(),
                 .beta = FF::random_element(),
                 .gamma = FF::random_element(),
                 .public_input_delta = FF::random_element(),
                 .lookup_grand_product_delta = FF::random_element() };
    }

    /**
     * @brief Given an array of Univariates, create a new array containing only the i-th evaluations
     * of all the univariates.
     *
     * @note Not really optimized, mainly used for testing that the relations evaluate to the same value when
     * evaluated as Univariates, Expressions, or index-by-index
     * @todo(Adrian) Maybe this is more helpful as part of a `check_logic` function.
     *
     * @tparam NUM_UNIVARIATES number of univariates in the input array (deduced from `univariates`)
     * @tparam univariate_length number of evaluations (deduced from `univariates`)
     * @param univariates array of Univariates
     * @param i index of the evaluations we want to take from each univariate
     * @return std::array<FF, NUM_UNIVARIATES> such that result[j] = univariates[j].value_at(i)
     */
    template <size_t univariate_length>
    static PurportedEvaluations transposed_univariate_array_at(ExtendedEdges<univariate_length> univariates, size_t i)
    {
        ASSERT(i < univariate_length);
        std::array<FF, Flavor::NUM_ALL_ENTITIES> result;
        size_t result_idx = 0; // TODO(#391) zip
        for (auto& univariate : univariates) {
            result[result_idx] = univariate.value_at(i);
            ++result_idx;
        }
        return result;
    };

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
    template <size_t FULL_RELATION_LENGTH>
    static void validate_evaluations(const Univariate<FF, FULL_RELATION_LENGTH>& expected_evals,
                                     const auto relation,
                                     const ExtendedEdges<FULL_RELATION_LENGTH>& extended_edges,
                                     const RelationParameters<FF>& relation_parameters)
    {

        // Compute the expression index-by-index
        Univariate<FF, FULL_RELATION_LENGTH> expected_evals_index{ 0 };
        for (size_t i = 0; i < FULL_RELATION_LENGTH; ++i) {
            // Get an array of the same size as `extended_edges` with only the i-th element of each extended edge.
            PurportedEvaluations evals_i = transposed_univariate_array_at(extended_edges, i);
            // Evaluate the relation
            relation.add_full_relation_value_contribution(
                expected_evals_index.value_at(i), evals_i, relation_parameters);
        }
        EXPECT_EQ(expected_evals, expected_evals_index);

        // Compute the expression using the class, that converts the extended edges to UnivariateView
        auto expected_evals_view = Univariate<FF, relation.RELATION_LENGTH>(0);
        // The scaling factor is essentially 1 since we are working with degree 1 univariates
        relation.add_edge_contribution(expected_evals_view, extended_edges, relation_parameters, 1);

        // Tiny hack to reduce `expected_evals` to be of size `relation.RELATION_LENGTH`
        Univariate<FF, relation.RELATION_LENGTH> expected_evals_restricted{
            UnivariateView<FF, relation.RELATION_LENGTH>(expected_evals)
        };
        EXPECT_EQ(expected_evals_restricted, expected_evals_view);
    };
};

TEST_F(StandardRelationConsistency, ArithmeticRelation)
{
    using Flavor = honk::flavor::Standard;
    using FF = typename Flavor::FF;
    static constexpr size_t FULL_RELATION_LENGTH = 5;
    using ExtendedEdges = typename Flavor::template ExtendedEdges<FULL_RELATION_LENGTH>;
    static const size_t NUM_POLYNOMIALS = Flavor::NUM_ALL_ENTITIES;

    const auto relation_parameters = compute_mock_relation_parameters();
    auto run_test = [&relation_parameters](bool is_random_input) {
        std::array<Univariate<FF, INPUT_UNIVARIATE_LENGTH>, NUM_POLYNOMIALS> input_polynomials;
        ExtendedEdges extended_edges;
        if (!is_random_input) {
            // evaluation form, i.e. input_univariate(0) = 1, input_univariate(1) = 2,.. The polynomial is x+1.
            for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
                input_polynomials[i] = Univariate<FF, INPUT_UNIVARIATE_LENGTH>({ 1, 2 });
            }
            compute_mock_extended_edges<FULL_RELATION_LENGTH>(extended_edges, input_polynomials);
        } else {
            // input_univariates are random polynomials of degree one
            for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
                input_polynomials[i] =
                    Univariate<FF, INPUT_UNIVARIATE_LENGTH>({ FF::random_element(), FF::random_element() });
            }
            compute_mock_extended_edges<FULL_RELATION_LENGTH>(extended_edges, input_polynomials);
        };
        auto relation = ArithmeticRelation<FF>();
        // Manually compute the expected edge contribution
        const auto& w_l = extended_edges.w_l;
        const auto& w_r = extended_edges.w_r;
        const auto& w_o = extended_edges.w_o;
        const auto& q_m = extended_edges.q_m;
        const auto& q_l = extended_edges.q_l;
        const auto& q_r = extended_edges.q_r;
        const auto& q_o = extended_edges.q_o;
        const auto& q_c = extended_edges.q_c;

        // We first compute the evaluations using UnivariateViews, with the provided hard-coded formula.
        // Ensure that expression changes are detected.
        // expected_evals, length 4, extends to { { 5, 22, 57, 116, 205} } for input polynomial {1, 2}
        auto expected_evals = (q_m * w_r * w_l) + (q_r * w_r) + (q_l * w_l) + (q_o * w_o) + (q_c);
        validate_evaluations(expected_evals, relation, extended_edges, relation_parameters);
    };
    run_test(/* is_random_input=*/true);
    run_test(/* is_random_input=*/false);
};

TEST_F(StandardRelationConsistency, GrandProductComputationRelation)
{
    using Flavor = honk::flavor::Standard;
    using FF = typename Flavor::FF;
    static constexpr size_t FULL_RELATION_LENGTH = 5;
    using ExtendedEdges = typename Flavor::template ExtendedEdges<FULL_RELATION_LENGTH>;
    static const size_t NUM_POLYNOMIALS = Flavor::NUM_ALL_ENTITIES;

    const auto relation_parameters = compute_mock_relation_parameters();
    auto run_test = [&relation_parameters](bool is_random_input) {
        ExtendedEdges extended_edges;
        std::array<Univariate<FF, INPUT_UNIVARIATE_LENGTH>, NUM_POLYNOMIALS> input_polynomials;
        if (!is_random_input) {
            // evaluation form, i.e. input_univariate(0) = 1, input_univariate(1) = 2,.. The polynomial is x+1.
            for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
                input_polynomials[i] = Univariate<FF, INPUT_UNIVARIATE_LENGTH>({ 1, 2 });
            }
            compute_mock_extended_edges<FULL_RELATION_LENGTH>(extended_edges, input_polynomials);
        } else {
            // input_univariates are random polynomials of degree one
            for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
                input_polynomials[i] =
                    Univariate<FF, INPUT_UNIVARIATE_LENGTH>({ FF::random_element(), FF::random_element() });
            }
            compute_mock_extended_edges<FULL_RELATION_LENGTH>(extended_edges, input_polynomials);
        };
        auto relation = GrandProductComputationRelation<FF>();

        const auto& beta = relation_parameters.beta;
        const auto& gamma = relation_parameters.gamma;
        const auto& public_input_delta = relation_parameters.public_input_delta;
        // TODO(#225)(luke): Write a test that illustrates the following?
        // Note: the below z_perm_shift = X^2 will fail because it results in a relation of degree 2*1*1*1 = 5 which
        // cannot be represented by 5 points. Therefore when we do the calculation then barycentrically extend, we are
        // effectively exprapolating a 4th degree polynomial instead of the correct 5th degree poly
        // auto z_perm_shift = Univariate<FF, 5>({ 1, 4, 9, 16, 25 }); // X^2

        // Manually compute the expected edge contribution
        const auto& w_1 = extended_edges.w_l;
        const auto& w_2 = extended_edges.w_r;
        const auto& w_3 = extended_edges.w_o;
        const auto& sigma_1 = extended_edges.sigma_1;
        const auto& sigma_2 = extended_edges.sigma_2;
        const auto& sigma_3 = extended_edges.sigma_3;
        const auto& id_1 = extended_edges.id_1;
        const auto& id_2 = extended_edges.id_2;
        const auto& id_3 = extended_edges.id_3;
        const auto& z_perm = extended_edges.z_perm;
        const auto& z_perm_shift = extended_edges.z_perm_shift;
        const auto& lagrange_first = extended_edges.lagrange_first;
        const auto& lagrange_last = extended_edges.lagrange_last;

        // We first compute the evaluations using UnivariateViews, with the provided hard-coded formula.
        // Ensure that expression changes are detected.
        // expected_evals in the below step { { 27, 250, 1029, 2916, 6655 } } - { { 27, 125, 343, 729, 1331 } }
        auto expected_evals = (z_perm + lagrange_first) * (w_1 + id_1 * beta + gamma) * (w_2 + id_2 * beta + gamma) *
                                  (w_3 + id_3 * beta + gamma) -
                              (z_perm_shift + lagrange_last * public_input_delta) * (w_1 + sigma_1 * beta + gamma) *
                                  (w_2 + sigma_2 * beta + gamma) * (w_3 + sigma_3 * beta + gamma);

        validate_evaluations(expected_evals, relation, extended_edges, relation_parameters);
    };
    run_test(/* is_random_input=*/true);
    run_test(/* is_random_input=*/false);
};

TEST_F(StandardRelationConsistency, GrandProductInitializationRelation)
{
    using Flavor = honk::flavor::Standard;
    using FF = typename Flavor::FF;
    static constexpr size_t FULL_RELATION_LENGTH = 5;
    using ExtendedEdges = typename Flavor::template ExtendedEdges<FULL_RELATION_LENGTH>;
    static const size_t NUM_POLYNOMIALS = Flavor::NUM_ALL_ENTITIES;

    const auto relation_parameters = compute_mock_relation_parameters();
    auto run_test = [&relation_parameters](bool is_random_input) {
        ExtendedEdges extended_edges;
        std::array<Univariate<FF, INPUT_UNIVARIATE_LENGTH>, NUM_POLYNOMIALS> input_polynomials;
        if (!is_random_input) {
            // evaluation form, i.e. input_univariate(0) = 1, input_univariate(1) = 2,.. The polynomial is x+1.
            for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
                input_polynomials[i] = Univariate<FF, INPUT_UNIVARIATE_LENGTH>({ 1, 2 });
            }
            compute_mock_extended_edges<FULL_RELATION_LENGTH>(extended_edges, input_polynomials);
        } else {
            // input_univariates are random polynomials of degree one
            for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
                input_polynomials[i] =
                    Univariate<FF, INPUT_UNIVARIATE_LENGTH>({ FF::random_element(), FF::random_element() });
            }
            compute_mock_extended_edges<FULL_RELATION_LENGTH>(extended_edges, input_polynomials);
        };
        auto relation = GrandProductInitializationRelation<FF>();
        const auto& z_perm_shift = extended_edges.z_perm_shift;
        const auto& lagrange_last = extended_edges.lagrange_last;
        // We first compute the evaluations using UnivariateViews, with the provided hard-coded formula.
        // Ensure that expression changes are detected.
        // expected_evals, lenght 3 (coeff form = x^2 + x), extends to { { 0, 2, 6, 12, 20 } }
        auto expected_evals = z_perm_shift * lagrange_last;

        validate_evaluations(expected_evals, relation, extended_edges, relation_parameters);
    };
    run_test(/* is_random_input=*/true);
    run_test(/* is_random_input=*/false);
};

} // namespace proof_system::honk_relation_tests
