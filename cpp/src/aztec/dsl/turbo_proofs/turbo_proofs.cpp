
#include "turbo_proofs.hpp"
#include "plonk/proof_system/types/proof.hpp"
#include <common/log.hpp>
#include <plonk/proof_system/commitment_scheme/kate_commitment_scheme.hpp>
#include <proof_system/proving_key/serialize.hpp>
#include <dsl/acir_format/acir_format.hpp>
#include <stdlib/types/types.hpp>
#include <srs/reference_string/pippenger_reference_string.hpp>
#include <sstream>
#include <iostream>
#include <common/streams.hpp>

using namespace plonk::stdlib::types;

namespace turbo_proofs {

static std::shared_ptr<bonk::proving_key> proving_key;
static std::shared_ptr<bonk::verification_key> verification_key;
static std::shared_ptr<acir_format::acir_format> constraint_system;

void c_init_circuit_def(uint8_t const* constraint_system_buf)
{
    auto cs = from_buffer<acir_format::acir_format>(constraint_system_buf);
    init_circuit(cs);
}
void init_circuit(acir_format::acir_format cs)
{
    constraint_system = std::make_shared<acir_format::acir_format>(cs);
}

uint32_t c_get_circuit_size(uint8_t const* constraint_system_buf)
{
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
    auto crs_factory = std::make_unique<bonk::ReferenceStringFactory>();
    auto composer = create_circuit(constraint_system, std::move(crs_factory));

    auto prover = composer.create_prover();
    auto circuit_size = prover.get_circuit_size();

    return static_cast<uint32_t>(circuit_size);
}

uint32_t c_get_exact_circuit_size(uint8_t const* constraint_system_buf)
{
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
    auto crs_factory = std::make_unique<bonk::ReferenceStringFactory>();
    auto composer = create_circuit(constraint_system, std::move(crs_factory));

    auto num_gates = composer.get_num_gates();
    return static_cast<uint32_t>(num_gates);
}

void init_proving_key(std::unique_ptr<bonk::ReferenceStringFactory>&& crs_factory)
{
    auto composer = create_circuit(*constraint_system, std::move(crs_factory));
    proving_key = composer.compute_proving_key();
}

void init_verification_key(std::unique_ptr<bonk::ReferenceStringFactory>&& crs_factory)
{
    if (!proving_key) {
        std::abort();
    }
    // Patch the 'nothing' reference string fed to init_proving_key.
    // TODO(Blaine): Is this correct?
    proving_key->reference_string = crs_factory->get_prover_crs(proving_key->circuit_size);

    verification_key = Composer::compute_verification_key_base(proving_key, crs_factory->get_verifier_crs());
}

plonk::TurboProver new_prover(std::vector<fr> witness)
{
    Composer composer(proving_key, nullptr);
    create_circuit_with_witness(composer, *constraint_system, witness);

    info("composer gates: ", composer.get_num_gates());

    plonk::TurboProver prover = composer.create_prover();

    return prover;
}

bool verify_proof(plonk::proof const& proof)
{
    TurboVerifier verifier(verification_key, Composer::create_manifest(verification_key->num_public_inputs));

    std::unique_ptr<KateCommitmentScheme<turbo_settings>> kate_commitment_scheme =
        std::make_unique<KateCommitmentScheme<turbo_settings>>();
    verifier.commitment_scheme = std::move(kate_commitment_scheme);

    return verifier.verify_proof(proof);
}

size_t c_composer__new_proof(void* pippenger,
                             uint8_t const* g2x,
                             uint8_t const* constraint_system_buf,
                             uint8_t const* witness_buf,
                             uint8_t** proof_data_buf)
{

    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);

    auto crs_factory = std::make_unique<PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);

    auto witness = from_buffer<std::vector<fr>>(witness_buf);
    auto composer = create_circuit_with_witness(constraint_system, witness, std::move(crs_factory));

    // aligned_free((void*)witness_buf);
    // aligned_free((void*)g2x);
    // aligned_free((void*)constraint_system_buf);

    auto prover = composer.create_prover();
    auto heapProver = new TurboProver(std::move(prover));
    auto& proof_data = heapProver->construct_proof().proof_data;
    *proof_data_buf = proof_data.data();

    return proof_data.size();
}

