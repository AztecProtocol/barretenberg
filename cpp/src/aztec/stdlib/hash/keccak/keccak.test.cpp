#include "keccak.hpp"
#include <crypto/keccak/keccak.hpp>
#include <gtest/gtest.h>
#include <plonk/composer/ultra_composer.hpp>

using namespace barretenberg;
using namespace plonk;

typedef waffle::UltraComposer Composer;
typedef stdlib::byte_array<Composer> byte_array;
typedef stdlib::public_witness_t<Composer> public_witness_t;

namespace std {
inline std::ostream& operator<<(std::ostream& os, std::vector<uint8_t> const& t)
{
    os << "[ ";
    for (auto e : t) {
        os << std::setfill('0') << std::hex << std::setw(2) << (int)e << " ";
    }
    os << "]";
    return os;
}
} // namespace std

// TODO this should probs b in the crypto module
namespace crypto {
namespace keccak {
std::vector<uint8_t> hash(const std::vector<uint8_t>& data)
{
    auto hash_result = ethash_keccak256(&data[0], data.size());

    std::vector<uint8_t> output;
    output.resize(32);

    memcpy((void*)&output[0], (void*)&hash_result.word64s[0], 32);
    return output;
}
} // namespace keccak
} // namespace crypto

TEST(stdlib_keccak, test_single_block)
{
    Composer composer = Composer();
    std::string input = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz01";
    std::vector<uint8_t> input_v(input.begin(), input.end());

    byte_array input_arr(&composer, input_v);
    byte_array output = stdlib::keccak<Composer>::hash(input_arr);

    std::vector<uint8_t> expected = crypto::keccak::hash(input_v);

    EXPECT_EQ(output.get_value(), expected);

    auto prover = composer.create_prover();

    printf("composer gates = %zu\n", composer.get_num_gates());
    auto verifier = composer.create_verifier();

    auto proof = prover.construct_proof();

    bool proof_result = verifier.verify_proof(proof);
    EXPECT_EQ(proof_result, true);
}

TEST(stdlib_keccak, test_double_block)
{
    Composer composer = Composer();
    std::string input = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
    std::vector<uint8_t> input_v(input.begin(), input.end());

    byte_array input_arr(&composer, input_v);
    byte_array output = stdlib::keccak<Composer>::hash(input_arr);

    std::vector<uint8_t> expected = crypto::keccak::hash(input_v);

    EXPECT_EQ(output.get_value(), expected);

    auto prover = composer.create_prover();
    std::cout << "prover gates = " << prover.n << std::endl;
    printf("composer gates = %zu\n", composer.get_num_gates());
    auto verifier = composer.create_verifier();

    auto proof = prover.construct_proof();

    bool proof_result = verifier.verify_proof(proof);
    EXPECT_EQ(proof_result, true);
}
