#include "barretenberg/crypto/ecdsa/ecdsa.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/honk/proof_system/ultra_prover.hpp"
#include "barretenberg/honk/proof_system/ultra_verifier.hpp"
#include <benchmark/benchmark.h>
#include <cstddef>
#include "barretenberg/honk/composer/standard_honk_composer.hpp"
#include "barretenberg/plonk/composer/standard_plonk_composer.hpp"
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

namespace standard_plonk_bench {

using StandardPlonk = proof_system::plonk::StandardPlonkComposer;

// Log number of gates for test circuit
constexpr size_t MIN_LOG_NUM_GATES = bench_utils::BenchParams::MIN_LOG_NUM_GATES;
constexpr size_t MAX_LOG_NUM_GATES = bench_utils::BenchParams::MAX_LOG_NUM_GATES;
// Number of times to repeat each benchmark
constexpr size_t NUM_REPETITIONS = bench_utils::BenchParams::NUM_REPETITIONS;

/**
 * @brief Benchmark: Construction of a Standard proof for a circuit determined by the provided circuit function
 */
template <typename Composer>
void construct_proof_standard(State& state, void (*test_circuit_function)(Composer&, size_t)) noexcept
{
    auto num_gates = static_cast<size_t>(1 << (size_t)state.range(0));
    for (auto _ : state) {
        // Constuct circuit and prover; don't include this part in measurement
        state.PauseTiming();
        auto composer = Composer();
        test_circuit_function(composer, num_gates);
        auto ext_prover = composer.create_prover();
        state.ResumeTiming();

        // Construct proof
        auto proof = ext_prover.construct_proof();
    }
}

BENCHMARK_CAPTURE(construct_proof_standard, arithmetic, &bench_utils::generate_basic_arithmetic_circuit<StandardPlonk>)
    ->DenseRange(MIN_LOG_NUM_GATES, MAX_LOG_NUM_GATES)
    ->Repetitions(NUM_REPETITIONS);

} // namespace standard_plonk_bench