#pragma once
#include <type_traits>

#include "relation.hpp"
#include "../polynomials/univariate.hpp"
#include <barretenberg/common/constexpr_utils.hpp>

// TODO(luke): change name of this file to permutation_grand_product_relation(s).hpp and move 'init' relation into it.

namespace proof_system::honk::sumcheck {

template <typename FF> class GrandProductComputationRelation {
  public:
    constexpr bool scale_by_random_polynomial() { return true; }

    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 5;

    /**
     * @brief Compute contribution of the permutation relation for a given edge (internal function)
     *
     * @details There are 2 relations associated with enforcing the wire copy relations
     * This file handles the relation that confirms faithful calculation of the grand
     * product polynomial Z_perm. (Initialization relation Z_perm(0) = 1 is handled elsewhere).
     *
     *  C(extended_edges(X)...) =
     *      ( z_perm(X) + lagrange_first(X) )*P(X)
     *         - ( z_perm_shift(X) + delta * lagrange_last(X))*Q(X),
     * where P(X) = Prod_{i=1:3} w_i(X) + β*(n*(i-1) + idx(X)) + γ
     *       Q(X) = Prod_{i=1:3} w_i(X) + β*σ_i(X) + γ
     *
     * @param evals transformed to `evals + C(extended_edges(X)...)*scaling_factor`
     * @param extended_edges an std::array containing the fully extended Univariate edges.
     * @param parameters contains beta, gamma, and public_input_delta, ....
     * @param scaling_factor optional term to scale the evaluation before adding to evals.
     */
    inline void add_edge_contribution(Univariate<FF, RELATION_LENGTH>& evals,
                                      const auto& extended_edges,
                                      const RelationParameters<FF>& relation_parameters,
                                      const FF& scaling_factor) const
    {
        const auto& beta = relation_parameters.beta;
        const auto& gamma = relation_parameters.gamma;
        const auto& public_input_delta = relation_parameters.public_input_delta;

        auto w_1 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.w_l);
        auto w_2 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.w_r);
        auto w_3 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.w_o);
        auto sigma_1 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.sigma_1);
        auto sigma_2 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.sigma_2);
        auto sigma_3 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.sigma_3);
        auto id_1 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.id_1);
        auto id_2 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.id_2);
        auto id_3 = UnivariateView<FF, RELATION_LENGTH>(extended_edges.id_3);
        auto z_perm = UnivariateView<FF, RELATION_LENGTH>(extended_edges.z_perm);
        auto z_perm_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges.z_perm_shift);
        auto lagrange_first = UnivariateView<FF, RELATION_LENGTH>(extended_edges.lagrange_first);
        auto lagrange_last = UnivariateView<FF, RELATION_LENGTH>(extended_edges.lagrange_last);

        // Contribution (1)
        evals += (((z_perm + lagrange_first) * (w_1 + id_1 * beta + gamma) * (w_2 + id_2 * beta + gamma) *
                   (w_3 + id_3 * beta + gamma)) -
                  ((z_perm_shift + lagrange_last * public_input_delta) * (w_1 + sigma_1 * beta + gamma) *
                   (w_2 + sigma_2 * beta + gamma) * (w_3 + sigma_3 * beta + gamma))) *
                 scaling_factor;
    };

    void add_full_relation_value_contribution(FF& full_honk_relation_value,
                                              auto& purported_evaluations,
                                              const RelationParameters<FF>& relation_parameters,
                                              const FF& scaling_factor) const
    {
        const auto& beta = relation_parameters.beta;
        const auto& gamma = relation_parameters.gamma;
        const auto& public_input_delta = relation_parameters.public_input_delta;

        auto w_1 = purported_evaluations.w_l;
        auto w_2 = purported_evaluations.w_r;
        auto w_3 = purported_evaluations.w_o;
        auto sigma_1 = purported_evaluations.sigma_1;
        auto sigma_2 = purported_evaluations.sigma_2;
        auto sigma_3 = purported_evaluations.sigma_3;
        auto id_1 = purported_evaluations.id_1;
        auto id_2 = purported_evaluations.id_2;
        auto id_3 = purported_evaluations.id_3;
        auto z_perm = purported_evaluations.z_perm;
        auto z_perm_shift = purported_evaluations.z_perm_shift;
        auto lagrange_first = purported_evaluations.lagrange_first;
        auto lagrange_last = purported_evaluations.lagrange_last;

        // Contribution (1)
        full_honk_relation_value +=
            ((z_perm + lagrange_first) * (w_1 + beta * id_1 + gamma) * (w_2 + beta * id_2 + gamma) *
                 (w_3 + beta * id_3 + gamma) -
             (z_perm_shift + lagrange_last * public_input_delta) * (w_1 + beta * sigma_1 + gamma) *
                 (w_2 + beta * sigma_2 + gamma) * (w_3 + beta * sigma_3 + gamma)) *
            scaling_factor;
    };
};

