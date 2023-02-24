#pragma once

#include "../claim.hpp"
#include "polynomials/polynomial.hpp"

#include <memory>
#include <utility>

namespace honk::pcs::kzg {
/**
 * @brief A transformed polynomial commitment opening claim of the form (P₀, P₁) ∈ 𝔾₁
 * which should satisfy e(P₀, [1]₂) ⋅ e(P₁, [x]₂)=1.
 *
 * @tparam Params CommitmentScheme parameters, where the verification key VK has a
 * `pairing_check` function.
 */
template <typename Params> class BilinearAccumulator {
    using VK = typename Params::VK;
    using Fr = typename Params::Fr;
    using Commitment = typename Params::Commitment;

  public:
    /**
     * @brief Construct a new Bilinear Accumulator object given a claim (C,r,v) and proof π.
     *      - P₀ = C − v⋅[1]₁ + r⋅[x]₁
     *      - P₁ = −π
     * @param claim an OpeningClaim (C,r,v)
     * @param proof a Commitment π
     */
    BilinearAccumulator(const OpeningClaim<Params>& claim, const Commitment& proof)
        : lhs(claim.commitment - (Commitment::one() * claim.eval) + (proof * claim.opening_point))
        , rhs(-proof)
    {}

    /**
     * @brief verifies the accumulator with a pairing check
     *
     * @param vk VerificationKey
     * @return e(P₀,[1]₁)e(P₁,[x]₂)≡ [1]ₜ
     */
    bool verify(VK* vk) const { return vk->pairing_check(lhs, rhs); };

    bool operator==(const BilinearAccumulator& other) const = default;

    Commitment lhs, rhs;
};

template <typename Params> class BilinearAccumulatorModified {
    using VK = typename Params::VK;
    using Fr = typename Params::Fr;
    using Commitment = typename Params::Commitment;

  public:
    /**
     * @brief Construct a new Bilinear Accumulator object given a claim (C,r,v) and proof π.
     *      - P₀ = C − v⋅[1]₁ + r⋅[x]₁
     *      - P₁ = −π
     * @param claim an OpeningClaim (C,r,v)
     * @param proof a Commitment π
     */
    BilinearAccumulatorModified(const OpeningClaimModified<Params>& claim, const Commitment& proof)
        : lhs(claim.commitment - (Commitment::one() * claim.opening_pair.evaluation) +
              (proof * claim.opening_pair.query))
        , rhs(-proof)
    {}

    /**
     * @brief verifies the accumulator with a pairing check
     *
     * @param vk VerificationKey
     * @return e(P₀,[1]₁)e(P₁,[x]₂)≡ [1]ₜ
     */
    bool verify(VK* vk) const { return vk->pairing_check(lhs, rhs); };

    bool operator==(const BilinearAccumulatorModified& other) const = default;

    Commitment lhs, rhs;
};

template <typename Params> class UnivariateOpeningScheme {
    using CK = typename Params::CK;

    using Fr = typename Params::Fr;
    using Commitment = typename Params::Commitment;
    using CommitmentAffine = typename Params::C;
    using Polynomial = barretenberg::Polynomial<Fr>;

  public:
    using Accumulator = BilinearAccumulator<Params>;
    using AccumulatorModified = BilinearAccumulatorModified<Params>;
    using Proof = Commitment;
    using OpeningPair = OpeningPair<Params>;

    struct Output {
        Accumulator accumulator;
        Proof proof;
    };
    struct OutputModified {
        OpeningPair opening_pair;
        Polynomial witness;
    };
    /**
     * @brief Compute an accumulator for a single polynomial commitment opening claim
     *
     * @param ck CommitmentKey
     * @param claim OpeningClaim = (C,r,v)
     * @param polynomial the witness polynomial for C
     * @return Output{Accumulator, Proof}
     */
    static void reduce_prove_modified(std::shared_ptr<CK> ck,
                                      const OpeningPair& opening_pair,
                                      const Polynomial& polynomial,
                                      const auto& transcript)
    {
        Polynomial quotient(polynomial);
        quotient[0] -= opening_pair.evaluation;
        quotient.factor_roots(opening_pair.query);
        Proof proof = ck->commit(quotient);

        transcript->add_element("W", static_cast<CommitmentAffine>(proof).to_buffer());
    };

    /**
     * @brief Compute an accumulator for a single polynomial commitment opening claim
     *
     * @param ck CommitmentKey
     * @param claim OpeningClaim = (C,r,v)
     * @param polynomial the witness polynomial for C
     * @return Output{Accumulator, Proof}
     */
    static Output reduce_prove(std::shared_ptr<CK> ck, const OpeningClaim<Params>& claim, const Polynomial& polynomial)
    {
        Polynomial quotient(polynomial);
        quotient[0] -= claim.eval;
        quotient.factor_roots(claim.opening_point);
        Proof proof = ck->commit(quotient);

        return Output{ Accumulator(claim, proof), proof };
    };

    /**
     * @brief Computes the accumulator for a single polynomial commitment opening claim
     * This reduction is non-interactive and always succeeds.
     *
     * @param claim OpeningClaim (C,r,v)
     * @param proof a commitment to Q(X) = ( P(X) - v )/( X - r)
     * @return Accumulator
     */
    static AccumulatorModified reduce_verify_modified(const OpeningClaimModified<Params>& claim, const Proof& proof)
    {
        return AccumulatorModified(claim, proof);
    };

    /**
     * @brief Computes the accumulator for a single polynomial commitment opening claim
     * This reduction is non-interactive and always succeeds.
     *
     * @param claim OpeningClaim (C,r,v)
     * @param proof a commitment to Q(X) = ( P(X) - v )/( X - r)
     * @return Accumulator
     */
    static Accumulator reduce_verify(const OpeningClaim<Params>& claim, const Proof& proof)
    {
        return Accumulator(claim, proof);
    };
};
} // namespace honk::pcs::kzg