#include "barretenberg/crypto/ecdsa/ecdsa.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include <benchmark/benchmark.h>
#include <cstddef>
#include "barretenberg/stdlib/primitives/composers/composers_fwd.hpp"
#include "barretenberg/stdlib/primitives/composers/composers.hpp"
#include "barretenberg/stdlib/encryption/ecdsa/ecdsa.hpp"
#include "barretenberg/stdlib/hash/keccak/keccak.hpp"
#include "barretenberg/stdlib/primitives/curves/secp256k1.hpp"
#include "barretenberg/stdlib/primitives/packed_byte_array/packed_byte_array.hpp"
#include "barretenberg/stdlib/hash/sha256/sha256.hpp"
#include "barretenberg/stdlib/primitives/bool/bool.hpp"
#include "barretenberg/stdlib/primitives/field/field.hpp"
#include "barretenberg/stdlib/primitives/witness/witness.hpp"
#include "barretenberg/stdlib/merkle_tree/merkle_tree.hpp"
#include "barretenberg/stdlib/merkle_tree/membership.hpp"
#include "barretenberg/stdlib/merkle_tree/memory_store.hpp"
#include "barretenberg/stdlib/merkle_tree/memory_tree.hpp"
#include "barretenberg/benchmark/honk_bench/benchmark_utilities.hpp"

using namespace benchmark;

namespace ultra_honk_bench {

using UltraHonk = proof_system::honk::UltraHonkComposer;

// Number of times to perform operation of interest in the benchmark circuits, e.g. # of hashes to perform
constexpr size_t MIN_NUM_ITERATIONS = bench_utils::BenchParams::MIN_NUM_ITERATIONS;
constexpr size_t MAX_NUM_ITERATIONS = bench_utils::BenchParams::MAX_NUM_ITERATIONS;
// Number of times to repeat each benchmark
constexpr size_t NUM_REPETITIONS = bench_utils::BenchParams::NUM_REPETITIONS;

/**
 * @brief Benchmark: Construction of a Ultra Honk proof for a circuit determined by the provided text circuit function
 */
template <typename Composer>
void construct_proof(State& state, void (*test_circuit_function)(Composer&, size_t)) noexcept
{
    auto num_iterations = static_cast<size_t>(state.range(0));
    for (auto _ : state) {
        // Constuct circuit and prover; don't include this part in measurement
        state.PauseTiming();
        auto composer = Composer();
        test_circuit_function(composer, num_iterations);
        auto ext_prover = composer.create_prover();
        state.ResumeTiming();

        // Construct proof
        auto proof = ext_prover.construct_proof();
    }
}

BENCHMARK_CAPTURE(construct_proof, sha256, &bench_utils::generate_sha256_test_circuit<UltraHonk>)
    ->DenseRange(MIN_NUM_ITERATIONS, MAX_NUM_ITERATIONS)
    ->Repetitions(NUM_REPETITIONS);
BENCHMARK_CAPTURE(construct_proof, keccak, &bench_utils::generate_keccak_test_circuit<UltraHonk>)
    ->DenseRange(MIN_NUM_ITERATIONS, MAX_NUM_ITERATIONS)
    ->Repetitions(NUM_REPETITIONS);
BENCHMARK_CAPTURE(construct_proof,
                  ecdsa_verification,
                  &bench_utils::generate_ecdsa_verification_test_circuit<UltraHonk>)
    ->DenseRange(MIN_NUM_ITERATIONS, MAX_NUM_ITERATIONS)
    ->Repetitions(NUM_REPETITIONS);
BENCHMARK_CAPTURE(construct_proof, merkle_membership, &bench_utils::generate_merkle_membership_test_circuit<UltraHonk>)
    ->DenseRange(MIN_NUM_ITERATIONS, MAX_NUM_ITERATIONS)
    ->Repetitions(NUM_REPETITIONS);

} // namespace ultra_honk_bench