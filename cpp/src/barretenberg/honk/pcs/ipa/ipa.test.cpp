#include "ipa.hpp"
#include "barretenberg/common/mem.hpp"
#include <gtest/gtest.h>
#include "barretenberg/polynomials/polynomial_arithmetic.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/ecc/curves/bn254/fq12.hpp"
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/honk/pcs/commitment_key.test.hpp"
#include "barretenberg/transcript/manifest.hpp"
using namespace barretenberg;
namespace honk::pcs::ipa {

template <class Params> class IpaCommitmentTest : public CommitmentTest<Params> {
  public:
    using Fr = typename Params::Fr;
    using element = typename Params::Commitment;
    using affine_element = typename Params::C;
    using CK = typename Params::CK;
    using VK = typename Params::VK;
    using Polynomial = barretenberg::Polynomial<Fr>;
};

// Creating mock manifest to test only the IPA PCS

static transcript::Manifest create_mock_manifest(const size_t num_ipa_rounds)
{
    constexpr size_t g1_size = 64;
    constexpr size_t fr_size = 32;
    std::vector<transcript::Manifest::RoundManifest> manifest_rounds;

    std::vector<transcript::Manifest::ManifestEntry> aux_generator_entries;
    aux_generator_entries.emplace_back(transcript::Manifest::ManifestEntry(
        { .name = "ipa_commitment", .num_bytes = g1_size, .derived_by_verifier = false }));
    aux_generator_entries.emplace_back(transcript::Manifest::ManifestEntry(
        { .name = "ipa_challenge_point", .num_bytes = fr_size, .derived_by_verifier = false }));
    aux_generator_entries.emplace_back(transcript::Manifest::ManifestEntry(
        { .name = "ipa_eval", .num_bytes = fr_size, .derived_by_verifier = false }));
    manifest_rounds.emplace_back(transcript::Manifest::RoundManifest(aux_generator_entries,
                                                                     /* challenge_name = */ "ipa_aux",
                                                                     /* num_challenges_in */ 1));
    std::vector<transcript::Manifest::ManifestEntry> ipa_round_challenges_entries;
    for (size_t i = 0; i < num_ipa_rounds; i++) {
        auto label = std::to_string(i);
        ipa_round_challenges_entries.emplace_back(transcript::Manifest::ManifestEntry(
            { .name = "L_" + label, .num_bytes = g1_size, .derived_by_verifier = false }));
        ipa_round_challenges_entries.emplace_back(transcript::Manifest::ManifestEntry(
            { .name = "R_" + label, .num_bytes = g1_size, .derived_by_verifier = false }));
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest(ipa_round_challenges_entries,
                                                                         /* challenge_name = */ "ipa_round_" + label,
                                                                         /* num_challenges_in */ 1));
        ipa_round_challenges_entries.clear();
    };

    auto output = transcript::Manifest(manifest_rounds);
    return output;
}

TYPED_TEST_SUITE(IpaCommitmentTest, IpaCommitmentSchemeParams);

TYPED_TEST(IpaCommitmentTest, commit)
{
    constexpr size_t n = 128;
    auto poly = this->random_polynomial(n);
    barretenberg::g1::element commitment = this->commit(poly);
    auto srs_elements = this->ck()->srs.get_monomial_points();
    barretenberg::g1::element expected = srs_elements[0] * poly[0];
    for (size_t i = 1; i < n; i++) {
        expected += srs_elements[i] * poly[i];
    }
    EXPECT_EQ(expected.normalize(), commitment.normalize());
}

TYPED_TEST(IpaCommitmentTest, open)
{
    using IPA = InnerProductArgument<TypeParam>;
    // generate a random polynomial, degree needs to be a power of two
    size_t n = 128;
    const size_t log_n = static_cast<size_t>(numeric::get_msb(n));
    using Transcript = transcript::StandardTranscript;
    auto transcript = std::make_shared<Transcript>(create_mock_manifest(log_n));
    auto poly = this->random_polynomial(n);
    auto [challenge_point, eval] = this->random_eval(poly);
    barretenberg::g1::element commitment = this->commit(poly);

    auto opening_pair = OpeningPair<TypeParam>{ challenge_point, eval };
    auto opening_claim = OpeningClaim<TypeParam>{ opening_pair, commitment };

    auto proof = IPA::reduce_prove(this->ck(), opening_pair, poly, transcript);
    auto result = IPA::reduce_verify(this->vk(), opening_claim, proof, transcript);
    EXPECT_TRUE(result);
}
/*
 * In the below test, we create a 2d array of Polynomials P[i][j] of dimension t * m.
 * we use t random challenge points (z_1, z_2,..., z_t).
 * The polynomial Q_i to be opened at challenge point z_i is given as
 * Q_i = \sum_{j = 0}^{m-1} \gamma_i^{j} * P[i][j]
 * t = num_rows
 * m = num_polys_per_row
 */
TYPED_TEST(IpaCommitmentTest, batch_open)
{
    using IPA = InnerProductArgument<TypeParam>;
    using Polynomial = barretenberg::Polynomial<Fr>;
    size_t poly_size = 128;
    const size_t log_n = static_cast<size_t>(numeric::get_msb(poly_size));
    using Transcript = transcript::StandardTranscript;
    auto transcript = std::make_shared<Transcript>(create_mock_manifest(log_n));
    size_t num_rows = 5;
    // size_t num_polys_per_row = 5;
    std::vector<Polynomial> polynomials;
    for (size_t i = 0; i < num_rows; ++i) {
        polynomials.emplace_back(this->random_polynomial(poly_size));
    }

    std::vector<Fr> opening_challenges(num_rows);
    for (size_t i = 0; i < num_rows; ++i) {
        opening_challenges[i] = Fr::random_element();
    }
    IPA::test_transfer_poly(num_rows, polynomials, opening_challenges);
    // auto result =
    //     IPA::batch_prove_and_verify(this->ck(), this->vk(), num_rows, polynomials, opening_challenges, transcript);
    // opening_claims = output.first;
    // proofs = output.second;
    // info("before batch_verify");
    // auto result = IPA::batch_verify(this->vk(), num_rows, opening_claims, proofs, transcript);
    // EXPECT_TRUE(result);
}

} // namespace honk::pcs::ipa
