#pragma once

/**
 * @brief Provides interfaces for different 'CommitmentKey' classes.
 *
 */

#include "barretenberg/polynomials/polynomial_arithmetic.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/srs/reference_string/file_reference_string.hpp"
#include "barretenberg/ecc/curves/bn254/scalar_multiplication/scalar_multiplication.hpp"
#include "barretenberg/ecc/curves/bn254/pairing.hpp"
#include "barretenberg/numeric/bitop/pow.hpp"

#include <cstddef>
#include <string_view>
#include <memory>

namespace proof_system::honk::pcs {

namespace kzg {

/**
 * @brief CommitmentKey object over a pairing group 𝔾₁, using a structured reference string (SRS).
 * The SRS is given as a list of 𝔾₁ points
 *  { [xʲ]₁ }ⱼ where 'x' is unknown.
 *
 * TODO(#218)(Adrian): This class should take ownership of the SRS, and handle reading the file from disk.
 */
class CommitmentKey {
    using Fr = typename barretenberg::g1::Fr;
    // C is a "raw commitment" resulting to be fed to the transcript.
    using Commitment = typename barretenberg::g1::affine_element;
    // Commitment represent's a homomorphically computed group element.
    using GroupElement = barretenberg::g1::element;
    using Polynomial = barretenberg::Polynomial<Fr>;

  public:
    CommitmentKey() = delete;

    /**
     * @brief Construct a new Kate Commitment Key object from existing SRS
     *
     * @param n
     * @param path
     *
     */
    CommitmentKey(const size_t num_points, std::string_view path)
        : pippenger_runtime_state(num_points)
        , srs(num_points, std::string(path))
    {}

    /**
     * @brief Uses the ProverSRS to create a commitment to p(X)
     *
     * @param polynomial a univariate polynomial p(X) = ∑ᵢ aᵢ⋅Xⁱ ()
     * @return Commitment computed as C = [p(x)] = ∑ᵢ aᵢ⋅[xⁱ]₁ where x is the secret trapdoor
     */
    Commitment commit(std::span<const Fr> polynomial)
    {
        const size_t degree = polynomial.size();
        ASSERT(degree <= srs.get_monomial_size());
        return barretenberg::scalar_multiplication::pippenger_unsafe(
            const_cast<Fr*>(polynomial.data()), srs.get_monomial_points(), degree, pippenger_runtime_state);
    };

    //   private:
    barretenberg::scalar_multiplication::pippenger_runtime_state pippenger_runtime_state;
    proof_system::FileReferenceString srs;
};

class VerificationKey {
    using GroupElement = typename barretenberg::g1::element;
    using Commitment = typename barretenberg::g1::affine_element;

  public:
    VerificationKey() = delete;

    /**
     * @brief Construct a new Kate Verification Key object from existing SRS
     *
     * @param num_points
     * @param verifier_srs verifier G2 point
     */
    VerificationKey(size_t num_points, std::string_view path)
        : pippenger_runtime_state(num_points)
        , verifier_srs(std::string(path))
    {}

    /**
     * @brief verifies a pairing equation over 2 points using the verifier SRS
     *
     * @param p0 = P₀
     * @param p1 = P₁
     * @return e(P₀,[1]₁)e(P₁,[x]₂) ≡ [1]ₜ
     */
    bool pairing_check(const GroupElement& p0, const GroupElement& p1)
    {
        Commitment pairing_points[2]{ p0, p1 };
        // The final pairing check of step 12.
        // TODO(Adrian): try to template parametrise the pairing + fq12 output :/
        barretenberg::fq12 result = barretenberg::pairing::reduced_ate_pairing_batch_precomputed(
            pairing_points, verifier_srs.get_precomputed_g2_lines(), 2);

        return (result == barretenberg::fq12::one());
    }

    barretenberg::scalar_multiplication::pippenger_runtime_state pippenger_runtime_state;
    proof_system::VerifierFileReferenceString verifier_srs;
};

struct Params {
    using Fr = typename barretenberg::g1::Fr;
    using Commitment = typename barretenberg::g1::affine_element;
    using GroupElement = barretenberg::g1::element;

    using Polynomial = barretenberg::Polynomial<Fr>;

