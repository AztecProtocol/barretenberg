#include "simple.hpp"
#include <barretenberg/plonk/proof_system/types/proof.hpp>
#include <barretenberg/plonk/proof_system/proving_key/serialize.hpp>
#include <barretenberg/crypto/sha256/sha256.hpp>
#include <memory>

namespace examples::simple {

using namespace proof_system::plonk;
using namespace stdlib::types;

void build_circuit(Composer& composer)
{
    // while (composer.get_num_gates() <= 262144) {
    while (composer.get_num_gates() <= 65536) {
        plonk::stdlib::pedersen_commitment<Composer>::compress(field_ct(witness_ct(&composer, 1)),
                                                               field_ct(witness_ct(&composer, 1)));
    }
}

Composer* create_composer(std::shared_ptr<proof_system::ReferenceStringFactory> const& crs_factory)
{
    auto composer = std::make_unique<Composer>(crs_factory);
    info("composer gates: ", composer->get_num_gates());
    build_circuit(*composer);

    if (composer->failed()) {
        std::string error = format("composer logic failed: ", composer->err());
        throw_or_abort(error);
    }

    info("public inputs: ", composer->public_inputs.size());
    info("composer gates: ", composer->get_num_gates());

    info("computing proving key...");
    auto pk = composer->compute_proving_key();
    // auto pk_buf = to_buffer(*pk);
    // info("pk hash: ", sha256::sha256(pk_buf), " ", pk_buf.size());

    return composer.release();
}

proof create_proof(Composer* composer)
{
    info("computing proof...");
    // auto prover = composer->create_ultra_with_keccak_prover();
    auto prover = composer->create_prover();
    return prover.construct_proof();
}

bool verify_proof(Composer* composer, proof_system::plonk::proof const& proof)
{
    info("computing verification key...");
    composer->compute_verification_key();

    // auto verifier = composer->create_ultra_with_keccak_verifier();
    auto verifier = composer->create_verifier();
    return verifier.verify_proof(proof);
}

void delete_composer(Composer* composer)
{
    delete composer;
}

} // namespace examples::simple
