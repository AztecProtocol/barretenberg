
#include "acir_proofs.hpp"
#include "barretenberg/plonk/proof_system/proving_key/serialize.hpp"
#include "barretenberg/dsl/acir_format/acir_format.hpp"
#include "barretenberg/dsl/types.hpp"
#include "barretenberg/srs/reference_string/pippenger_reference_string.hpp"
#include "barretenberg/plonk/proof_system/verification_key/sol_gen.hpp"

namespace acir_proofs {

size_t get_solidity_verifier(uint8_t const* g2x, uint8_t const* vk_buf, uint8_t** output_buf)
{
    auto crs = std::make_shared<VerifierMemReferenceString>(g2x);
    proof_system::plonk::verification_key_data vk_data;
    read(vk_buf, vk_data);
    auto verification_key = std::make_shared<proof_system::plonk::verification_key>(std::move(vk_data), crs);

    std::ostringstream stream;
    // TODO(blaine): Should we just use "VerificationKey" generically?
    output_vk_sol(stream, verification_key, "UltraVerificationKey");

    auto content_str = stream.str();
    auto raw_buf = (uint8_t*)malloc(content_str.size());
    memcpy(raw_buf, (void*)content_str.data(), content_str.size());
    *output_buf = raw_buf;

    return content_str.size();
}

uint32_t get_exact_circuit_size(uint8_t const* constraint_system_buf)
{
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
    auto crs_factory = std::make_unique<proof_system::ReferenceStringFactory>();
    auto composer = create_circuit(constraint_system, std::move(crs_factory));

    auto num_gates = composer.get_num_gates();
    return static_cast<uint32_t>(num_gates);
}

uint32_t get_total_circuit_size(uint8_t const* constraint_system_buf)
{
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
    auto crs_factory = std::make_unique<proof_system::ReferenceStringFactory>();
    auto composer = create_circuit(constraint_system, std::move(crs_factory));

    return static_cast<uint32_t>(composer.get_total_circuit_size());
}

size_t init_proving_key(uint8_t const* constraint_system_buf, uint8_t const** pk_buf)
{
    info("INSIDE INIT PKEY");
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);

    // We know that we don't actually need any CRS to create a proving key, so just feed in a nothing.
    // Hacky, but, right now it needs *something*.
    auto crs_factory = std::make_unique<ReferenceStringFactory>();
    auto composer = create_circuit(constraint_system, std::move(crs_factory));
    info("composer.err(): ", composer.err());
    auto proving_key = composer.compute_proving_key();
    // info("composer.circuit_finalised: ", composer.circuit_finalised);
    proving_key->composer_type = proof_system::ComposerType::PLOOKUP;
    // info("proving_key->circuit_size: ", proving_key->circuit_size);

    auto buffer = to_buffer(*proving_key);
    auto raw_buf = (uint8_t*)malloc(buffer.size());
    memcpy(raw_buf, (void*)buffer.data(), buffer.size());
    *pk_buf = raw_buf;

    return buffer.size();
}

size_t init_verification_key(void* pippenger, uint8_t const* g2x, uint8_t const* pk_buf, uint8_t const** vk_buf)
{
    std::shared_ptr<ProverReferenceString> crs;
    plonk::proving_key_data pk_data;
    read(pk_buf, pk_data);
    auto proving_key = std::make_shared<plonk::proving_key>(std::move(pk_data), crs);

    auto crs_factory = std::make_unique<PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    proving_key->reference_string = crs_factory->get_prover_crs(proving_key->circuit_size);

    acir_format::Composer composer(proving_key, nullptr);
    auto verification_key =
        acir_format::Composer::compute_verification_key_base(proving_key, crs_factory->get_verifier_crs());

    // The composer_type has not yet been set. We need to set the composer_type for when we later read in and
    // construct the verification key so that we have the correct polynomial manifest
    verification_key->composer_type = proof_system::ComposerType::PLOOKUP;

    auto buffer = to_buffer(*verification_key);
    auto raw_buf = (uint8_t*)malloc(buffer.size());
    memcpy(raw_buf, (void*)buffer.data(), buffer.size());
    *vk_buf = raw_buf;

    return buffer.size();
}

