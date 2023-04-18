#include <cstdint>
#include <barretenberg/ecc/curves/bn254/g1.hpp>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

using namespace barretenberg;
using affine_element = g1::affine_element;

WASM_EXPORT void ecc_new_pippenger(uint8_t const* points, uint32_t const* num_points, out_ptr out);

WASM_EXPORT void ecc_delete_pippenger(in_ptr pippenger);

WASM_EXPORT void ecc_pippenger_unsafe(in_ptr pippenger_ptr,
                                      in_ptr scalars_ptr,
                                      uint32_t const* from,
                                      uint32_t const* range,
                                      affine_element::out_buf result_ptr);

WASM_EXPORT void ecc_g1_sum(in_ptr points_ptr, uint32_t const* num_points, affine_element::out_buf result_ptr);
}
