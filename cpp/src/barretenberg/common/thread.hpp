#pragma once
#include <vector>
#include <thread>
#include <atomic>
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

template <typename Func> void parallel_for(size_t num_iterations, Func func)
{
#ifdef NO_MULTITHREADING
    for (size_t i = 0; i < num_iterations; ++i) {
        func(i);
    }
#else
    std::atomic<size_t> current_iteration(0);

    auto worker = [&]() {
        while (true) {
            size_t index = current_iteration.fetch_add(1, std::memory_order_relaxed);

            if (index >= num_iterations) {
                break;
            }

            func(index);
        }
    };

    auto num_threads = std::min(num_iterations, get_num_cpus());

    if (num_threads == 1) {
        worker();
        return;
    }

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& thread : threads) {
        thread.join();
    }
#endif
}
