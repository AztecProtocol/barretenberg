#pragma once
#include "relation.hpp"
#include "barretenberg/honk/flavor/flavor.hpp"
#include "../polynomials/univariate.hpp"

namespace proof_system::honk::sumcheck {

template <typename FF> class LookupGrandProductComputationRelation {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 4; // deg(z_lookup * wire_accum * q_lookup) = 3
    using MULTIVARIATE = proof_system::honk::UltraArithmetization::POLYNOMIAL;

    /**
     * @brief Compute contribution of the lookup grand prod relation for a given edge (internal function)
     *
     * @details This the relation confirms faithful calculation of the lookup grand
     * product polynomial Z_lookup.
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
        const auto& eta = FF::one(); // TODO(luke): add to relation params
        const auto& beta = relation_parameters.beta;
        const auto& gamma = relation_parameters.gamma;

        auto one_plus_beta = FF::one() + beta;
        auto gamma_by_one_plus_beta = gamma * one_plus_beta;
        auto eta_sqr = eta * eta;
        auto eta_cube = eta_sqr * eta;
        auto delta_factor = gamma_by_one_plus_beta.pow(8);

        auto w_1 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_1]);
        auto w_2 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_2]);
        auto w_3 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_3]);

        auto w_1_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_1_SHIFT]);
        auto w_2_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_2_SHIFT]);
        auto w_3_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_3_SHIFT]);

        auto table_1 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::TABLE_1]);
        auto table_2 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::TABLE_2]);
        auto table_3 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::TABLE_3]);
        auto table_4 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::TABLE_4]);

        auto table_1_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::TABLE_1_SHIFT]);
        auto table_2_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::TABLE_2_SHIFT]);
        auto table_3_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::TABLE_3_SHIFT]);
        auto table_4_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::TABLE_3_SHIFT]);

        auto s_accum = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::S_ACCUM]);
        auto s_accum_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::S_ACCUM_SHIFT]);

        auto z_lookup = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Z_LOOKUP]);
        auto z_lookup_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Z_LOOKUP_SHIFT]);

        auto table_index = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_3]);
        auto column_1_step_size = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_2]);
        auto column_2_step_size = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_M]);
        auto column_3_step_size = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_C]);
        auto q_lookup = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::QLOOKUPTYPE]);

        auto lagrange_first = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::LAGRANGE_FIRST]);
        auto lagrange_last = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::LAGRANGE_LAST]);

        auto wire_accum = (w_1 + column_1_step_size * w_1_shift) + (w_2 + column_2_step_size * w_2_shift) * eta +
                          (w_3 + column_3_step_size * w_3_shift) * eta_sqr + table_index * eta_cube;

        auto table_accum = table_1 + table_2 * eta + table_3 * eta_sqr + table_4 * eta_cube;
        auto table_accum_shift =
            table_1_shift + table_2_shift * eta + table_3_shift * eta_sqr + table_4_shift * eta_cube;

        // Contribution (1)
        auto tmp = (q_lookup * wire_accum + gamma);
        tmp *= (table_accum + table_accum_shift * beta + gamma_by_one_plus_beta);
        tmp *= one_plus_beta;
        tmp *= (z_lookup + lagrange_first);
        tmp -=
            (z_lookup_shift + lagrange_last * delta_factor) * (s_accum + s_accum_shift * beta + gamma_by_one_plus_beta);
        evals += tmp * scaling_factor;
    };

    void add_full_relation_value_contribution(FF& full_honk_relation_value,
                                              auto& purported_evaluations,
                                              const RelationParameters<FF>& relation_parameters) const
    {
        const auto& eta = FF::one(); // TODO(luke): add to relation params
        const auto& beta = relation_parameters.beta;
        const auto& gamma = relation_parameters.gamma;

        auto one_plus_beta = FF::one() + beta;
        auto gamma_by_one_plus_beta = gamma * one_plus_beta;
        auto eta_sqr = eta * eta;
        auto eta_cube = eta_sqr * eta;
        auto delta_factor = gamma_by_one_plus_beta.pow(8);

        auto w_1 = purported_evaluations[MULTIVARIATE::W_1];
        auto w_2 = purported_evaluations[MULTIVARIATE::W_2];
        auto w_3 = purported_evaluations[MULTIVARIATE::W_3];

        auto w_1_shift = purported_evaluations[MULTIVARIATE::W_1_SHIFT];
        auto w_2_shift = purported_evaluations[MULTIVARIATE::W_2_SHIFT];
        auto w_3_shift = purported_evaluations[MULTIVARIATE::W_3_SHIFT];

        auto table_1 = purported_evaluations[MULTIVARIATE::TABLE_1];
        auto table_2 = purported_evaluations[MULTIVARIATE::TABLE_2];
        auto table_3 = purported_evaluations[MULTIVARIATE::TABLE_3];
        auto table_4 = purported_evaluations[MULTIVARIATE::TABLE_4];

        auto table_1_shift = purported_evaluations[MULTIVARIATE::TABLE_1_SHIFT];
        auto table_2_shift = purported_evaluations[MULTIVARIATE::TABLE_2_SHIFT];
        auto table_3_shift = purported_evaluations[MULTIVARIATE::TABLE_3_SHIFT];
        auto table_4_shift = purported_evaluations[MULTIVARIATE::TABLE_3_SHIFT];

        auto s_accum = purported_evaluations[MULTIVARIATE::S_ACCUM];
        auto s_accum_shift = purported_evaluations[MULTIVARIATE::S_ACCUM_SHIFT];
        auto z_lookup = purported_evaluations[MULTIVARIATE::Z_LOOKUP];
        auto z_lookup_shift = purported_evaluations[MULTIVARIATE::Z_LOOKUP_SHIFT];

        auto table_index = purported_evaluations[MULTIVARIATE::Q_3];
        auto column_1_step_size = purported_evaluations[MULTIVARIATE::Q_2];
        auto column_2_step_size = purported_evaluations[MULTIVARIATE::Q_M];
        auto column_3_step_size = purported_evaluations[MULTIVARIATE::Q_C];
        auto q_lookup = purported_evaluations[MULTIVARIATE::QLOOKUPTYPE];

        auto lagrange_first = purported_evaluations[MULTIVARIATE::LAGRANGE_FIRST];
        auto lagrange_last = purported_evaluations[MULTIVARIATE::LAGRANGE_LAST];

        auto wire_accum = (w_1 + column_1_step_size * w_1_shift) + (w_2 + column_2_step_size * w_2_shift) * eta +
                          (w_3 + column_3_step_size * w_3_shift) * eta_sqr + table_index * eta_cube;

        auto table_accum = table_1 + table_2 * eta + table_3 * eta_sqr + table_4 * eta_cube;
        auto table_accum_shift =
            table_1_shift + table_2_shift * eta + table_3_shift * eta_sqr + table_4_shift * eta_cube;

        // Contribution (1)
        auto tmp = (q_lookup * wire_accum + gamma);
        tmp *= (table_accum + beta * table_accum_shift + gamma_by_one_plus_beta);
        tmp *= one_plus_beta;
        tmp *= (z_lookup + lagrange_first);
        tmp -=
            (z_lookup_shift + lagrange_last * delta_factor) * (s_accum + beta * s_accum_shift + gamma_by_one_plus_beta);
        full_honk_relation_value += tmp;
    };
};
} // namespace proof_system::honk::sumcheck