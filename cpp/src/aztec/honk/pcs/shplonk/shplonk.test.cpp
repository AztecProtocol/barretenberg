#include "shplonk_multi.hpp"
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
template <class Params> class ShplonkTest : public CommitmentTest<Params> {
    using Base = CommitmentTest<Params>;

    using Fr = typename Params::Fr;

    using Commitment = typename Params::Commitment;
    using Polynomial = barretenberg::Polynomial<Fr>;

  public:
    std::vector<Fr> random_opening_set(const size_t m)
    {
        std::vector<Fr> opening_set(m);
        for (size_t i = 0; i < m; ++i) {
            auto x = this->random_element();
            opening_set[i] = x;
        }
        return opening_set;
    }

    void add_random_batch_opening_sub_claims(std::vector<MultiOpeningClaim<Params>>& multi_claims,
                                             std::vector<Polynomial>& polys,
                                             const std::vector<Fr>& queries,
                                             std::span<const size_t> poly_sizes)
    {
        using Opening = typename MultiOpeningClaim<Params>::Opening;
        auto& new_claim = multi_claims.emplace_back(MultiOpeningClaim<Params>{ queries, {} });

        for (const size_t poly_size : poly_sizes) {
            const auto& p = polys.emplace_back(Base::random_polynomial(poly_size));
            const auto& c = Base::commit(p);
            std::vector<Fr> evals;
            for (const auto query : queries) {
                evals.push_back(p.evaluate(query));
            }
            new_claim.openings.emplace_back(Opening{ c, evals });
        }
    };
};

TYPED_TEST_SUITE(ShplonkTest, CommitmentSchemeParams);

TYPED_TEST(ShplonkTest, SinglePolyTwoPoints)
{
    using Shplonk = MultiBatchOpeningScheme<TypeParam>;
    using MultiOpeningClaim = MultiOpeningClaim<TypeParam>;
    using Fr = typename TypeParam::Fr;
    using Polynomial = barretenberg::Polynomial<Fr>;
    constexpr size_t n = 16;
    const size_t log_n = 4;

    auto queries = this->random_opening_set(2);
    std::vector<MultiOpeningClaim> claims;
    std::vector<Polynomial> polys;

    this->add_random_batch_opening_sub_claims(claims, polys, queries, std::array{ n });

    using Transcript = transcript::StandardTranscript;
    auto transcript = std::make_shared<Transcript>(StandardHonk::create_unrolled_manifest(0, log_n));

    transcript->mock_inputs_prior_to_challenge("nu");

    const auto [prover_claim, witness, proof] = Shplonk::reduce_prove(this->ck(), claims, polys, transcript);

    this->verify_opening_claim(prover_claim, witness);
    const auto verifier_claim = Shplonk::reduce_verify(claims, proof, transcript);
    EXPECT_EQ(prover_claim, verifier_claim);
    this->verify_opening_claim(prover_claim, witness);
}

TYPED_TEST(ShplonkTest, TwoPolysDifferentSizeAtTwoDifferentPoints)
{
    using Shplonk = MultiBatchOpeningScheme<TypeParam>;
    using MultiOpeningClaim = MultiOpeningClaim<TypeParam>;
    using Fr = typename TypeParam::Fr;
    using Polynomial = barretenberg::Polynomial<Fr>;
    const size_t n = 16;
    const size_t log_n = 4;

    std::vector<MultiOpeningClaim> claims;
    std::vector<Polynomial> polys;

    auto queries = this->random_opening_set(2);

    this->add_random_batch_opening_sub_claims(claims, polys, { queries[0] }, std::array{ n });
    this->add_random_batch_opening_sub_claims(claims, polys, { queries[1] }, std::array{ n - 1 });

    using Transcript = transcript::StandardTranscript;
    auto transcript = std::make_shared<Transcript>(StandardHonk::create_unrolled_manifest(0, log_n));

    transcript->mock_inputs_prior_to_challenge("nu");

    const auto [prover_claim, witness, proof] = Shplonk::reduce_prove(this->ck(), claims, polys, transcript);

    this->verify_opening_claim(prover_claim, witness);
    const auto verifier_claim = Shplonk::reduce_verify(claims, proof, transcript);
    EXPECT_EQ(prover_claim, verifier_claim);
    this->verify_opening_claim(prover_claim, witness);
}

TYPED_TEST(ShplonkTest, ThreePolysDifferentSizesDifferentQueries)
{
    using Shplonk = MultiBatchOpeningScheme<TypeParam>;
    using MultiOpeningClaim = MultiOpeningClaim<TypeParam>;
    using Fr = typename TypeParam::Fr;
    using Polynomial = barretenberg::Polynomial<Fr>;
    const size_t n = 16;
    const size_t log_n = 4;

    std::vector<MultiOpeningClaim> claims;
    std::vector<Polynomial> polys;

    auto queries = this->random_opening_set(3);

    this->add_random_batch_opening_sub_claims(claims, polys, { queries[0] }, std::array{ n });
    this->add_random_batch_opening_sub_claims(claims, polys, { queries[1], queries[2] }, std::array{ n - 1, n + 2 });
    this->add_random_batch_opening_sub_claims(claims, polys, { queries[0], queries[2] }, std::array{ n });

    using Transcript = transcript::StandardTranscript;
    auto transcript = std::make_shared<Transcript>(StandardHonk::create_unrolled_manifest(0, log_n));

    transcript->mock_inputs_prior_to_challenge("nu");

    const auto [prover_claim, witness, proof] = Shplonk::reduce_prove(this->ck(), claims, polys, transcript);

    this->verify_opening_claim(prover_claim, witness);
    const auto verifier_claim = Shplonk::reduce_verify(claims, proof, transcript);
    EXPECT_EQ(prover_claim, verifier_claim);
    this->verify_opening_claim(prover_claim, witness);
}

// Test of Shplonk prover/verifier using real Gemini claim
TYPED_TEST(ShplonkTest, GeminiModified)
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

    auto gemini_prover_output = Gemini::reduce_prove_modified(this->ck(),
                                                              u,
                                                              multilinear_evals,
                                                              multilinear_evals_shifted,
                                                              multilinear_polynomials,
                                                              multilinear_polynomials_to_be_shifted,
                                                              transcript);

    const auto [prover_opening_pair, shplonk_prover_witness] = Shplonk::reduce_prove_modified(
        this->ck(), gemini_prover_output.opening_pairs, gemini_prover_output.witnesses, transcript);

    this->verify_opening_pair(prover_opening_pair, shplonk_prover_witness);

    // Reconstruct a Gemini proof object consisting of
    // - d Fold poly evaluations a_0, ..., a_{d-1}
    // - (d-1) Fold polynomial commitments [Fold^(1)], ..., [Fold^(d-1)]
    auto gemini_proof = Gemini::reconstruct_proof_from_transcript(transcript, log_n);

    auto gemini_verifier_claim = Gemini::reduce_verify_modified(u,
                                                                multilinear_evals,
                                                                multilinear_evals_shifted,
                                                                multilinear_commitments,
                                                                multilinear_commitments_to_be_shifted,
                                                                gemini_proof,
                                                                transcript);

    // Reconstruct the Shplonk Proof (commitment [Q]) from the transcript
    auto shplonk_proof = transcript->get_group_element("Q");

    const auto verifier_claim = Shplonk::reduce_verify_modified(gemini_verifier_claim, shplonk_proof, transcript);

    this->verify_opening_claim_modified(verifier_claim, shplonk_prover_witness);
}
} // namespace honk::pcs::shplonk