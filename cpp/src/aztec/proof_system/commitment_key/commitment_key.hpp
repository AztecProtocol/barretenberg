#pragma once

/**
 * @brief Provides interfaces for different 'CommitmentKey' classes.
 *
 */

#include "polynomials/evaluation_domain.hpp"
#include "srs/reference_string/reference_string.hpp"
#include <limits>
#include <polynomials/polynomial_arithmetic.hpp>
#include <polynomials/polynomial.hpp>
#include <srs/reference_string/file_reference_string.hpp>
#include <ecc/curves/bn254/scalar_multiplication/scalar_multiplication.hpp>
#include <ecc/curves/bn254/pairing.hpp>
#include <numeric/bitop/pow.hpp>

#include <string_view>
#include <memory>
#include <utility>

namespace waffle::pcs {

namespace kzg {

/**
 * @brief CommitmentKey object over a pairing group 𝔾₁, using a structured reference string (SRS).
 * The SRS is given as a list of 𝔾₁ points
 *  { [xʲ]₁ }ⱼ where 'x' is unknown.
 *
 * @todo This class should take ownership of the SRS, and handle reading the file from disk.
 */
class CommitmentKey {
    using Fr = typename barretenberg::g1::Fr;
    // CommitmentAffine is a "raw commitment" resulting to be fed to the transcript.
    using CommitmentAffine = typename barretenberg::g1::affine_element;
    // Commitment represent's a homomorphically computed group element.
    using Commitment = barretenberg::g1::element;

    using Polynomial = barretenberg::Polynomial<Fr>;

  public:
    CommitmentKey() = delete;

    CommitmentKey(std::shared_ptr<ProverReferenceString> const& crs)
        : srs(crs)
        , pippenger_runtime_state(std::make_shared<barretenberg::scalar_multiplication::pippenger_runtime_state>(
              std::max(crs->get_monomial_size(), crs->get_lagrange_size())))
    {}
    CommitmentKey(std::shared_ptr<ReferenceStringFactory> const& crs, const size_t num_points)
        : CommitmentKey(crs->get_prover_crs(num_points))
    {}

    /**
     * @brief Construct a new Kate Commitment Key object from existing SRS
     *
     * @param n
     * @param path
     *
     * @todo change path to string_view
     */
    CommitmentKey(std::string_view path, const size_t num_points)
        : CommitmentKey(std::make_shared<waffle::FileReferenceString>(num_points, std::string(path)))
    {}

    size_t max_supported_size() const { return srs->get_monomial_size(); }
    size_t max_supported_size_lagrange() const { return srs->get_lagrange_size(); }

    /**
     * @brief Uses the ProverSRS to create a commitment to p(X)
     *
     * @param polynomial a univariate polynomial p(X) = ∑ᵢ aᵢ⋅Xⁱ ()
     * @return Commitment computed as C = [p(x)] = ∑ᵢ aᵢ⋅[xⁱ]₁
     */
    CommitmentAffine commit(std::span<const Fr> polynomial) const
    {
        const size_t degree = polynomial.size();
        ASSERT(degree <= srs->get_monomial_size());
        return barretenberg::scalar_multiplication::pippenger_unsafe(
            const_cast<Fr*>(polynomial.data()), srs->get_monomial_points(), degree, *pippenger_runtime_state);
    };

    /**
     * @brief Uses the ProverSRS to create a commitment to p(X)
     *
     * @param polynomial a univariate polynomial in its evaluation form p(X) = ∑ᵢ p(ωⁱ)⋅Lᵢ(X)
     * @return Commitment computed as C = [p(x)] = ∑ᵢ p(ωⁱ)⋅[Lᵢ(x)]₁
     */
    CommitmentAffine commit_lagrange(std::span<const Fr> polynomial) const
    {
        const size_t degree = polynomial.size();
        ASSERT(degree == srs->get_lagrange_size());
        ASSERT(numeric::is_power_of_two(degree));
        return barretenberg::scalar_multiplication::pippenger_unsafe(
            const_cast<Fr*>(polynomial.data()), srs->get_lagrange_points(), degree, *pippenger_runtime_state);
    };

  private:
    std::shared_ptr<waffle::ProverReferenceString> srs;
    std::shared_ptr<barretenberg::scalar_multiplication::pippenger_runtime_state> pippenger_runtime_state;
};

class VerificationKey {
    using Fr = typename barretenberg::g1::Fr;
    using CommitmentAffine = typename barretenberg::g1::affine_element;

    using Commitment = barretenberg::g1::element;
    using Polynomial = barretenberg::Polynomial<Fr>;

  public:
    VerificationKey() = delete;

    /**
     * @brief Construct a new Kate Commitment Key object from existing SRS
     *
     *
     * @param verifier_srs verifier G2 point
     */

    VerificationKey(std::shared_ptr<VerifierReferenceString> crs)
        : verifier_srs(std::move(crs))
    {}

    VerificationKey(std::shared_ptr<ReferenceStringFactory> const& crs)
        : VerificationKey(crs->get_verifier_crs())
    {}

