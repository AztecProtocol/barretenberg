#include <stdint.h>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void ecc_grumpkin__mul(uint8_t const* point_buf, uint8_t const* scalar_buf, uint8_t* result);
// multiplies a vector of points by a single scalar. Returns a vector of points (this is NOT a multi-exponentiation)
WASM_EXPORT void ecc_grumpkin__batch_mul(uint8_t const* point_buf,
                                         uint8_t const* scalar_buf,
                                         uint32_t num_points,
                                         uint8_t* result);
WASM_EXPORT void ecc_grumpkin__get_random_scalar_mod_circuit_modulus(uint8_t* result);
WASM_EXPORT void ecc_grumpkin__reduce512_buffer_mod_circuit_modulus(uint8_t* input, uint8_t* result);
}