bool c_composer__verify_proof(
    void* pippenger, uint8_t const* g2x, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length)
{
    bool verified = false;
#ifndef __wasm__
    try {
#endif

        auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
        auto crs_factory = std::make_unique<PippengerReferenceStringFactory>(
            reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
        auto composer = create_circuit(constraint_system, std::move(crs_factory));
        plonk::proof pp = { std::vector<uint8_t>(proof, proof + length) };

        auto verifier = composer.create_verifier();

        verified = verifier.verify_proof(pp);

#ifndef __wasm__
    } catch (const std::exception& e) {
        verified = false;
        info(e.what());
    }
#endif
    return verified;
}

uint32_t c_composer__smart_contract(void* pippenger,
                                    uint8_t const* g2x,
                                    uint8_t const* constraint_system_buf,
                                    uint8_t** output_buf)
{
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
    auto crs_factory = std::make_unique<PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    auto composer = create_circuit(constraint_system, std::move(crs_factory));

    auto verification_key = composer.compute_verification_key();

    std::ostringstream stream;
    // output_vk_sol_method(stream, verification_key);

    auto content_str = stream.str();
    std::vector<uint8_t> buffer(content_str.begin(), content_str.end());

    *output_buf = buffer.data();
    return static_cast<uint32_t>(buffer.size());
}

size_t c_init_proving_key(uint8_t const* constraint_system_buf, uint8_t const** pk_buf)
{
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);
    // We know that we don't actually need any CRS to create a proving key, so just feed in a nothing.
    // Hacky, but, right now it needs *something*.
    auto crs_factory = std::make_unique<ReferenceStringFactory>();
    auto composer = create_circuit(constraint_system, std::move(crs_factory));
    auto proving_key = composer.compute_proving_key();

    // Computing the size of the serialized key is non trivial. We know it's ~331mb.
    // Allocate a buffer large enough to hold it, and abort if we overflow.
    // This is to keep memory usage down.
    size_t total_buf_len = 350 * 1024 * 1024;
    auto raw_buf = (uint8_t*)malloc(total_buf_len);
    auto raw_buf_end = raw_buf;
    write(raw_buf_end, *proving_key);
    *pk_buf = raw_buf;
    auto len = static_cast<uint32_t>(raw_buf_end - raw_buf);
    if (len > total_buf_len) {
        info("Buffer overflow serializing proving key.");
        std::abort();
    }
    return len;
}

size_t c_init_verification_key(void* pippenger, uint8_t const* g2x, uint8_t const* pk_buf, uint8_t const** vk_buf)
{
    std::shared_ptr<ProverReferenceString> crs;
    bonk::proving_key_data pk_data;
    read(pk_buf, pk_data);
    auto proving_key = std::make_shared<bonk::proving_key>(std::move(pk_data), crs);

    auto crs_factory = std::make_unique<PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    // TODO(Blaine): Is this correct?
    proving_key->reference_string = crs_factory->get_prover_crs(proving_key->circuit_size);

    TurboComposer composer(proving_key, nullptr);
    auto verification_key =
        plonk::stdlib::types::Composer::compute_verification_key_base(proving_key, crs_factory->get_verifier_crs());

    // The composer_type has not yet been set. We need to set the composer_type for when we later read in and
    // construct the verification key so that we have the correct polynomial manifest
    verification_key->composer_type = ComposerType::TURBO;

    auto buffer = to_buffer(*verification_key);
    auto raw_buf = (uint8_t*)malloc(buffer.size());
    memcpy(raw_buf, (void*)buffer.data(), buffer.size());
    *vk_buf = raw_buf;

    return buffer.size();
}

size_t c_new_proof(void* pippenger,
                   uint8_t const* g2x,
                   uint8_t const* pk_buf,
                   uint8_t const* constraint_system_buf,
                   uint8_t const* witness_buf,
                   uint8_t** proof_data_buf)
{
    auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);

    std::shared_ptr<ProverReferenceString> crs;
    bonk::proving_key_data pk_data;
    read(pk_buf, pk_data);
    auto proving_key = std::make_shared<bonk::proving_key>(std::move(pk_data), crs);

    auto witness = from_buffer<std::vector<fr>>(witness_buf);

    auto crs_factory = std::make_unique<PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger), g2x);
    // TODO(Blaine): Is this correct?
    proving_key->reference_string = crs_factory->get_prover_crs(proving_key->circuit_size);

    TurboComposer composer(proving_key, nullptr);
    create_circuit_with_witness(composer, constraint_system, witness);

    auto prover = composer.create_prover();
    auto heapProver = new TurboProver(std::move(prover));
    auto& proof_data = heapProver->construct_proof().proof_data;
    *proof_data_buf = proof_data.data();

    return proof_data.size();
}

bool c_verify_proof(
    uint8_t const* g2x, uint8_t const* vk_buf, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length)
{
    bool verified = false;

#ifndef __wasm__
    try {
#endif
        auto constraint_system = from_buffer<acir_format::acir_format>(constraint_system_buf);

        auto crs = std::make_shared<VerifierMemReferenceString>(g2x);
        bonk::verification_key_data vk_data;
        read(vk_buf, vk_data);
        auto verification_key = std::make_shared<bonk::verification_key>(std::move(vk_data), crs);

        TurboComposer composer(nullptr, verification_key);
        create_circuit(composer, constraint_system);
        plonk::proof pp = { std::vector<uint8_t>(proof, proof + length) };

        auto verifier = composer.create_verifier();

        verified = verifier.verify_proof(pp);
#ifndef __wasm__
    } catch (const std::exception& e) {
        verified = false;
        info(e.what());
    }
#endif
    return verified;
}

} // namespace turbo_proofs