    VerificationKey(std::string_view path)
        : VerificationKey(std::make_shared<VerifierFileReferenceString>(std::string(path)))
    {}

    /**
     * @brief verifies a pairing equation over 2 points using the verifier SRS
     *
     * @param p0 = P₀
     * @param p1 = P₁
     * @return e(P₀,[1]₁)e(P₁,[x]₂) ≡ [1]ₜ
     */
    bool pairing_check(const CommitmentAffine& p0, const CommitmentAffine& p1) const
    {
        CommitmentAffine pairing_points[2]{ p0, p1 };
        // The final pairing check of step 12.
        // TODO: try to template parametrise the pairing + fq12 output :/
        barretenberg::fq12 result = barretenberg::pairing::reduced_ate_pairing_batch_precomputed(
            pairing_points, verifier_srs->get_precomputed_g2_lines(), 2);

        return (result == barretenberg::fq12::one());
    }

  private:
    std::shared_ptr<waffle::VerifierReferenceString> verifier_srs;
};

struct Params {
    using Fr = typename barretenberg::g1::Fr;
    using CommitmentAffine = typename barretenberg::g1::affine_element;

    using Commitment = barretenberg::g1::element;
    using Polynomial = barretenberg::Polynomial<Fr>;

    using CK = CommitmentKey;
    using VK = VerificationKey;
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
    using CommitmentAffine = typename G::affine_element;

    using Commitment = typename G::element;
    using Polynomial = barretenberg::Polynomial<Fr>;

  public:
    CommitmentKey(std::shared_ptr<ProverReferenceString> const&) {}
    CommitmentKey(std::shared_ptr<ReferenceStringFactory> const&, const size_t) {}

    /**
     * @brief Construct a new Kate Commitment Key object from existing SRS
     *
     * @param n
     * @param path
     *
     * @todo change path to string_view
     */
    CommitmentKey(std::string_view, const size_t) {}

    size_t max_supported_size() const { return std::numeric_limits<size_t>::max(); }
    size_t max_supported_size_lagrange() const { return std::numeric_limits<size_t>::max(); }
    /**
     * @brief efficiently create a KZG commitment to p(X) using the trapdoor 'secret'
     * Uses only 1 group scalar multiplication, and 1 polynomial evaluation
     *
     *
     * @param polynomial a univariate polynomial p(X)
     * @return Commitment computed as C = p(secret)•[1]_1 .
     */
    CommitmentAffine commit(std::span<const Fr> polynomial) const
    {
        const Fr eval_secret = barretenberg::polynomial_arithmetic::evaluate(polynomial, trapdoor<G>);
        return CommitmentAffine::one() * eval_secret;
    };

    /**
     * @brief efficiently create a KZG commitment to p(X)  using the trapdoor 'secret'
     * Uses only 1 group scalar multiplication, and 1 polynomial evaluation
     *
     *
     * @param polynomial a univariate polynomial p(X) given in Lagrange form
     * @return Commitment computed as C = p(secret)•[1]_1 .
     */
    CommitmentAffine commit_lagrange(std::span<const Fr> polynomial) const
    {
        const size_t degree = polynomial.size();
        ASSERT(numeric::is_power_of_two(degree));
        const EvaluationDomain<Fr> domain{ degree };
        const Fr eval_secret =
            polynomial_arithmetic::compute_barycentric_evaluation(polynomial.data(), degree, trapdoor<G>, domain);
        return CommitmentAffine::one() * eval_secret;
    };
};

template <typename G> class VerificationKey {
    using Fr = typename G::Fr;
    using CommitmentAffine = typename G::affine_element;

    using Commitment = typename G::element;
    using Polynomial = barretenberg::Polynomial<Fr>;

  public:
    VerificationKey(std::string_view){};
    VerificationKey(std::shared_ptr<VerifierReferenceString>) {}
    VerificationKey(std::shared_ptr<ReferenceStringFactory> const&) {}

    /**
     * @brief verifies a pairing equation over 2 points using the trapdoor
     *
     * @param p0 = P₀
     * @param p1 = P₁
     * @return P₀ - x⋅P₁ ≡ [1]
     */
    bool pairing_check(const CommitmentAffine& p0, const CommitmentAffine& p1) const
    {
        Commitment result = Commitment(p0) + Commitment(p1) * trapdoor<G>;
        return result.is_point_at_infinity();
    }
};

template <typename G> struct Params {
    using Fr = typename G::Fr;
    using CommitmentAffine = typename G::affine_element;

    using Commitment = typename G::element;
    using Polynomial = barretenberg::Polynomial<Fr>;

    using CK = CommitmentKey<G>;
    using VK = VerificationKey<G>;
};
} // namespace fake

using Params = fake::Params<barretenberg::g1>;
using CommitmentKey = fake::CommitmentKey<barretenberg::g1>;
using VerificationKey = fake::VerificationKey<barretenberg::g1>;
} // namespace waffle::pcs