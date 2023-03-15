#pragma once
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "./program_settings.hpp"
#include "barretenberg/proof_system/verification_key/verification_key.hpp"
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include <memory>

namespace honk {
template <typename program_settings> class Verifier {

  public:
    Verifier(std::shared_ptr<bonk::verification_key> verifier_key = nullptr);
    Verifier(Verifier&& other);
    Verifier(const Verifier& other) = delete;
    Verifier& operator=(const Verifier& other) = delete;
    Verifier& operator=(Verifier&& other);

    bool verify_proof(const plonk::proof& proof);

    std::shared_ptr<bonk::verification_key> key;
    std::map<std::string, barretenberg::g1::affine_element> kate_g1_elements;
    std::map<std::string, barretenberg::fr> kate_fr_elements;
    std::shared_ptr<pcs::kzg::VerificationKey> kate_verification_key;
};

extern template class Verifier<honk::standard_verifier_settings>;

typedef Verifier<honk::standard_verifier_settings> StandardVerifier;

} // namespace honk
