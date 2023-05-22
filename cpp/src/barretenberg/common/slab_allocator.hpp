#pragma once
#include <unordered_map>
#include <map>
#include <list>
#include <memory>
#include "./log.hpp"
#ifndef NO_MULTITHREADING
#include <mutex>
#endif

namespace barretenberg {

/**
 * Allocates a bunch of memory slabs sized to serve an UltraPLONK proof construction.
 * If you want normal memory allocator behaviour, just don't call this init function.
 *
 * WARNING: If client code is still holding onto slabs from previous use, when those slabs
 * are released they'll end up back in the allocator. That's probably not desired as presumably
 * those slabs are now too small, so they're effectively leaked. But good client code should be releasing
 * it's resources promptly anyway. It's not considered "proper use" to call init, take slab, and call init
 * again, before releasing the slab.
 *
 * TODO: Take a composer type and allocate slabs according to those requirements?
 * TODO: De-globalise. Init the allocator and pass around. Use a PolynomialFactory (PolynomialStore?).
 * TODO: Consider removing, but once due-dilligence has been done that we no longer have memory limitations.
 */
void init_slab_allocator(size_t circuit_size);

/**
 * Returns a slab from the preallocated pool of slabs, or fallback to a new heap allocation (32 byte aligned).
 */
std::shared_ptr<void> get_mem_slab(size_t size);

template <typename T> class ContainerSlabAllocator {
  public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;

    ContainerSlabAllocator() = default;
    ~ContainerSlabAllocator() = default;

    template <typename U> struct rebind {
        using other = ContainerSlabAllocator<U>;
    };

    pointer allocate(size_type n)
    {
        // info("ContainerSlabAllocator allocating: ", n * sizeof(T));
        std::shared_ptr<void> ptr = get_mem_slab(n * sizeof(T));
        slab = ptr; // Keep a copy of the shared_ptr so the memory is not freed.
        return static_cast<pointer>(ptr.get());
    }

    void deallocate(pointer /*p*/, size_type /*unused*/)
    {
        // Memory will be automatically deallocated when slab goes out of scope and its ref count becomes 0.
    }

  private:
    std::shared_ptr<void> slab;
};

} // namespace barretenberg