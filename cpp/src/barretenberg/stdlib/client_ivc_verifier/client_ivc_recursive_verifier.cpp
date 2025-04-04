#include "client_ivc_recursive_verifier.hpp"

namespace bb::stdlib::recursion::honk {

/**
 * @brief Performs recursive verification of the Client IVC proof.
 */
ClientIVCRecursiveVerifier::Output ClientIVCRecursiveVerifier::verify(const ClientIVC::Proof& proof)
{
    // Construct stdlib Mega verification key
    auto stdlib_mega_vk = std::make_shared<RecursiveVerificationKey>(builder.get(), ivc_verification_key.mega);

    // Dummy aggregation object until we do proper aggregation
    auto agg_obj = aggregation_state<Builder>::construct_default(*builder);

    // Perform recursive decider verification
    MegaVerifier verifier{ builder.get(), stdlib_mega_vk };
    verifier.verify_proof(proof.mega_proof, agg_obj);

    // Perform Goblin recursive verification
    GoblinVerifierInput goblin_verification_key{ ivc_verification_key.eccvm, ivc_verification_key.translator };
    GoblinVerifier goblin_verifier{ builder.get(), goblin_verification_key };
    GoblinRecursiveVerifierOutput output = goblin_verifier.verify(proof.goblin_proof);

    return output;
}

} // namespace bb::stdlib::recursion::honk
