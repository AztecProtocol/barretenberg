
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
{}

void AcirComposer::init_proving_key(acir_format::acir_format&& constraint_system)
{
    constraint_system_ = std::move(constraint_system);
    auto composer = create_circuit(constraint_system_, crs_factory_);
    exact_circuit_size_ = composer.get_num_gates();
    total_circuit_size_ = composer.get_total_circuit_size();
    proving_key_ = composer.compute_proving_key();
}

std::vector<uint8_t> AcirComposer::create_proof(std::vector<fr> const& witness)
{
    acir_format::Composer composer(proving_key_, nullptr, total_circuit_size_);

    create_circuit_with_witness(composer, constraint_system_, witness);

    auto prover = composer.create_ultra_with_keccak_prover();
    return prover.construct_proof().proof_data;
}

void AcirComposer::init_verification_key()
{
    if (!proving_key_) {
        throw_or_abort("init_proving_key must be called first.");
    }
    acir_format::Composer composer(proving_key_, nullptr);
    verification_key_ = composer.compute_verification_key();

    // verification_key_ =
    //     acir_format::Composer::compute_verification_key_base(proving_key_, crs_factory_->get_verifier_crs());

    // The composer_type has not yet been set. We need to set the composer_type for when we later read in and
    // construct the verification key so that we have the correct polynomial manifest
    // verification_key->composer_type = proof_system::ComposerType::PLOOKUP;
}

bool AcirComposer::verify_proof(std::vector<uint8_t> const& proof)
{
    acir_format::Composer composer(nullptr, verification_key_);
    composer.create_ultra_with_keccak_verifier();
    auto verifier = composer.create_ultra_with_keccak_verifier();
    return verifier.verify_proof({ proof });
}

std::string AcirComposer::get_solidity_verifier()
{
    std::ostringstream stream;
    output_vk_sol(stream, verification_key_, "UltraVerificationKey");
    return stream.str();
}

} // namespace acir_proofs
