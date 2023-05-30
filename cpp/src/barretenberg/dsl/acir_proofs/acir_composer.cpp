#include "acir_composer.hpp"
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/throw_or_abort.hpp"
#include "barretenberg/plonk/proof_system/proving_key/serialize.hpp"
#include "barretenberg/dsl/acir_format/acir_format.hpp"
#include "barretenberg/dsl/types.hpp"
#include "barretenberg/srs/reference_string/pippenger_reference_string.hpp"
#include "barretenberg/plonk/proof_system/verification_key/sol_gen.hpp"
#include "barretenberg/srs/reference_string/reference_string.hpp"
#include "barretenberg/stdlib/recursion/verifier/verifier.hpp"

namespace acir_proofs {

using namespace proof_system::plonk;

AcirComposer::AcirComposer(std::shared_ptr<proof_system::ReferenceStringFactory> const& crs_factory)
    : crs_factory_(crs_factory)
    , composer_(0, 0, 0)
{}

void AcirComposer::init_proving_key(acir_format::acir_format& constraint_system, size_t size_hint)
{
    composer_ = acir_format::create_circuit(constraint_system, crs_factory_, size_hint);

    // We are done with the constraint system at this point, and we need the memory slab back.
    // constraint_system = acir_format::acir_format();
    constraint_system.constraints.clear();
    constraint_system.constraints.shrink_to_fit();

    total_circuit_size_ = composer_.get_total_circuit_size();
    circuit_subgroup_size_ = composer_.get_circuit_subgroup_size(total_circuit_size_);

    proving_key_ = composer_.compute_proving_key();
}

std::vector<uint8_t> AcirComposer::create_proof(acir_format::acir_format& constraint_system,
                                                acir_format::WitnessVector& witness,
                                                bool is_recursive)
{
    composer_ = acir_format::Composer(proving_key_, verification_key_, circuit_subgroup_size_);
    // You can't produce the verification key unless you manually set the crs. Which seems like a bug.
    composer_.crs_factory_ = crs_factory_;

    create_circuit_with_witness(composer_, constraint_system, witness);

    // We are done with the constraint system at this point, and we need the memory slab back.
    constraint_system.constraints.clear();
    constraint_system.constraints.shrink_to_fit();
    witness.clear();
    witness.shrink_to_fit();

    std::vector<uint8_t> proof;
    if (is_recursive) {
        auto prover = composer_.create_prover();
        proof = prover.construct_proof().proof_data;
    } else {
        auto prover = composer_.create_ultra_with_keccak_prover();
        proof = prover.construct_proof().proof_data;
    }
    return proof;
}

std::shared_ptr<proof_system::plonk::verification_key> AcirComposer::init_verification_key()
{
    return verification_key_ = composer_.compute_verification_key();
}

bool AcirComposer::verify_proof(std::vector<uint8_t> const& proof, bool is_recursive)
{
    bool verified = false;
    if (is_recursive) {
        auto verifier = composer_.create_verifier();
        verified = verifier.verify_proof({ proof });
    } else {
        auto verifier = composer_.create_ultra_with_keccak_verifier();
        verified = verifier.verify_proof({ proof });
    }
    return verified;
}

std::string AcirComposer::get_solidity_verifier()
{
    std::ostringstream stream;
    output_vk_sol(stream, verification_key_, "UltraVerificationKey");
    return stream.str();
}

std::vector<barretenberg::fr> AcirComposer::serialize_proof_into_fields(std::vector<uint8_t> const& proof,
                                                                        size_t num_inner_public_inputs)
{
    transcript::StandardTranscript transcript(proof,
                                              acir_format::Composer::create_manifest(num_inner_public_inputs),
                                              transcript::HashType::PlookupPedersenBlake3s,
                                              16);

    std::vector<barretenberg::fr> output = acir_format::export_transcript_in_recursion_format(transcript);
    return output;
}

// The composer should already have a verification key initialized
std::vector<barretenberg::fr> AcirComposer::serialize_verification_key_into_fields()
{
    std::vector<barretenberg::fr> output = acir_format::export_key_in_recursion_format(verification_key_);
    return output;
}

} // namespace acir_proofs
