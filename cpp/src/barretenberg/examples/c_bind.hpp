#include <cstdint>
#include <barretenberg/common/serialize.hpp>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void examples_simple_create_and_verify_proof(in_ptr pippenger, uint8_t const* g2x, bool* valid);
}
