#include "acir_composer.hpp"
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/throw_or_abort.hpp"
#include "barretenberg/plonk/proof_system/proving_key/serialize.hpp"
#include "barretenberg/dsl/acir_format/acir_format.hpp"
#include "barretenberg/dsl/types.hpp"
#include "barretenberg/srs/reference_string/pippenger_reference_string.hpp"
#include "barretenberg/plonk/proof_system/verification_key/sol_gen.hpp"
#include "barretenberg/srs/reference_string/reference_string.hpp"

namespace acir_proofs {

AcirComposer::AcirComposer(std::shared_ptr<proof_system::ReferenceStringFactory> const& crs_factory)
    : crs_factory_(crs_factory)
    , composer_(0, 0, 0)
{}

void AcirComposer::init_proving_key(acir_format::acir_format& constraint_system, size_t size_hint)
{
    auto composer = create_circuit(constraint_system, crs_factory_, size_hint);

    // We are done with the constraint system at this point, and we need the memory slab back.
    // constraint_system = acir_format::acir_format();
    constraint_system.constraints.clear();
    constraint_system.constraints.shrink_to_fit();

    exact_circuit_size_ = composer.get_num_gates();
    info("**** EXACT ", exact_circuit_size_);
    total_circuit_size_ = composer.get_total_circuit_size();
    info("**** TOTAL ", total_circuit_size_);
    // Exact or total fed in here?
    circuit_subgroup_size_ = composer.get_circuit_subgroup_size(exact_circuit_size_);
    info("**** SUBGROUP ", circuit_subgroup_size_);
    proving_key_ = composer.compute_proving_key();
}

std::vector<uint8_t> AcirComposer::create_proof(acir_format::acir_format& constraint_system,
                                                acir_format::WitnessVector& witness)
{
    // composer_ = acir_format::Composer(proving_key_, nullptr, circuit_subgroup_size_);
    composer_ = acir_format::Composer(crs_factory_, 1 << 19);

    create_circuit_with_witness(composer_, constraint_system, witness);

    // We are done with the constraint system at this point, and we need the memory slab back.
    constraint_system.constraints.clear();
    constraint_system.constraints.shrink_to_fit();
    witness.clear();
    witness.shrink_to_fit();

    auto prover = composer_.create_ultra_with_keccak_prover();
    auto proof = prover.construct_proof().proof_data;
    return proof;
}

void AcirComposer::init_verification_key()
{
    composer_.compute_verification_key();
    // if (!proving_key_) {
    //     throw_or_abort("init_proving_key must be called first.");
    // }
    // // acir_format::Composer composer(proving_key_, nullptr);
    // // verification_key_ = composer.compute_verification_key();

    // verification_key_ =
    //     acir_format::Composer::compute_verification_key_base(proving_key_, crs_factory_->get_verifier_crs());

    // // The composer_type has not yet been set. We need to set the composer_type for when we later read in and
    // // construct the verification key so that we have the correct polynomial manifest
    // verification_key_->composer_type = proof_system::ComposerType::PLOOKUP;
}

bool AcirComposer::verify_proof(std::vector<uint8_t> const& proof)
{
    auto verifier = composer_.create_ultra_with_keccak_verifier();
    return verifier.verify_proof({ proof });
}

std::string AcirComposer::get_solidity_verifier()
{
    std::ostringstream stream;
    output_vk_sol(stream, verification_key_, "UltraVerificationKey");
    return stream.str();
}

} // namespace acir_proofs
