#include "slab_allocator.hpp"
#include <barretenberg/common/log.hpp>
#include <barretenberg/common/mem.hpp>
#include <numeric>

namespace barretenberg {

void SlabAllocator::init(size_t circuit_size_hint)
{
    if (circuit_size_hint <= circuit_size_hint_) {
        return;
    }

    circuit_size_hint_ = circuit_size_hint;
    memory_store.clear();
    prealloc_num.clear();

    // info("slab allocator initing for size: ", circuit_size_hint);

    if (circuit_size_hint == 0ULL) {
        return;
    }

    // Over-allocate because we know there are requests for circuit_size + n. (somewhat arbitrary n = 128)
    // Think max I saw was 65 extra related to pippenger runtime state. Likely related to the machine having 64 cores.
    // Strange things may happen here if double to 128 cores, might request 129 extra?
    size_t overalloc = 128;
    size_t small_size = 32 * (circuit_size_hint + overalloc);
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
                               7;     // Miscellaneous.
    prealloc_num[small_size * 2] = 1; // Miscellaneous.
    prealloc_num[large_size] = 4 +    // Coset-fft wires.
                               15 +   // Coset-fft constraint selectors.
                               8 +    // Coset-fft perm selectors.
                               1 +    // Coset-fft sorted poly.
                               1 +    // Pippenger point_schedule.
                               3;     // Miscellaneous.
    prealloc_num[large_size * 2] = 3; // Proving key evaluation domain roots. Pippenger point_pairs.

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
#ifndef NO_MULTITHREADING
    std::unique_lock<std::mutex> lock(memory_store_mutex);
#endif

    auto it = memory_store.lower_bound(req_size);

    // Can use a preallocated slab that is less than 2 times the requested size.
    if (it != memory_store.end() && it->first < req_size * 2) {
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
#ifndef NO_MULTITHREADING
    std::unique_lock<std::mutex> lock(memory_store_mutex);
#endif
    // info("Pooling poly memory of size: ", size, " total: ", get_total_size());
    memory_store[size].push_back(ptr);
}

SlabAllocator allocator;

void init_slab_allocator(size_t circuit_size)
{
    allocator.init(circuit_size);
}

// auto init = ([]() {
//     init_slab_allocator(524288);
//     return 0;
// })();

std::shared_ptr<void> get_mem_slab(size_t size)
{
    return allocator.get(size);
}
} // namespace barretenberg
