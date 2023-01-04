#include <common/throw_or_abort.hpp>
#include <plonk/proof_system/constants.hpp>
#include "./verifier.hpp"
#include "../../plonk/proof_system/public_inputs/public_inputs.hpp"
#include <ecc/curves/bn254/fq12.hpp>
#include <ecc/curves/bn254/pairing.hpp>
#include <ecc/curves/bn254/scalar_multiplication/scalar_multiplication.hpp>
#include <polynomials/polynomial_arithmetic.hpp>

using namespace barretenberg;
using namespace honk::sumcheck;

namespace honk {
template <typename program_settings>
HonkVerifier<program_settings>::HonkVerifier(std::shared_ptr<waffle::verification_key> verifier_key,
                                             const transcript::Manifest& input_manifest)
    : manifest(input_manifest)
    , key(verifier_key)
{}

template <typename program_settings>
HonkVerifier<program_settings>::HonkVerifier(HonkVerifier&& other)
    : manifest(other.manifest)
    , key(other.key)
    , commitment_scheme(std::move(other.commitment_scheme))
{}

template <typename program_settings>
HonkVerifier<program_settings>& HonkVerifier<program_settings>::operator=(HonkVerifier&& other)
{
    key = other.key;
    manifest = other.manifest;
    commitment_scheme = (std::move(other.commitment_scheme));
    kate_g1_elements.clear();
    kate_fr_elements.clear();
    return *this;
}

/**
* @brief This function verifies a Honk proof for given program settings.
*
* TODO(luke): Complete this description
* @detail A Standard Honk proof is of the form:
    Multilinear evaluations:
        w_i(X),        i = 1,2,3
        sigma_i(X),    i = 1,2,3
        q_i(X),        i = 1,2,3,4,5
        z_perm(X),
        L_0(X),
        id(X)

    Univariate polynomials (evaluations of Univarites on small set of points):
        S_l, l = 0, ..., d-1

    Multilinear commitments:
        [w_i]_1,    i = 1,2,3
        [z_perm],
        Gemini/Shplonk contributions (TBD)
*/
template <typename program_settings> bool HonkVerifier<program_settings>::verify_proof(const waffle::plonk_proof& proof)
{
    key->program_width = program_settings::program_width;

    // Add the proof data to the transcript, according to the manifest. Also initialise the transcript's hash type
    // and challenge bytes.
    auto transcript = transcript::StandardTranscript(
        proof.proof_data, manifest, program_settings::hash_type, program_settings::num_challenge_bytes);

    // From the verification key, also add n & l (the circuit size and the number of public inputs) to the
    // transcript.
    transcript.add_element("circuit_size",
                           { static_cast<uint8_t>(key->n >> 24),
                             static_cast<uint8_t>(key->n >> 16),
                             static_cast<uint8_t>(key->n >> 8),
                             static_cast<uint8_t>(key->n) });

    transcript.add_element("public_input_size",
                           { static_cast<uint8_t>(key->num_public_inputs >> 24),
                             static_cast<uint8_t>(key->num_public_inputs >> 16),
                             static_cast<uint8_t>(key->num_public_inputs >> 8),
                             static_cast<uint8_t>(key->num_public_inputs) });

    // Compute challenges from the proof data, based on the manifest, using the Fiat-Shamir heuristic
    transcript.apply_fiat_shamir("init");
    transcript.apply_fiat_shamir("eta");
    transcript.apply_fiat_shamir("beta");
    // transcript.apply_fiat_shamir("alpha");

    // Compute some basic public polys like id(X), pow(X), and any required Lagrange polys

    // Execute Sumcheck Verifier
    // TODO(luke): putting template param stuff here for convenience; to be cleaned up later
    using Transcript = transcript::StandardTranscript;
    using FF = barretenberg::fr;
    const size_t num_polys(StandardArithmetization::NUM_POLYNOMIALS);
    const size_t multivariate_d(1);
    using Multivariates = Multivariates<FF, num_polys, multivariate_d>;
    auto sumcheck = Sumcheck<Multivariates, Transcript, ArithmeticRelation>(transcript);
    // sumcheck.execute_verifier(); // Need to mock prover in tests for this to run

    // Run Gemini verification stuff:
    // compute simulated Folded poly commitments
    // compute purported evaulation of Fold^(0)
    // computed simulated commitment [Q_z]

    // Do pairing check
    barretenberg::fq12 result = barretenberg::fq12::one();

    return (result == barretenberg::fq12::one());
}

template class HonkVerifier<waffle::standard_verifier_settings>;

} // namespace honk