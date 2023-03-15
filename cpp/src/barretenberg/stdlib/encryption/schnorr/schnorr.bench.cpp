#include "schnorr.hpp"
#include <benchmark/benchmark.h>
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"
#include "barretenberg/stdlib/types/types.hpp"
#include "barretenberg/crypto/schnorr/schnorr.hpp"

using namespace benchmark;
using namespace plonk::stdlib::types;

constexpr size_t NUM_MSG_SIZES = 5;
constexpr size_t MSG_SIZE = 32;
constexpr size_t START_BYTES = MSG_SIZE;
constexpr size_t MAX_BYTES = START_BYTES * (1UL << (NUM_MSG_SIZES - 1));

char get_random_char()
{
    return static_cast<char>(barretenberg::fr::random_element().data[0] % 8);
}

std::string generate_message_string(size_t num_bytes)
{
    std::string message_string;
    message_string.resize(num_bytes);
    for (size_t i = 0; i < num_bytes; ++i) {
        message_string[i] = get_random_char();
    }
    return message_string;
}

crypto::schnorr::signature generate_signature(const std::string& message_string,
                                              const crypto::schnorr::key_pair<grumpkin::fr, grumpkin::g1>& account)
{
    crypto::schnorr::signature signature =
        crypto::schnorr::construct_signature<Blake2sHasher, grumpkin::fq, grumpkin::fr, grumpkin::g1>(message_string,
                                                                                                      account);

    bool first_result = crypto::schnorr::verify_signature<Blake2sHasher, grumpkin::fq, grumpkin::fr, grumpkin::g1>(
        message_string, account.public_key, signature);
    ASSERT(first_result);

    return signature;
}

void generate_test_plonk_circuit(Composer& composer, size_t num_bytes)
{

    // generate a random key pair
    crypto::schnorr::key_pair<grumpkin::fr, grumpkin::g1> account;
    account.private_key = grumpkin::fr::random_element();
    account.public_key = grumpkin::g1::one * account.private_key;

    // generate a message
    auto message_string = generate_message_string(num_bytes);

    // generate a signature
    auto sig = generate_signature(message_string, account);

    // verify this signature in a circuit
    point_ct pub_key = stdlib::create_point_witness(composer, account.public_key);
    stdlib::schnorr::signature_bits sig_ct = stdlib::schnorr::convert_signature(&composer, sig);
    byte_array_ct message(&composer, message_string);
    stdlib::schnorr::verify_signature(message, pub_key, sig_ct);
}

Composer composers[NUM_MSG_SIZES];
plonk::stdlib::types::Prover provers[NUM_MSG_SIZES];
plonk::stdlib::types::Verifier verifiers[NUM_MSG_SIZES];
plonk::proof proofs[NUM_MSG_SIZES];

void construct_witnesses_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t msg_size = static_cast<size_t>((state.range(0)));
        size_t idx = numeric::get_msb(uint64_t(msg_size)) - 5;
        composers[idx] = Composer();
        generate_test_plonk_circuit(composers[idx], msg_size);
    }
}
BENCHMARK(construct_witnesses_bench)->ArgsProduct({ benchmark::CreateRange(START_BYTES, MAX_BYTES, 2) });

void preprocess_witnesses_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t msg_size = static_cast<size_t>((state.range(0)));
        size_t idx = numeric::get_msb(uint64_t(msg_size)) - 5;
        provers[idx] = composers[idx].create_prover();
    }
}
BENCHMARK(preprocess_witnesses_bench)->ArgsProduct({ benchmark::CreateRange(START_BYTES, MAX_BYTES, 2) });

void construct_instances_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t msg_size = static_cast<size_t>((state.range(0)));
        size_t idx = numeric::get_msb(uint64_t(msg_size)) - 5;
        verifiers[idx] = composers[idx].create_verifier();
    }
}
BENCHMARK(construct_instances_bench)->ArgsProduct({ benchmark::CreateRange(START_BYTES, MAX_BYTES, 2) });

void construct_proofs_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t msg_size = static_cast<size_t>((state.range(0)));
        size_t idx = numeric::get_msb(uint64_t(msg_size)) - 5;
        proofs[idx] = provers[idx].construct_proof();
        state.PauseTiming();
        provers[idx].reset();
        info("circuit size = ", composers[idx].get_num_gates());
        info("num_bytes = ", state.range(0));
        state.ResumeTiming();
    }
}
BENCHMARK(construct_proofs_bench)->ArgsProduct({ benchmark::CreateRange(START_BYTES, MAX_BYTES, 2) });

void verify_proofs_bench(State& state) noexcept
{
    for (auto _ : state) {
        size_t msg_size = static_cast<size_t>((state.range(0)));
        size_t idx = numeric::get_msb(uint64_t(msg_size)) - 5;
        verifiers[idx].verify_proof(proofs[idx]);
    }
}
BENCHMARK(verify_proofs_bench)->ArgsProduct({ benchmark::CreateRange(START_BYTES, MAX_BYTES, 2) });

BENCHMARK_MAIN();
