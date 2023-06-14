#include <benchmark/benchmark.h>

#include "barretenberg/proof_system/circuit_constructors/ultra_circuit_constructor.hpp"
#include "barretenberg/honk/composer/composer_helper/ultra_honk_composer_helper.hpp"
#include "barretenberg/stdlib/encryption/ecdsa/ecdsa.hpp"
#include "barretenberg/stdlib/hash/keccak/keccak.hpp"
#include "barretenberg/stdlib/primitives/curves/secp256k1.hpp"
#include "barretenberg/stdlib/hash/sha256/sha256.hpp"
#include "barretenberg/stdlib/merkle_tree/merkle_tree.hpp"
#include "barretenberg/stdlib/merkle_tree/membership.hpp"
#include "barretenberg/stdlib/merkle_tree/memory_store.hpp"

using namespace benchmark;
using namespace proof_system::plonk;

namespace ultra_honk_bench {

using Builder = proof_system::UltraCircuitConstructor;
using Composer = proof_system::honk::UltraHonkComposerHelper;

// Number of times to perform operation of interest in the benchmark circuits, e.g. # of hashes to perform
constexpr size_t MIN_NUM_ITERATIONS = 10;
constexpr size_t MAX_NUM_ITERATIONS = 10;
// Number of times to repeat each benchmark
constexpr size_t NUM_REPETITIONS = 1;

/**
 * @brief Generate test circuit with specified number of sha256 hashes
 */
void generate_sha256_test_circuit(Builder& builder, size_t num_iterations)
{
    std::string in;
    in.resize(32);
    for (size_t i = 0; i < 32; ++i) {
        in[i] = 0;
    }
    stdlib::packed_byte_array<Builder> input(&builder, in);
    for (size_t i = 0; i < num_iterations; i++) {
        input = stdlib::sha256<Builder>(input);
    }
}

/**
 * @brief Generate test circuit with specified number of keccak hashes
 *
 * @param composer
 * @param num_iterations
 */
void generate_keccak_test_circuit(Builder& builder, size_t num_iterations)
{
    std::string in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz01";

    stdlib::byte_array<Builder> input(&builder, in);
    for (size_t i = 0; i < num_iterations; i++) {
        input = stdlib::keccak<Builder>::hash(input);
    }
}

/**
 * @brief Generate test circuit with specified number of ecdsa verifications
 */
void generate_ecdsa_verification_test_circuit(Builder& builder, size_t num_iterations)
{
    using curve = stdlib::secp256k1<Builder>;

    std::string message_string = "Instructions unclear, ask again later.";

    crypto::ecdsa::key_pair<curve::fr, curve::g1> account;
    account.private_key = curve::fr::random_element();
    account.public_key = curve::g1::one * account.private_key;

    crypto::ecdsa::signature signature =
        crypto::ecdsa::construct_signature<Sha256Hasher, curve::fq, curve::fr, curve::g1>(message_string, account);

    bool first_result = crypto::ecdsa::verify_signature<Sha256Hasher, curve::fq, curve::fr, curve::g1>(
        message_string, account.public_key, signature);

    std::vector<uint8_t> rr(signature.r.begin(), signature.r.end());
    std::vector<uint8_t> ss(signature.s.begin(), signature.s.end());
    uint8_t vv = signature.v;

    curve::g1_bigfr_ct public_key = curve::g1_bigfr_ct::from_witness(&builder, account.public_key);

    stdlib::ecdsa::signature<Builder> sig{ curve::byte_array_ct(&builder, rr),
                                           curve::byte_array_ct(&builder, ss),
                                           stdlib::uint8<Builder>(&builder, vv) };

    curve::byte_array_ct message(&builder, message_string);

    for (size_t i = 0; i < num_iterations; i++) {
        stdlib::ecdsa::verify_signature<Builder, curve, curve::fq_ct, curve::bigfr_ct, curve::g1_bigfr_ct>(
            message, public_key, sig);
    }
}

/**
 * @brief Generate test circuit with specified number of merkle membership checks
 * @todo (luke): should we consider deeper tree? non-zero leaf values? variable index?
 */
void generate_merkle_membership_test_circuit(Builder& builder, size_t num_iterations)
{
    using namespace proof_system::plonk::stdlib;
    using field_ct = field_t<Builder>;
    using witness_ct = witness_t<Builder>;
    using witness_ct = witness_t<Builder>;
    using MemStore = merkle_tree::MemoryStore;
    using MerkleTree_ct = merkle_tree::MerkleTree<MemStore>;

    MemStore store;
    auto db = MerkleTree_ct(store, 3);

    // Check that the leaf at index 0 has value 0.
    auto zero = field_ct(witness_ct(&builder, fr::zero())).decompose_into_bits();
    field_ct root = witness_ct(&builder, db.root());

    for (size_t i = 0; i < num_iterations; i++) {
        merkle_tree::check_membership(
            root, merkle_tree::create_witness_hash_path(builder, db.get_hash_path(0)), field_ct(0), zero);
    }
}

/**
 * @brief Benchmark: Construction of a Ultra Honk proof for a circuit determined by the provided text circuit function
 */
void construct_proof_ultra(State& state, void (*test_circuit_function)(Builder&, size_t)) noexcept
{
    barretenberg::srs::init_crs_factory("../srs_db/ignition");
    auto num_iterations = static_cast<size_t>(state.range(0));
    for (auto _ : state) {
        state.PauseTiming();
        auto builder = Builder();
        test_circuit_function(builder, num_iterations);
        auto composer = Composer();
        auto ext_prover = composer.create_prover(builder);
        state.ResumeTiming();

        auto proof = ext_prover.construct_proof();
    }
}

BENCHMARK_CAPTURE(construct_proof_ultra, sha256, &generate_sha256_test_circuit)
    ->DenseRange(MIN_NUM_ITERATIONS, MAX_NUM_ITERATIONS)
    ->Repetitions(NUM_REPETITIONS);
BENCHMARK_CAPTURE(construct_proof_ultra, keccak, &generate_keccak_test_circuit)
    ->DenseRange(MIN_NUM_ITERATIONS, MAX_NUM_ITERATIONS)
    ->Repetitions(NUM_REPETITIONS);
BENCHMARK_CAPTURE(construct_proof_ultra, ecdsa_verification, &generate_ecdsa_verification_test_circuit)
    ->DenseRange(MIN_NUM_ITERATIONS, MAX_NUM_ITERATIONS)
    ->Repetitions(NUM_REPETITIONS);
BENCHMARK_CAPTURE(construct_proof_ultra, merkle_membership, &generate_merkle_membership_test_circuit)
    ->DenseRange(MIN_NUM_ITERATIONS, MAX_NUM_ITERATIONS)
    ->Repetitions(NUM_REPETITIONS);

} // namespace ultra_honk_bench