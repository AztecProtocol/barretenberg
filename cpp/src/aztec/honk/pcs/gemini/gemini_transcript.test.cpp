#include "gemini.hpp"

#include "../commitment_key.test.hpp"
#include <gtest/gtest.h>

namespace honk::pcs::gemini {

template <class Params> class GeminiTranscriptTest : public CommitmentTest<Params> {};

TYPED_TEST_SUITE(GeminiTranscriptTest, CommitmentSchemeParams);

TYPED_TEST(GeminiTranscriptTest, single_no_oracle)
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

    this->consume(u);

    auto [prover_claim, witness, proof] =
        Gemini::reduce_prove_with_transcript(this->ck(), u, claims, {}, { &poly }, {}, this->prover_challenges);

    this->verify_batch_opening_claim(prover_claim, witness);

    auto verifier_claim = Gemini::reduce_verify_with_transcript(u, claims, {}, proof, this->prover_challenges);

    this->verify_batch_opening_claim(verifier_claim, witness);

    EXPECT_EQ(prover_claim, verifier_claim);
}

} // namespace honk::pcs::gemini