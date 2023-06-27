#pragma once
#include <array>
#include <tuple>

#include "../../polynomials/univariate.hpp"
#include "../relation.hpp"
#include "barretenberg/numeric/uint256/uint256.hpp"
#include <barretenberg/common/constexpr_utils.hpp>

namespace proof_system::honk::sumcheck {

template <typename FF, template <typename> typename TypeMuncher> class ECCVMLookupRelationBase {
  public:
    constexpr bool scale_by_random_polynomial() { return false; }

    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 20;

    using UnivariateView = TypeMuncher<FF>::template RelationType<RELATION_LENGTH>::UnivariateView;
    using Univariate = TypeMuncher<FF>::template RelationType<RELATION_LENGTH>::Univariate;
    using RelationParameters = proof_system::honk::sumcheck::RelationParameters<FF>;

    // numerator degree = ?
    // denominator degree = ?
    static constexpr size_t READ_TERMS = 4;
    static constexpr size_t WRITE_TERMS = 2;

    static const UnivariateView get_element_view(const auto& container, const size_t index = 0)
    {
        if constexpr (std::is_same<typeof(container), const std::vector<FF>>::value) {
            return container[index];
        } else {
            return UnivariateView(container);
        }
    }

    static Univariate convert_to_wnaf(const auto& s0, const auto& s1)
    {
        auto t = s0 + s0;
        t += t;
        t += s1;

        auto naf = t + t - 15;
        return naf;
    }

    /**
     * @brief
     *
     * @tparam read_index
     * @param extended_edges
     * @param relation_params
     * @param index
     * @return Univariate
     */
    template <size_t read_index>
    static Univariate compute_read_term_predicate(const auto& extended_edges,
                                                  const RelationParameters&,
                                                  const size_t index = 0)

    {
        if constexpr (read_index == 0) {
            return Univariate(get_element_view(extended_edges.msm_q_add1, index));
        }
        if constexpr (read_index == 1) {
            return Univariate(get_element_view(extended_edges.msm_q_add2, index));
        }
        if constexpr (read_index == 2) {
            return Univariate(get_element_view(extended_edges.msm_q_add3, index));
        }
        if constexpr (read_index == 3) {
            return Univariate(get_element_view(extended_edges.msm_q_add4, index));
        }
        return Univariate(1);
    }

    template <size_t write_index>
    static Univariate compute_write_term_predicate(const auto& extended_edges,
                                                   const RelationParameters&,
                                                   const size_t index = 0)

    {
        if constexpr (write_index == 0) {
            return Univariate(get_element_view(extended_edges.q_wnaf, index));
        }
        if constexpr (write_index == 1) {
            return Univariate(get_element_view(extended_edges.q_wnaf, index));
        }
        return Univariate(1);
    }

    template <size_t write_index>
    static Univariate compute_write_term(const auto& extended_edges,
                                         const RelationParameters& relation_params,
                                         const size_t index = 0)
    {
        static_assert(write_index < WRITE_TERMS);

        // what are we looking up?
        // we want to map:
        // 1: point pc
        // 2: point slice
        // 3: point x
        // 4: point y
        // for each point in our point table, we want to map `slice` to (x, -y) AND `slice + 8` to (x, y)

        // round starts at 0 and increments to 7
        // point starts at 15[P] and decrements to [P]
        // a slice value of 0 maps to -15[P]
        // 1 -> -13[P]
        // 7 -> -[P]
        // 8 -> P
        // 15 -> 15[P]
        // negative points map pc, round, x, -y
        // positive points map pc, 15 - (round * 2), x, y
        const auto& table_pc = get_element_view(extended_edges.table_pc, index);
        const auto& tx = get_element_view(extended_edges.table_tx, index);
        const auto& ty = get_element_view(extended_edges.table_ty, index);
        const auto& table_round = get_element_view(extended_edges.table_round, index);

        const auto& q_wnaf = get_element_view(extended_edges.q_wnaf, index);

        const auto table_round2 = table_round + table_round;
        const auto table_round4 = table_round2 + table_round2;

        const auto& gamma = relation_params.gamma;
        const auto& eta = relation_params.eta;
        const auto& eta_sqr = relation_params.eta_sqr;
        const auto& eta_cube = relation_params.eta_cube;

        // slice value : (wnaf value) : lookup term
        // 0 : -15 : 0
        // 1 : -13 : 1
        // 7 : -1 : 7
        // 8 : 1 : 0
        // 9 : 3 : 1
        // 15 : 15 : 7

        // slice value : negative term : positive term
        // 0 : 0 : 7
        // 1 : 1 : 6
        // 2 : 2 : 5
        // 3 : 3 : 4
        // 7 : 7 : 0

        // | 0 | 15[P].x | 15[P].y  | 0, -15[P].x, -15[P].y | 15, 15[P].x, 15[P].y |
        // | 1 | 13[P].x | 13[P].y | 1, -13[P].x, -13[P].y | 14, 13[P].x, 13[P].y
        // | 2 | 11[P].x | 11[P].y
        // | 3 |  9[P].x |  9[P].y
        // | 4 |  7[P].x |  7[P].y
        // | 5 |  5[P].x |  5[P].y
        // | 6 |  3[P].x |  3[P].y
        // | 7 |  1[P].x |  1[P].y | 7, -[P].x, -[P].y | 8 , [P].x, [P].y |

        const auto negative_term = table_pc + gamma + table_round * eta + tx * eta_sqr - ty * eta_cube;
        const auto positive_slice_value = -(table_round) + 15;
        const auto positive_term = table_pc + gamma + positive_slice_value * eta + tx * eta_sqr + ty * eta_cube;

        // todo optimise this?
        if constexpr (write_index == 0) {
            return positive_term;
        }
        if constexpr (write_index == 1) {
            return negative_term;
        }
        return Univariate(1);
    }

    /**
     * @brief
     *
     * @tparam read_index
     * @param extended_edges
     * @param relation_params
     * @param index
     * @return Univariate
     */
    template <size_t read_index>
    static Univariate compute_read_term(const auto& extended_edges,
                                        const RelationParameters& relation_params,
                                        const size_t index = 0)
    {
        // read term:
        // pc, slice, x, y
        static_assert(read_index < READ_TERMS);
        const auto& gamma = relation_params.gamma;
        const auto& eta = relation_params.eta;
        const auto& eta_sqr = relation_params.eta_sqr;
        const auto& eta_cube = relation_params.eta_cube;
        const auto& msm_pc = get_element_view(extended_edges.msm_pc, index);
        const auto& msm_count = get_element_view(extended_edges.msm_count, index);
        const auto& msm_size_of_msm = get_element_view(extended_edges.msm_size_of_msm, index);
        const auto& msm_slice1 = get_element_view(extended_edges.msm_slice1, index);
        const auto& msm_slice2 = get_element_view(extended_edges.msm_slice2, index);
        const auto& msm_slice3 = get_element_view(extended_edges.msm_slice3, index);
        const auto& msm_slice4 = get_element_view(extended_edges.msm_slice4, index);
        const auto& msm_add1 = get_element_view(extended_edges.msm_q_add1, index);
        const auto& msm_add2 = get_element_view(extended_edges.msm_q_add2, index);
        const auto& msm_add3 = get_element_view(extended_edges.msm_q_add3, index);
        const auto& msm_add4 = get_element_view(extended_edges.msm_q_add4, index);
        const auto& msm_x1 = get_element_view(extended_edges.msm_x1, index);
        const auto& msm_x2 = get_element_view(extended_edges.msm_x2, index);
        const auto& msm_x3 = get_element_view(extended_edges.msm_x3, index);
        const auto& msm_x4 = get_element_view(extended_edges.msm_x4, index);
        const auto& msm_y1 = get_element_view(extended_edges.msm_y1, index);
        const auto& msm_y2 = get_element_view(extended_edges.msm_y2, index);
        const auto& msm_y3 = get_element_view(extended_edges.msm_y3, index);
        const auto& msm_y4 = get_element_view(extended_edges.msm_y4, index);

        // how do we get pc value
        // row pc = value of pc after msm
        // row count = num processed points in round
        // size_of_msm = msm_size
        // value of pc at start of msm = msm_pc - msm_size_of_msm
        // value of current pc = msm_pc - msm_size_of_msm + msm_count + (0,1,2,3)
        const auto current_pc = msm_pc - msm_count;

        const auto read_term1 = (current_pc) + gamma + msm_slice1 * eta + msm_x1 * eta_sqr + msm_y1 * eta_cube;
        const auto read_term2 = (current_pc - 1) + gamma + msm_slice2 * eta + msm_x2 * eta_sqr + msm_y2 * eta_cube;
        const auto read_term3 = (current_pc - 2) + gamma + msm_slice3 * eta + msm_x3 * eta_sqr + msm_y3 * eta_cube;
        const auto read_term4 = (current_pc - 3) + gamma + msm_slice4 * eta + msm_x4 * eta_sqr + msm_y4 * eta_cube;

        if constexpr (read_index == 0) {
            return read_term1;
        }
        if constexpr (read_index == 1) {
            return read_term2;
        }
        if constexpr (read_index == 2) {
            return read_term3;
        }
        if constexpr (read_index == 3) {
            return read_term4;
        }
        return Univariate(1);
    }

    /**
     * @brief Expression for the StandardArithmetic gate.
     * @details The relation is defined as C(extended_edges(X)...) =
     *    (q_m * w_r * w_l) + (q_l * w_l) + (q_r * w_r) + (q_o * w_o) + q_c
     *
     * @param evals transformed to `evals + C(extended_edges(X)...)*scaling_factor`
     * @param extended_edges an std::array containing the fully extended Univariate edges.
     * @param parameters contains beta, gamma, and public_input_delta, ....
     * @param scaling_factor optional term to scale the evaluation before adding to evals.
     */
    void add_edge_contribution(Univariate& evals,
                               const auto& extended_edges,
                               const RelationParameters& relation_params,
                               const FF&) const
    {
        auto evaluation = Univariate(0);

        const auto& lookup_inverses = extended_edges.lookup_inverses;

        constexpr size_t NUM_TOTAL_TERMS = READ_TERMS + WRITE_TERMS;
        std::array<Univariate, NUM_TOTAL_TERMS> lookup_terms;
        std::array<Univariate, NUM_TOTAL_TERMS> denominator_accumulator;

        barretenberg::constexpr_for<0, READ_TERMS, 1>(
            [&]<size_t i>() { lookup_terms[i] = compute_read_term<i>(extended_edges, relation_params, 0); });
        barretenberg::constexpr_for<0, WRITE_TERMS, 1>([&]<size_t i>() {
            lookup_terms[i + READ_TERMS] = compute_write_term<i>(extended_edges, relation_params, 0);
        });

        barretenberg::constexpr_for<0, NUM_TOTAL_TERMS, 1>(
            [&]<size_t i>() { denominator_accumulator[i] = lookup_terms[i]; });

        barretenberg::constexpr_for<0, NUM_TOTAL_TERMS - 1, 1>(
            [&]<size_t i>() { denominator_accumulator[i + 1] *= denominator_accumulator[i]; });

        auto inverse_accumulator = lookup_inverses; // denominator_accumulator[NUM_TOTAL_TERMS - 1];

        const auto& row_has_write = extended_edges.q_wnaf;
        const auto& row_has_read = extended_edges.msm_q_add + extended_edges.msm_q_skew;
        const auto inverse_exists = row_has_write + row_has_read - (row_has_write * row_has_read);

        const auto accumulator = denominator_accumulator[NUM_TOTAL_TERMS - 1];
        evaluation += accumulator * lookup_inverses - inverse_exists;

        for (size_t i = 0; i < NUM_TOTAL_TERMS - 1; ++i) {
            denominator_accumulator[NUM_TOTAL_TERMS - 1 - i] =
                denominator_accumulator[NUM_TOTAL_TERMS - 2 - i] * inverse_accumulator;
            inverse_accumulator *= lookup_terms[NUM_TOTAL_TERMS - 1 - i];
        }
        denominator_accumulator[0] = inverse_accumulator;

        barretenberg::constexpr_for<0, READ_TERMS, 1>([&]<size_t i>() {
            evaluation +=
                compute_read_term_predicate<i>(extended_edges, relation_params, 0) * denominator_accumulator[i];
        });

        barretenberg::constexpr_for<0, WRITE_TERMS, 1>([&]<size_t i>() {
            const auto p = compute_write_term_predicate<i>(extended_edges, relation_params, 0);
            const auto& lookup_read_count = extended_edges.template lookup_read_counts<i>();
            evaluation -= p * (denominator_accumulator[i + READ_TERMS] * lookup_read_count);
        });

        evals += evaluation;
    }
};

} // namespace proof_system::honk::sumcheck
