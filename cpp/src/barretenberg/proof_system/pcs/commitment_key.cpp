/**
 * @brief Provides interfaces for different 'CommitmentKey' classes.
 *
 * TODO(#218)(Mara): This class should handle any modification to the SRS (e.g compute pippenger point table) to
 * simplify the codebase.
 */

#include "commitment_key.hpp"
#include "barretenberg/ecc/curves/bn254/pairing.hpp"

namespace proof_system::pcs {

namespace kzg {
using Fr = typename barretenberg::g1::Fr;
using Commitment = typename barretenberg::g1::affine_element;
using GroupElement = barretenberg::g1::element;

using Polynomial = barretenberg::Polynomial<Fr>;

/**
 * @brief Construct a new Kate Commitment Key object from existing SRS
 *
 * @param n
 * @param path
 *
 */
Params::CommitmentKey::CommitmentKey(const size_t num_points,
                                     std::shared_ptr<barretenberg::srs::factories::CrsFactory> crs_factory)
    : pippenger_runtime_state(num_points)
    , srs(crs_factory->get_prover_crs(num_points))
{}

// Note: This constructor is used only by Plonk; For Honk the CommitmentKey is solely responsible for extracting
// the srs.
Params::CommitmentKey::CommitmentKey(const size_t num_points,
                                     std::shared_ptr<barretenberg::srs::factories::ProverCrs<curve::BN254>> prover_srs)
    : pippenger_runtime_state(num_points)
    , srs(prover_srs)
{}

/**
 * @brief Uses the ProverSRS to create a commitment to p(X)
 *
 * @param polynomial a univariate polynomial p(X) = ∑ᵢ aᵢ⋅Xⁱ ()
 * @return Commitment computed as C = [p(x)] = ∑ᵢ aᵢ⋅[xⁱ]₁ where x is the secret trapdoor
 */
Commitment Params::CommitmentKey::commit(std::span<const Fr> polynomial)
{
    const size_t degree = polynomial.size();
    ASSERT(degree <= srs->get_monomial_size());
    return barretenberg::scalar_multiplication::pippenger_unsafe<curve::BN254>(
        const_cast<Fr*>(polynomial.data()), srs->get_monomial_points(), degree, pippenger_runtime_state);
};

/**
 * @brief Construct a new Kate Verification Key object from existing SRS
 *
 * @param num_points
 * @param verifier_srs verifier G2 point
 */
Params::VerificationKey::VerificationKey([[maybe_unused]] size_t num_points,
                                         std::shared_ptr<barretenberg::srs::factories::CrsFactory> crs_factory)
    : verifier_srs(crs_factory->get_verifier_crs())
{}

/**
 * @brief verifies a pairing equation over 2 points using the verifier SRS
 *
 * @param p0 = P₀
 * @param p1 = P₁
 * @return e(P₀,[1]₁)e(P₁,[x]₂) ≡ [1]ₜ
 */
bool Params::VerificationKey::pairing_check(const GroupElement& p0, const GroupElement& p1)
{
    Commitment pairing_points[2]{ p0, p1 };
    // The final pairing check of step 12.
    // TODO(Adrian): try to template parametrise the pairing + fq12 output :/
    barretenberg::fq12 result = barretenberg::pairing::reduced_ate_pairing_batch_precomputed(
        pairing_points, verifier_srs->get_precomputed_g2_lines(), 2);

    return (result == barretenberg::fq12::one());
}

} // namespace kzg

namespace ipa {

using Fr = typename barretenberg::g1::Fr;
using Commitment = typename barretenberg::g1::affine_element;
using GroupElement = barretenberg::g1::element;

using Polynomial = barretenberg::Polynomial<Fr>;

/**
 * @brief Construct a new IPA Commitment Key object from existing SRS..
 *
 * @param num_points
 * @param path
 *
 */
Params::CommitmentKey::CommitmentKey(const size_t num_points,
                                     std::shared_ptr<barretenberg::srs::factories::CrsFactory> crs_factory)
    : pippenger_runtime_state(num_points)
    , srs(crs_factory->get_prover_crs(num_points))
{}

/**
 * @brief Uses the ProverSRS to create an unblinded commitment to p(X)
 *
 * @param polynomial a univariate polynomial p(X) = ∑ᵢ aᵢ⋅Xⁱ ()
 * @return Commitment computed as C = [p(x)] = ∑ᵢ aᵢ⋅Gᵢ where Gᵢ is the i-th element of the SRS
 */
Commitment Params::CommitmentKey::commit(std::span<const Fr> polynomial)
{
    const size_t degree = polynomial.size();
    ASSERT(degree <= srs->get_monomial_size());
    return barretenberg::scalar_multiplication::pippenger_unsafe<curve::BN254>(
        const_cast<Fr*>(polynomial.data()), srs->get_monomial_points(), degree, pippenger_runtime_state);
};

/**
 * @brief Construct a new IPA Verification Key object from existing SRS
 *
 *
 * @param num_points specifies the length of the SRS
 * @param path is the location to the SRS file
 */
Params::VerificationKey::VerificationKey(size_t num_points,
                                         std::shared_ptr<barretenberg::srs::factories::CrsFactory> crs_factory)
    : pippenger_runtime_state(num_points)
    , srs(crs_factory->get_prover_crs(num_points))
{}

} // namespace ipa

} // namespace proof_system::pcs
