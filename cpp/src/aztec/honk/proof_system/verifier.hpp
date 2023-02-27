#pragma once
#include "honk/pcs/commitment_key.hpp"
#include <honk/pcs/gemini/gemini.hpp>
#include <honk/pcs/shplonk/shplonk_single.hpp>
#include <honk/pcs/kzg/kzg.hpp>
#include "honk/proof_system/program_settings.hpp"
#include "plonk/proof_system/types/proof.hpp"
#include <cstdint>

namespace honk {
template <typename program_settings> class Verifier {

  public:
    Verifier(std::shared_ptr<bonk::verification_key> verifier_key = nullptr);
    Verifier(Verifier&& other);
    Verifier(const Verifier& other) = delete;
    Verifier& operator=(const Verifier& other) = delete;
    Verifier& operator=(Verifier&& other);

    // TODO(luke): proof is just an std::vector<uint8_t>; probably shouldn't even exist
    // Cody: Idk, what's wrong with an informative alias?
    // An improvement would be to template by flavor and then have proof contain even more info,
    // so it's easy to extract particular elements without looking at the manifest and counting
    // numbers of bytes, for instance.
    bool verify_proof(const plonk::proof& proof);

    std::shared_ptr<bonk::verification_key> key;
    std::map<std::string, barretenberg::g1::affine_element> kate_g1_elements;
    std::map<std::string, barretenberg::fr> kate_fr_elements;
    std::shared_ptr<pcs::kzg::VerificationKey> kate_verification_key;
};

extern template class Verifier<honk::standard_verifier_settings>;

typedef Verifier<honk::standard_verifier_settings> StandardVerifier;

} // namespace honk