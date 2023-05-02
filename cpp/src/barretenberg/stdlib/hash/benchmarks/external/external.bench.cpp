/**
 * @file external.bench.cpp
 * @author Kesha (Rumata888)
 * @brief Benchmarks for external benchmarking projects (e.g. delendum-xyz)
 *
 */
#include "../../sha256/sha256.hpp"
#include "../../blake3s/blake3s.hpp"
#include <benchmark/benchmark.h>
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/plonk/composer/ultra_composer.hpp"
#include "barretenberg/plonk/proof_system/prover/prover.hpp"
#include "barretenberg/stdlib/primitives/packed_byte_array/packed_byte_array.hpp"

using namespace benchmark;

using Composer = proof_system::plonk::UltraComposer;
using Prover = proof_system::plonk::UltraProver;
using Verifier = proof_system::plonk::UltraVerifier;

constexpr size_t PROOF_COUNT_LOG = 10;
constexpr size_t NUM_PROOFS = 3;

/**
 * @brief Main function generating a circuit with num_iterations sequential sha256 hashes, where the output of a
 * previous iteration is fed into the next one
 *
 * @param composer
 * @param num_iterations
 */
void generate_test_sha256_plonk_circuit(Composer& composer, size_t num_iterations)
{
    std::string in;
    in.resize(32);
    for (size_t i = 0; i < 32; ++i) {
        in[i] = 0;
    }
    proof_system::plonk::stdlib::packed_byte_array<Composer> input(&composer, in);
    for (size_t i = 0; i < num_iterations; i++) {
        input = proof_system::plonk::stdlib::sha256<Composer>(input);
    }
}
// /**
//  * @brief If CPU scaling is disabled, we need to warm up to get an accurate reading on the first benchmark
//  *
//  * @param state
//  */
// void warm_up(State& state)
// {
//     for (auto _ : state) {

//         size_t num_iterations = static_cast<size_t>(state.range(0));
//         auto composer = Composer();
//         generate_test_sha256_plonk_circuit(composer, num_iterations);
//         info("Gate number: ", composer.get_num_gates());
//         auto prover = composer.create_prover();
//         auto proof = prover.construct_proof();
//         info("Proof Size for SHA256 hash count ", num_iterations, ": ", proof.proof_data.size());
//     }
// }

// BENCHMARK(warm_up)->DenseRange(1, 3);
Composer external_composers[NUM_PROOFS];
Prover external_provers[NUM_PROOFS];
Verifier external_verifiers[NUM_PROOFS];
plonk::proof external_proofs[NUM_PROOFS];

/**
 * @brief Construct the circuit for sequential sha256 proofs and compute the proof for each case
 *
 * @param state
 */
void generate_sha256_proof_bench(State& state) noexcept
{
    for (auto _ : state) {

        size_t idx = static_cast<size_t>(state.range(0));
        size_t num_iterations = 1;
        for (size_t i = 0; i < idx; i++) {
            num_iterations *= PROOF_COUNT_LOG;
        }
        external_composers[idx] = Composer();
        generate_test_sha256_plonk_circuit(external_composers[idx], num_iterations);
        // info("Gate number: ", external_composers[idx].get_num_gates());
        external_provers[idx] = external_composers[idx].create_prover();
        external_proofs[idx] = external_provers[idx].construct_proof();
        // info("Proof Size for SHA256 hash count ", num_iterations, ": ", external_proofs[idx].proof_data.size());
    }
}

/**
 * @brief We have to warm up the benchmarking function first, otherwise we spend 50% more time than expected
 *
 */
BENCHMARK(generate_sha256_proof_bench)->DenseRange(0, 2)->MinWarmUpTime(10)->MinTime(2)->Unit(benchmark::kMillisecond);
/**
 * @brief Create sha256 verifier
 *
 * @details We don't want to benchmark this
 *
 * @param state
 */
static void generate_sha256_verifier(const State& state)
{

    size_t idx = static_cast<size_t>(state.range(0));
    external_verifiers[idx] = external_composers[idx].create_verifier();
}
/**
 * @brief Benchmark sha256 verification
 *
 * @param state
 */
void verify_sha256_proof_bench(State& state) noexcept
{
    for (auto _ : state) {

        size_t idx = static_cast<size_t>(state.range(0));
        external_verifiers[idx].verify_proof(external_proofs[idx]);
    }
}

BENCHMARK(verify_sha256_proof_bench)->DenseRange(0, 2)->Setup(generate_sha256_verifier)->Unit(benchmark::kMillisecond);

/**
 * @brief Main function for generating Blake 3 circuits
 *
 * @param composer
 * @param num_iterations
 */
void generate_test_blake3s_plonk_circuit(Composer& composer, size_t num_iterations)
{
    std::string in;
    in.resize(32);
    for (size_t i = 0; i < 32; ++i) {
        in[i] = 0;
    }
    proof_system::plonk::stdlib::packed_byte_array<Composer> input(&composer, in);
    for (size_t i = 0; i < num_iterations; i++) {
        input = proof_system::plonk::stdlib::blake3s<Composer>(input);
    }
}

/**
 * @brief Blake3 circuit construction and proof creation benchmark function
 *
 * @param state
 */
void generate_blake3s_proof_bench(State& state) noexcept
{
    for (auto _ : state) {

        size_t idx = static_cast<size_t>(state.range(0));
        size_t num_iterations = 1;
        for (size_t i = 0; i < idx; i++) {
            num_iterations *= PROOF_COUNT_LOG;
        }
        external_composers[idx] = Composer();
        generate_test_blake3s_plonk_circuit(external_composers[idx], num_iterations);
        // info("Gate number: ", external_composers[idx].get_num_gates());
        external_provers[idx] = external_composers[idx].create_prover();
        external_proofs[idx] = external_provers[idx].construct_proof();
        // Proof size with no public inputs is always 2144
        // info("Proof Size for Blake3s hash count ", num_iterations, ": ", external_proofs[idx].proof_data.size());
    }
}

BENCHMARK(generate_blake3s_proof_bench)->DenseRange(0, 2)->MinWarmUpTime(10)->MinTime(2)->Unit(benchmark::kMillisecond);

/**
 * @brief Create blake 3 verifier
 *
 * @details We don't benchmark verifier creation
 *
 * @param state
 */
static void generate_blake3s_verifier(const State& state)
{

    size_t idx = static_cast<size_t>(state.range(0));
    external_verifiers[idx] = external_composers[idx].create_verifier();
}

/**
 * @brief Benchmark blake3 proof verification
 *
 * @param state
 */
void verify_blake3s_proof_bench(State& state) noexcept
{
    for (auto _ : state) {

        size_t idx = static_cast<size_t>(state.range(0));
        external_verifiers[idx].verify_proof(external_proofs[idx]);
    }
}

BENCHMARK(verify_blake3s_proof_bench)
    ->DenseRange(0, 2)
    ->Setup(generate_blake3s_verifier)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();