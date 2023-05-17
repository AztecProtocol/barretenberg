#include "ipa.hpp"
#include "barretenberg/common/mem.hpp"
#include <gtest/gtest.h>
#include "barretenberg/ecc/curves/types.hpp"
#include "barretenberg/polynomials/polynomial_arithmetic.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/ecc/curves/bn254/fq12.hpp"
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/honk/pcs/commitment_key.test.hpp"
using namespace barretenberg;
namespace proof_system::honk::pcs::ipa {

class IPATests : public CommitmentTest<Params> {
  public:
    using Fr = typename Params::Fr;
    using element = typename Params::GroupElement;
    using affine_element = typename Params::Commitment;
    using CK = typename Params::CommitmentKey;
    using VK = typename Params::VerificationKey;
    using Polynomial = barretenberg::Polynomial<Fr>;
};

TEST_F(IPATests, Commit)
{
    constexpr size_t n = 4;
    auto poly = this->random_polynomial(n);
    barretenberg::g1::element commitment = this->commit(poly);
    auto srs_elements = this->ck()->srs.get_monomial_points();
    barretenberg::g1::element expected = srs_elements[0] * poly[0];
    // The SRS stored in the commitment key is the result after applying the pippenger point table so the
    // values at odd indices contain the point {srs[i-1].x * beta, srs[i-1].y}, where beta is the endomorphism
    // G_vec_local should use only the original SRS thus we extract only the even indices.
    for (size_t i = 2; i < 2 * n; i += 2) {
        expected += g1::element(srs_elements[i]) * poly[i >> 1];
    }
    EXPECT_EQ(expected.normalize(), commitment.normalize());
}

TEST_F(IPATests, Open)
{
    using IPA = IPA<Params>;
    // generate a random polynomial, degree needs to be a power of two
    size_t n = 128;
    auto poly = this->random_polynomial(n);
    auto [x, eval] = this->random_eval(poly);
    auto commitment = this->commit(poly);
    const OpeningPair<Params> opening_pair = { x, eval };
    const OpeningClaim<Params> opening_claim{ opening_pair, commitment };

    // initialize empty prover transcript
    ProverTranscript<Fr> prover_transcript;
    IPA::compute_opening_proof(this->ck(), opening_pair, poly, prover_transcript);

    // initialize verifier transcript from proof data
    VerifierTranscript<Fr> verifier_transcript{ prover_transcript.proof_data };

    auto result = IPA::verify(this->vk(), opening_claim, verifier_transcript);
    EXPECT_TRUE(result);

    EXPECT_EQ(prover_transcript.get_manifest(), verifier_transcript.get_manifest());
}
} // namespace proof_system::honk::pcs::ipa
