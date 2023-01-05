#include <benchmark/benchmark.h>
#include <ecc/curves/bn254/fr.hpp>
#include <numeric/bitop/get_msb.hpp>
#include <rollup/proofs/join_split/join_split_tx.hpp>
#include <rollup/proofs/join_split/join_split_circuit.hpp>
#include <rollup/proofs/join_split/compute_circuit_data.hpp>
#include <rollup/proofs/account/account_tx.hpp>
#include <rollup/proofs/account/account.hpp>
#include <rollup/proofs/account/compute_circuit_data.hpp>
#include <stdlib/types/types.hpp>
#include <stdlib/merkle_tree/memory_store.hpp>
#include <stdlib/merkle_tree/merkle_tree.hpp>

using namespace benchmark;
using namespace plonk::stdlib::types;
using namespace rollup::proofs;
using namespace plonk::stdlib::merkle_tree;

// Number of proof constructions we want to benchmark proof construction times.
static constexpr size_t NUM_PROOFS = 20;
namespace Circuits {
enum { JOIN_SPLIT = 0, ACCOUNT = 1, NUM = 2 };
};

Prover provers[Circuits::NUM];
Verifier verifiers[Circuits::NUM];
waffle::plonk_proof proofs[Circuits::NUM];

/**
 * This function enforces the constraint for either of:
 * (1) join-split circuit
 * (2) account circuit
 * using noop transaction to generate a valid witness for benchmarking the prover computation.
 *
 * TODO: write noop txs for other circuits for benchmarking.
 */
void apply_circuit(Composer& composer, const size_t circuit_type)
{
    auto store = std::make_unique<MemoryStore>();
    auto tree = std::make_unique<MerkleTree<MemoryStore>>(*store, 32);
    join_split::join_split_tx js_tx = join_split::noop_tx();
    js_tx.old_data_root = tree->root();
    account::account_tx acc_tx = account::noop_tx();
    acc_tx.merkle_root = tree->root();
    switch (circuit_type) {
    case Circuits::JOIN_SPLIT:
        join_split::join_split_circuit(composer, js_tx);
        break;
    case Circuits::ACCOUNT:
        account::account_circuit(composer, acc_tx);
        break;
    default:
        break;
    }
}

/**
 * Performs the circuit logic and measures the time taken for it.
 */
void build_circuit_bench(State& state) noexcept
{
    for (auto _ : state) {
        Composer composer = Composer();
        apply_circuit(composer, static_cast<size_t>(state.range(0)));
    }
}
BENCHMARK(build_circuit_bench)->DenseRange(0, Circuits::NUM - 1, 1);

/**
 * Computes the witness data from a fresh composer for a given circuit.
 * We measure the time it takes to compute the witness AFTER the circuit is built.
 */
void construct_witnesses_bench(State& state) noexcept
{
    for (auto _ : state) {
        state.PauseTiming();
        Composer composer = Composer();
        apply_circuit(composer, static_cast<size_t>(state.range(0)));
        composer.compute_proving_key();
        state.ResumeTiming();
        composer.compute_witness();
    }
}
BENCHMARK(construct_witnesses_bench)->DenseRange(0, Circuits::NUM - 1, 1);

/**
 * Computes the proving key from a fresh composer for a given circuit.
 * We measure the time it takes to compute the proving key AFTER the circuit logic is built.
 */
void construct_proving_keys_bench(State& state) noexcept
{
    for (auto _ : state) {
        state.PauseTiming();
        Composer composer = Composer();
        apply_circuit(composer, static_cast<size_t>(state.range(0)));
        state.ResumeTiming();
        composer.compute_proving_key();
        state.PauseTiming();
        provers[static_cast<size_t>(state.range(0))] = composer.create_prover();
        state.ResumeTiming();
    }
}
BENCHMARK(construct_proving_keys_bench)->DenseRange(0, Circuits::NUM - 1, 1);

/**
 * Computes the verifier (also verification key) from a fresh composer for a given circuit.
 * We measure the time taken to create/compute a verifier AFTER the circuit is built.
 */
void construct_instances_bench(State& state) noexcept
{
    for (auto _ : state) {
        state.PauseTiming();
        Composer composer = Composer();
        apply_circuit(composer, static_cast<size_t>(state.range(0)));
        composer.create_prover();
        state.ResumeTiming();
        verifiers[static_cast<size_t>(state.range(0))] = composer.create_verifier();
    }
}
BENCHMARK(construct_instances_bench)->DenseRange(0, Circuits::NUM - 1, 1);

/**
 * Computes proofs (# = NUM_PROOFS) from the cached composer for a given circuit.
 */
void construct_proofs_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t idx = static_cast<size_t>(state.range(0));
        proofs[idx] = provers[idx].construct_proof();
        state.PauseTiming();
        provers[idx].reset();
        state.ResumeTiming();
    }
}
BENCHMARK(construct_proofs_bench)->DenseRange(0, Circuits::NUM - 1, 1)->Iterations(NUM_PROOFS);

/**
 * Verifies the cached proofs.
 */
void verify_proofs_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t idx = static_cast<size_t>(state.range(0));
        verifiers[idx].verify_proof(proofs[idx]);
    }
}
BENCHMARK(verify_proofs_bench)->DenseRange(0, Circuits::NUM - 1, 1);

BENCHMARK_MAIN();