#include "prover.hpp"
#include <env/data_store.hpp>

#define WASM_EXPORT __attribute__((visibility("default")))

using namespace barretenberg;

extern "C" {

/**
 * Called by `barretenberg_wasm.test.ts` to test the asyncify intrumentation and logic that
 * allows for WASM code to make calls to async code in JS.
 */
WASM_EXPORT void* test_async_func(size_t size, int val)
{
    {
        info("setting ", size, " bytes of data...");
        auto addr = malloc(size);
        memset(addr, val, size);
        set_data("some_key", addr, size);
        free(addr);
        info("done.");
    }
    {
        size_t length;
        void* addr = get_data("some_key", &length);
        info("data addr: ", addr, " length: ", length);
        // aligned_free(addr);
        return addr;
    }
}

typedef std::conditional_t<waffle::SYSTEM_COMPOSER == waffle::TURBO, waffle::TurboProver, waffle::UltraProver>
    WasmProver;

WASM_EXPORT void prover_process_queue(WasmProver* prover)
{
    prover->queue.process_queue();
}

WASM_EXPORT size_t prover_get_circuit_size(WasmProver* prover)
{
    return prover->get_circuit_size();
}

WASM_EXPORT void prover_get_work_queue_item_info(WasmProver* prover, uint8_t* result)
{
    auto info = prover->get_queued_work_item_info();
    memcpy(result, &info, sizeof(info));
}

WASM_EXPORT fr* prover_get_scalar_multiplication_data(WasmProver* prover, size_t work_item_number)
{
    return prover->get_scalar_multiplication_data(work_item_number);
}

WASM_EXPORT size_t prover_get_scalar_multiplication_size(WasmProver* prover, size_t work_item_number)
{
    return prover->get_scalar_multiplication_size(work_item_number);
}

WASM_EXPORT void prover_put_scalar_multiplication_data(WasmProver* prover,
                                                       g1::element* result,
                                                       const size_t work_item_number)
{
    prover->put_scalar_multiplication_data(*result, work_item_number);
}

WASM_EXPORT fr* prover_get_fft_data(WasmProver* prover, fr* shift_factor, size_t work_item_number)
{
    auto data = prover->get_fft_data(work_item_number);
    *shift_factor = data.shift_factor;
    return data.data;
}

WASM_EXPORT void prover_put_fft_data(WasmProver* prover, fr* result, size_t work_item_number)
{
    prover->put_fft_data(result, work_item_number);
}

WASM_EXPORT fr* prover_get_ifft_data(WasmProver* prover, size_t work_item_number)
{
    return prover->get_ifft_data(work_item_number);
}

WASM_EXPORT void prover_put_ifft_data(WasmProver* prover, fr* result, size_t work_item_number)
{
    prover->put_ifft_data(result, work_item_number);
}

WASM_EXPORT void prover_execute_preamble_round(WasmProver* prover)
{
    prover->execute_preamble_round();
}

WASM_EXPORT void prover_execute_first_round(WasmProver* prover)
{
    prover->execute_first_round();
}

WASM_EXPORT void prover_execute_second_round(WasmProver* prover)
{
    prover->execute_second_round();
}

WASM_EXPORT void prover_execute_third_round(WasmProver* prover)
{
    prover->execute_third_round();
}

WASM_EXPORT void prover_execute_fourth_round(WasmProver* prover)
{
    prover->execute_fourth_round();
}

WASM_EXPORT void prover_execute_fifth_round(WasmProver* prover)
{
    prover->execute_fifth_round();
}

WASM_EXPORT void prover_execute_sixth_round(WasmProver* prover)
{
    prover->execute_sixth_round();
}

WASM_EXPORT size_t prover_export_proof(WasmProver* prover, uint8_t** proof_data_buf)
{
    auto& proof_data = prover->export_proof().proof_data;
    *proof_data_buf = proof_data.data();
    return proof_data.size();
}

WASM_EXPORT void coset_fft_with_generator_shift(fr* coefficients, fr* constant, evaluation_domain* domain)
{
    polynomial_arithmetic::coset_fft_with_generator_shift(coefficients, *domain, *constant);
}

WASM_EXPORT void ifft(fr* coefficients, evaluation_domain* domain)
{
    polynomial_arithmetic::ifft(coefficients, *domain);
}

WASM_EXPORT void* new_evaluation_domain(size_t circuit_size)
{
    auto domain = new evaluation_domain(circuit_size);
    domain->compute_lookup_table();
    return domain;
}

WASM_EXPORT void delete_evaluation_domain(void* domain)
{
    delete reinterpret_cast<evaluation_domain*>(domain);
}
}
