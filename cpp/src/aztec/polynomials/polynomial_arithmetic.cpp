#include "polynomial_arithmetic.hpp"
#include "iterate_over_domain.hpp"
#include <common/assert.hpp>
#include <common/mem.hpp>
#include <math.h>
#include <memory.h>
#include <numeric/bitop/get_msb.hpp>
#include <common/max_threads.hpp>

namespace barretenberg {
namespace polynomial_arithmetic {
namespace {
static fr* working_memory = nullptr;
static size_t current_size = 0;

// const auto init = []() {
//     constexpr size_t max_num_elements = (1 << 20);
//     working_memory = (fr*)(aligned_alloc(64, max_num_elements * 4 * sizeof(fr)));
//     memset((void*)working_memory, 1, max_num_elements * 4 * sizeof(fr));
//     current_size = (max_num_elements * 4);
//     return 1;
// }();

fr* get_scratch_space(const size_t num_elements)
{
    if (num_elements > current_size) {
        if (working_memory) {
            aligned_free(working_memory);
        }
        working_memory = (fr*)(aligned_alloc(64, num_elements * sizeof(fr)));
        current_size = num_elements;
    }
    return working_memory;
}

} // namespace
// namespace
// {
inline uint32_t reverse_bits(uint32_t x, uint32_t bit_length)
{
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return (((x >> 16) | (x << 16))) >> (32 - bit_length);
}

inline bool is_power_of_two(uint64_t x)
{
    return x && !(x & (x - 1));
}

void copy_polynomial(const fr* src, fr* dest, size_t num_src_coefficients, size_t num_target_coefficients)
{
    // TODO: fiddle around with avx asm to see if we can speed up
    memcpy((void*)dest, (void*)src, num_src_coefficients * sizeof(fr));

    if (num_target_coefficients > num_src_coefficients) {
        // fill out the polynomial coefficients with zeroes
        memset((void*)(dest + num_src_coefficients), 0, (num_target_coefficients - num_src_coefficients) * sizeof(fr));
    }
}

void fft_inner_serial(std::vector<fr*> coeffs, const size_t domain_size, const std::vector<fr*>& root_table)
{
    // Assert that the number of polynomials is a power of two.
    const size_t num_polys = coeffs.size();
    ASSERT(is_power_of_two(num_polys));
    const size_t poly_domain_size = domain_size / num_polys;
    ASSERT(is_power_of_two(poly_domain_size));

    fr temp;
    size_t log2_size = (size_t)numeric::get_msb(domain_size);
    size_t log2_poly_size = (size_t)numeric::get_msb(poly_domain_size);
    // efficiently separate odd and even indices - (An introduction to algorithms, section 30.3)

    for (size_t i = 0; i <= domain_size; ++i) {
        uint32_t swap_index = (uint32_t)reverse_bits((uint32_t)i, (uint32_t)log2_size);
        // TODO: should probably use CMOV here insead of an if statement
        if (i < swap_index) {
            size_t even_poly_idx = i >> log2_poly_size;
            size_t even_elem_idx = i % poly_domain_size;
            size_t odd_poly_idx = swap_index >> log2_poly_size;
            size_t odd_elem_idx = swap_index % poly_domain_size;
            fr::__swap(coeffs[even_poly_idx][even_elem_idx], coeffs[odd_poly_idx][odd_elem_idx]);
        }
    }

    // For butterfly operations, we use lazy reduction techniques.
    // Modulus is 254 bits, so we can 'overload' a field element by 4x and still fit it in 4 machine words.
    // We can validate that field elements are <2p and not risk overflowing. Means we can cut
    // two modular reductions from the main loop

    // perform first butterfly iteration explicitly: x0 = x0 + x1, x1 = x0 - x1
    for (size_t l = 0; l < num_polys; l++) {
        for (size_t k = 0; k < poly_domain_size; k += 2) {
            fr::__copy(coeffs[l][k + 1], temp);
            coeffs[l][k + 1] = coeffs[l][k] - coeffs[l][k + 1];
            coeffs[l][k] += temp;
        }
    }

    for (size_t m = 2; m < domain_size; m *= 2) {
        const size_t i = (size_t)numeric::get_msb(m);
        for (size_t k = 0; k < domain_size; k += (2 * m)) {
            for (size_t j = 0; j < m; ++j) {
                const size_t even_poly_idx = (k + j) >> log2_poly_size;
                const size_t even_elem_idx = (k + j) & (poly_domain_size - 1);
                const size_t odd_poly_idx = (k + j + m) >> log2_poly_size;
                const size_t odd_elem_idx = (k + j + m) & (poly_domain_size - 1);

                temp = root_table[i - 1][j] * coeffs[odd_poly_idx][odd_elem_idx];
                coeffs[odd_poly_idx][odd_elem_idx] = coeffs[even_poly_idx][even_elem_idx] - temp;
                coeffs[even_poly_idx][even_elem_idx] += temp;
            }
        }
    }
}

void scale_by_generator(fr* coeffs,
                        fr* target,
                        const evaluation_domain& domain,
                        const fr& generator_start,
                        const fr& generator_shift,
                        const size_t generator_size)
{
#ifndef NO_MULTITHREADING
#pragma omp parallel for
#endif
    for (size_t j = 0; j < domain.num_threads; ++j) {
        fr thread_shift = generator_shift.pow(static_cast<uint64_t>(j * (generator_size / domain.num_threads)));
        fr work_generator = generator_start * thread_shift;
        const size_t offset = j * (generator_size / domain.num_threads);
        const size_t end = offset + (generator_size / domain.num_threads);
        for (size_t i = offset; i < end; ++i) {
            target[i] = coeffs[i] * work_generator;
            work_generator *= generator_shift;
        }
    }
}
/**
 * Compute multiplicative subgroup (g.X)^n.
 *
 * Compute the subgroup for X in roots of unity of (2^log2_subgroup_size)*n.
 * X^n will loop through roots of unity (2^log2_subgroup_size).
 *
 * @param log2_subgroup_size Log_2 of the subgroup size.
 * @param src_domain The domain of size n.
 * @param subgroup_roots Pointer to the array for saving subgroup members.
 * */
void compute_multiplicative_subgroup(const size_t log2_subgroup_size,
                                     const evaluation_domain& src_domain,
                                     fr* subgroup_roots)
{
    size_t subgroup_size = 1UL << log2_subgroup_size;
    // Step 1: get primitive 4th root of unity
    fr subgroup_root = fr::get_root_of_unity(log2_subgroup_size);

    // Step 2: compute the cofactor term g^n
    fr accumulator = src_domain.generator;
    for (size_t i = 0; i < src_domain.log2_size; ++i) {
        accumulator.self_sqr();
    }

    // Step 3: fill array with subgroup_size values of (g.X)^n, scaled by the cofactor
    subgroup_roots[0] = accumulator;
    for (size_t i = 1; i < subgroup_size; ++i) {
        subgroup_roots[i] = subgroup_roots[i - 1] * subgroup_root;
    }
}

void fft_inner_parallel(std::vector<fr*> coeffs,
                        const evaluation_domain& domain,
                        const fr&,
                        const std::vector<fr*>& root_table)
{
    // hmm  // fr* scratch_space = (fr*)aligned_alloc(64, sizeof(fr) * domain.size);
    fr* scratch_space = get_scratch_space(domain.size);

    const size_t num_polys = coeffs.size();
    ASSERT(is_power_of_two(num_polys));
    const size_t poly_size = domain.size / num_polys;
    ASSERT(is_power_of_two(poly_size));
    const size_t poly_mask = poly_size - 1;
    const size_t log2_poly_size = (size_t)numeric::get_msb(poly_size);

#ifndef NO_MULTITHREADING
#pragma omp parallel
#endif
    {
// First FFT round is a special case - no need to multiply by root table, because all entries are 1.
// We also combine the bit reversal step into the first round, to avoid a redundant round of copying data
#ifndef NO_MULTITHREADING
#pragma omp for
#endif
        for (size_t j = 0; j < domain.num_threads; ++j) {
            fr temp_1;
            fr temp_2;
            for (size_t i = (j * domain.thread_size); i < ((j + 1) * domain.thread_size); i += 2) {
                uint32_t next_index_1 = (uint32_t)reverse_bits((uint32_t)i + 2, (uint32_t)domain.log2_size);
                uint32_t next_index_2 = (uint32_t)reverse_bits((uint32_t)i + 3, (uint32_t)domain.log2_size);
                __builtin_prefetch(&coeffs[next_index_1]);
                __builtin_prefetch(&coeffs[next_index_2]);

                uint32_t swap_index_1 = (uint32_t)reverse_bits((uint32_t)i, (uint32_t)domain.log2_size);
                uint32_t swap_index_2 = (uint32_t)reverse_bits((uint32_t)i + 1, (uint32_t)domain.log2_size);

                size_t poly_idx_1 = swap_index_1 >> log2_poly_size;
                size_t elem_idx_1 = swap_index_1 & poly_mask;
                size_t poly_idx_2 = swap_index_2 >> log2_poly_size;
                size_t elem_idx_2 = swap_index_2 & poly_mask;

                fr::__copy(coeffs[poly_idx_1][elem_idx_1], temp_1);
                fr::__copy(coeffs[poly_idx_2][elem_idx_2], temp_2);
                scratch_space[i + 1] = temp_1 - temp_2;
                scratch_space[i] = temp_1 + temp_2;
            }
        }

        // hard code exception for when the domain size is tiny - we won't execute the next loop, so need to manually
        // reduce + copy
        if (domain.size <= 2) {
            coeffs[0][0] = scratch_space[0];
            coeffs[0][1] = scratch_space[1];
        }

        // outer FFT loop
        for (size_t m = 2; m < (domain.size); m <<= 1) {
#ifndef NO_MULTITHREADING
#pragma omp for
#endif
            for (size_t j = 0; j < domain.num_threads; ++j) {
                fr temp;

                // Ok! So, what's going on here? This is the inner loop of the FFT algorithm, and we want to break it
                // out into multiple independent threads. For `num_threads`, each thread will evaluation `domain.size /
                // num_threads` of the polynomial. The actual iteration length will be half of this, because we leverage
                // the fact that \omega^{n/2} = -\omega (where \omega is a root of unity)

                // Here, `start` and `end` are used as our iterator limits, so that we can use our iterator `i` to
                // directly access the roots of unity lookup table
                const size_t start = j * (domain.thread_size >> 1);
                const size_t end = (j + 1) * (domain.thread_size >> 1);

                // For all but the last round of our FFT, the roots of unity that we need, will be a subset of our
                // lookup table. e.g. for a size 2^n FFT, the 2^n'th roots create a multiplicative subgroup of order 2^n
                //      the 1st round will use the roots from the multiplicative subgroup of order 2 : the 2'th roots of
                //      unity the 2nd round will use the roots from the multiplicative subgroup of order 4 : the 4'th
                //      roots of unity
                // i.e. each successive FFT round will double the set of roots that we need to index.
                // We have already laid out the `root_table` container so that each FFT round's roots are linearly
                // ordered in memory. For all FFT rounds, the number of elements we're iterating over is greater than
                // the size of our lookup table. We need to access this table in a cyclical fasion - i.e. for a subgroup
                // of size x, the first x iterations will index the subgroup elements in order, then for the next x
                // iterations, we loop back to the start.

                // We could implement the algorithm by having 2 nested loops (where the inner loop iterates over the
                // root table), but we want to flatten this out - as for the first few rounds, the inner loop will be
                // tiny and we'll have quite a bit of unneccesary branch checks For each iteration of our flattened
                // loop, indexed by `i`, the element of the root table we need to access will be `i % (current round
                // subgroup size)` Given that each round subgroup size is `m`, which is a power of 2, we can index the
                // root table with a very cheap `i & (m - 1)` Which is why we have this odd `block_mask` variable
                const size_t block_mask = m - 1;

                // The next problem to tackle, is we now need to efficiently index the polynomial element in
                // `scratch_space` in our flattened loop If we used nested loops, the outer loop (e.g. `y`) iterates
                // from 0 to 'domain size', in steps of 2 * m, with the inner loop (e.g. `z`) iterating from 0 to m. We
                // have our inner loop indexer with `i & (m - 1)`. We need to add to this our outer loop indexer, which
                // is equivalent to taking our indexer `i`, masking out the bits used in the 'inner loop', and doubling
                // the result. i.e. polynomial indexer = (i & (m - 1)) + ((i & ~(m - 1)) >> 1) To simplify this, we
                // cache index_mask = ~block_mask, meaning that our indexer is just `((i & index_mask) << 1 + (i &
                // block_mask)`
                const size_t index_mask = ~block_mask;

                // `round_roots` fetches the pointer to this round's lookup table. We use `numeric::get_msb(m) - 1` as
                // our indexer, because we don't store the precomputed root values for the 1st round (because they're
                // all 1).
                const fr* round_roots = root_table[static_cast<size_t>(numeric::get_msb(m)) - 1];

                // Finally, we want to treat the final round differently from the others,
                // so that we can reduce out of our 'coarse' reduction and store the output in `coeffs` instead of
                // `scratch_space`
                if (m != (domain.size >> 1)) {
                    for (size_t i = start; i < end; ++i) {
                        size_t k1 = (i & index_mask) << 1;
                        size_t j1 = i & block_mask;
                        temp = round_roots[j1] * scratch_space[k1 + j1 + m];
                        scratch_space[k1 + j1 + m] = scratch_space[k1 + j1] - temp;
                        scratch_space[k1 + j1] += temp;
                    }
                } else {
                    for (size_t i = start; i < end; ++i) {
                        size_t k1 = (i & index_mask) << 1;
                        size_t j1 = i & block_mask;

                        size_t poly_idx_1 = (k1 + j1) >> log2_poly_size;
                        size_t elem_idx_1 = (k1 + j1) & poly_mask;
                        size_t poly_idx_2 = (k1 + j1 + m) >> log2_poly_size;
                        size_t elem_idx_2 = (k1 + j1 + m) & poly_mask;

                        temp = round_roots[j1] * scratch_space[k1 + j1 + m];
                        coeffs[poly_idx_2][elem_idx_2] = scratch_space[k1 + j1] - temp;
                        coeffs[poly_idx_1][elem_idx_1] = scratch_space[k1 + j1] + temp;
                    }
                }
            }
        }
    }
}

void fft_inner_parallel(
    fr* coeffs, fr* target, const evaluation_domain& domain, const fr&, const std::vector<fr*>& root_table)
{
    // hmm  // fr* scratch_space = (fr*)aligned_alloc(64, sizeof(fr) * domain.size);
#ifndef NO_MULTITHREADING
#pragma omp parallel
#endif
    {
// First FFT round is a special case - no need to multiply by root table, because all entries are 1.
// We also combine the bit reversal step into the first round, to avoid a redundant round of copying data
#ifndef NO_MULTITHREADING
#pragma omp for
#endif
        for (size_t j = 0; j < domain.num_threads; ++j) {
            fr temp_1;
            fr temp_2;
            for (size_t i = (j * domain.thread_size); i < ((j + 1) * domain.thread_size); i += 2) {
                uint32_t next_index_1 = (uint32_t)reverse_bits((uint32_t)i + 2, (uint32_t)domain.log2_size);
                uint32_t next_index_2 = (uint32_t)reverse_bits((uint32_t)i + 3, (uint32_t)domain.log2_size);
                __builtin_prefetch(&coeffs[next_index_1]);
                __builtin_prefetch(&coeffs[next_index_2]);

                uint32_t swap_index_1 = (uint32_t)reverse_bits((uint32_t)i, (uint32_t)domain.log2_size);
                uint32_t swap_index_2 = (uint32_t)reverse_bits((uint32_t)i + 1, (uint32_t)domain.log2_size);

                fr::__copy(coeffs[swap_index_1], temp_1);
                fr::__copy(coeffs[swap_index_2], temp_2);
                target[i + 1] = temp_1 - temp_2;
                target[i] = temp_1 + temp_2;
            }
        }

        // hard code exception for when the domain size is tiny - we won't execute the next loop, so need to manually
        // reduce + copy
        if (domain.size <= 2) {
            coeffs[0] = target[0];
            coeffs[1] = target[1];
        }

        // outer FFT loop
        for (size_t m = 2; m < (domain.size); m <<= 1) {
#ifndef NO_MULTITHREADING
#pragma omp for
#endif
            for (size_t j = 0; j < domain.num_threads; ++j) {
                fr temp;

                // Ok! So, what's going on here? This is the inner loop of the FFT algorithm, and we want to break it
                // out into multiple independent threads. For `num_threads`, each thread will evaluation `domain.size /
                // num_threads` of the polynomial. The actual iteration length will be half of this, because we leverage
                // the fact that \omega^{n/2} = -\omega (where \omega is a root of unity)

                // Here, `start` and `end` are used as our iterator limits, so that we can use our iterator `i` to
                // directly access the roots of unity lookup table
                const size_t start = j * (domain.thread_size >> 1);
                const size_t end = (j + 1) * (domain.thread_size >> 1);

                // For all but the last round of our FFT, the roots of unity that we need, will be a subset of our
                // lookup table. e.g. for a size 2^n FFT, the 2^n'th roots create a multiplicative subgroup of order 2^n
                //      the 1st round will use the roots from the multiplicative subgroup of order 2 : the 2'th roots of
                //      unity the 2nd round will use the roots from the multiplicative subgroup of order 4 : the 4'th
                //      roots of unity
                // i.e. each successive FFT round will double the set of roots that we need to index.
                // We have already laid out the `root_table` container so that each FFT round's roots are linearly
                // ordered in memory. For all FFT rounds, the number of elements we're iterating over is greater than
                // the size of our lookup table. We need to access this table in a cyclical fasion - i.e. for a subgroup
                // of size x, the first x iterations will index the subgroup elements in order, then for the next x
                // iterations, we loop back to the start.

                // We could implement the algorithm by having 2 nested loops (where the inner loop iterates over the
                // root table), but we want to flatten this out - as for the first few rounds, the inner loop will be
                // tiny and we'll have quite a bit of unneccesary branch checks For each iteration of our flattened
                // loop, indexed by `i`, the element of the root table we need to access will be `i % (current round
                // subgroup size)` Given that each round subgroup size is `m`, which is a power of 2, we can index the
                // root table with a very cheap `i & (m - 1)` Which is why we have this odd `block_mask` variable
                const size_t block_mask = m - 1;

                // The next problem to tackle, is we now need to efficiently index the polynomial element in
                // `scratch_space` in our flattened loop If we used nested loops, the outer loop (e.g. `y`) iterates
                // from 0 to 'domain size', in steps of 2 * m, with the inner loop (e.g. `z`) iterating from 0 to m. We
                // have our inner loop indexer with `i & (m - 1)`. We need to add to this our outer loop indexer, which
                // is equivalent to taking our indexer `i`, masking out the bits used in the 'inner loop', and doubling
                // the result. i.e. polynomial indexer = (i & (m - 1)) + ((i & ~(m - 1)) >> 1) To simplify this, we
                // cache index_mask = ~block_mask, meaning that our indexer is just `((i & index_mask) << 1 + (i &
                // block_mask)`
                const size_t index_mask = ~block_mask;

                // `round_roots` fetches the pointer to this round's lookup table. We use `numeric::get_msb(m) - 1` as
                // our indexer, because we don't store the precomputed root values for the 1st round (because they're
                // all 1).
                const fr* round_roots = root_table[static_cast<size_t>(numeric::get_msb(m)) - 1];

                // Finally, we want to treat the final round differently from the others,
                // so that we can reduce out of our 'coarse' reduction and store the output in `coeffs` instead of
                // `scratch_space`
                if (m != (domain.size >> 1)) {
                    for (size_t i = start; i < end; ++i) {
                        size_t k1 = (i & index_mask) << 1;
                        size_t j1 = i & block_mask;
                        temp = round_roots[j1] * target[k1 + j1 + m];
                        target[k1 + j1 + m] = target[k1 + j1] - temp;
                        target[k1 + j1] += temp;
                    }
                } else {
                    for (size_t i = start; i < end; ++i) {
                        size_t k1 = (i & index_mask) << 1;
                        size_t j1 = i & block_mask;
                        temp = round_roots[j1] * target[k1 + j1 + m];
                        target[k1 + j1 + m] = target[k1 + j1] - temp;
                        target[k1 + j1] = target[k1 + j1] + temp;
                    }
                }
            }
        }
    }
}

void fft(fr* coeffs, const evaluation_domain& domain)
{
    fft_inner_parallel({ coeffs }, domain, domain.root, domain.get_round_roots());
}

void fft(fr* coeffs, fr* target, const evaluation_domain& domain)
{
    fft_inner_parallel(coeffs, target, domain, domain.root, domain.get_round_roots());
}

void fft(std::vector<fr*> coeffs, const evaluation_domain& domain)
{
    fft_inner_parallel(coeffs, domain.size, domain.root, domain.get_round_roots());
}

void ifft(fr* coeffs, const evaluation_domain& domain)
{
    fft_inner_parallel({ coeffs }, domain, domain.root_inverse, domain.get_inverse_round_roots());
    ITERATE_OVER_DOMAIN_START(domain);
    coeffs[i] *= domain.domain_inverse;
    ITERATE_OVER_DOMAIN_END;
}

void ifft(fr* coeffs, fr* target, const evaluation_domain& domain)
{
    fft_inner_parallel(coeffs, target, domain, domain.root_inverse, domain.get_inverse_round_roots());
    ITERATE_OVER_DOMAIN_START(domain);
    target[i] *= domain.domain_inverse;
    ITERATE_OVER_DOMAIN_END;
}

void ifft(std::vector<fr*> coeffs, const evaluation_domain& domain)
{
    fft_inner_parallel(coeffs, domain, domain.root_inverse, domain.get_inverse_round_roots());

    const size_t num_polys = coeffs.size();
    ASSERT(is_power_of_two(num_polys));
    const size_t poly_size = domain.size / num_polys;
    ASSERT(is_power_of_two(poly_size));
    const size_t poly_mask = poly_size - 1;
    const size_t log2_poly_size = (size_t)numeric::get_msb(poly_size);

    ITERATE_OVER_DOMAIN_START(domain);
    coeffs[i >> log2_poly_size][i & poly_mask] *= domain.domain_inverse;
    ITERATE_OVER_DOMAIN_END;
}

void fft_with_constant(fr* coeffs, const evaluation_domain& domain, const fr& value)
{
    fft_inner_parallel({ coeffs }, domain, domain.root, domain.get_round_roots());
    ITERATE_OVER_DOMAIN_START(domain);
    coeffs[i] *= value;
    ITERATE_OVER_DOMAIN_END;
}

void coset_fft(fr* coeffs, const evaluation_domain& domain)
{
    scale_by_generator(coeffs, coeffs, domain, fr::one(), domain.generator, domain.generator_size);
    fft(coeffs, domain);
}

void coset_fft(fr* coeffs, fr* target, const evaluation_domain& domain)
{
    scale_by_generator(coeffs, target, domain, fr::one(), domain.generator, domain.generator_size);
    fft(coeffs, target, domain);
}

void coset_fft(std::vector<fr*> coeffs, const evaluation_domain& domain)
{
    const size_t num_polys = coeffs.size();
    ASSERT(is_power_of_two(num_polys));
    const size_t poly_size = domain.size / num_polys;
    const fr generator_pow_n = domain.generator.pow(poly_size);
    fr generator_start = 1;

    for (size_t i = 0; i < num_polys; i++) {
        scale_by_generator(coeffs[i], coeffs[i], domain, generator_start, domain.generator, poly_size);
        generator_start *= generator_pow_n;
    }
    fft(coeffs, domain);
}

void coset_fft(fr* coeffs, const evaluation_domain& domain, const evaluation_domain&, const size_t domain_extension)
{
    const size_t log2_domain_extension = static_cast<size_t>(numeric::get_msb(domain_extension));
    fr primitive_root = fr::get_root_of_unity(domain.log2_size + log2_domain_extension);

    // fr work_root = domain.generator.sqr();
    // work_root = domain.generator.sqr();
    fr* scratch_space = get_scratch_space(domain.size * domain_extension);

    // fr* temp_memory = static_cast<fr*>(aligned_alloc(64, sizeof(fr) * domain.size *
    // domain_extension));

    std::vector<fr> coset_generators(domain_extension);
    coset_generators[0] = domain.generator;
    for (size_t i = 1; i < domain_extension; ++i) {
        coset_generators[i] = coset_generators[i - 1] * primitive_root;
    }
    for (size_t i = domain_extension - 1; i < domain_extension; --i) {
        scale_by_generator(coeffs, coeffs + (i * domain.size), domain, fr::one(), coset_generators[i], domain.size);
    }

    for (size_t i = 0; i < domain_extension; ++i) {
        fft_inner_parallel(coeffs + (i * domain.size),
                           scratch_space + (i * domain.size),
                           domain,
                           domain.root,
                           domain.get_round_roots());
    }

    if (domain_extension == 4) {
#ifndef NO_MULTITHREADING
#pragma omp parallel for
#endif
        for (size_t j = 0; j < domain.num_threads; ++j) {
            const size_t start = j * domain.thread_size;
            const size_t end = (j + 1) * domain.thread_size;
            for (size_t i = start; i < end; ++i) {
                fr::__copy(scratch_space[i], coeffs[(i << 2UL)]);
                fr::__copy(scratch_space[i + (1UL << domain.log2_size)], coeffs[(i << 2UL) + 1UL]);
                fr::__copy(scratch_space[i + (2UL << domain.log2_size)], coeffs[(i << 2UL) + 2UL]);
                fr::__copy(scratch_space[i + (3UL << domain.log2_size)], coeffs[(i << 2UL) + 3UL]);
            }
        }
        for (size_t i = 0; i < domain.size; ++i) {
            for (size_t j = 0; j < domain_extension; ++j) {
                fr::__copy(scratch_space[i + (j << domain.log2_size)], coeffs[(i << log2_domain_extension) + j]);
            }
        }
    } else {
        for (size_t i = 0; i < domain.size; ++i) {
            for (size_t j = 0; j < domain_extension; ++j) {
                fr::__copy(scratch_space[i + (j << domain.log2_size)], coeffs[(i << log2_domain_extension) + j]);
            }
        }
    }
}

void coset_fft_with_constant(fr* coeffs, const evaluation_domain& domain, const fr& constant)
{
    fr start = constant;
    scale_by_generator(coeffs, coeffs, domain, start, domain.generator, domain.generator_size);
    fft(coeffs, domain);
}

void coset_fft_with_generator_shift(fr* coeffs, const evaluation_domain& domain, const fr& constant)
{
    scale_by_generator(coeffs, coeffs, domain, fr::one(), domain.generator * constant, domain.generator_size);
    fft(coeffs, domain);
}

void ifft_with_constant(fr* coeffs, const evaluation_domain& domain, const fr& value)
{
    fft_inner_parallel({ coeffs }, domain, domain.root_inverse, domain.get_inverse_round_roots());
    fr T0 = domain.domain_inverse * value;
    ITERATE_OVER_DOMAIN_START(domain);
    coeffs[i] *= T0;
    ITERATE_OVER_DOMAIN_END;
}

void coset_ifft(fr* coeffs, const evaluation_domain& domain)
{
    ifft(coeffs, domain);
    scale_by_generator(coeffs, coeffs, domain, fr::one(), domain.generator_inverse, domain.size);
}

void coset_ifft(std::vector<fr*> coeffs, const evaluation_domain& domain)
{
    ifft(coeffs, domain);

    const size_t num_polys = coeffs.size();
    ASSERT(is_power_of_two(num_polys));
    const size_t poly_size = domain.size / num_polys;
    const fr generator_inv_pow_n = domain.generator_inverse.pow(poly_size);
    fr generator_start = 1;

    for (size_t i = 0; i < num_polys; i++) {
        scale_by_generator(coeffs[i], coeffs[i], domain, generator_start, domain.generator_inverse, poly_size);
        generator_start *= generator_inv_pow_n;
    }
}

void add(const fr* a_coeffs, const fr* b_coeffs, fr* r_coeffs, const evaluation_domain& domain)
{
    ITERATE_OVER_DOMAIN_START(domain);
    r_coeffs[i] = a_coeffs[i] + b_coeffs[i];
    ITERATE_OVER_DOMAIN_END;
}

void sub(const fr* a_coeffs, const fr* b_coeffs, fr* r_coeffs, const evaluation_domain& domain)
{
    ITERATE_OVER_DOMAIN_START(domain);
    r_coeffs[i] = a_coeffs[i] - b_coeffs[i];
    ITERATE_OVER_DOMAIN_END;
}

void mul(const fr* a_coeffs, const fr* b_coeffs, fr* r_coeffs, const evaluation_domain& domain)
{
    ITERATE_OVER_DOMAIN_START(domain);
    r_coeffs[i] = a_coeffs[i] * b_coeffs[i];
    ITERATE_OVER_DOMAIN_END;
}

fr evaluate(const fr* coeffs, const fr& z, const size_t n)
{
#ifndef NO_MULTITHREADING
    size_t num_threads = max_threads::compute_num_threads();
#else
    size_t num_threads = 1;
#endif
    size_t range_per_thread = n / num_threads;
    size_t leftovers = n - (range_per_thread * num_threads);
    fr* evaluations = new fr[num_threads];
#ifndef NO_MULTITHREADING
#pragma omp parallel for
#endif
    for (size_t j = 0; j < num_threads; ++j) {
        fr z_acc = z.pow(static_cast<uint64_t>(j * range_per_thread));
        size_t offset = j * range_per_thread;
        evaluations[j] = fr::zero();
        size_t end = (j == num_threads - 1) ? offset + range_per_thread + leftovers : offset + range_per_thread;
        for (size_t i = offset; i < end; ++i) {
            fr work_var = z_acc * coeffs[i];
            evaluations[j] += work_var;
            z_acc *= z;
        }
    }

    fr r = fr::zero();
    for (size_t j = 0; j < num_threads; ++j) {
        r += evaluations[j];
    }
    delete[] evaluations;
    return r;
}

fr evaluate(const std::vector<fr*> coeffs, const fr& z, const size_t large_n)
{
    const size_t num_polys = coeffs.size();
    const size_t poly_size = large_n / num_polys;
    ASSERT(is_power_of_two(poly_size));
    const size_t log2_poly_size = (size_t)numeric::get_msb(poly_size);
#ifndef NO_MULTITHREADING
    size_t num_threads = max_threads::compute_num_threads();
#else
    size_t num_threads = 1;
#endif
    size_t range_per_thread = large_n / num_threads;
    size_t leftovers = large_n - (range_per_thread * num_threads);
    fr* evaluations = new fr[num_threads];
#ifndef NO_MULTITHREADING
#pragma omp parallel for
#endif
    for (size_t j = 0; j < num_threads; ++j) {
        fr z_acc = z.pow(static_cast<uint64_t>(j * range_per_thread));
        size_t offset = j * range_per_thread;
        evaluations[j] = fr::zero();
        size_t end = (j == num_threads - 1) ? offset + range_per_thread + leftovers : offset + range_per_thread;
        for (size_t i = offset; i < end; ++i) {
            fr work_var = z_acc * coeffs[i >> log2_poly_size][i & (poly_size - 1)];
            evaluations[j] += work_var;
            z_acc *= z;
        }
    }

    fr r = fr::zero();
    for (size_t j = 0; j < num_threads; ++j) {
        r += evaluations[j];
    }
    delete[] evaluations;
    return r;
}

// For L_1(X) = (X^{n} - 1 / (X - 1)) * (1 / n)
// Compute the 2n-fft of L_1(X)
// We can use this to compute the 2n-fft evaluations of any L_i(X).
// We can consider `l_1_coefficients` to be a 2n-sized vector of the evaluations of L_1(X),
// for all X = 2n'th roots of unity.
// To compute the vector for the 2n-fft transform of L_i(X), we perform a (2i)-left-shift of this vector
void compute_lagrange_polynomial_fft(fr* l_1_coefficients,
                                     const evaluation_domain& src_domain,
                                     const evaluation_domain& target_domain)
{
    // L_1(X) = (X^{n} - 1 / (X - 1)) * (1 / n)
    // when evaluated at the 2n'th roots of unity, the term X^{n} forms a subgroup of order 2
    // w = n'th root of unity
    // w' = 2n'th root of unity = w^{1/2}
    // for even powers of w', X^{n} = w^{2in/2} = 1
    // for odd powers of w', X = w^{i}w^{n/2} -> X^{n} = w^{in}w^{n/2} = -1

    // We also want to compute fft using subgroup union a coset (the multiplicative generator g), so we're not dividing
    // by zero

    // Step 1: compute the denominator for each evaluation: 1 / (X.g - 1)
    // fr work_root;
    fr multiplicand = target_domain.root;

#ifndef NO_MULTITHREADING
#pragma omp parallel for
#endif
    for (size_t j = 0; j < target_domain.num_threads; ++j) {
        const fr root_shift = multiplicand.pow(static_cast<uint64_t>(j * target_domain.thread_size));
        fr work_root = src_domain.generator * root_shift;
        size_t offset = j * target_domain.thread_size;
        for (size_t i = offset; i < offset + target_domain.thread_size; ++i) {
            l_1_coefficients[i] = work_root - fr::one();
            work_root *= multiplicand;
        }
    }

    // use Montgomery's trick to invert all of these at once
    fr::batch_invert(l_1_coefficients, target_domain.size);

    // next: compute numerator multiplicand: w'^{n}.g^n
    // Here, w' is the primitive 2n'th root of unity
    // and w is the primitive n'th root of unity
    // i.e. w' = w^{1/2}
    // The polynomial X^n, when evaluated at all 2n'th roots of unity, forms a subgroup of order 2.
    // For even powers of w', X^n = w'^{2in} = w^{in} = 1
    // For odd powers of w', X^n = w'^{1 + 2in} = w^{n/2}w^{in} = w^{n/2} = -1

    // The numerator term, therefore, can only take two values
    // For even indices: (X^{n} - 1)/n = (g^n - 1)/n
    // For odd indices: (X^{n} - 1)/n = (-g^n - 1)/n

    size_t log2_subgroup_size = target_domain.log2_size - src_domain.log2_size;
    size_t subgroup_size = 1UL << log2_subgroup_size;
    ASSERT(target_domain.log2_size >= src_domain.log2_size);

    fr* subgroup_roots = new fr[subgroup_size];
    compute_multiplicative_subgroup(log2_subgroup_size, src_domain, &subgroup_roots[0]);

    // Each element of `subgroup_roots[i]` contains some root wi^n
    // want to compute (1/n)(wi^n - 1)
    for (size_t i = 0; i < subgroup_size; ++i) {
        subgroup_roots[i] -= fr::one();
        subgroup_roots[i] *= src_domain.domain_inverse;
    }
    // TODO: this is disgusting! Fix it fix it fix it fix it...
    if (subgroup_size >= target_domain.thread_size) {
        for (size_t i = 0; i < target_domain.size; i += subgroup_size) {
            for (size_t j = 0; j < subgroup_size; ++j) {
                l_1_coefficients[i + j] *= subgroup_roots[j];
            }
        }
    } else {
#ifndef NO_MULTITHREADING
#pragma omp parallel for
#endif
        for (size_t k = 0; k < target_domain.num_threads; ++k) {
            size_t offset = k * target_domain.thread_size;
            for (size_t i = offset; i < offset + target_domain.thread_size; i += subgroup_size) {
                for (size_t j = 0; j < subgroup_size; ++j) {
                    l_1_coefficients[i + j] *= subgroup_roots[j];
                }
            }
        }
    }
    delete[] subgroup_roots;
}

void divide_by_pseudo_vanishing_polynomial(std::vector<fr*> coeffs,
                                           const evaluation_domain& src_domain,
                                           const evaluation_domain& target_domain,
                                           const size_t num_roots_cut_out_of_vanishing_polynomial)
{
    // Older version:
    // the PLONK divisor polynomial is equal to the vanishing polynomial divided by the vanishing polynomial for the
    // last subgroup element Z_H(X) = \prod_{i=1}^{n-1}(X - w^i) = (X^n - 1) / (X - w^{n-1}) i.e. we divide by vanishing
    // polynomial, then multiply by degree-1 polynomial (X - w^{n-1})

    // Updated version:
    // We wish to implement this function such that it supports a modified vanishing polynomial, in which
    // k (= num_roots_cut_out_of_vanishing_polynomial) roots are cut out. i.e.
    //                           (X^n - 1)
    // Z*_H(X) = ------------------------------------------
    //           (X - w^{n-1}).(X - w^{n-2})...(X - w^{k})
    //
    // We set the default value of k as 4 so as to ensure that the evaluation domain is 4n. The reason for cutting out
    // some roots is described here: https://hackmd.io/@zacwilliamson/r1dm8Rj7D#The-problem-with-this-approach.
    // Briefly, the reason we need to cut roots is because on adding randomness to permutation polynomial z(X),
    // its degree becomes (n + 2), so for fft evaluation, we will need an evaluation domain of size >= 4(n + 2) = 8n
    // since size of evalutation domain needs to be a power of two. To avoid this, we need to bring down the degree
    // of the permutation polynomial (after adding randomness) to <= n.
    //
    //
    // NOTE: If in future, there arises a need to cut off more zeros, this method will not require any changes.
    //

    // Assert that the number of polynomials in coeffs is a power of 2.
    const size_t num_polys = coeffs.size();
    ASSERT(is_power_of_two(num_polys));
    const size_t poly_size = target_domain.size / num_polys;
    ASSERT(is_power_of_two(poly_size));
    const size_t poly_mask = poly_size - 1;
    const size_t log2_poly_size = (size_t)numeric::get_msb(poly_size);

    // `fft_point_evaluations` should be in point-evaluation form, evaluated at the 4n'th roots of unity mulitplied by
    // `target_domain`'s coset generator P(X) = X^n - 1 will form a subgroup of order 4 when evaluated at these points
    // If X = w^i, P(X) = 1
    // If X = w^{i + j/4}, P(X) = w^{n/4} = w^{n/2}^{n/2} = sqrt(-1)
    // If X = w^{i + j/2}, P(X) = -1
    // If X = w^{i + j/2 + k/4}, P(X) = w^{n/4}.-1 = -w^{i} = -sqrt(-1)
    // i.e. the 4th roots of unity
    size_t log2_subgroup_size = target_domain.log2_size - src_domain.log2_size;
    size_t subgroup_size = 1UL << log2_subgroup_size;
    ASSERT(target_domain.log2_size >= src_domain.log2_size);

    fr* subgroup_roots = new fr[subgroup_size];
    compute_multiplicative_subgroup(log2_subgroup_size, src_domain, &subgroup_roots[0]);

    // Step 3: fill array with values of (g.X)^n - 1, scaled by the cofactor
    for (size_t i = 0; i < subgroup_size; ++i) {
        subgroup_roots[i] -= fr::one();
    }

    // Step 4: invert array entries to compute denominator term of 1/Z_H*(X)
    fr::batch_invert(&subgroup_roots[0], subgroup_size);

    // The numerator term of Z_H*(X) is the polynomial (X - w^{n-1})(X - w^{n-2})...(X - w^{n-k})
    // => (g.w_i - w^{n-1})(g.w_i - w^{n-2})...(g.w_i - w^{n-k})
    // Compute w^{n-1}
    std::vector<fr> numerator_constants(num_roots_cut_out_of_vanishing_polynomial);
    if (num_roots_cut_out_of_vanishing_polynomial > 0) {
        numerator_constants[0] = -src_domain.root_inverse;
        for (size_t i = 1; i < num_roots_cut_out_of_vanishing_polynomial; ++i) {
            numerator_constants[i] = numerator_constants[i - 1] * src_domain.root_inverse;
        }
    }
    // Compute first value of g.w_i

    // Step 5: iterate over point evaluations, scaling each one by the inverse of the vanishing polynomial
    if (subgroup_size >= target_domain.thread_size) {
        fr work_root = src_domain.generator;
        for (size_t i = 0; i < target_domain.size; i += subgroup_size) {
            for (size_t j = 0; j < subgroup_size; ++j) {
                size_t poly_idx = (i + j) >> log2_poly_size;
                size_t elem_idx = (i + j) & poly_mask;
                coeffs[poly_idx][elem_idx] *= subgroup_roots[j];

                for (size_t k = 0; k < num_roots_cut_out_of_vanishing_polynomial; ++k) {
                    coeffs[poly_idx][elem_idx] *= work_root + numerator_constants[k];
                }
                work_root *= target_domain.root;
            }
        }
    } else {
#ifndef NO_MULTITHREADING
#pragma omp parallel for
#endif
        for (size_t k = 0; k < target_domain.num_threads; ++k) {
            size_t offset = k * target_domain.thread_size;
            const fr root_shift = target_domain.root.pow(static_cast<uint64_t>(offset));
            fr work_root = src_domain.generator * root_shift;
            for (size_t i = offset; i < offset + target_domain.thread_size; i += subgroup_size) {
                for (size_t j = 0; j < subgroup_size; ++j) {
                    size_t poly_idx = (i + j) >> log2_poly_size;
                    size_t elem_idx = (i + j) & poly_mask;
                    coeffs[poly_idx][elem_idx] *= subgroup_roots[j];

                    for (size_t k = 0; k < num_roots_cut_out_of_vanishing_polynomial; ++k) {
                        coeffs[poly_idx][elem_idx] *= work_root + numerator_constants[k];
                    }

                    work_root *= target_domain.root;
                }
            }
        }
    }
    delete[] subgroup_roots;
}

fr compute_kate_opening_coefficients(const fr* src, fr* dest, const fr& z, const size_t n)
{
    // if `coeffs` represents F(X), we want to compute W(X)
    // where W(X) = F(X) - F(z) / (X - z)
    // i.e. divide by the degree-1 polynomial [-z, 1]

    // We assume that the commitment is well-formed and that there is no remainder term.
    // Under these conditions we can perform this polynomial division in linear time with good constants
    fr f = evaluate(src, z, n);

    // compute (1 / -z)
    fr divisor = -z.invert();

    // we're about to shove these coefficients into a pippenger multi-exponentiation routine, where we need
    // to convert out of montgomery form. So, we can use lazy reduction techniques here without triggering overflows
    dest[0] = src[0] - f;
    dest[0] *= divisor;
    for (size_t i = 1; i < n; ++i) {
        dest[i] = src[i] - dest[i - 1];
        dest[i] *= divisor;
    }

    return f;
}

barretenberg::polynomial_arithmetic::lagrange_evaluations get_lagrange_evaluations(
    const fr& z, const evaluation_domain& domain, const size_t num_roots_cut_out_of_vanishing_polynomial)
{
    // compute Z_H*(z), l_start(z), l_{end}(z)
    // Note that as we modify the vanishing polynomial by cutting out some roots, we must simultaneously ensure that
    // the lagrange polynomials we require would be l_1(z) and l_{n-k}(z) where k =
    // num_roots_cut_out_of_vanishing_polynomial. For notational simplicity, we call l_1 as l_start and l_{n-k} as
    // l_end.
    //
    // NOTE: If in future, there arises a need to cut off more zeros, this method will not require any changes.
    //

    fr z_pow = z;
    for (size_t i = 0; i < domain.log2_size; ++i) {
        z_pow.self_sqr();
    }

    fr numerator = z_pow - fr::one();

    fr denominators[3];

    // compute denominator of Z_H*(z)
    // (z - w^{n-1})(z - w^{n-2})...(z - w^{n - num_roots_cut_out_of_vanishing_poly})
    fr work_root = domain.root_inverse;
    denominators[0] = fr::one();
    for (size_t i = 0; i < num_roots_cut_out_of_vanishing_polynomial; ++i) {
        denominators[0] *= (z - work_root);
        work_root *= domain.root_inverse;
    }

    // The expressions of the lagrange polynomials are:
    //           (X^n - 1)
    // L_1(X) = -----------
    //             X - 1
    //
    // L_{i}(X) = L_1(X.w^{-i})
    //                                                      (X^n - 1)
    // => L_{n-k}(X) = L_1(X.w^{k-n}) = L_1(X.w^{k + 1}) = ----------------
    //                                                      (X.w^{k+1} - 1)
    //
    denominators[1] = z - fr::one();

    // compute w^{num_roots_cut_out_of_vanishing_polynomial + 1}
    fr l_end_root = (num_roots_cut_out_of_vanishing_polynomial & 1) ? domain.root.sqr() : domain.root;
    for (size_t i = 0; i < num_roots_cut_out_of_vanishing_polynomial / 2; ++i) {
        l_end_root *= domain.root.sqr();
    }
    denominators[2] = (z * l_end_root) - fr::one();
    fr::batch_invert(denominators, 3);

    barretenberg::polynomial_arithmetic::lagrange_evaluations result;
    result.vanishing_poly = numerator * denominators[0];
    numerator = numerator * domain.domain_inverse;
    result.l_start = numerator * denominators[1];
    result.l_end = numerator * denominators[2];

    return result;
}

// computes r = \sum_{i=0}^{num_coeffs}(L_i(z).f_i)
// start with L_1(z) = ((z^n - 1)/n).(1 / z - 1)
// L_i(z) = L_1(z.w^{1-i}) = ((z^n - 1) / n).(1 / z.w^{1-i} - 1)
fr compute_barycentric_evaluation(const fr* coeffs,
                                  const size_t num_coeffs,
                                  const fr& z,
                                  const evaluation_domain& domain)
{
    fr* denominators = static_cast<fr*>(aligned_alloc(64, sizeof(fr) * num_coeffs));

    fr numerator = z;
    for (size_t i = 0; i < domain.log2_size; ++i) {
        numerator.self_sqr();
    }
    numerator -= fr::one();
    numerator *= domain.domain_inverse;

    denominators[0] = z - fr::one();
    fr work_root = domain.root_inverse;
    for (size_t i = 1; i < num_coeffs; ++i) {
        denominators[i] = work_root * z;
        denominators[i] -= fr::one();
        work_root *= domain.root_inverse;
    }

    fr::batch_invert(denominators, num_coeffs);

    fr result = fr::zero();

    for (size_t i = 0; i < num_coeffs; ++i) {
        fr temp = coeffs[i] * denominators[i];
        result = result + temp;
    }

    result = result * numerator;

    aligned_free(denominators);

    return result;
}

// Convert an fft with `current_size` point evaluations, to one with `current_size >> compress_factor` point evaluations
void compress_fft(const fr* src, fr* dest, const size_t cur_size, const size_t compress_factor)
{
    // iterate from top to bottom, allows `dest` to overlap with `src`
    size_t log2_compress_factor = (size_t)numeric::get_msb(compress_factor);
    ASSERT(1UL << log2_compress_factor == compress_factor);
    size_t new_size = cur_size >> log2_compress_factor;
    for (size_t i = 0; i < new_size; ++i) {
        fr::__copy(src[i << log2_compress_factor], dest[i]);
    }
}

} // namespace polynomial_arithmetic
} // namespace barretenberg
