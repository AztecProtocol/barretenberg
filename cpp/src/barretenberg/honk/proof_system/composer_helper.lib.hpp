// WORKTODO
// #pragma once
// #include "barretenberg/plonk/proof_system/proving_key/proving_key.hpp"
// #include "barretenberg/plonk/proof_system/verification_key/verification_key.hpp"

// namespace proof_system::plonk {

// /**
//  * @brief Computes the verification key by computing the:
//  * (1) commitments to the selector, permutation, and lagrange (first/last) polynomials,
//  * (2) sets the polynomial manifest using the data from proving key.
//  */
// std::shared_ptr<plonk::verification_key> compute_verification_key_common(
//     std::shared_ptr<plonk::proving_key> const& proving_key, std::shared_ptr<VerifierReferenceString> const& vrs)
// {
//     auto circuit_verification_key = std::make_shared<plonk::verification_key>(
//         proving_key->circuit_size, proving_key->num_public_inputs, vrs, proving_key->composer_type);
//     // TODO(kesha): Dirty hack for now. Need to actually make commitment-agnositc
//     auto commitment_key = proof_system::honk::pcs::kzg::CommitmentKey(proving_key->circuit_size,
//     "../srs_db/ignition");

//     for (size_t i = 0; i < proving_key->polynomial_manifest.size(); ++i) {
//         const auto& poly_info = proving_key->polynomial_manifest[i];

//         const std::string poly_label(poly_info.polynomial_label);
//         const std::string selector_commitment_label(poly_info.commitment_label);

//         if (poly_info.source == PolynomialSource::SELECTOR || poly_info.source == PolynomialSource::PERMUTATION ||
//             poly_info.source == PolynomialSource::OTHER) {
//             // Fetch the polynomial in its vector form.

//             // Commit to the constraint selector polynomial and insert the commitment in the verification key.

//             auto poly_commitment = commitment_key.commit(proving_key->polynomial_store.get(poly_label));
//             circuit_verification_key->commitments.insert({ selector_commitment_label, poly_commitment });
//         }
//     }

//     // Set the polynomial manifest in verification key.
//     circuit_verification_key->polynomial_manifest =
//     proof_system::plonk::PolynomialManifest(proving_key->composer_type);

//     return circuit_verification_key;
// }

// } // namespace proof_system::plonk
