#include "transcript.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/honk/sumcheck/polynomials/univariate.hpp"
#include "barretenberg/numeric/bitop/get_msb.hpp"
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <string>

using namespace proof_system;

/**
 * @brief Test and demonstrate the basic functionality of the prover and verifier transcript
 *
 */
TEST(TranscriptTests, ProverAndVerifierBasic)
{
    constexpr size_t LENGTH = 8;

    using Fr = barretenberg::fr;
    using Univariate = std::array<Fr, LENGTH>;
    using Commitment = barretenberg::g1::affine_element;

    std::array<Fr, LENGTH> evaluations;
    for (auto& eval : evaluations) {
        eval = Fr::random_element();
    }

    // Add some junk to the transcript and compute challenges
    uint32_t data = 25;
    auto scalar = Fr::random_element();
    auto commitment = Commitment::one();
    auto univariate = Univariate(evaluations);

    // Instantiate a prover transcript and mock an example protocol
    ProverTranscript<Fr> prover_transcript;

    // round 0
    prover_transcript.send_to_verifier("data", data);
    Fr alpha = prover_transcript.get_challenge("alpha");

    // round 1
    prover_transcript.send_to_verifier("scalar", scalar);
    prover_transcript.send_to_verifier("commitment", commitment);
    Fr beta = prover_transcript.get_challenge("beta");

    // round 2
    prover_transcript.send_to_verifier("univariate", univariate);
    auto [gamma, delta] = prover_transcript.get_challenges("gamma", "delta");

    // Instantiate a verifier transcript from the raw bytes of the prover transcript; receive data and generate
    // challenges according to the example protocol
    VerifierTranscript<Fr> verifier_transcript(prover_transcript.proof_data);

    // round 0
    auto data_received = verifier_transcript.template receive_from_prover<uint32_t>("data");
    Fr verifier_alpha = verifier_transcript.get_challenge("alpha");

    // round 1
    auto scalar_received = verifier_transcript.template receive_from_prover<Fr>("scalar");
    auto commitment_received = verifier_transcript.template receive_from_prover<Commitment>("commitment");
    Fr verifier_beta = verifier_transcript.get_challenge("beta");

    // round 2
    auto univariate_received = verifier_transcript.template receive_from_prover<Univariate>("univariate");
    auto [verifier_gamma, verifier_delta] = verifier_transcript.get_challenges("gamma", "delta");

    // Check the correctness of the elements received by the verifier
    EXPECT_EQ(data_received, data);
    EXPECT_EQ(scalar_received, scalar);
    EXPECT_EQ(commitment_received, commitment);
    EXPECT_EQ(univariate_received, univariate);

    // Check consistency of prover and verifier challenges
    EXPECT_EQ(alpha, verifier_alpha);
    EXPECT_EQ(beta, verifier_beta);
    EXPECT_EQ(gamma, verifier_gamma);
    EXPECT_EQ(delta, verifier_delta);

    // Check consistency of the generated manifests
    EXPECT_EQ(prover_transcript.get_manifest(), verifier_transcript.get_manifest());
}

/**
 * @brief Demonstrate extent to which verifier transcript is flexible / constrained
 *
 */
TEST(TranscriptTests, VerifierMistake)
{
    using Fr = barretenberg::fr;

    auto scalar_1 = Fr::random_element();
    auto scalar_2 = Fr::random_element();

    ProverTranscript<Fr> prover_transcript;

    prover_transcript.send_to_verifier("scalar1", scalar_1);
    prover_transcript.send_to_verifier("scalar2", scalar_2);
    auto prover_alpha = prover_transcript.get_challenge("alpha");

    VerifierTranscript<Fr> verifier_transcript(prover_transcript.proof_data);

    verifier_transcript.template receive_from_prover<Fr>("scalar1");
    // accidentally skip receipt of "scalar2"...
    // but then generate a challenge anyway
    auto verifier_alpha = verifier_transcript.get_challenge("alpha");

    // Challenges will not agree but neither will the manifests
    EXPECT_NE(prover_alpha, verifier_alpha);
    EXPECT_NE(prover_transcript.get_manifest(), verifier_transcript.get_manifest());
}
