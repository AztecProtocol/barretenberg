#include "gemini.hpp"

#include "barretenberg/proof_system/pcs/commitment_key.test.hpp"
#include "barretenberg/proof_system/transcript/transcript.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/proof_system/pcs/shplonk/shplonk_single.hpp"
#include "barretenberg/proof_system/pcs/kzg/kzg.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <span>

namespace proof_system::honk::pcs::gemini {

template <class Params> class GeminiTest : public CommitmentTest<Params> {
    using Gemini = MultilinearReductionScheme<Params>;
    using Fr = typename Params::Fr;
    using GroupElement = typename Params::GroupElement;
    using Polynomial = typename barretenberg::Polynomial<Fr>;

  public:
    void execute_gemini_and_verify_claims(size_t log_n,
                                          std::vector<Fr> multilinear_evaluation_point,
                                          std::vector<Fr> multilinear_evaluations,
                                          std::vector<std::span<Fr>> multilinear_polynomials,
                                          std::vector<std::span<Fr>> multilinear_polynomials_to_be_shifted,
                                          std::vector<GroupElement> multilinear_commitments,
                                          std::vector<GroupElement> multilinear_commitments_to_be_shifted)
    {
        auto prover_transcript = ProverTranscript<Fr>::init_empty();

        const Fr rho = Fr::random_element();

        std::vector<Fr> rhos = Gemini::powers_of_rho(rho, multilinear_evaluations.size());

        // Compute batched multivariate evaluation
        Fr batched_evaluation = Fr::zero();
        for (size_t i = 0; i < multilinear_evaluations.size(); ++i) {
            batched_evaluation += multilinear_evaluations[i] * rhos[i];
        }

        Polynomial batched_unshifted(1 << log_n);
        Polynomial batched_to_be_shifted(1 << log_n);
        GroupElement batched_commitment_unshifted = GroupElement::zero();
        GroupElement batched_commitment_to_be_shifted = GroupElement::zero();
        const size_t num_unshifted = multilinear_polynomials.size();
        const size_t num_shifted = multilinear_polynomials_to_be_shifted.size();
        for (size_t i = 0; i < num_unshifted; ++i) {
            batched_unshifted.add_scaled(multilinear_polynomials[i], rhos[i]);
            batched_commitment_unshifted += multilinear_commitments[i] * rhos[i];
        }
        for (size_t i = 0; i < num_shifted; ++i) {
            size_t rho_idx = num_unshifted + i;
            batched_to_be_shifted.add_scaled(multilinear_polynomials_to_be_shifted[i], rhos[rho_idx]);
            batched_commitment_to_be_shifted += multilinear_commitments_to_be_shifted[i] * rhos[rho_idx];
        }

        // Compute:
        // - (d+1) opening pairs: {r, \hat{a}_0}, {-r^{2^i}, a_i}, i = 0, ..., d-1
        // - (d+1) Fold polynomials Fold_{r}^(0), Fold_{-r}^(0), and Fold^(i), i = 0, ..., d-1
        auto fold_polynomials = Gemini::compute_fold_polynomials(
            multilinear_evaluation_point, std::move(batched_unshifted), std::move(batched_to_be_shifted));

        for (size_t l = 0; l < log_n - 1; ++l) {
            std::string label = "FOLD_" + std::to_string(l + 1);
            auto commitment = this->ck()->commit(fold_polynomials[l + 2]);
            prover_transcript.send_to_verifier(label, commitment);
        }

        const Fr r_challenge = prover_transcript.get_challenge("Gemini:r");

        auto prover_output = Gemini::compute_fold_polynomial_evaluations(
            multilinear_evaluation_point, std::move(fold_polynomials), r_challenge);

        for (size_t l = 0; l < log_n; ++l) {
            std::string label = "Gemini:a_" + std::to_string(l);
            const auto& evaluation = prover_output.opening_pairs[l + 1].evaluation;
            prover_transcript.send_to_verifier(label, evaluation);
        }

        // Check that the Fold polynomials have been evaluated correctly in the prover
        this->verify_batch_opening_pair(prover_output.opening_pairs, prover_output.witnesses);

        auto verifier_transcript = VerifierTranscript<Fr>::init_empty(prover_transcript);

        // Compute:
        // - Single opening pair: {r, \hat{a}_0}
        // - 2 partially evaluated Fold polynomial commitments [Fold_{r}^(0)] and [Fold_{-r}^(0)]
        // Aggregate: d+1 opening pairs and d+1 Fold poly commitments into verifier claim
        auto verifier_claim = Gemini::reduce_verify(multilinear_evaluation_point,
                                                    batched_evaluation,
                                                    batched_commitment_unshifted,
                                                    batched_commitment_to_be_shifted,
                                                    verifier_transcript);

        // Check equality of the opening pairs computed by prover and verifier
        for (size_t i = 0; i < (log_n + 1); ++i) {
            ASSERT_EQ(prover_output.opening_pairs[i], verifier_claim[i].opening_pair);
        }

        // Explicitly verify the claims computed by the verfier
        this->verify_batch_opening_claim(verifier_claim, prover_output.witnesses);
    }
};

TYPED_TEST_SUITE(GeminiTest, CommitmentSchemeParams);

TYPED_TEST(GeminiTest, Single)
{
    using Fr = typename TypeParam::Fr;
    using GroupElement = typename TypeParam::GroupElement;

    const size_t n = 16;
    const size_t log_n = 4;

    auto u = this->random_evaluation_point(log_n);
    auto poly = this->random_polynomial(n);
    auto commitment = this->commit(poly);
    auto eval = poly.evaluate_mle(u);

    // Collect multilinear polynomials evaluations, and commitments for input to prover/verifier
    std::vector<Fr> multilinear_evaluations = { eval };
    std::vector<std::span<Fr>> multilinear_polynomials = { poly };
    std::vector<std::span<Fr>> multilinear_polynomials_to_be_shifted = {};
    std::vector<GroupElement> multilinear_commitments = { commitment };
    std::vector<GroupElement> multilinear_commitments_to_be_shifted = {};

    this->execute_gemini_and_verify_claims(log_n,
                                           u,
                                           multilinear_evaluations,
                                           multilinear_polynomials,
                                           multilinear_polynomials_to_be_shifted,
                                           multilinear_commitments,
                                           multilinear_commitments_to_be_shifted);
}

TYPED_TEST(GeminiTest, SingleShift)
{
    using Fr = typename TypeParam::Fr;
    using GroupElement = typename TypeParam::GroupElement;

    const size_t n = 16;
    const size_t log_n = 4;

    auto u = this->random_evaluation_point(log_n);

    // shiftable polynomial must have 0 as last coefficient
    auto poly = this->random_polynomial(n);
    poly[0] = Fr::zero();

    auto commitment = this->commit(poly);
    auto eval_shift = poly.evaluate_mle(u, true);

    // Collect multilinear polynomials evaluations, and commitments for input to prover/verifier
    std::vector<Fr> multilinear_evaluations = { eval_shift };
    std::vector<std::span<Fr>> multilinear_polynomials = {};
    std::vector<std::span<Fr>> multilinear_polynomials_to_be_shifted = { poly };
    std::vector<GroupElement> multilinear_commitments = {};
    std::vector<GroupElement> multilinear_commitments_to_be_shifted = { commitment };

    this->execute_gemini_and_verify_claims(log_n,
                                           u,
                                           multilinear_evaluations,
                                           multilinear_polynomials,
                                           multilinear_polynomials_to_be_shifted,
                                           multilinear_commitments,
                                           multilinear_commitments_to_be_shifted);
}

TYPED_TEST(GeminiTest, Double)
{
    using Fr = typename TypeParam::Fr;
    using GroupElement = typename TypeParam::GroupElement;

    const size_t n = 16;
    const size_t log_n = 4;

    auto u = this->random_evaluation_point(log_n);

    auto poly1 = this->random_polynomial(n);
    auto poly2 = this->random_polynomial(n);

    auto commitment1 = this->commit(poly1);
    auto commitment2 = this->commit(poly2);

    auto eval1 = poly1.evaluate_mle(u);
    auto eval2 = poly2.evaluate_mle(u);

    // Collect multilinear polynomials evaluations, and commitments for input to prover/verifier
    std::vector<Fr> multilinear_evaluations = { eval1, eval2 };
    std::vector<std::span<Fr>> multilinear_polynomials = { poly1, poly2 };
    std::vector<std::span<Fr>> multilinear_polynomials_to_be_shifted = {};
    std::vector<GroupElement> multilinear_commitments = { commitment1, commitment2 };
    std::vector<GroupElement> multilinear_commitments_to_be_shifted = {};

    this->execute_gemini_and_verify_claims(log_n,
                                           u,
                                           multilinear_evaluations,
                                           multilinear_polynomials,
                                           multilinear_polynomials_to_be_shifted,
                                           multilinear_commitments,
                                           multilinear_commitments_to_be_shifted);
}

TYPED_TEST(GeminiTest, DoubleWithShift)
{
    // using Gemini = MultilinearReductionScheme<TypeParam>;
    using Fr = typename TypeParam::Fr;
    using GroupElement = typename TypeParam::GroupElement;

    const size_t n = 16;
    const size_t log_n = 4;

    auto u = this->random_evaluation_point(log_n);

    auto poly1 = this->random_polynomial(n);
    auto poly2 = this->random_polynomial(n);
    poly2[0] = Fr::zero(); // necessary for polynomial to be 'shiftable'

    auto commitment1 = this->commit(poly1);
    auto commitment2 = this->commit(poly2);

    auto eval1 = poly1.evaluate_mle(u);
    auto eval2 = poly2.evaluate_mle(u);
    auto eval2_shift = poly2.evaluate_mle(u, true);

    // Collect multilinear polynomials evaluations, and commitments for input to prover/verifier
    std::vector<Fr> multilinear_evaluations = { eval1, eval2, eval2_shift };
    std::vector<std::span<Fr>> multilinear_polynomials = { poly1, poly2 };
    std::vector<std::span<Fr>> multilinear_polynomials_to_be_shifted = { poly2 };
    std::vector<GroupElement> multilinear_commitments = { commitment1, commitment2 };
    std::vector<GroupElement> multilinear_commitments_to_be_shifted = { commitment2 };

    this->execute_gemini_and_verify_claims(log_n,
                                           u,
                                           multilinear_evaluations,
                                           multilinear_polynomials,
                                           multilinear_polynomials_to_be_shifted,
                                           multilinear_commitments,
                                           multilinear_commitments_to_be_shifted);
}

/**
 * @brief Test full PCS protocol: Gemini, Shplonk, KZG and pairing check
 * @details Demonstrates the full PCS protocol as it is used in the construction and verification
 * of a single Honk proof. (Expository comments included throughout).
 *
 */
TYPED_TEST(GeminiTest, GeminiShplonkKzgWithShift)
{
    using Shplonk = shplonk::SingleBatchOpeningScheme<TypeParam>;
    using Gemini = gemini::MultilinearReductionScheme<TypeParam>;
    using KZG = kzg::KZG<TypeParam>;
    using Fr = typename TypeParam::Fr;
    using GroupElement = typename TypeParam::GroupElement;
    using Polynomial = typename barretenberg::Polynomial<Fr>;

    const size_t n = 16;
    const size_t log_n = 4;

    Fr rho = Fr::random_element();

    // Generate multilinear polynomials, their commitments (genuine and mocked) and evaluations (genuine) at a random
    // point.
    const auto mle_opening_point = this->random_evaluation_point(log_n); // sometimes denoted 'u'
    auto poly1 = this->random_polynomial(n);
    auto poly2 = this->random_polynomial(n);
    poly2[0] = Fr::zero(); // this property is required of polynomials whose shift is used

    GroupElement commitment1 = this->commit(poly1);
    GroupElement commitment2 = this->commit(poly2);

    auto eval1 = poly1.evaluate_mle(mle_opening_point);
    auto eval2 = poly2.evaluate_mle(mle_opening_point);
    auto eval2_shift = poly2.evaluate_mle(mle_opening_point, true);

    // Collect multilinear evaluations for input to prover
    std::vector<Fr> multilinear_evaluations = { eval1, eval2, eval2_shift };

    std::vector<Fr> rhos = Gemini::powers_of_rho(rho, multilinear_evaluations.size());

    // Compute batched multivariate evaluation
    Fr batched_evaluation = Fr::zero();
    for (size_t i = 0; i < rhos.size(); ++i) {
        batched_evaluation += multilinear_evaluations[i] * rhos[i];
    }

    // Compute batched polynomials
    Polynomial batched_unshifted(n);
    Polynomial batched_to_be_shifted(n);
    batched_unshifted.add_scaled(poly1, rhos[0]);
    batched_unshifted.add_scaled(poly2, rhos[1]);
    batched_to_be_shifted.add_scaled(poly2, rhos[2]);

    // Compute batched commitments
    GroupElement batched_commitment_unshifted = GroupElement::zero();
    GroupElement batched_commitment_to_be_shifted = GroupElement::zero();
    batched_commitment_unshifted = commitment1 * rhos[0] + commitment2 * rhos[1];
    batched_commitment_to_be_shifted = commitment2 * rhos[2];

    auto prover_transcript = ProverTranscript<Fr>::init_empty();

    // Run the full prover PCS protocol:

    // Compute:
    // - (d+1) opening pairs: {r, \hat{a}_0}, {-r^{2^i}, a_i}, i = 0, ..., d-1
    // - (d+1) Fold polynomials Fold_{r}^(0), Fold_{-r}^(0), and Fold^(i), i = 0, ..., d-1
    auto fold_polynomials = Gemini::compute_fold_polynomials(
        mle_opening_point, std::move(batched_unshifted), std::move(batched_to_be_shifted));

    for (size_t l = 0; l < log_n - 1; ++l) {
        std::string label = "FOLD_" + std::to_string(l + 1);
        auto commitment = this->ck()->commit(fold_polynomials[l + 2]);
        prover_transcript.send_to_verifier(label, commitment);
    }

    const Fr r_challenge = prover_transcript.get_challenge("Gemini:r");

    const auto [gemini_opening_pairs, gemini_witnesses] =
        Gemini::compute_fold_polynomial_evaluations(mle_opening_point, std::move(fold_polynomials), r_challenge);

    for (size_t l = 0; l < log_n; ++l) {
        std::string label = "Gemini:a_" + std::to_string(l);
        const auto& evaluation = gemini_opening_pairs[l + 1].evaluation;
        prover_transcript.send_to_verifier(label, evaluation);
    }

    // Shplonk prover output:
    // - opening pair: (z_challenge, 0)
    // - witness: polynomial Q - Q_z
    const Fr nu_challenge = prover_transcript.get_challenge("Shplonk:nu");
    auto batched_quotient_Q = Shplonk::compute_batched_quotient(gemini_opening_pairs, gemini_witnesses, nu_challenge);
    prover_transcript.send_to_verifier("Shplonk:Q", this->ck()->commit(batched_quotient_Q));

    const Fr z_challenge = prover_transcript.get_challenge("Shplonk:z");
    const auto [shplonk_opening_pair, shplonk_witness] = Shplonk::compute_partially_evaluated_batched_quotient(
        gemini_opening_pairs, gemini_witnesses, std::move(batched_quotient_Q), nu_challenge, z_challenge);

    // KZG prover:
    // - Adds commitment [W] to transcript
    KZG::compute_opening_proof(this->ck(), shplonk_opening_pair, shplonk_witness, prover_transcript);

    // Run the full verifier PCS protocol with genuine opening claims (genuine commitment, genuine evaluation)

    auto verifier_transcript = VerifierTranscript<Fr>::init_empty(prover_transcript);

    // Gemini verifier output:
    // - claim: d+1 commitments to Fold_{r}^(0), Fold_{-r}^(0), Fold^(l), d+1 evaluations a_0_pos, a_l, l = 0:d-1
    auto gemini_verifier_claim = Gemini::reduce_verify(mle_opening_point,
                                                       batched_evaluation,
                                                       batched_commitment_unshifted,
                                                       batched_commitment_to_be_shifted,
                                                       verifier_transcript);

    // Shplonk verifier claim: commitment [Q] - [Q_z], opening point (z_challenge, 0)
    const auto shplonk_verifier_claim = Shplonk::reduce_verify(gemini_verifier_claim, verifier_transcript);

    // KZG verifier:
    // aggregates inputs [Q] - [Q_z] and [W] into an 'accumulator' (can perform pairing check on result)
    bool verified = KZG::verify(this->vk(), shplonk_verifier_claim, verifier_transcript);

    // Final pairing check: e([Q] - [Q_z] + z[W], [1]_2) = e([W], [x]_2)

    EXPECT_EQ(verified, true);
}

} // namespace proof_system::honk::pcs::gemini
