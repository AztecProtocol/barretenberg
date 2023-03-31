#include "sha256.hpp"
#include <benchmark/benchmark.h>
#include <ecc/curves/bn254/fr.hpp>
#include <plonk/composer/ultra_composer.hpp>
#include <stdlib/types/types.hpp>

using namespace benchmark;
using namespace plonk::stdlib::types;

constexpr size_t NUM_HASHES = 11;

char get_random_char()
{
    return static_cast<char>(barretenberg::fr::random_element().data[0] % 8);
}

void generate_test_plonk_circuit(Composer& composer, size_t num_bytes)
{
    std::string in;
    in.resize(num_bytes);
    for (size_t i = 0; i < num_bytes; ++i) {
        in[i] = 48; // ascii for 0
    }
    packed_byte_array_ct input(&composer, in);
    plonk::stdlib::sha256(input);
}

stdlib::types::Composer composers[NUM_HASHES];
stdlib::types::Prover provers[NUM_HASHES];
stdlib::types::Verifier verifiers[NUM_HASHES];
waffle::plonk_proof proofs[NUM_HASHES];

void construct_witnesses_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t idx = ((numeric::get_msb64(static_cast<uint64_t>((state.range(0)))))) - 6;
        composers[idx] = Composer();
        generate_test_plonk_circuit(composers[idx], static_cast<size_t>(state.range(0)));
    }
}
BENCHMARK(construct_witnesses_bench)->RangeMultiplier(2)->Range(64, 1 << 16);

void compute_proving_key(State& state) noexcept
{
    for (auto _ : state) {
        size_t idx = ((numeric::get_msb64(static_cast<uint64_t>((state.range(0)))))) - 6;
        provers[idx] = composers[idx].create_prover();
        // std::cout << "prover subgroup size = " << provers[idx].key->small_domain.size << std::endl;
        // printf("num bytes = %" PRIx64 ", num gates = %zu\n", state.range(0), composers[idx].get_num_gates());
    }
}
BENCHMARK(compute_proving_key)->RangeMultiplier(2)->Range(64, 1 << 16);

void compute_verification_key(State& state) noexcept
{
    for (auto _ : state) {
        size_t idx = ((numeric::get_msb64(static_cast<uint64_t>((state.range(0)))))) - 6;
        verifiers[idx] = composers[idx].create_verifier();
    }
}

BENCHMARK(compute_verification_key)->RangeMultiplier(2)->Range(64, 1 << 16);

void construct_proofs_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t idx = ((numeric::get_msb64(static_cast<uint64_t>((state.range(0)))))) - 6;
        proofs[idx] = provers[idx].construct_proof();
        state.PauseTiming();
        provers[idx].reset();
        state.ResumeTiming();
    }
}
BENCHMARK(construct_proofs_bench)->RangeMultiplier(2)->Range(64, 1 << 16);

void verify_proofs_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t idx = ((numeric::get_msb64(static_cast<uint64_t>((state.range(0)))))) - 6;
        verifiers[idx].verify_proof(proofs[idx]);
    }
}
BENCHMARK(verify_proofs_bench)->RangeMultiplier(2)->Range(64, 1 << 16);

BENCHMARK_MAIN();
