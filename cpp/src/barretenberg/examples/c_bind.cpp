#include "c_bind.hpp"
#include "simple/simple.hpp"
#include "barretenberg/srs/reference_string/pippenger_reference_string.hpp"

extern "C" {

using namespace proof_system::plonk::stdlib::types;

WASM_EXPORT void examples_simple_create_and_verify_proof(in_ptr pippenger, uint8_t const* g2x_buf, bool* valid)
{
    auto g2x = from_buffer<std::vector<uint8_t>>(g2x_buf);
    auto crs_factory = std::make_shared<proof_system::PippengerReferenceStringFactory>(
        (scalar_multiplication::Pippenger*)(*pippenger), g2x.data());

    auto* composer_ptr = examples::simple::create_composer(crs_factory);

    auto proof = examples::simple::create_proof(composer_ptr);

    *valid = examples::simple::verify_proof(composer_ptr, proof);
    examples::simple::delete_composer(composer_ptr);
}
}
