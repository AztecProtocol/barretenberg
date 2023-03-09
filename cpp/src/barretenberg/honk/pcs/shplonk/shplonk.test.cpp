#include "shplonk_single.hpp"
#include "../gemini/gemini.hpp"

#include <gtest/internal/gtest-internal.h>
#include <random>
#include <iterator>
#include <algorithm>
#include <vector>

#include "../commitment_key.test.hpp"
#include "barretenberg/honk/pcs/claim.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
namespace honk::pcs::shplonk {
template <class Params> class ShplonkTest : public CommitmentTest<Params> {};

TYPED_TEST_SUITE(ShplonkTest, CommitmentSchemeParams);

// Test of Shplonk prover/verifier for two polynomials of different size, each opened at a single (different) point
TYPED_TEST(ShplonkTest, ShplonkSimple)
{
    using Shplonk = SingleBatchOpeningScheme<TypeParam>;
    using Fr = typename TypeParam::Fr;
    using Polynomial = typename barretenberg::Polynomial<Fr>;
    using OpeningPair = OpeningPair<TypeParam>;
    using OpeningClaim = OpeningClaim<TypeParam>;

    const size_t n = 16;
    const size_t log_n = 4;

    using Transcript = transcript::StandardTranscript;
    auto transcript = std::make_shared<Transcript>(StandardHonk::create_manifest(0, log_n));
    transcript->mock_inputs_prior_to_challenge("nu");

    // Generate two random (unrelated) polynomials of two different sizes, as well as their evaluations at a (single but
    // different) random point and their commitments.
    const auto r1 = Fr::random_element();
    auto poly1 = this->random_polynomial(n);
    const auto eval1 = poly1.evaluate(r1);
    const auto commitment1 = this->commit(poly1);

    const auto r2 = Fr::random_element();
    auto poly2 = this->random_polynomial(n / 2);
    const auto eval2 = poly2.evaluate(r2);
    const auto commitment2 = this->commit(poly2);

    // Aggregate polynomials and their opening pairs
    std::vector<OpeningPair> opening_pairs = { { r1, eval1 }, { r2, eval2 } };
    std::vector<Polynomial> polynomials = { poly1, poly2 };

    // Execute the shplonk prover functionality
    const auto [prover_opening_pair, shplonk_prover_witness] =
        Shplonk::reduce_prove(this->ck(), opening_pairs, polynomials, transcript);

    // An intermediate check to confirm the opening of the shplonk prover witness Q
    this->verify_opening_pair(prover_opening_pair, shplonk_prover_witness);

    // Aggregate polynomial commitments and their opening pairs
    std::vector<OpeningClaim> opening_claims;
    opening_claims.emplace_back(OpeningClaim{ opening_pairs[0], commitment1 });
    opening_claims.emplace_back(OpeningClaim{ opening_pairs[1], commitment2 });

    // Reconstruct the Shplonk Proof (commitment [Q]) from the transcript
    auto shplonk_proof = transcript->get_group_element("Q");

    // Execute the shplonk verifier functionality
    const auto verifier_claim = Shplonk::reduce_verify(opening_claims, shplonk_proof, transcript);

    this->verify_opening_claim(verifier_claim, shplonk_prover_witness);
}
} // namespace honk::pcs::shplonk
