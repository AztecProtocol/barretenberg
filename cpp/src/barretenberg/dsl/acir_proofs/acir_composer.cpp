#include "acir_composer.hpp"
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/throw_or_abort.hpp"
#include "barretenberg/plonk/proof_system/proving_key/serialize.hpp"
#include "barretenberg/dsl/acir_format/acir_format.hpp"
#include "barretenberg/dsl/types.hpp"
#include "barretenberg/plonk/proof_system/verification_key/verification_key.hpp"
#include "barretenberg/srs/factories/crs_factory.hpp"
#include "barretenberg/plonk/proof_system/verification_key/sol_gen.hpp"
#include "barretenberg/srs/factories/crs_factory.hpp"

namespace acir_proofs {

AcirComposer::AcirComposer()
    : composer_(0, 0, 0)
{}

void AcirComposer::create_circuit(acir_format::acir_format& constraint_system)
{
    composer_ = acir_format::create_circuit(constraint_system, nullptr);

    // We are done with the constraint system at this point, and we need the memory slab back.
    constraint_system.constraints.clear();
    constraint_system.constraints.shrink_to_fit();

    exact_circuit_size_ = composer_.get_num_gates();
    total_circuit_size_ = composer_.get_total_circuit_size();
    circuit_subgroup_size_ = composer_.get_circuit_subgroup_size(total_circuit_size_);
}

void AcirComposer::init_proving_key(std::shared_ptr<barretenberg::srs::factories::CrsFactory> const& crs_factory,
                                    acir_format::acir_format& constraint_system,
                                    size_t size_hint)
{
    composer_ = acir_format::create_circuit(constraint_system, crs_factory, size_hint);

    // We are done with the constraint system at this point, and we need the memory slab back.
    constraint_system.constraints.clear();
    constraint_system.constraints.shrink_to_fit();

    exact_circuit_size_ = composer_.get_num_gates();
    total_circuit_size_ = composer_.get_total_circuit_size();
    circuit_subgroup_size_ = composer_.get_circuit_subgroup_size(total_circuit_size_);

    proving_key_ = composer_.compute_proving_key();
}

std::vector<uint8_t> AcirComposer::create_proof(
    std::shared_ptr<barretenberg::srs::factories::CrsFactory> const& crs_factory,
    acir_format::acir_format& constraint_system,
    acir_format::WitnessVector& witness,
    bool is_recursive)
{
    composer_ = acir_format::Composer(proving_key_, verification_key_, circuit_subgroup_size_);
    // You can't produce the verification key unless you manually set the crs. Which seems like a bug.
    composer_.composer_helper.crs_factory_ = crs_factory;

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

void AcirComposer::load_verification_key(std::shared_ptr<barretenberg::srs::factories::CrsFactory> const& crs_factory,
                                         proof_system::plonk::verification_key_data&& data)
{
    verification_key_ =
        std::make_shared<proof_system::plonk::verification_key>(std::move(data), crs_factory->get_verifier_crs());
    composer_ = acir_format::Composer(proving_key_, verification_key_, circuit_subgroup_size_);
}

bool AcirComposer::verify_proof(std::vector<uint8_t> const& proof, bool is_recursive)
{
    // TODO: Hack. Shouldn't need to do this. 2144 is size with no public inputs.
    if ((proof.size() - 2144) / 32 != verification_key_->num_public_inputs) {
        return false;
    };

    if (is_recursive) {
        auto verifier = composer_.create_verifier();
        return verifier.verify_proof({ proof });
    } else {
        auto verifier = composer_.create_ultra_with_keccak_verifier();
        return verifier.verify_proof({ proof });
    }
}

std::string AcirComposer::get_solidity_verifier()
{
    std::ostringstream stream;
    output_vk_sol(stream, verification_key_, "UltraVerificationKey");
    return stream.str();
}

/**
 * @brief Takes in a proof buffer and converts into a vector of field elements.
 *        The Recursion opcode requires the proof serialized as a vector of witnesses.
 *        Use this method to get the witness values!
 *
 * @param proof
 * @param num_inner_public_inputs - number of public inputs on the proof being serialized
 */
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

/**
 * @brief Takes in a verification key buffer and converts into a vector of field elements.
 *        The Recursion opcode requires the vk serialized as a vector of witnesses.
 *        Use this method to get the witness values!
 *        The composer should already have a verification key initialized.
 */
std::vector<barretenberg::fr> AcirComposer::serialize_verification_key_into_fields()
{
    std::vector<barretenberg::fr> output = acir_format::export_key_in_recursion_format(verification_key_);
    return output;
}

} // namespace acir_proofs
