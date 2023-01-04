#pragma once
#include "../../plonk/proof_system/types/plonk_proof.hpp"
#include "../../plonk/proof_system/types/program_settings.hpp"
#include "../../plonk/proof_system/verification_key/verification_key.hpp"
#include <transcript/manifest.hpp>
#include <plonk/proof_system/commitment_scheme/commitment_scheme.hpp>
#include "../sumcheck/polynomials/multivariates.hpp"
#include "../sumcheck/sumcheck.hpp"
#include "../sumcheck/relations/arithmetic_relation.hpp"

namespace honk {
template <typename program_settings> class HonkVerifier {

  public:
    HonkVerifier(std::shared_ptr<waffle::verification_key> verifier_key = nullptr,
                 const transcript::Manifest& manifest = transcript::Manifest({}));
    HonkVerifier(HonkVerifier&& other);
    HonkVerifier(const HonkVerifier& other) = delete;
    HonkVerifier& operator=(const HonkVerifier& other) = delete;
    HonkVerifier& operator=(HonkVerifier&& other);

    bool verify_proof(const waffle::plonk_proof& proof);
    transcript::Manifest manifest;

    std::shared_ptr<waffle::verification_key> key;
    std::map<std::string, barretenberg::g1::affine_element> kate_g1_elements;
    std::map<std::string, barretenberg::fr> kate_fr_elements;
    std::unique_ptr<waffle::CommitmentScheme> commitment_scheme;
};

extern template class HonkVerifier<waffle::standard_verifier_settings>;

typedef HonkVerifier<waffle::standard_verifier_settings> Verifier;

} // namespace honk