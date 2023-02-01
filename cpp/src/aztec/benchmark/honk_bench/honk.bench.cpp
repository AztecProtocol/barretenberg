#include <benchmark/benchmark.h>
#include <honk/composer/standard_honk_composer.hpp>
#include <stdlib/primitives/field/field.hpp>

using namespace benchmark;

constexpr size_t MAX_GATES = 1 << 6;
// constexpr size_t NUM_CIRCUITS = 2;
constexpr size_t START = 1 << 5;

void generate_test_honk_circuit(honk::StandardHonkComposer& composer, size_t num_gates)
{
    plonk::stdlib::field_t a(plonk::stdlib::witness_t(&composer, barretenberg::fr::random_element()));
    plonk::stdlib::field_t b(plonk::stdlib::witness_t(&composer, barretenberg::fr::random_element()));
    plonk::stdlib::field_t c(&composer);
    for (size_t i = 0; i < (num_gates / 4) - 4; ++i) {
        c = a + b;
        c = a * c;
        a = b * b;
        b = c * c;
    }
}

void construct_witnesses_bench(State& state) noexcept
{
    for (auto _ : state) {
        auto composer = honk::StandardHonkComposer(static_cast<size_t>(state.range(0)));
        generate_test_honk_circuit(composer, static_cast<size_t>(state.range(0)));
        honk::Prover prover = composer.create_prover();
    }
}

BENCHMARK(construct_witnesses_bench)->RangeMultiplier(2)->Range(START, MAX_GATES);

BENCHMARK_MAIN();