// TODO(luke): With Cody's Flavor work it should be easier to create a simple templated relation
// for handling arbitrary width. For now I'm duplicating the width 3 logic for width 4.
template <typename FF> class UltraGrandProductComputationRelation {
  public:
    constexpr bool scale_by_random_polynomial() { return true; }

    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 6;

    static constexpr size_t NUMERATOR_TERMS = 4;
    static constexpr size_t DENOMINATOR_TERMS = 4;

    Univariate<FF, RELATION_LENGTH>& get_element(auto& container, const size_t = 0) const
    {
        if constexpr (std::is_same<typeof(container), std::vector<FF>>::value) {
            return container[index];
        }
        return container;
    }

    UnivariateView<FF, RELATION_LENGTH>& get_element_view(auto& container, const size_t = 0) const
    {
        if constexpr (std::is_same<typeof(container), std::vector<FF>>::value) {
            return UnivariateView<FF, RELATION_LENGTH>(container[index]);
        }
        return container;
    }

    template <size_t denominator_index>
    Univariate<FF, RELATION_LENGTH> compute_denominator_term(const auto& extended_edges,
                                                             const RelationParameters<FF>& relation_parameters,
                                                             const size_t index = 0) const
    {
        static_assert(denominator_index < DENOMINATOR_TERMS);
        const auto& w_1 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.w_l, index));
        const auto& w_2 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.w_r, index));
        const auto& w_3 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.w_o, index));
        const auto& w_4 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.w_4, index));
        auto sigma_1 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.sigma_1, index));
        auto sigma_2 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.sigma_2, index));
        auto sigma_3 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.sigma_3, index));
        auto sigma_4 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.sigma_4, index));

        const auto& beta = relation_parameters.beta;
        const auto& gamma = relation_parameters.gamma;

        if constexpr (denominator_index == 0) {
            return (w_1 + sigma_1 * beta + gamma);
        } else if constexpr (denominator_index == 1) {
            return (w_2 + sigma_2 * beta + gamma);
        } else if constexpr (denominator_index == 2) {
            return (w_3 + sigma_3 * beta + gamma);
        }
        return (w_4 + sigma_4 * beta + gamma);
    }

    template <size_t numerator_index>
    Univariate<FF, RELATION_LENGTH> compute_numerator_term(const auto& extended_edges,
                                                           const RelationParameters<FF>& relation_parameters,
                                                           const size_t index = 0) const
    {
        static_assert(numerator_index < NUMERATOR_TERMS);
        const auto& w_1 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.w_l, index));
        const auto& w_2 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.w_r, index));
        const auto& w_3 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.w_o, index));
        const auto& w_4 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.w_4, index));
        auto id_1 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.id_1, index));
        auto id_2 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.id_2, index));
        auto id_3 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.id_3, index));
        auto id_4 = UnivariateView<FF, RELATION_LENGTH>(get_element(extended_edges.id_4, index));

        const auto& beta = relation_parameters.beta;
        const auto& gamma = relation_parameters.gamma;

        if constexpr (numerator_index == 0) {
            return (w_1 + id_1 * beta + gamma);
        } else if constexpr (numerator_index == 1) {
            return (w_2 + id_2 * beta + gamma);
        } else if constexpr (numerator_index == 2) {
            return (w_3 + id_3 * beta + gamma);
        }
        return (w_4 + id_4 * beta + gamma);
    }

    // void compute_grand_product(const auto& extended_edges, const RelationParameters<FF>& relation_parameters) const
    // {
    //     // step 1. iterate over .. polynomials and propagate...
    //     std::array<std::vector<FF>, DENOMINATOR_TERMS> denominator_accumulator;
    //     std::array<std::vector<FF>, NUMERATOR_TERMS> numerator_accumulator;

    //     const size_t polynomial_size = extended_edges[0].size();

    //     barretenberg::constexpr_for<0, DENOMINATOR_TERMS, 1>([&]<size_t denominator_index>() {
    //         denominator_accumulator[denominator_index].reserve(polynomial_size);
    //         for (size_t j = 0; j < polynomial_size; ++j) {
    //             denominator_accumulator[denominator_index].emplace_back(
    //                 compute_denominator_term<denominator_index>(extended_edges, relation_parameters, j));
    //         }

    //         for (size_t j = 0; j < polynomial_size - 1; ++j) {
    //             denominator_accumulator[denominator_index][j + 1] *= denominator_accumulator[denominator_index][j];
    //         }
    //     });
    //     barretenberg::constexpr_for<0, NUMERATOR_TERMS, 1>([&]<size_t numerator_index>() {
    //         numerator_accumulator[numerator_index].reserve(polynomial_size);
    //         for (size_t j = 0; j < polynomial_size; ++j) {
    //             numerator_accumulator[numerator_index] =
    //                 compute_numerator_term<numerator_index>(extended_edges, relation_parameters, j);
    //         }

    //         for (size_t j = 0; j < polynomial_size - 1; ++j) {
    //             numerator_accumulator[numerator_index][j + 1] *= numerator_accumulator[numerator_index][j];
    //         }
    //     });

    //     // Step (4)
    //     // Use Montgomery batch inversion to compute z_perm[i+1] = numerator_accumulator[0][i] /
    //     // denominator_accumulator[0][i]. At the end of this computation, the quotient numerator_accumulator[0] /
    //     // denominator_accumulator[0] is stored in numerator_accumulator[0].
    //     // Note: Since numerator_accumulator[0][i] corresponds to z_lookup[i+1], we only iterate up to i = (n - 2).
    //     FF* inversion_coefficients = &denominator_accumulator[1][0]; // arbitrary scratch space
    //     FF inversion_accumulator = FF::one();
    //     for (size_t i = 0; i < polynomial_size - 1; ++i) {
    //         inversion_coefficients[i] = numerator_accumulator[0][i] * inversion_accumulator;
    //         inversion_accumulator *= denominator_accumulator[0][i];
    //     }
    //     inversion_accumulator = inversion_accumulator.invert(); // perform single inversion per thread
    //     for (size_t i = polynomial_size - 2; i != std::numeric_limits<size_t>::max(); --i) {
    //         numerator_accumulator[0][i] = inversion_accumulator * inversion_coefficients[i];
    //         inversion_accumulator *= denominator_accumulator[0][i];
    //     }

    //     extended_edges.z_perm[0] = 0;
    //     for (size_t i = 0; i < polynomial_size - 1; ++i) {
    //         extended_edges.z_perm[i] = numerator_accumulator[0][i];
    //     }
    // }

    /**
     * @brief Compute contribution of the permutation relation for a given edge (internal function)
     *
     * @details This the relation confirms faithful calculation of the grand
     * product polynomial Z_perm.
     *
     * @param evals transformed to `evals + C(extended_edges(X)...)*scaling_factor`
     * @param extended_edges an std::array containing the fully extended Univariate edges.
     * @param parameters contains beta, gamma, and public_input_delta, ....
     * @param scaling_factor optional term to scale the evaluation before adding to evals.
     */
    inline void add_edge_contribution(Univariate<FF, RELATION_LENGTH>& evals,
                                      const auto& extended_edges,
                                      const RelationParameters<FF>& relation_parameters,
                                      const FF& scaling_factor) const
    {
        const auto& public_input_delta = relation_parameters.public_input_delta;

        Univariate<FF, RELATION_LENGTH> numerator_evaluation = Univariate<FF, RELATION_LENGTH>(1);
        barretenberg::constexpr_for<0, NUMERATOR_TERMS, 1>([&]<size_t numerator_index>() {
            numerator_evaluation *= compute_numerator_term<numerator_index>(extended_edges, relation_parameters, 0);
        });

        Univariate<FF, RELATION_LENGTH> denominator_evaluation = Univariate<FF, RELATION_LENGTH>(1);
        barretenberg::constexpr_for<0, DENOMINATOR_TERMS, 1>([&]<size_t denominator_index>() {
            denominator_evaluation *=
                compute_denominator_term<denominator_index>(extended_edges, relation_parameters, 0);
        });

        auto z_perm = UnivariateView<FF, RELATION_LENGTH>(extended_edges.z_perm);
        auto z_perm_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges.z_perm_shift);
        auto lagrange_first = UnivariateView<FF, RELATION_LENGTH>(extended_edges.lagrange_first);
        auto lagrange_last = UnivariateView<FF, RELATION_LENGTH>(extended_edges.lagrange_last);

        evals += ((z_perm + lagrange_first) * numerator_evaluation -
                  (z_perm_shift + lagrange_last * public_input_delta) * denominator_evaluation) *
                 scaling_factor;
        // Contribution (1)
        // evals += (((z_perm + lagrange_first) * (w_1 + id_1 * beta + gamma) * (w_2 + id_2 * beta + gamma) *
        //            (w_3 + id_3 * beta + gamma) * (w_4 + id_4 * beta + gamma)) -
        //           ((z_perm_shift + lagrange_last * public_input_delta) * (w_1 + sigma_1 * beta + gamma) *
        //            (w_2 + sigma_2 * beta + gamma) * (w_3 + sigma_3 * beta + gamma) * (w_4 + sigma_4 * beta + gamma)))
        //            *
        //          scaling_factor;
    };

    void add_full_relation_value_contribution(FF& full_honk_relation_value,
                                              auto& purported_evaluations,
                                              const RelationParameters<FF>& relation_parameters,
                                              const FF& scaling_factor) const
    {
        const auto& beta = relation_parameters.beta;
        const auto& gamma = relation_parameters.gamma;
        const auto& public_input_delta = relation_parameters.public_input_delta;

        auto w_1 = purported_evaluations.w_l;
        auto w_2 = purported_evaluations.w_r;
        auto w_3 = purported_evaluations.w_o;
        auto w_4 = purported_evaluations.w_4;
        auto sigma_1 = purported_evaluations.sigma_1;
        auto sigma_2 = purported_evaluations.sigma_2;
        auto sigma_3 = purported_evaluations.sigma_3;
        auto sigma_4 = purported_evaluations.sigma_4;
        auto id_1 = purported_evaluations.id_1;
        auto id_2 = purported_evaluations.id_2;
        auto id_3 = purported_evaluations.id_3;
        auto id_4 = purported_evaluations.id_4;
        auto z_perm = purported_evaluations.z_perm;
        auto z_perm_shift = purported_evaluations.z_perm_shift;
        auto lagrange_first = purported_evaluations.lagrange_first;
        auto lagrange_last = purported_evaluations.lagrange_last;

        // Contribution (1)
        full_honk_relation_value +=
            ((z_perm + lagrange_first) * (w_1 + beta * id_1 + gamma) * (w_2 + beta * id_2 + gamma) *
                 (w_3 + beta * id_3 + gamma) * (w_4 + beta * id_4 + gamma) -
             (z_perm_shift + lagrange_last * public_input_delta) * (w_1 + beta * sigma_1 + gamma) *
                 (w_2 + beta * sigma_2 + gamma) * (w_3 + beta * sigma_3 + gamma) * (w_4 + beta * sigma_4 + gamma)) *
            scaling_factor;
    };
};
} // namespace proof_system::honk::sumcheck
