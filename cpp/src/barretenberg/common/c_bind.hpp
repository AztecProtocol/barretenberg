#include <cstddef>
#include "./wasm_export.hpp"
#include "./serialize.hpp"

extern "C" {

WASM_EXPORT void test_threads(uint32_t const* threads, uint32_t const* iterations, uint32_t* out);

WASM_EXPORT void test_thread_abort();

WASM_EXPORT void test_abort();
}
