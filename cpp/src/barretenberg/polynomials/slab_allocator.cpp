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

    // These numbers are for Ultra, our most greedy system, so they should easily serve Standard/Turbo.
    // Miscellaneous slabs are just an effort to account for other slabs of memory needed throughout
    // prover computation (scratch space and other temporaries). We can't account for all of these
    // as we are at limit, so they are mostly dynamically allocated. This ultimately leads to failure
    // on repeated prover runs as the memory becomes fragmented. Maybe best to just recreate the WASM
    // for each proof for now, if not too expensive.
    prealloc_num[small_size] = 4 +    // Monomial wires.
                               4 +    // Lagrange wires.
                               15 +   // Monomial constraint selectors.
                               15 +   // Lagrange constraint selectors.
                               8 +    // Monomial perm selectors.
                               8 +    // Lagrange perm selectors.
                               1 +    // Monomial sorted poly.
                               5 +    // Lagrange sorted poly.
                               2 +    // Perm poly.
                               4 +    // Quotient poly.
                               1;     // Miscellaneous.
    prealloc_num[large_size] = 4 +    // Coset-fft wires.
                               15 +   // Coset-fft constraint selectors.
                               8 +    // Coset-fft perm selectors.
                               1 +    // Coset-fft sorted poly.
                               1;     // Miscellaneous.
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

#ifdef __wasm__
SlabAllocator allocator(524288);
#else
SlabAllocator allocator(0);
#endif

std::shared_ptr<void> get_mem_slab(size_t size)
{
    return allocator.get(size);
}
} // namespace barretenberg
