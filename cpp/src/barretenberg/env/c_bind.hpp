#include <barretenberg/common/wasm_export.hpp>
#include <barretenberg/common/serialize.hpp>

#define WASM_EXPORT __attribute__((visibility("default")))
#define ASYNC

extern "C" {

ASYNC WASM_EXPORT void env_set_data(in_str_buf key_buf, uint8_t const* buffer);

ASYNC WASM_EXPORT void env_get_data(in_str_buf key_buf, uint8_t** out_ptr);
}
