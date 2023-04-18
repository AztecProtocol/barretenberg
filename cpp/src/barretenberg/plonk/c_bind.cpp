#include "./c_bind.hpp"
#include "./proof_system/prover/prover.hpp"
#include "barretenberg/common/serialize.hpp"
#include <barretenberg/env/data_store.hpp>
#include <barretenberg/env/crs.hpp>
#include <barretenberg/proof_system/types/composer_type.hpp>

using namespace barretenberg;

extern "C" {

using Prover =
    std::conditional_t<plonk::SYSTEM_COMPOSER == ComposerType::TURBO, plonk::TurboProver, plonk::UltraProver>;

WASM_EXPORT void plonk_prover_process_queue(in_ptr prover)
{
    ((Prover*)prover)->queue.process_queue();
}

WASM_EXPORT void plonk_prover_get_circuit_size(in_ptr prover, uint32_t* out)
{
    uint32_t size = (uint32_t)((Prover*)prover)->get_circuit_size();
    auto* dst = (uint8_t*)out;
    serialize::write(dst, size);
}

WASM_EXPORT void plonk_prover_export_proof(in_ptr prover, uint8_t** proof_data_buf)
{
    auto& proof_data = ((Prover*)prover)->export_proof().proof_data;
    *proof_data_buf = to_heap_buffer(proof_data);
}

// WASM_EXPORT void prover_get_work_queue_item_info(WasmProver* prover, uint8_t* result)
// {
//     auto info = prover->get_queued_work_item_info();
//     memcpy(result, &info, sizeof(info));
// }

// WASM_EXPORT fr* prover_get_scalar_multiplication_data(WasmProver* prover, size_t work_item_number)
// {
//     return prover->get_scalar_multiplication_data(work_item_number);
// }

// WASM_EXPORT size_t prover_get_scalar_multiplication_size(WasmProver* prover, size_t work_item_number)
// {
//     return prover->get_scalar_multiplication_size(work_item_number);
// }

// WASM_EXPORT void prover_put_scalar_multiplication_data(WasmProver* prover,
//                                                        g1::element* result,
//                                                        const size_t work_item_number)
// {
//     prover->put_scalar_multiplication_data(*result, work_item_number);
// }

// WASM_EXPORT fr* prover_get_fft_data(WasmProver* prover, fr* shift_factor, size_t work_item_number)
// {
//     auto data = prover->get_fft_data(work_item_number);
//     *shift_factor = data.shift_factor;
//     return data.data;
// }

// WASM_EXPORT void prover_put_fft_data(WasmProver* prover, fr* result, size_t work_item_number)
// {
//     prover->put_fft_data(result, work_item_number);
// }

// WASM_EXPORT fr* prover_get_ifft_data(WasmProver* prover, size_t work_item_number)
// {
//     return prover->get_ifft_data(work_item_number);
// }

// WASM_EXPORT void prover_put_ifft_data(WasmProver* prover, fr* result, size_t work_item_number)
// {
//     prover->put_ifft_data(result, work_item_number);
// }

// WASM_EXPORT void prover_execute_preamble_round(WasmProver* prover)
// {
//     prover->execute_preamble_round();
// }

// WASM_EXPORT void prover_execute_first_round(WasmProver* prover)
// {
//     prover->execute_first_round();
// }

// WASM_EXPORT void prover_execute_second_round(WasmProver* prover)
// {
//     prover->execute_second_round();
// }

// WASM_EXPORT void prover_execute_third_round(WasmProver* prover)
// {
//     prover->execute_third_round();
// }

// WASM_EXPORT void prover_execute_fourth_round(WasmProver* prover)
// {
//     prover->execute_fourth_round();
// }

// WASM_EXPORT void prover_execute_fifth_round(WasmProver* prover)
// {
//     prover->execute_fifth_round();
// }

// WASM_EXPORT void prover_execute_sixth_round(WasmProver* prover)
// {
//     prover->execute_sixth_round();
// }

// WASM_EXPORT void coset_fft_with_generator_shift(fr* coefficients, fr* constant, evaluation_domain* domain)
// {
//     polynomial_arithmetic::coset_fft_with_generator_shift(coefficients, *domain, *constant);
// }

// WASM_EXPORT void ifft(fr* coefficients, evaluation_domain* domain)
// {
//     polynomial_arithmetic::ifft(coefficients, *domain);
// }

// WASM_EXPORT void* new_evaluation_domain(size_t circuit_size)
// {
//     auto domain = new evaluation_domain(circuit_size);
//     domain->compute_lookup_table();
//     return domain;
// }

// WASM_EXPORT void delete_evaluation_domain(void* domain)
// {
//     delete reinterpret_cast<evaluation_domain*>(domain);
// }
}
