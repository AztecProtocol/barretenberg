#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/honk/proof_system/prover.hpp"
#include "barretenberg/honk/proof_system/verifier.hpp"
#include <benchmark/benchmark.h>
#include <cstddef>
#include "barretenberg/honk/composer/ultra_honk_composer.hpp"
#include "barretenberg/stdlib/primitives/field/field.hpp"

using namespace benchmark;

namespace ultra_honk_bench {

using Composer = proof_system::honk::UltraHonkComposer;

constexpr size_t MIN_LOG_NUM_GATES = 16;
constexpr size_t MAX_LOG_NUM_GATES = 16;
// To get good statistics, number of Repetitions must be sufficient. ~30 Repetitions gives good results.
constexpr size_t NUM_REPETITIONS = 1;

void generate_test_circuit(auto& composer, size_t num_gates)
{
    for (size_t j = 0; j < (num_gates / 4) - 4; ++j) {
        uint64_t left = static_cast<uint64_t>(j);
        uint64_t right = static_cast<uint64_t>(j + 1);
        uint32_t left_idx = composer.add_variable(fr(left));
        uint32_t right_idx = composer.add_variable(fr(right));
        uint32_t result_idx = composer.add_variable(fr(left ^ right));

        uint32_t add_idx = composer.add_variable(fr(left) + fr(right) + composer.get_variable(result_idx));
        composer.create_big_add_gate({ left_idx, right_idx, result_idx, add_idx, fr(1), fr(1), fr(1), fr(-1), fr(0) });
    }
    info("composer.num_gates = ", composer.num_gates);
}

/**
 * @brief Benchmark: Creation of a Standard Honk prover
 */
void create_prover_ultra(State& state) noexcept
{
    for (auto _ : state) {
        state.PauseTiming();
        auto num_gates = 1 << (size_t)state.range(0);
        auto composer = Composer();
        generate_test_circuit(composer, static_cast<size_t>(num_gates));
        state.ResumeTiming();

        composer.create_prover();
    }
}
BENCHMARK(create_prover_ultra)->DenseRange(MIN_LOG_NUM_GATES, MAX_LOG_NUM_GATES, 1)->Repetitions(NUM_REPETITIONS);

/**
 * @brief Benchmark: Creation of a Standard Honk verifier
 */
void create_verifier_ultra(State& state) noexcept
{
    for (auto _ : state) {
        state.PauseTiming();
        auto num_gates = 1 << (size_t)state.range(0);
        auto composer = Composer();
        generate_test_circuit(composer, static_cast<size_t>(num_gates));
        state.ResumeTiming();

        composer.create_verifier();
    }
}
BENCHMARK(create_verifier_ultra)->DenseRange(MIN_LOG_NUM_GATES, MAX_LOG_NUM_GATES, 1)->Repetitions(NUM_REPETITIONS);

/**
 * @brief Benchmark: Construction of a Standard Honk proof
 */
void construct_proof_ultra(State& state) noexcept
{
    auto num_gates = 1 << (size_t)state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        auto composer = Composer();
        generate_test_circuit(composer, static_cast<size_t>(num_gates));
        auto ext_prover = composer.create_prover();
        state.ResumeTiming();

        auto proof = ext_prover.construct_proof();
    }
    state.SetComplexityN(num_gates); // Set up for computation of constant C where prover ~ C*N
}
BENCHMARK(construct_proof_ultra)
    ->DenseRange(MIN_LOG_NUM_GATES, MAX_LOG_NUM_GATES, 1)
    ->Repetitions(NUM_REPETITIONS)
    ->Complexity(oN);

/**
 * @brief Benchmark: Verification of a Standard Honk proof
 */
void verify_proof_ultra(State& state) noexcept
{
    for (auto _ : state) {
        state.PauseTiming();
        auto num_gates = (size_t)state.range(0);
        auto composer = Composer();
        generate_test_circuit(composer, static_cast<size_t>(num_gates));
        auto prover = composer.create_prover();
        auto proof = prover.construct_proof();
        auto verifier = composer.create_verifier();
        state.ResumeTiming();

        verifier.verify_proof(proof);
    }
}
// Note: enforcing Iterations == 1 for now. Otherwise proof construction will occur many times and this bench will take
// a long time. (This is because the time limit for benchmarks does not include the time-excluded setup, and
// verification itself is pretty fast).
// Note: disabling this bench for now since it is not of primary interest
// BENCHMARK(verify_proof_ultra)->DenseRange(MIN_LOG_NUM_GATES, MAX_LOG_NUM_GATES, 1)->Iterations(1);
} // namespace ultra_honk_bench