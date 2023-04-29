#pragma once
#include "barretenberg/numeric/bitop/get_msb.hpp"
#include <thread>

namespace max_threads {
//
// This method will compute the number of threads which would be used
// for computation in barretenberg. We set it to the max number of threads
// possible for a system (using the openmp package). However, if any system
// has max number of threads which is NOT a power of two, we set number of threads
// to be used as the previous power of two.
inline size_t compute_num_threads()
{
#ifndef NO_MULTITHREADING
#ifdef __wasm__
    // TODO: Call back to host to discover number of webworkers.
    size_t num_threads = 4;
#else
    size_t num_threads = std::thread::hardware_concurrency();
#endif
#else
    size_t num_threads = 1;
#endif

    // ensure that num_threads is a power of two
    num_threads = static_cast<size_t>(1ULL << numeric::get_msb(num_threads));
    return num_threads;
}
} // namespace max_threads
