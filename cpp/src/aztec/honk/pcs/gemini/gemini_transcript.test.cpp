#include "gemini.hpp"

#include "../commitment_key.test.hpp"
#include <gtest/gtest.h>

namespace honk::pcs::gemini {

template <class Params> class GeminiTranscriptTest : public CommitmentTest<Params> {
    // transcript::Manifest create_mock_manifest(const size_t num_public_inputs)
    // {
    //     // add public inputs....
    //     constexpr size_t g1_size = 64;
    //     constexpr size_t fr_size = 32;
    //     const size_t public_input_size = fr_size * num_public_inputs;
    //     const transcript::Manifest output = transcript::Manifest(
    //         { transcript::Manifest::RoundManifest(
    //             { { "circuit_size", 4, true }, { "public_input_size", 4, true } }, "init", 1),
    //         transcript::Manifest::RoundManifest({ { "public_inputs", public_input_size, false },
    //                                                 { "W_1", g1_size, false },
    //                                                 { "W_2", g1_size, false },
    //                                                 { "W_3", g1_size, false } },
    //                                             "beta",
    //                                             2),
    //         transcript::Manifest::RoundManifest({ { "Z_PERM", g1_size, false } }, "alpha", 1),
    //         transcript::Manifest::RoundManifest(
    //             { { "T_1", g1_size, false }, { "T_2", g1_size, false }, { "T_3", g1_size, false } }, "z", 1),
    //         transcript::Manifest::RoundManifest({ { "w_1", fr_size, false },
    //                                                 { "w_2", fr_size, false },
    //                                                 { "w_3", fr_size, false },
    //                                                 { "w_3_omega", fr_size, false },
    //                                                 { "z_perm_omega", fr_size, false },
    //                                                 { "sigma_1", fr_size, false },
    //                                                 { "sigma_2", fr_size, false },
    //                                                 { "r", fr_size, false },
    //                                                 { "t", fr_size, true } },
    //                                             "nu",
    //                                             10),
    //         transcript::Manifest::RoundManifest(
    //             { { "PI_Z", g1_size, false }, { "PI_Z_OMEGA", g1_size, false } }, "separator", 1) });
    //     return output;
    // }
};

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
        Gemini::reduce_prove(this->ck(), u, claims, {}, { &poly }, {}, this->prover_challenges);

    this->verify_batch_opening_claim(prover_claim, witness);

    auto verifier_claim = Gemini::reduce_verify_with_existing_transcript(u, claims, {}, proof, this->prover_challenges);

    this->verify_batch_opening_claim(verifier_claim, witness);

    EXPECT_EQ(prover_claim, verifier_claim);
}

} // namespace honk::pcs::gemini