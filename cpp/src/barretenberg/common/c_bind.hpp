#include <cstddef>
#include "./wasm_export.hpp"

extern "C" {

WASM_EXPORT void* bbmalloc(size_t size);

WASM_EXPORT void bbfree(void* ptr);
}
