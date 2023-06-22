#pragma once

/**
 * @brief Provides interfaces for different 'CommitmentKey' classes.
 *
 * TODO(#218)(Mara): This class should handle any modification to the SRS (e.g compute pippenger point table) to
 * simplify the codebase.
 */

#include "barretenberg/ecc/curves/bn254/bn254.hpp"
#include "barretenberg/polynomials/polynomial_arithmetic.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/srs/factories/crs_factory.hpp"
#include "barretenberg/srs/factories/file_crs_factory.hpp"
#include "barretenberg/ecc/scalar_multiplication/scalar_multiplication.hpp"
#include "barretenberg/ecc/curves/bn254/pairing.hpp"
#include "barretenberg/numeric/bitop/pow.hpp"

#include <cstddef>
#include <string_view>
#include <memory>

namespace proof_system::pcs {

namespace kzg {

struct Params {
    using Fr = typename barretenberg::g1::Fr;
    using Commitment = typename barretenberg::g1::affine_element;
    using GroupElement = barretenberg::g1::element;

    using Polynomial = barretenberg::Polynomial<Fr>;

    class CommitmentKey;
    class VerificationKey;
    /**
     * @brief CommitmentKey object over a pairing group 𝔾₁, using a structured reference string (SRS).
     * The SRS is given as a list of 𝔾₁ points { [xʲ]₁ }ⱼ where 'x' is unknown. The SRS stored in the commitment key is
     * after applying the pippenger_point_table thus being double the size of what is loaded from path.
     *
     *
     */
    class CommitmentKey {

      public:
        barretenberg::scalar_multiplication::pippenger_runtime_state<curve::BN254> pippenger_runtime_state;
        std::shared_ptr<barretenberg::srs::factories::ProverCrs<curve::BN254>> srs;

        CommitmentKey() = delete;

        CommitmentKey(const size_t num_points, std::shared_ptr<barretenberg::srs::factories::CrsFactory> crs_factory);

        CommitmentKey(const size_t num_points,
                      std::shared_ptr<barretenberg::srs::factories::ProverCrs<curve::BN254>> prover_srs);

        Commitment commit(std::span<const Fr> polynomial);
    };

    class VerificationKey {

      public:
        std::shared_ptr<barretenberg::srs::factories::VerifierCrs> verifier_srs;

        VerificationKey() = delete;

        VerificationKey([[maybe_unused]] size_t num_points,
                        std::shared_ptr<barretenberg::srs::factories::CrsFactory> crs_factory);

        bool pairing_check(const GroupElement& p0, const GroupElement& p1);
    };
};

} // namespace kzg

namespace ipa {

struct Params {
    using Fr = typename barretenberg::g1::Fr;
    using Commitment = typename barretenberg::g1::affine_element;
    using GroupElement = barretenberg::g1::element;

    using Polynomial = barretenberg::Polynomial<Fr>;

    class CommitmentKey;
    class VerificationKey;

    class CommitmentKey {

      public:
        barretenberg::scalar_multiplication::pippenger_runtime_state<curve::BN254> pippenger_runtime_state;
        std::shared_ptr<barretenberg::srs::factories::ProverCrs<curve::BN254>> srs;

        CommitmentKey() = delete;

        CommitmentKey(const size_t num_points, std::shared_ptr<barretenberg::srs::factories::CrsFactory> crs_factory);

        Commitment commit(std::span<const Fr> polynomial);
    };

    class VerificationKey {
      public:
        barretenberg::scalar_multiplication::pippenger_runtime_state<curve::BN254> pippenger_runtime_state;
        std::shared_ptr<barretenberg::srs::factories::ProverCrs<curve::BN254>> srs;

        VerificationKey() = delete;

        VerificationKey(size_t num_points, std::shared_ptr<barretenberg::srs::factories::CrsFactory> crs_factory);
    };
};

} // namespace ipa

namespace fake {

// Define a common trapdoor for both keys
namespace {
template <typename G> constexpr typename G::Fr trapdoor(5);
}

template <typename G> struct Params {
    using Fr = typename G::Fr;
    using Commitment = typename G::affine_element;
    using GroupElement = typename G::element;

    using Polynomial = barretenberg::Polynomial<Fr>;

    template <G> class CommitmentKey;
    template <G> class VerificationKey;

    /**
     * @brief Simulates a KZG CommitmentKey, but where we know the secret trapdoor
     * which allows us to commit to polynomials using a single group multiplication.
     *
     * @tparam G the commitment group
     */
    template <G> class CommitmentKey {

      public:
        /**
         * @brief efficiently create a KZG commitment to p(X) using the trapdoor 'secret'
         * Uses only 1 group scalar multiplication, and 1 polynomial evaluation
         *
         *
         * @param polynomial a univariate polynomial p(X)
         * @return Commitment computed as C = p(secret)•[1]_1 .
         */
        Commitment commit(std::span<const Fr> polynomial)
        {
            const Fr eval_secret = barretenberg::polynomial_arithmetic::evaluate(polynomial, trapdoor<G>);
            return Commitment::one() * eval_secret;
        };
    };

    template <G> class VerificationKey {

      public:
        /**
         * @brief verifies a pairing equation over 2 points using the trapdoor
         *
         * @param p0 = P₀
         * @param p1 = P₁
         * @return P₀ - x⋅P₁ ≡ [1]
         */
        bool pairing_check(const Commitment& p0, const Commitment& p1)
        {
            Commitment result = p0 + p1 * trapdoor<G>;
            return result.is_point_at_infinity();
        }
    };
};
} // namespace fake

} // namespace proof_system::pcs
