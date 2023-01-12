#include "gemini.hpp"

#include "../commitment_key.test.hpp"
#include <gtest/gtest.h>

namespace honk::pcs::gemini {

template <class Params> class GeminiTranscriptTest : public CommitmentTest<Params> {};

TYPED_TEST_SUITE(GeminiTranscriptTest, CommitmentSchemeParams);

TYPED_TEST(GeminiTranscriptTest, single_with_transcript)
{
    using Gemini = MultilinearReductionScheme<TypeParam>;
    using MLEOpeningClaim = MLEOpeningClaim<TypeParam>;

    const size_t n = 16;
    const size_t m = 4; // = log(n)

    auto u = this->random_evaluation_point(m);
    auto poly = this->random_polynomial(n);
    auto commitment = this->commit(poly);
    auto eval = poly.evaluate_mle(u);

    // create opening claim
    auto claims = { MLEOpeningClaim{ commitment, eval } };

    this->mock_transcript_interactions_up_to_gemini(this->prover_challenges);

    auto [prover_claim, witness, proof] =
        Gemini::reduce_prove_with_transcript(this->ck(), u, claims, {}, { &poly }, {}, this->prover_challenges);

    this->verify_batch_opening_claim(prover_claim, witness);

    auto verifier_claim = Gemini::reduce_verify_with_transcript(u, claims, {}, proof, this->prover_challenges);

    this->verify_batch_opening_claim(verifier_claim, witness);

    EXPECT_EQ(prover_claim, verifier_claim);
}

TYPED_TEST(GeminiTranscriptTest, shift_with_transcript)
{
    using Gemini = MultilinearReductionScheme<TypeParam>;
    using Fr = typename TypeParam::Fr;
    using MLEOpeningClaim = MLEOpeningClaim<TypeParam>;

    const size_t n = 16;
    const size_t m = 4; // = log(n)

    auto u = this->random_evaluation_point(m);

    // shiftable polynomial must have 0 as last coefficient
    auto poly = this->random_polynomial(n);
    poly[0] = Fr::zero();

    auto commitment = this->commit(poly);
    auto eval_shift = poly.evaluate_mle(u, true);

    // create opening claim
    auto claims_shift = {
        MLEOpeningClaim{ commitment, eval_shift },
    };

    this->mock_transcript_interactions_up_to_gemini(this->prover_challenges);

    auto [prover_claim, witness, proof] =
        Gemini::reduce_prove_with_transcript(this->ck(), u, {}, claims_shift, {}, { &poly }, this->prover_challenges);

    this->verify_batch_opening_claim(prover_claim, witness);

    auto verifier_claim = Gemini::reduce_verify_with_transcript(u, {}, claims_shift, proof, this->prover_challenges);

    EXPECT_EQ(prover_claim, verifier_claim);
}

TYPED_TEST(GeminiTranscriptTest, double_with_transcript)
{
    using Gemini = MultilinearReductionScheme<TypeParam>;
    using MLEOpeningClaim = MLEOpeningClaim<TypeParam>;

    const size_t n = 16;
    const size_t m = 4; // = log(n)

    auto u = this->random_evaluation_point(m);

    auto poly1 = this->random_polynomial(n);
    auto poly2 = this->random_polynomial(n);

    auto commitment1 = this->commit(poly1);
    auto commitment2 = this->commit(poly2);

    auto eval1 = poly1.evaluate_mle(u);
    auto eval2 = poly2.evaluate_mle(u);

    const auto claims = {
        MLEOpeningClaim{ commitment1, eval1 },
        MLEOpeningClaim{ commitment2, eval2 },
    };

    this->mock_transcript_interactions_up_to_gemini(this->prover_challenges);

    auto [prover_claim, witness, proof] = Gemini::reduce_prove_with_transcript(
        this->ck(), u, claims, {}, { &poly1, &poly2 }, {}, this->prover_challenges);

    this->verify_batch_opening_claim(prover_claim, witness);

    auto verifier_claim = Gemini::reduce_verify_with_transcript(u, claims, {}, proof, this->prover_challenges);

    this->verify_batch_opening_claim(verifier_claim, witness);
    EXPECT_EQ(prover_claim, verifier_claim);
}

TYPED_TEST(GeminiTranscriptTest, double_shift_with_transcript)
{
    using Gemini = MultilinearReductionScheme<TypeParam>;
    using Fr = typename TypeParam::Fr;
    using MLEOpeningClaim = MLEOpeningClaim<TypeParam>;

    const size_t n = 16;
    const size_t m = 4; // = log(n)

    auto u = this->random_evaluation_point(m);

    auto poly1 = this->random_polynomial(n);
    auto poly2 = this->random_polynomial(n);
    poly2[0] = Fr::zero();

    auto commitment1 = this->commit(poly1);
    auto commitment2 = this->commit(poly2);

    auto eval1 = poly1.evaluate_mle(u);
    auto eval2 = poly2.evaluate_mle(u);
    auto eval2_shift = poly2.evaluate_mle(u, true);

    auto claims = {
        MLEOpeningClaim{ commitment1, eval1 },
        MLEOpeningClaim{ commitment2, eval2 },
    };

    auto claims_shift = {
        MLEOpeningClaim{ commitment2, eval2_shift },
    };

    this->mock_transcript_interactions_up_to_gemini(this->prover_challenges);

    auto [prover_claim, witness, proof] = Gemini::reduce_prove_with_transcript(
        this->ck(), u, claims, claims_shift, { &poly1, &poly2 }, { &poly2 }, this->prover_challenges);

    this->verify_batch_opening_claim(prover_claim, witness);

    auto verifier_claim =
        Gemini::reduce_verify_with_transcript(u, claims, claims_shift, proof, this->prover_challenges);

    ASSERT_EQ(prover_claim, verifier_claim);
}

} // namespace honk::pcs::gemini