/**
 * Originally added for testing the asyncify functionality, but it could be useful to have direct access to the data
 * store from the host environment. The data store is usually implemented by the host environment however.
 */
#include "c_bind.hpp"
#include "data_store.hpp"
#include "crs.hpp"
#include <cstdlib>
#include <barretenberg/common/serialize.hpp>
#include <barretenberg/common/timer.hpp>
#include <barretenberg/crypto/blake2s/blake2s.hpp>
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
        blake2::blake2s(data);
        (v->counter)++;
    }
    info("thread end with counter at: ", static_cast<size_t>(v->counter), " ", t.seconds(), "s");
}

extern "C" {

WASM_EXPORT void env_test_threads(uint32_t const* thread_num, uint32_t const* iterations, uint32_t* out)
{
    Timer t;
    size_t NUM_THREADS = ntohl(*thread_num);
    std::vector<std::thread> threads(NUM_THREADS);
    test_threads_data test_data;
    test_data.iterations = ntohl(*iterations) / NUM_THREADS;

    for (size_t i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(thread_test_entry_point, &test_data);
    }

    for (size_t i = 0; i < NUM_THREADS; i++) {
        // if (threads[i].joinable()) {
        threads[i].join();
        // }
    }

    info("test complete with counter at: ", static_cast<size_t>(test_data.counter), " ", t.seconds(), "s");
    *out = htonl(test_data.counter);
}

WASM_EXPORT void env_set_data(in_str_buf key_buf, uint8_t const* buffer)
{
    auto key = from_buffer<std::string>(key_buf);
    auto buf = from_buffer<std::vector<uint8_t>>(buffer);
    set_data(key.c_str(), buf.data(), buf.size());
}

WASM_EXPORT void env_get_data(in_str_buf key_buf, uint8_t** out_ptr)
{
    auto key = from_buffer<std::string>(key_buf);
    size_t length = 0;
    // auto* ptr = get_data(key.c_str(), &length);
    *out_ptr = (uint8_t*)aligned_alloc(64, 4 + length);
    // auto vec = std::vector<uint8_t>(ptr, ptr + length);
    // free(ptr);
    // auto* dst = *out_ptr;
    // write(dst, vec);
}

/**
 * @brief Simple wrapper for env_load_verifier_crs.
 * @return The CRS.
 */
// WASM_EXPORT void* env__load_verifier_crs(uint8_t**)
// {
//     return env_load_verifier_crs();
// }

// /**
//  * @brief Simple wrapper for env_load_verifier_crs.
//  * @param The number of points to load of the prover CRS.
//  * @return The CRS.
//  */
// WASM_EXPORT void* env__load_prover_crs(size_t num_points)
// {
//     return env_load_prover_crs(num_points);
// }
}
