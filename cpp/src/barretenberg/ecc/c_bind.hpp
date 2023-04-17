#include <cstdint>
#include <barretenberg/ecc/curves/bn254/g1.hpp>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

using namespace barretenberg;
using affine_element = g1::affine_element;

WASM_EXPORT void ecc_new_pippenger(void const* points, uint32_t const* num_points, void** out_ptr);

WASM_EXPORT void ecc_delete_pippenger(void const* pippenger);

WASM_EXPORT void ecc_pippenger_unsafe(void const* pippenger_ptr,
                                      void const* scalars_ptr,
                                      uint32_t const* from,
                                      uint32_t const* range,
                                      affine_element::out_buf result_ptr);

WASM_EXPORT void ecc_g1_sum(void const* points_ptr, uint32_t const* num_points, affine_element::out_buf result_ptr);
}
