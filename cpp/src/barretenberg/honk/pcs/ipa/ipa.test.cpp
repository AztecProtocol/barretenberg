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
    barretenberg::g1::element expected = g1::element(srs_elements[0]) * poly[0];
    for (size_t i = 1; i < 2 * n; i += 2) {
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

TEST_F(IPATests, pippenger_unsafe_short_inputs)
{
    constexpr size_t num_points = 128;

    auto scalars = this->random_polynomial(num_points);

    // g1::affine_element* points =
    //     (g1::affine_element*)aligned_alloc(32, sizeof(g1::affine_element) * num_points * 2 + 1);

    auto points = this->ck()->srs.get_monomial_points();
    info(this->ck()->srs.get_monomial_size());

    g1::element expected;
    expected.self_set_infinity();
    for (size_t i = 0; i < num_points; ++i) {
        g1::element temp = points[i] * scalars[i];
        expected += temp;
    }
    expected = expected.normalize();
    scalar_multiplication::generate_pippenger_point_table(points, points, num_points);
    scalar_multiplication::pippenger_runtime_state state(num_points);

    g1::element result =
        scalar_multiplication::pippenger_unsafe(const_cast<Fr*>(scalars.data()), points, num_points, state);
    result = result.normalize();

    EXPECT_EQ(result == expected, true);
}
} // namespace proof_system::honk::pcs::ipa
