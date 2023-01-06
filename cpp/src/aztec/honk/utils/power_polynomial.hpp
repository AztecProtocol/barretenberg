#include <polynomial.hpp>
#include <common/max_threads.hpp>

#ifndef NO_MULTITHREADING
#include "omp.h"
#endif
namespace honk {

barretenberg::polynomial generate_power_polynomial_vector(barretenberg::fr zeta, size_t vector_size)
{
    barretenberg::polynomial pow_vector(vector_size);
    constexpr size_t usefulness_margin = 4;
    size_t num_threads = max_threads::compute_num_threads();
    if (vector_size < (usefulness_margin * num_threads)) {
        num_threads = 1;
    }
    size_t thread_size = vector_size / num_threads;
    if ((num_threads % thread_size) != 0) {
        thread_size += 1;
    }
#ifndef NO_MULTITHREADING
#pragma omp parallel for
#endif
    for (size_t i = 0; i < num_threads; i++) {
        barretenberg::fr starting_power = zeta.pow(i * thread_size);
        for (size_t j = 0; j < (thread_size - 1); j++) {
            pow_vector.coefficients[i * thread_size + j] = starting_power;
            starting_power *= zeta;
        }
        pow_vector.coefficients[i * (thread_size + 1) - 1] = starting_power;
    }
}
} // namespace honk
