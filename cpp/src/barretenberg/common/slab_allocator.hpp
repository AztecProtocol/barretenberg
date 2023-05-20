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

} // namespace barretenberg