    using CommitmentKey = CommitmentKey;
    using VerificationKey = VerificationKey;
};

} // namespace kzg

namespace fake {

// Define a common trapdoor for both keys
namespace {
template <typename G> constexpr typename G::Fr trapdoor(5);
}

/**
 * @brief Simulates a KZG CommitmentKey, but where we know the secret trapdoor
 * which allows us to commit to polynomials using a single group multiplication.
 *
 * @tparam G the commitment group
 */
template <typename G> class CommitmentKey {
    using Fr = typename G::Fr;
    using Commitment = typename G::affine_element;

    using GroupElement = typename G::element;
    using Polynomial = barretenberg::Polynomial<Fr>;

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

template <typename G> class VerificationKey {
    using Commitment = typename G::affine_element;

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

template <typename G> struct Params {
    using Fr = typename G::Fr;
    using Commitment = typename G::affine_element;
    using GroupElement = typename G::element;

    using Polynomial = barretenberg::Polynomial<Fr>;

    using CommitmentKey = CommitmentKey<G>;
    using VerificationKey = VerificationKey<G>;
};
} // namespace fake

namespace ipa {

/**
 * @brief CommitmentKey object over a group 𝔾₁, using a structured reference string (SRS).
 * The SRS is given as a list of uniquely derived and random 𝔾₁ points of a specified size.
 *
 * @todo This class should take ownership of the SRS, and handle reading the file from disk.
 *
 */
class CommitmentKey {
    using Fr = typename barretenberg::g1::Fr;
    // C is a "raw commitment" resulting to be fed to the transcript.
    using Commitment = typename barretenberg::g1::affine_element;
    // Commitment represent's a homomorphically computed group element.
    using GroupElement = barretenberg::g1::element;

    using Polynomial = barretenberg::Polynomial<Fr>;

  public:
    CommitmentKey() = delete;

    /**
     * @brief Construct a new IPA Commitment Key object from existing SRS
     *
     * @param num_points
     * @param path
     *
     */
    CommitmentKey(const size_t num_points, std::string_view path)
        : pippenger_runtime_state(num_points)
        , srs(num_points, std::string(path))
    {}

    /**
     * @brief Uses the ProverSRS to create an unblinded commitment to p(X)
     *
     * @param polynomial a univariate polynomial p(X) = ∑ᵢ aᵢ⋅Xⁱ ()
     * @return Commitment computed as C = [p(x)] = ∑ᵢ aᵢ⋅Gᵢ where Gᵢ is the i-th element of the SRS
     */
    Commitment commit(std::span<const Fr> polynomial)
    {
        const size_t degree = polynomial.size();
        ASSERT(degree <= srs.get_monomial_size());
        return barretenberg::scalar_multiplication::pippenger_unsafe(
            const_cast<Fr*>(polynomial.data()), srs.get_monomial_points(), degree, pippenger_runtime_state);
    };

    barretenberg::scalar_multiplication::pippenger_runtime_state pippenger_runtime_state;
    proof_system::FileReferenceString srs;
};

class VerificationKey {
    using Fr = typename barretenberg::g1::Fr;
    // C is a "raw commitment" resulting to be fed to the transcript.
    using Commitment = typename barretenberg::g1::affine_element;
    // Commitment represent's a homomorphically computed group element.
    using GroupElement = barretenberg::g1::element;

    using Polynomial = barretenberg::Polynomial<Fr>;

  public:
    VerificationKey() = delete;

    /**
     * @brief Construct a new IPA Verification Key object from existing SRS
     *
     *
     * @param num_points specifies the length of the SRS
     * @param path is the location to the SRS file
     */
    VerificationKey(const size_t num_points, std::string_view path)
        : pippenger_runtime_state(num_points)
        , srs(num_points, std::string(path))
    {}

    barretenberg::scalar_multiplication::pippenger_runtime_state pippenger_runtime_state;
    proof_system::FileReferenceString srs;
};

struct Params {
    using Fr = typename barretenberg::g1::Fr;
    using Commitment = typename barretenberg::g1::affine_element;
    using GroupElement = barretenberg::g1::element;

    using Polynomial = barretenberg::Polynomial<Fr>;

    using CommitmentKey = CommitmentKey;
    using VerificationKey = VerificationKey;
};

} // namespace ipa

} // namespace proof_system::honk::pcs
