#include "work_queue.hpp"

#include "barretenberg/ecc/curves/bn254/scalar_multiplication/scalar_multiplication.hpp"
#include "barretenberg/polynomials/polynomial_arithmetic.hpp"

namespace proof_system::plonk {

work_queue::work_queue(proving_key* prover_key, transcript::StandardTranscript* prover_transcript)
    : key(prover_key)
    , transcript(prover_transcript)
    , work_item_queue()
{}

work_queue::work_item_info work_queue::get_queued_work_item_info() const
{
    uint32_t scalar_mul_count = 0;
    uint32_t fft_count = 0;
    uint32_t ifft_count = 0;
    for (const auto& item : work_item_queue) {
        if (item.work_type == WorkType::SCALAR_MULTIPLICATION) {
            ++scalar_mul_count;
        }
        if (item.work_type == WorkType::SMALL_FFT) {
            ++fft_count;
        }
        if (item.work_type == WorkType::IFFT) {
            ++ifft_count;
        }
    }
    return work_item_info{ scalar_mul_count, fft_count, ifft_count };
}

barretenberg::fr* work_queue::get_scalar_multiplication_data(const size_t work_item_number) const
{
    size_t count = 0;
    for (const auto& item : work_item_queue) {
        if (item.work_type == WorkType::SCALAR_MULTIPLICATION) {
            if (count == work_item_number) {
                return item.mul_scalars;
            }
            ++count;
        }
    }
    return nullptr;
}

size_t work_queue::get_scalar_multiplication_size(const size_t work_item_number) const
{
    size_t count = 0;
    for (const auto& item : work_item_queue) {
        if (item.work_type == WorkType::SCALAR_MULTIPLICATION) {
            if (count == work_item_number) {
                return static_cast<size_t>(static_cast<uint256_t>(item.constant));
            }
            ++count;
        }
    }
    return 0;
}

barretenberg::fr* work_queue::get_ifft_data(const size_t work_item_number) const
{
    size_t count = 0;
    for (const auto& item : work_item_queue) {
        if (item.work_type == WorkType::IFFT) {
            if (count == work_item_number) {
                barretenberg::polynomial& wire = key->polynomial_store.get(item.tag + "_lagrange");
                return wire.get_coefficients();
            }
            ++count;
        }
    }
    return nullptr;
}

void work_queue::put_ifft_data(barretenberg::fr* result, const size_t work_item_number)
{
    size_t count = 0;
    for (const auto& item : work_item_queue) {
        if (item.work_type == WorkType::IFFT) {
            if (count == work_item_number) {
                barretenberg::polynomial wire(key->circuit_size);
                memcpy((void*)wire.get_coefficients(), result, key->circuit_size * sizeof(barretenberg::fr));
                key->polynomial_store.put(item.tag, std::move(wire));
                return;
            }
            ++count;
        }
    }
}

work_queue::queued_fft_inputs work_queue::get_fft_data(const size_t work_item_number) const
{
    size_t count = 0;
    for (const auto& item : work_item_queue) {
        if (item.work_type == WorkType::SMALL_FFT) {
            if (count == work_item_number) {
                barretenberg::polynomial& wire = key->polynomial_store.get(item.tag);
                return { wire.get_coefficients(), key->large_domain.root.pow(static_cast<uint64_t>(item.index)) };
            }
            ++count;
        }
    }
    return { nullptr, barretenberg::fr(0) };
}

void work_queue::put_fft_data(barretenberg::fr* result, const size_t work_item_number)
{
    size_t count = 0;
    for (const auto& item : work_item_queue) {
        if (item.work_type == WorkType::SMALL_FFT) {
            if (count == work_item_number) {
                const size_t n = key->circuit_size;
                barretenberg::polynomial wire_fft(4 * n + 4);

                for (size_t i = 0; i < n; ++i) {
                    wire_fft[4 * i + item.index] = result[i];
                }
                wire_fft[4 * n + item.index] = result[0];

                key->polynomial_store.put(item.tag + "_fft", std::move(wire_fft));

                return;
            }
            ++count;
        }
    }
}

void work_queue::put_scalar_multiplication_data(const barretenberg::g1::affine_element result,
                                                const size_t work_item_number)
{
    size_t count = 0;
    for (const auto& item : work_item_queue) {
        if (item.work_type == WorkType::SCALAR_MULTIPLICATION) {
            if (count == work_item_number) {
                transcript->add_element(item.tag, result.to_buffer());
                return;
            }
            ++count;
        }
    }
}

void work_queue::flush_queue()
{
    work_item_queue = std::vector<work_item>();
}

void work_queue::add_to_queue(const work_item& item)
{
#if defined(__wasm__)
    // #if 1
    if (item.work_type == WorkType::FFT) {
        const auto large_root = key->large_domain.root;
        barretenberg::fr coset_shifts[4]{
            barretenberg::fr(1), large_root, large_root.sqr(), large_root.sqr() * large_root
        };
        work_item_queue.push_back({
            WorkType::SMALL_FFT,
            nullptr,
            item.tag,
            coset_shifts[0],
            0,
        });
        work_item_queue.push_back({
            WorkType::SMALL_FFT,
            nullptr,
            item.tag,
            coset_shifts[1],
            1,
        });
        work_item_queue.push_back({
            WorkType::SMALL_FFT,
            nullptr,
            item.tag,
            coset_shifts[2],
            2,
        });
        work_item_queue.push_back({
            WorkType::SMALL_FFT,
            nullptr,
            item.tag,
            coset_shifts[3],
            3,
        });
    } else {
        work_item_queue.push_back(item);
    }
#else
    work_item_queue.push_back(item);
#endif
}

void work_queue::process_queue()
{
    for (const auto& item : work_item_queue) {
        switch (item.work_type) {
        // most expensive op
        case WorkType::SCALAR_MULTIPLICATION: {
            // Note: work_item.constant is an Fr type (see SMALL_FFT), but here it is interpreted simply as a size_t
            auto msm_size = static_cast<size_t>(static_cast<uint256_t>(item.constant));

            ASSERT(msm_size <= key->reference_string->get_monomial_size());

            barretenberg::g1::affine_element* srs_points = key->reference_string->get_monomial_points();

            // Run pippenger multi-scalar multiplication.
            auto runtime_state = barretenberg::scalar_multiplication::pippenger_runtime_state(msm_size);
            barretenberg::g1::affine_element result(barretenberg::scalar_multiplication::pippenger_unsafe(
                item.mul_scalars, srs_points, msm_size, runtime_state));

            transcript->add_element(item.tag, result.to_buffer());

            break;
        }
        // About 20% of the cost of a scalar multiplication. For WASM, might be a bit more expensive
        // due to the need to copy memory between web workers
        case WorkType::SMALL_FFT: {
            using namespace barretenberg;
            const size_t n = key->circuit_size;
            polynomial& wire = key->polynomial_store.get(item.tag);

            polynomial wire_copy(wire, n);
            wire_copy.coset_fft_with_generator_shift(key->small_domain, item.constant);

            if (item.index != 0) {
                polynomial& old_wire_fft = key->polynomial_store.get(item.tag + "_fft");
                for (size_t i = 0; i < n; ++i) {
                    old_wire_fft[4 * i + item.index] = wire_copy[i];
                }
                old_wire_fft[4 * n + item.index] = wire_copy[0];
            } else {
                polynomial wire_fft(4 * n + 4);
                for (size_t i = 0; i < n; ++i) {
                    wire_fft[4 * i + item.index] = wire_copy[i];
                }
                key->polynomial_store.put(item.tag + "_fft", std::move(wire_fft));
            }
            break;
        }
        case WorkType::FFT: {
            using namespace barretenberg;
            polynomial& wire = key->polynomial_store.get(item.tag);
            polynomial wire_fft(wire, 4 * key->circuit_size + 4);

            wire_fft.coset_fft(key->large_domain);
            for (size_t i = 0; i < 4; i++) {
                wire_fft[4 * key->circuit_size + i] = wire_fft[i];
            }

            key->polynomial_store.put(item.tag + "_fft", std::move(wire_fft));

            break;
        }
        // 1/4 the cost of an fft (each fft has 1/4 the number of elements)
        case WorkType::IFFT: {
            using namespace barretenberg;
            // retrieve wire in lagrange form
            polynomial& wire_lagrange = key->polynomial_store.get(item.tag + "_lagrange");

            // Compute wire monomial form via ifft on lagrange form then add it to the store
            polynomial wire_monomial(key->circuit_size);
            polynomial_arithmetic::ifft(&wire_lagrange[0], &wire_monomial[0], key->small_domain);
            key->polynomial_store.put(item.tag, std::move(wire_monomial));

            break;
        }
        default: {
        }
        }
    }
    work_item_queue = std::vector<work_item>();
}

std::vector<work_queue::work_item> work_queue::get_queue() const
{
    return work_item_queue;
}

} // namespace proof_system::plonk
