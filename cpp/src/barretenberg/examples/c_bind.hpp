#include <cstdint>
#include <barretenberg/common/serialize.hpp>
#include "barretenberg/common/wasm_export.hpp"

extern "C" {

WASM_EXPORT void examples_simple_create_and_verify_proof(in_ptr pippenger, uint8_t const* g2x, bool* valid);
}
