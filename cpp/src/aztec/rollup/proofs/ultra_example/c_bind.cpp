#include "c_bind.h"
#include "ultra_example.hpp"
#include <common/streams.hpp>
#include <cstdint>
#include <srs/reference_string/pippenger_reference_string.hpp>
#include <sstream>

using namespace barretenberg;

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void ultra_example__init_proving_key()
{
    auto crs_factory = std::make_unique<waffle::ReferenceStringFactory>();
    rollup::proofs::ultra_example::init_proving_key(std::move(crs_factory));
}

WASM_EXPORT void ultra_example__init_verification_key(void* pippenger_ptr, uint8_t const* g2x)
{
    auto crs_factory = std::make_unique<waffle::PippengerReferenceStringFactory>(
        reinterpret_cast<scalar_multiplication::Pippenger*>(pippenger_ptr), g2x);
    rollup::proofs::ultra_example::init_verification_key(std::move(crs_factory));
}

WASM_EXPORT void* ultra_example__new_prover()
{
    auto prover = rollup::proofs::ultra_example::new_prover();
    return new waffle::UltraProver(std::move(prover));
}

WASM_EXPORT void ultra_example__delete_prover(void* prover)
{
    delete reinterpret_cast<waffle::UltraProver*>(prover);
}

WASM_EXPORT bool ultra_example__verify_proof(uint8_t* proof, uint32_t length)
{
    waffle::plonk_proof pp = { std::vector<uint8_t>(proof, proof + length) };
    return rollup::proofs::ultra_example::verify_proof(pp);
}
}
