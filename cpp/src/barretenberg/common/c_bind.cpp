#include "./c_bind.hpp"
#include "./mem.hpp"
#include "./timer.hpp"
#include "./serialize.hpp"
#include <algorithm>
#include <thread>

struct test_threads_data {
    std::atomic<size_t> counter = 0;
    size_t iterations = 0;
};

void thread_test_entry_point(test_threads_data* v)
{
    Timer t;
    info("thread start with counter at: ", static_cast<size_t>(v->counter));
    std::vector<uint8_t> data(1024);
    for (size_t i = 0; i < v->iterations; ++i) {
        // Do some meaningless work.
        std::for_each(data.begin(), data.end(), [](auto& i) { i = i & 0x80; });
        std::for_each(data.begin(), data.end(), [](auto& i) { i = i | 0x01; });
        std::for_each(data.begin(), data.end(), [](auto& i) { i = (uint8_t)(i << 3); });
        (v->counter)++;
    }
    info("thread end with counter at: ", static_cast<size_t>(v->counter), " ", t.seconds(), "s");
}

extern "C" {

WASM_EXPORT void test_threads(uint32_t const* thread_num, uint32_t const* iterations, uint32_t* out)
{
    info("test starting...");
    Timer t;
    size_t NUM_THREADS = ntohl(*thread_num);
    std::vector<std::thread> threads(NUM_THREADS);
    test_threads_data test_data;
    test_data.iterations = ntohl(*iterations) / NUM_THREADS;

    for (size_t i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(thread_test_entry_point, &test_data);
    }

    info("joining...");
    for (size_t i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    info("test complete with counter at: ", static_cast<size_t>(test_data.counter), " ", t.seconds(), "s");
    *out = htonl(test_data.counter);
}
}