size_t new_proof(void* pippenger,
                 uint8_t const* g2x,
                 uint8_t const* pk_buf,
                 uint8_t const* constraint_system_buf,
                 uint8_t const* witness_buf,
                 uint8_t** proof_data_buf)
{
    info("INSIDE NEW_PROOF");

    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);

    std::shared_ptr<ProverReferenceString> crs;
    plonk::proving_key_data pk_data;
    read(pk_buf, pk_data);
    auto proving_key = std::make_shared<plonk::proving_key>(std::move(pk_data), crs);

    auto witness = from_buffer<std::vector<fr>>(witness_buf);

    auto crs_factory = std::make_unique<PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    proving_key->reference_string = crs_factory->get_prover_crs(proving_key->circuit_size);

    acir_format::Composer composer(proving_key, nullptr);
    info("proving_key->circuit_size: ", proving_key->circuit_size);

    create_circuit_with_witness(composer, constraint_system, witness);

    info("composer.err(): ", composer.err());
    info("composer.get_num_gates(): ", composer.get_num_gates());
    info("composer.circuit_finalised: ", composer.circuit_finalised);
    info("composer num vars: ", composer.get_num_variables());

    auto prover = composer.create_ultra_with_keccak_prover();
    info("composer.circuit_finalised: ", composer.circuit_finalised);
    info("composer num vars: ", composer.get_num_variables());
    info("composer.variables.size(): ", composer.variables.size());

    // composer.crs_factory_ = std::move(crs_factory);
    // info("got crs");
    // auto proof = prover.construct_proof();
    // info("got proof");
    // auto verifier = composer.create_ultra_with_keccak_verifier();
    // auto verified = verifier.verify_proof(proof);
    // info("VERIFIED: ", verified);

    auto heapProver = new acir_format::Prover(std::move(prover));
    auto& proof_data = heapProver->construct_proof().proof_data;
    // auto& proof_data = proof.proof_data;
    *proof_data_buf = proof_data.data();
    info("proof_data.size(): ", proof_data.size());
    return proof_data.size();
}

bool verify_proof(
    uint8_t const* g2x, uint8_t const* vk_buf, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length)
{
    info("INSIDE VERIFY_PROOF");
    bool verified = false;

#ifndef __wasm__
    try {
#endif
        auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
        auto crs = std::make_shared<VerifierMemReferenceString>(g2x);
        plonk::verification_key_data vk_data;
        read(vk_buf, vk_data);
        auto verification_key = std::make_shared<proof_system::plonk::verification_key>(std::move(vk_data), crs);

        acir_format::Composer composer(nullptr, verification_key);
        create_circuit(composer, constraint_system);
        auto crs_factory = std::make_unique<ReferenceStringFactory>();
        composer.crs_factory_ = std::move(crs_factory);

        info("composer.err(): ", composer.err());
        info("composer.get_num_gates(): ", composer.get_num_gates());
        info("composer.circuit_finalised: ", composer.circuit_finalised);
        info("composer num vars: ", composer.get_num_variables());

        info("proof length: ", length);

        plonk::proof pp = { std::vector<uint8_t>(proof, proof + length) };
        info("got pp");

        auto verifier = composer.create_ultra_with_keccak_verifier();
        info("got verifier");
        info("composer.circuit_finalised: ", composer.circuit_finalised);
        info("composer num vars: ", composer.get_num_variables());

        verified = verifier.verify_proof(pp);
        info("verified proof");

#ifndef __wasm__
    } catch (const std::exception& e) {
        verified = false;
        info(e.what());
    }
#endif
    return verified;
}

} // namespace acir_proofs
