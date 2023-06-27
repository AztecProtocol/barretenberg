#pragma once
#include "ecc_msm_relation.hpp"
#include "ecc_point_table_relation.hpp"
#include "ecc_wnaf_relation.hpp"
#include "ecc_transcript_relation.hpp"
#include "ecc_set_relation.hpp"
#include "ecc_lookup_relation.hpp"

namespace proof_system::honk::sumcheck::eccvm {

/**
 * @brief type wrappers that present UnivariateView and Univariate as FF elements.
 *        Uses nested templates so that we can define a Relation template class where FF is a template, where
 * RelationType is strictly defined in terms of FF
 *
 * @tparam FF
 */
template <typename FF> struct AlgebraicTypesSuper {
    template <size_t> struct RelationType {
        using UnivariateView = FF;
        using Univariate = FF;
    };
};

/**
 * @brief type wrappers that present UnivariateView and Univariate as their intended types.
 *        Uses nested templates so that we can define a Relation template class where FF is a template, where
 * RelationType is strictly defined in terms of FF
 *
 * @tparam FF
 */
template <typename FF> struct SumcheckTypesSuper {
    template <size_t RELATION_LENGTH> struct RelationType {
        using UnivariateView = proof_system::honk::sumcheck::UnivariateView<FF, RELATION_LENGTH>;
        using Univariate = proof_system::honk::sumcheck::Univariate<FF, RELATION_LENGTH>;
    };
};

/**
 * @brief Wrapper class that defines a Verifier class using a field (FF) and a sumcheck Relation class
 *
 * @tparam FF
 */
template <typename FF> struct VerifierSuper {

    /**
     * @brief Verifier exposes the Sumcheck verification algorithm for a given Relation, using the algebra defined for
     * the sumcheck prover
     *
     * @tparam Relation
     */
    template <template <typename, template <typename> typename> typename Relation>
    struct Verifier : public Relation<FF, AlgebraicTypesSuper> {
        using RelationParameters = proof_system::honk::sumcheck::RelationParameters<FF>;

        void add_full_relation_value_contribution(FF& full_honk_relation_value,
                                                  const auto& purported_evaluations,
                                                  const RelationParameters& relation_parameters,
                                                  const FF& random_polynomial_evaluation) const

        {
            auto relation = Relation<FF, AlgebraicTypesSuper>();
            relation.add_edge_contribution(
                full_honk_relation_value, purported_evaluations, relation_parameters, random_polynomial_evaluation);
        }
    };
};

template <typename FF> using ECCMSMRelationAlgebra = ECCMSMRelationBase<FF, AlgebraicTypesSuper>;
template <typename FF> using ECCMSMRelationProver = ECCMSMRelationBase<FF, SumcheckTypesSuper>;
template <typename FF> using ECCMSMRelationVerifier = VerifierSuper<FF>::template Verifier<ECCMSMRelationBase>;

template <typename FF> using ECCVMWnafAlgebra = ECCVMWnafRelationBase<FF, AlgebraicTypesSuper>;
template <typename FF> using ECCVMWnafProver = ECCVMWnafRelationBase<FF, SumcheckTypesSuper>;
template <typename FF> using ECCVMWnafVerifier = VerifierSuper<FF>::template Verifier<ECCVMWnafRelationBase>;

template <typename FF> using ECCVMPointTableAlgebra = ECCVMPointTableRelationBase<FF, AlgebraicTypesSuper>;
template <typename FF> using ECCVMPointTableProver = ECCVMPointTableRelationBase<FF, SumcheckTypesSuper>;
template <typename FF>
using ECCVMPointTableVerifier = VerifierSuper<FF>::template Verifier<ECCVMPointTableRelationBase>;

template <typename FF> using ECCVMTranscriptAlgebra = ECCVMTranscriptRelationBase<FF, AlgebraicTypesSuper>;
template <typename FF> using ECCVMTranscriptProver = ECCVMTranscriptRelationBase<FF, SumcheckTypesSuper>;
template <typename FF>
using ECCVMTranscriptVerifier = VerifierSuper<FF>::template Verifier<ECCVMTranscriptRelationBase>;

template <typename FF> using ECCVMSetRelationAlgebra = ECCVMSetRelationBase<FF, AlgebraicTypesSuper>;
template <typename FF> using ECCVMSetRelationProver = ECCVMSetRelationBase<FF, SumcheckTypesSuper>;
template <typename FF> using ECCVMSetRelationVerifier = VerifierSuper<FF>::template Verifier<ECCVMSetRelationBase>;

template <typename FF> using ECCVMLookupRelationAlgebra = ECCVMLookupRelationBase<FF, AlgebraicTypesSuper>;
template <typename FF> using ECCVMLookupRelationProver = ECCVMLookupRelationBase<FF, SumcheckTypesSuper>;
template <typename FF>
using ECCVMLookupRelationVerifier = VerifierSuper<FF>::template Verifier<ECCVMLookupRelationBase>;

} // namespace proof_system::honk::sumcheck::eccvm