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

using namespace benchmark;

namespace bench_utils {

struct BenchParams {
    // Num iterations of the operation of interest in a test circuit, e.g. num sha256 hashes
    static constexpr size_t MIN_NUM_ITERATIONS = 10;
    static constexpr size_t MAX_NUM_ITERATIONS = 10;

    // Log num gates; for simple circuits only, e.g. standard arithmetic circuit
    static constexpr size_t MIN_LOG_NUM_GATES = 16;
    static constexpr size_t MAX_LOG_NUM_GATES = 16;

    static constexpr size_t NUM_REPETITIONS = 2;
};

/**
 * @brief Generate test circuit with basic arithmetic operations
 *
 * @param composer
 * @param num_iterations
 */
template <typename Composer> void generate_basic_arithmetic_circuit(Composer& composer, size_t num_gates)
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

/**
 * @brief Generate test circuit with specified number of sha256 hashes
 *
 * @param composer
 * @param num_iterations
 */
template <typename Composer> void generate_sha256_test_circuit(Composer& composer, size_t num_iterations)
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

/**
 * @brief Generate test circuit with specified number of keccak hashes
 *
 * @param composer
 * @param num_iterations
 */
template <typename Composer> void generate_keccak_test_circuit(Composer& composer, size_t num_iterations)
{
    std::string in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz01";

    proof_system::plonk::stdlib::byte_array<Composer> input(&composer, in);
    for (size_t i = 0; i < num_iterations; i++) {
        input = proof_system::plonk::stdlib::keccak<Composer>::hash(input);
    }
}

/**
 * @brief Generate test circuit with specified number of ecdsa verifications
 *
 * @param composer
 * @param num_iterations
 */
template <typename Composer> void generate_ecdsa_verification_test_circuit(Composer& composer, size_t num_iterations)
{
    using curve = proof_system::plonk::stdlib::secp256k1<Composer>;
    using fr = typename curve::fr;
    using fq = typename curve::fq;
    using g1 = typename curve::g1;

    std::string message_string = "Instructions unclear, ask again later.";

    crypto::ecdsa::key_pair<fr, g1> account;
    account.private_key = curve::fr::random_element();
    account.public_key = curve::g1::one * account.private_key;

    crypto::ecdsa::signature signature =
        crypto::ecdsa::construct_signature<Sha256Hasher, fq, fr, g1>(message_string, account);

    bool first_result =
        crypto::ecdsa::verify_signature<Sha256Hasher, fq, fr, g1>(message_string, account.public_key, signature);

    std::vector<uint8_t> rr(signature.r.begin(), signature.r.end());
    std::vector<uint8_t> ss(signature.s.begin(), signature.s.end());
    uint8_t vv = signature.v;

    typename curve::g1_bigfr_ct public_key = curve::g1_bigfr_ct::from_witness(&composer, account.public_key);

    proof_system::plonk::stdlib::ecdsa::signature<Composer> sig{ typename curve::byte_array_ct(&composer, rr),
                                                                 typename curve::byte_array_ct(&composer, ss),
                                                                 proof_system::plonk::stdlib::uint8<Composer>(&composer,
                                                                                                              vv) };

    typename curve::byte_array_ct message(&composer, message_string);

    for (size_t i = 0; i < num_iterations; i++) {
        proof_system::plonk::stdlib::ecdsa::verify_signature<Composer,
                                                             curve,
                                                             typename curve::fq_ct,
                                                             typename curve::bigfr_ct,
                                                             typename curve::g1_bigfr_ct>(message, public_key, sig);
    }
}

/**
 * @brief Generate test circuit with specified number of merkle membership checks
 *
 * @param composer
 * @param num_iterations
 * @todo (luke): should we consider deeper tree? non-zero leaf values? variable index?
 */
template <typename Composer> void generate_merkle_membership_test_circuit(Composer& composer, size_t num_iterations)
{
    using namespace proof_system::plonk::stdlib;
    using field_ct = field_t<Composer>;
    using witness_ct = witness_t<Composer>;
    using witness_ct = witness_t<Composer>;
    using MemStore = merkle_tree::MemoryStore;
    using MerkleTree_ct = merkle_tree::MerkleTree<MemStore>;

    MemStore store;
    auto db = MerkleTree_ct(store, 3);

    // Check that the leaf at index 0 has value 0.
    auto zero = field_ct(witness_ct(&composer, fr::zero())).decompose_into_bits();
    field_ct root = witness_ct(&composer, db.root());

    for (size_t i = 0; i < num_iterations; i++) {
        merkle_tree::check_membership(
            root, merkle_tree::create_witness_hash_path(composer, db.get_hash_path(0)), field_ct(0), zero);
    }
}

} // namespace bench_utils