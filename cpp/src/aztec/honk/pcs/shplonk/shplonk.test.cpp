#include "shplonk_single.hpp"
#include "../gemini/gemini.hpp"

#include <gtest/internal/gtest-internal.h>
#include <random>
#include <iterator>
#include <algorithm>

#include "../commitment_key.test.hpp"
#include "honk/pcs/claim.hpp"
#include "polynomials/polynomial.hpp"
namespace honk::pcs::shplonk {
template <class Params> class ShplonkTest : public CommitmentTest<Params> {};

TYPED_TEST_SUITE(ShplonkTest, CommitmentSchemeParams);

// Test of Shplonk prover/verifier using real Gemini claim
TYPED_TEST(ShplonkTest, GeminiShplonk)
{
    using Shplonk = SingleBatchOpeningScheme<TypeParam>;
    using Gemini = gemini::MultilinearReductionScheme<TypeParam>;
    using Fr = typename TypeParam::Fr;
    using Commitment = typename TypeParam::Commitment;

    const size_t n = 16;
    const size_t log_n = 4;

    using Transcript = transcript::StandardTranscript;
    auto transcript = std::make_shared<Transcript>(StandardHonk::create_unrolled_manifest(0, log_n));
    transcript->mock_inputs_prior_to_challenge("rho");

    const auto u = this->random_evaluation_point(log_n);
    auto poly = this->random_polynomial(n);
    const auto commitment = this->commit(poly);
    const auto eval = poly.evaluate_mle(u);

    // Collect multilinear polynomials evaluations, and commitments for input to prover/verifier
    std::vector<Fr> multilinear_evals = { eval };
    std::vector<Fr> multilinear_evals_shifted = {};
    std::vector<std::span<Fr>> multilinear_polynomials = { poly };
    std::vector<std::span<Fr>> multilinear_polynomials_to_be_shifted = {};
    std::vector<Commitment> multilinear_commitments = { commitment };
    std::vector<Commitment> multilinear_commitments_to_be_shifted = {};

    auto gemini_prover_output = Gemini::reduce_prove(this->ck(),
                                                     u,
                                                     multilinear_evals,
                                                     multilinear_evals_shifted,
                                                     multilinear_polynomials,
                                                     multilinear_polynomials_to_be_shifted,
                                                     transcript);

    const auto [prover_opening_pair, shplonk_prover_witness] = Shplonk::reduce_prove(
        this->ck(), gemini_prover_output.opening_pairs, gemini_prover_output.witnesses, transcript);

    this->verify_opening_pair(prover_opening_pair, shplonk_prover_witness);

    // Reconstruct a Gemini proof object consisting of
    // - d Fold poly evaluations a_0, ..., a_{d-1}
    // - (d-1) Fold polynomial commitments [Fold^(1)], ..., [Fold^(d-1)]
    auto gemini_proof = Gemini::reconstruct_proof_from_transcript(transcript, log_n);

    auto gemini_verifier_claim = Gemini::reduce_verify(u,
                                                       multilinear_evals,
                                                       multilinear_evals_shifted,
                                                       multilinear_commitments,
                                                       multilinear_commitments_to_be_shifted,
                                                       gemini_proof,
                                                       transcript);

    // Reconstruct the Shplonk Proof (commitment [Q]) from the transcript
    auto shplonk_proof = transcript->get_group_element("Q");

    const auto verifier_claim = Shplonk::reduce_verify(gemini_verifier_claim, shplonk_proof, transcript);

    this->verify_opening_claim(verifier_claim, shplonk_prover_witness);
}
} // namespace honk::pcs::shplonk