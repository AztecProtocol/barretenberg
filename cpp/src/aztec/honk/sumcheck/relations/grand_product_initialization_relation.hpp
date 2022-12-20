#include "relation.hpp"
#include "../../flavor/flavor.hpp"
#include "../polynomials/multivariates.hpp"
#include "../polynomials/univariate.hpp"
#include "../polynomials/barycentric_data.hpp"

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace honk::sumcheck {

template <typename Fr> class GrandProductInitializationRelation : public Relation<Fr> {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 3;
    using MULTIVARIATE = StandardHonk::MULTIVARIATE; // could just get from StandardArithmetization
    BarycentricData<Fr, RELATION_LENGTH, StandardHonk::MAX_RELATION_LENGTH> barycentric =
        BarycentricData<Fr, RELATION_LENGTH, StandardHonk::MAX_RELATION_LENGTH>();
    using UnivariateClass = Univariate<Fr, RELATION_LENGTH>;

  public:
    GrandProductInitializationRelation() = default;
    explicit GrandProductInitializationRelation(auto){}; // NOLINT(readability-named-parameter)

    /**
     * @brief Add contribution of the permutation relation for a given edge
     *
     * @detail There are 2 relations associated with enforcing the wire copy relations
     * This file handles the relation Z_perm(0) = 1 via the relation:
     *
     *                      C(X) = L_1(X)(z_perm(X) - 1)
     */
    void add_edge_contribution(auto& edge_extensions, UnivariateClass& evals)
    {
        auto z_perm = UnivariateView<Fr, RELATION_LENGTH>(edge_extensions[MULTIVARIATE::Z_PERM]);
        auto lagrange_1 = UnivariateView<Fr, RELATION_LENGTH>(edge_extensions[MULTIVARIATE::LAGRANGE_1]);
        auto one = Fr(1);

        evals += lagrange_1 * (z_perm - one);
    };

    void add_full_relation_value_contribution(auto& purported_evaluations, Fr& full_honk_relation_value)
    {
        auto z_perm = purported_evaluations[MULTIVARIATE::Z_PERM];
        auto lagrange_1 = purported_evaluations[MULTIVARIATE::LAGRANGE_1];
        auto one = Fr(1);

        full_honk_relation_value += lagrange_1 * (z_perm - one);
    };
};
} // namespace honk::sumcheck
