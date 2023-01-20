#include <array>
#include <tuple>

#include <proof_system/flavor/flavor.hpp>
#include "relation.hpp"
#include "../polynomials/multivariates.hpp"
#include "../polynomials/barycentric_data.hpp"
#include "../polynomials/univariate.hpp"

namespace honk::sumcheck {

template <typename FF> class ArithmeticRelation : public Relation<FF> {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 4;
    using MULTIVARIATE = StandardHonk::MULTIVARIATE; // could just get from StandardArithmetization

    // FUTURE OPTIMIZATION: successively extend as needed?

    // This relation takes no randomness, so it will not receive challenges.
    ArithmeticRelation() = default;
    explicit ArithmeticRelation(auto){}; // NOLINT(readability-named-parameter)

    // OPTIMIZATION?: Karatsuba in general, at least for some degrees?
    //       See https://hackmd.io/xGLuj6biSsCjzQnYN-pEiA?both
    void add_edge_contribution(auto extended_edges, Univariate<FF, RELATION_LENGTH>& evals)
    {
        auto w_l = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_L]);
        auto w_r = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_R]);
        auto w_o = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_O]);
        auto q_m = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_M]);
        auto q_l = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_L]);
        auto q_r = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_R]);
        auto q_o = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_O]);
        auto q_c = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Q_C]);

        info("w_l: ", w_l);
        info("w_r: ", w_r);
        info("w_o: ", w_o);
        info("q_m: ", q_m);
        info("q_l: ", q_l);
        info("q_r: ", q_r);
        info("q_o: ", q_o);
        info("q_c: ", q_c);

        evals += w_l * (q_m * w_r + q_l);
        evals += q_r * w_r;
        evals += q_o * w_o;
        evals += q_c;

        info("evals: ", evals);
    };

    void add_full_relation_value_contribution(auto& purported_evaluations, FF& full_honk_relation_value)
    {

        info("full_honk_relation_value: ", full_honk_relation_value);

        auto w_l = purported_evaluations[MULTIVARIATE::W_L];
        auto w_r = purported_evaluations[MULTIVARIATE::W_R];
        auto w_o = purported_evaluations[MULTIVARIATE::W_O];
        auto q_m = purported_evaluations[MULTIVARIATE::Q_M];
        auto q_l = purported_evaluations[MULTIVARIATE::Q_L];
        auto q_r = purported_evaluations[MULTIVARIATE::Q_R];
        auto q_o = purported_evaluations[MULTIVARIATE::Q_O];
        auto q_c = purported_evaluations[MULTIVARIATE::Q_C];

        info("w_l: ", w_l);
        info("w_r: ", w_r);
        info("w_o: ", w_o);
        info("q_m: ", q_m);
        info("q_l: ", q_l);
        info("q_r: ", q_r);
        info("q_o: ", q_o);
        info("q_c: ", q_c);

        full_honk_relation_value += w_l * (q_m * w_r + q_l);
        full_honk_relation_value += q_r * w_r;
        full_honk_relation_value += q_o * w_o;
        full_honk_relation_value += q_c;
    };
};
} // namespace honk::sumcheck
