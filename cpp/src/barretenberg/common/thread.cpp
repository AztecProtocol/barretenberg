#include "thread.hpp"
// #include "log.hpp"

void parallel_for(size_t num_iterations, const std::function<void(size_t)>& func)
{
#ifdef NO_MULTITHREADING
    for (size_t i = 0; i < num_iterations; ++i) {
        func(i);
    }
#else
    std::atomic<size_t> current_iteration(0);

    auto worker = [&](size_t) {
        // info("entered worker: ", thread_index);
        while (true) {
            size_t index = current_iteration.fetch_add(1, std::memory_order_seq_cst);

            if (index >= num_iterations) {
                break;
            }

            func(index);
        }
        // info("exited worker: ", thread_index);
    };

    auto num_threads = std::min(num_iterations, get_num_cpus());
    if (num_threads == 1) {
        // info("Executing on main thread as only 1 cpu or iteration. iterations: ", num_iterations);
        worker(0);
        return;
    }
    // info("Starting ", num_threads, " threads to handle ", num_iterations, " iterations.");

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }
    // info("joined!\n\n");
#endif
}

/**
 * Pure C pthreads implementation below. Worth keeping around for debugging until wasi pthreads is fully stable.
 * It can be slightly easier to debug the code when not using C++ lambdas etc. Also, it just removes the STL
 * layer from any question as to why something might not be as expected.
 */

// #include "thread.hpp"
// #include "log.hpp"
// #include <pthread.h>
// #include <atomic>
// #include <functional>
// #include <vector>

// struct Counter {
//     size_t value;
//     pthread_mutex_t mutex;

//     Counter()
//         : value(0)
//     {
//         pthread_mutex_init(&mutex, nullptr);
//     }

//     size_t fetchAdd(size_t n)
//     {
//         pthread_mutex_lock(&mutex);
//         size_t currentValue = value;
//         value += n;
//         pthread_mutex_unlock(&mutex);
//         return currentValue;
//     }
// };

// struct WorkerArgs {
//     Counter* current_iteration;
//     size_t num_iterations;
//     const std::function<void(size_t)>* func;
// };

// void* worker(void* args)
// {
//     auto* worker_args = static_cast<WorkerArgs*>(args);
//     Counter& current_iteration = *worker_args->current_iteration;
//     size_t num_iterations = worker_args->num_iterations;
//     const std::function<void(size_t)>& func = *worker_args->func;
//     // info("entered worker");

//     while (true) {
//         size_t index = current_iteration.fetchAdd(1);

//         if (index >= num_iterations) {
//             break;
//         }

//         func(index);
//     }

//     return nullptr;
// }

// void parallel_for(size_t num_iterations, const std::function<void(size_t)>& func)
// {
// #ifdef NO_MULTITHREADING
//     for (size_t i = 0; i < num_iterations; ++i) {
//         func(i);
//     }
// #else
//     Counter current_iteration;

//     auto num_threads = std::min(num_iterations, get_num_cpus());

//     if (num_threads == 1) {
//         // info("Executing on main thread as only 1 cpu or iteration. iterations: ", num_iterations);
//         WorkerArgs args{ &current_iteration, num_iterations, &func };
//         worker(&args);
//         return;
//     }
//     // info("Starting ", num_threads, " threads to handle ", num_iterations, " iterations.");

//     std::vector<pthread_t> threads;
//     threads.reserve(num_threads);

//     WorkerArgs args{ &current_iteration, num_iterations, &func };
//     for (size_t i = 0; i < num_threads; ++i) {
//         pthread_t thread;
//         pthread_create(&thread, nullptr, worker, &args);
//         threads.emplace_back(thread);
//     }

//     // info("joining...");
//     for (pthread_t& thread : threads) {
//         pthread_join(thread, nullptr);
//     }
//     // info("joined!\n\n");
// #endif
// }
