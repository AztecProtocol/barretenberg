#include <vector>
#include <thread>
#include <atomic>

inline size_t get_num_threads()
{
#ifdef __wasm__
    // TODO: Call back to host to discover number of webworkers.
    return 4;
#else
    return std::thread::hardware_concurrency();
#endif
}

template <typename Func> void parallel_for(size_t num_iterations, Func func)
{
#ifndef NO_MULTITHREADING
    auto num_threads = std::min(num_iterations, get_num_threads());
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

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

    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& thread : threads) {
        thread.join();
    }
#else
    for (size_t i = 0; i < num_iterations; ++i) {
        func(i);
    }
#endif
}
