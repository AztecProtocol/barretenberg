#include <cstddef>
#include "./wasm_export.hpp"
#include "./serialize.hpp"

extern "C" {

WASM_EXPORT void* bbmalloc(size_t size);

WASM_EXPORT void bbfree(void* ptr);

WASM_EXPORT void test_threads(uint32_t const* threads, uint32_t const* iterations, uint32_t* out);
}
