#include <barretenberg/common/wasm_export.hpp>
#include <barretenberg/common/serialize.hpp>

extern "C" {

WASM_EXPORT void env_test_threads(uint32_t const* threads, uint32_t const* iterations, uint32_t* out);

ASYNC_WASM_EXPORT void env_set_data(in_str_buf key_buf, uint8_t const* buffer);

ASYNC_WASM_EXPORT void env_get_data(in_str_buf key_buf, uint8_t** out_ptr);
}
