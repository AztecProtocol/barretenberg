
#include "kzg.hpp"
#include "../shplonk/shplonk_single.hpp"

#include "../commitment_key.test.hpp"
#include "barretenberg/proof_system/pcs/claim.hpp"
#include "barretenberg/proof_system/pcs/commitment_key.hpp"
#include "barretenberg/polynomials/polynomial.hpp"

#include "barretenberg/ecc/curves/bn254/g1.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace proof_system::pcs::kzg {

template <class Params> class KZGTest : public CommitmentTest<Params> {
  public:
    using Fr = typename Params::Fr;
    using Commitment = typename Params::Commitment;
    using GroupElement = typename Params::GroupElement;
    using Polynomial = barretenberg::Polynomial<Fr>;
};

TYPED_TEST_SUITE(KZGTest, CommitmentSchemeParams);

TYPED_TEST(KZGTest, single)
{
    const size_t n = 16;

    using KZG = KZG<TypeParam>;
    using Fr = typename TypeParam::Fr;

    auto witness = this->random_polynomial(n);
    barretenberg::g1::element commitment = this->commit(witness);

    auto challenge = Fr::random_element();
    auto evaluation = witness.evaluate(challenge);
    auto opening_pair = OpeningPair<TypeParam>{ challenge, evaluation };
    auto opening_claim = OpeningClaim<TypeParam>{ opening_pair, commitment };

    auto prover_transcript = ProverTranscript<Fr>::init_empty();

    KZG::compute_opening_proof(this->ck(), opening_pair, witness, prover_transcript);

    auto verifier_transcript = VerifierTranscript<Fr>::init_empty(prover_transcript);
    bool verified = KZG::verify(this->vk(), opening_claim, verifier_transcript);

    EXPECT_EQ(verified, true);
}

} // namespace proof_system::pcs::kzg
