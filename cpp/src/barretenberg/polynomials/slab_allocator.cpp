#include "slab_allocator.hpp"
#include <barretenberg/common/log.hpp>
#include <barretenberg/common/mem.hpp>
#include <numeric>

namespace barretenberg {

SlabAllocator::SlabAllocator(size_t circuit_size_hint)
{
    if (circuit_size_hint == 0ULL) {
        return;
    }

    // Over-allocate because we know there are requests for circuit_size + n. (arbitrary n = 32)
    size_t small_size = 32 * (circuit_size_hint + 32);
    size_t large_size = small_size * 4;

    prealloc_num[small_size] = 4 + 4 + 15 + 15 + 8 + 8 + 1 + 5 + 2 + 4 + 1 + 0;
    prealloc_num[large_size] = 4 + 15 + 8 + 1 + 1;
    prealloc_num[large_size * 2] = 1; // Proving key evaluation domain roots.

    for (auto& e : prealloc_num) {
        for (size_t i = 0; i < e.second; ++i) {
            auto size = e.first;
            memory_store[size].push_back(aligned_alloc(32, size));
            // info("Allocated memory slab of size: ", size, " total: ", get_total_size());
        }
    }
}

std::shared_ptr<void> SlabAllocator::get(size_t req_size)
{
    std::unique_lock<std::mutex> lock(memory_store_mutex);

    auto it = memory_store.lower_bound(req_size);

    if (it != memory_store.end()) {
        size_t size = it->first;
        auto ptr = it->second.back();
        it->second.pop_back();

        // info("Reusing memory slab of size: ", size, " for requested ", req_size, " total: ", get_total_size());

        if (it->second.empty()) {
            memory_store.erase(it);
        }

        return std::shared_ptr<void>(ptr, [this, size](void* p) { this->release(p, size); });
    }

    // info("Allocating unmanaged memory slab of size: ", req_size);
    return std::shared_ptr<void>(aligned_alloc(32, req_size), aligned_free);
}

size_t SlabAllocator::get_total_size()
{
    return std::accumulate(memory_store.begin(), memory_store.end(), size_t{ 0 }, [](size_t acc, const auto& kv) {
        return acc + kv.first * kv.second.size();
    });
}

void SlabAllocator::release(void* ptr, size_t size)
{
    std::unique_lock<std::mutex> lock(memory_store_mutex);
    // info("Pooling poly memory of size: ", size, " total: ", get_total_size());
    memory_store[size].push_back(ptr);
}

SlabAllocator allocator(524288);

std::shared_ptr<void> mem_slab_get(size_t size)
{
    return allocator.get(size);
}
} // namespace barretenberg
