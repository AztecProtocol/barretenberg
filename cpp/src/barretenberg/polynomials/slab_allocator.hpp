#pragma once
#include <unordered_map>
#include <map>
#include <list>
#include <mutex>
#include <memory>

namespace barretenberg {

class SlabAllocator {
  private:
    std::map<size_t, std::list<void*>> memory_store;
    std::map<size_t, size_t> prealloc_num;
    std::mutex memory_store_mutex;

  public:
    explicit SlabAllocator(size_t circuit_size_hint);

    std::shared_ptr<void> get(size_t size);

    size_t get_total_size();

  private:
    void release(void* ptr, size_t size);
};

std::shared_ptr<void> mem_slab_get(size_t size);

} // namespace barretenberg