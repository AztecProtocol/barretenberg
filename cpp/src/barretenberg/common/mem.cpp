#include "./mem.hpp"
#include "./wasm_export.hpp"
#include "./slab_allocator.hpp"

extern "C" {

WASM_EXPORT void* bbmalloc(size_t size)
{
    return barretenberg::get_mem_slab_raw(size);
}

WASM_EXPORT void bbfree(void* ptr)
{
    barretenberg::free_mem_slab_raw(ptr);
}
}