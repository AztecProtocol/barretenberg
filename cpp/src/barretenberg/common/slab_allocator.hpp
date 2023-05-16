#pragma once
#include <unordered_map>
#include <map>
#include <list>
#include <memory>
#ifndef NO_MULTITHREADING
#include <mutex>
#endif

namespace barretenberg {

/**
 * Allows preallocating memory slabs sized to serve the fact that these slabs of memory follow certain sizing patterns
 * and numbers based on prover system type and circuit size. Without the slab allocator, memory fragmentation prevents
 * proof construction when approaching memory space limits (4GB in WASM).
 *
 * If no circuit_size_hint is given to the constructor, it behaves as a standard memory allocator.
 */
class SlabAllocator {
  private:
    size_t circuit_size_hint_;
    std::map<size_t, std::list<void*>> memory_store;
    std::map<size_t, size_t> prealloc_num;
#ifndef NO_MULTITHREADING
    std::mutex memory_store_mutex;
#endif

  public:
    void init(size_t circuit_size_hint);

    std::shared_ptr<void> get(size_t size);

    size_t get_total_size();

  private:
    void release(void* ptr, size_t size);
};

void init_slab_allocator(size_t circuit_size);

std::shared_ptr<void> get_mem_slab(size_t size);

} // namespace barretenberg