/**
 * Originally added for testing the asyncify functionality, but it could be useful to have direct access to the data
 * store from the host environment. The data store is usually implemented by the host environment however.
 */
#include "c_bind.hpp"
#include "data_store.hpp"
#include "crs.hpp"
#include <cstdlib>
#include <barretenberg/common/serialize.hpp>

#include <pthread.h>
// #include <stdio.h>
// #include <string.h>

void* thread_entry_point(void* ctx)
{
    auto id = (size_t)ctx;
    info(" in thread ", id);
    return nullptr;
}

extern "C" {

WASM_EXPORT void env_test_pthreads()
{
    constexpr size_t NUM_THREADS = 10;
    pthread_t threads[10];
    for (size_t i = 0; i < NUM_THREADS; i++) {
        int ret = pthread_create(&threads[i], nullptr, &thread_entry_point, (void*)i);
        if (ret != 0) {
            info("failed to spawn thread: ", ret);
        }
    }
    for (size_t thread : threads) {
        pthread_join(thread, nullptr);
    }
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
