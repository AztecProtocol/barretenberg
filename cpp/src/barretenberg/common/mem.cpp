#include "./mem.hpp"
#include "./wasm_export.hpp"

extern "C" {

WASM_EXPORT void* bbmalloc(size_t size)
{
    return aligned_alloc(64, size);
}

WASM_EXPORT void bbfree(void* ptr)
{
    aligned_free(ptr);
}
}