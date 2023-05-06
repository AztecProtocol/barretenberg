#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <barretenberg/numeric/bitop/get_msb.hpp>

#ifdef __wasm__
extern "C" uint32_t env_hardware_concurrency();
#endif

inline size_t get_num_cpus()
{
#ifdef NO_MULTITHREADING
    return 1;
#else
#ifdef __wasm__
    return env_hardware_concurrency();
#else
    return std::thread::hardware_concurrency();
#endif
#endif
}

// For algorithms that need to be divided amongst power of 2 threads.
inline size_t get_num_cpus_pow2()
{
    return static_cast<size_t>(1ULL << numeric::get_msb(get_num_cpus()));
}

void parallel_for(size_t num_iterations, const std::function<void(size_t)>& func);