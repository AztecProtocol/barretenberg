#include <polynomials/polynomial.hpp>
#include <common/max_threads.hpp>

#ifndef NO_MULTITHREADING
#include "omp.h"
#endif
namespace honk {
namespace power_polynomial {
/**
 * @brief Generate the power polynomial vector
 *
 * @details Generates a vector, where v[i]=ζⁱ
 *
 * @param zeta
 * @param vector_size
 * @return barretenberg::polynomial
 */
barretenberg::polynomial generate_vector(barretenberg::fr zeta, size_t vector_size)
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
        pow_vector.coefficients[(i + 1) * thread_size - 1] = starting_power;
    }
    return pow_vector;
}

/**
 * @brief Evaluate the power polynomial on {x_0,..,x_d}
 *
 * @details The power polynomial can be efficiently evaluated as ∏( ( b^{2^i} - 1 ) * x_i + 1)
 *
 * @param zeta ζ
 * @param variables
 * @return barretenberg::fr
 */
barretenberg::fr evaluate(barretenberg::fr zeta, const std::vector<barretenberg::fr>& variables)
{
    barretenberg::fr evaluation = barretenberg::fr::one();
    for (size_t i = 0; i < variables.size(); i++) {
        // evaulutaion *= b^{2^i} - 1) * x_i + 1
        evaluation *= (zeta - 1) * variables[i] + 1;
        zeta *= zeta;
    }
    return evaluation;
}
} // namespace power_polynomial
} // namespace honk
