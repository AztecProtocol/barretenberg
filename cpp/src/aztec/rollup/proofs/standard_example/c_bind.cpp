#include "c_bind.h"
#include "standard_example.hpp"
#include <common/streams.hpp>
#include <cstdint>
#include <srs/reference_string/mem_reference_string.hpp>

using namespace barretenberg;
using namespace plonk::stdlib::types;

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void standard_example__init_circuit_def(uint8_t const* constraint_system_buf)
{
    rollup::proofs::standard_example::c_init_circuit_def(constraint_system_buf);
}

// Get the circuit size for the constraint system.
WASM_EXPORT uint32_t standard_example__get_circuit_size(uint8_t const* constraint_system_buf)
{
    return rollup::proofs::standard_example::c_get_circuit_size(constraint_system_buf);
}

// Get the exact circuit size for the constraint system.
WASM_EXPORT uint32_t standard_example__get_exact_circuit_size(uint8_t const* constraint_system_buf)
{
    return rollup::proofs::standard_example::c_get_exact_circuit_size(constraint_system_buf);
}

WASM_EXPORT void standard_example__init_proving_key()
{
    auto crs_factory = std::make_unique<waffle::ReferenceStringFactory>();
    rollup::proofs::standard_example::init_proving_key(std::move(crs_factory));
}

WASM_EXPORT void standard_example__init_verification_key(void* pippenger_ptr, uint8_t const* g2x)
{

    auto crs_factory = std::make_unique<waffle::PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger_ptr), g2x);
    rollup::proofs::standard_example::init_verification_key(std::move(crs_factory));
}

WASM_EXPORT void* standard_example__new_prover(uint8_t const* witness_buf)
{
    auto witness = from_buffer<std::vector<fr>>(witness_buf);

    auto prover = rollup::proofs::standard_example::new_prover(witness);
    return new waffle::TurboProver(std::move(prover));
}

WASM_EXPORT void standard_example__delete_prover(void* prover)
{
    delete reinterpret_cast<waffle::TurboProver*>(prover);
}

WASM_EXPORT bool standard_example__verify_proof(uint8_t* proof, uint32_t length)
{
    waffle::plonk_proof pp = { std::vector<uint8_t>(proof, proof + length) };
    return rollup::proofs::standard_example::verify_proof(pp);
}
}

extern "C" {

WASM_EXPORT size_t c_init_proving_key(uint8_t const* constraint_system_buf, uint8_t const** pk_buf)
{
    return rollup::proofs::standard_example::c_init_proving_key(constraint_system_buf, pk_buf);
}

WASM_EXPORT size_t c_init_verification_key(void* pippenger,
                                           uint8_t const* g2x,
                                           uint8_t const* pk_buf,
                                           uint8_t const** vk_buf)
{
    return rollup::proofs::standard_example::c_init_verification_key(pippenger, g2x, pk_buf, vk_buf);
}

WASM_EXPORT size_t c_new_proof(void* pippenger,
                               uint8_t const* g2x,
                               uint8_t const* pk_buf,
                               uint8_t const* constraint_system_buf,
                               uint8_t const* witness_buf,
                               uint8_t** proof_data_buf)
{
    return rollup::proofs::standard_example::c_new_proof(
        pippenger, g2x, pk_buf, constraint_system_buf, witness_buf, proof_data_buf);
}

WASM_EXPORT bool c_verify_proof(
    uint8_t const* g2x, uint8_t const* vk_buf, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length)
{
    return rollup::proofs::standard_example::c_verify_proof(g2x, vk_buf, constraint_system_buf, proof, length);
}
}

// standard format stuff

using namespace waffle;
using namespace std;

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

extern "C" {

// BACKWARDS COMPATIBILITY
WASM_EXPORT uint32_t composer__get_circuit_size(uint8_t const* constraint_system_buf)
{
    return rollup::proofs::standard_example::c_get_circuit_size(constraint_system_buf);
}

// BACKWARDS COMPATIBILITY
WASM_EXPORT uint32_t composer__smart_contract(void* pippenger,
                                              uint8_t const* g2x,
                                              uint8_t const* constraint_system_buf,
                                              uint8_t** output_buf)
{
    return rollup::proofs::standard_example::c_composer__smart_contract(
        pippenger, g2x, constraint_system_buf, output_buf);
}

// BACKWARDS COMPATIBILITY
WASM_EXPORT size_t composer__new_proof(void* pippenger,
                                       uint8_t const* g2x,
                                       uint8_t const* constraint_system_buf,
                                       uint8_t const* witness_buf,
                                       uint8_t** proof_data_buf)
{

    return rollup::proofs::standard_example::c_composer__new_proof(
        pippenger, g2x, constraint_system_buf, witness_buf, proof_data_buf);
}

// BACKWARDS COMPATIBILITY
WASM_EXPORT bool composer__verify_proof(
    void* pippenger, uint8_t const* g2x, uint8_t const* constraint_system_buf, uint8_t* proof, uint32_t length)
{
    return rollup::proofs::standard_example::c_composer__verify_proof(
        pippenger, g2x, constraint_system_buf, proof, length);
}
}
