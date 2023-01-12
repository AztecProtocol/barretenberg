#pragma once

#include <cstddef>
#include <gtest/gtest.h>

#include <concepts>
#include <algorithm>
#include <string_view>

#include <polynomials/polynomial.hpp>
#include <srs/reference_string/file_reference_string.hpp>
#include <ecc/curves/bn254/g1.hpp>

#include "../oracle/oracle.hpp"
#include "../../transcript/transcript_wrappers.hpp"
#include "../../proof_system/flavor/flavor.hpp"
#include "../../honk/sumcheck/polynomials/univariate.hpp"

#include "claim.hpp"
#include "commitment_key.hpp"
#include "transcript/manifest.hpp"

namespace honk::pcs {
namespace {
constexpr std::string_view kzg_srs_path = "../srs_db/ignition";
}

template <class CK> inline CK* CreateCommitmentKey();

template <> inline kzg::CommitmentKey* CreateCommitmentKey<kzg::CommitmentKey>()
{
    const size_t n = 128;
    return new kzg::CommitmentKey(n, kzg_srs_path);
}

template <typename CK> inline CK* CreateCommitmentKey()
// requires std::default_initializable<CK>
{
    return new CK();
}
template <class VK> inline VK* CreateVerificationKey();

template <> inline kzg::VerificationKey* CreateVerificationKey<kzg::VerificationKey>()
{
    return new kzg::VerificationKey(kzg_srs_path);
}

template <typename VK> inline VK* CreateVerificationKey()
// requires std::default_initializable<VK>
{
    return new VK();
}

template <typename Params> class CommitmentTest : public ::testing::Test {
    using CK = typename Params::CK;
    using VK = typename Params::VK;

    using Fr = typename Params::Fr;
    using Commitment = typename Params::Commitment;
    using Polynomial = typename Params::Polynomial;
    using Transcript = transcript::StandardTranscript;

  public:
    CommitmentTest()
        : prover_transcript{ honk::StandardHonk::create_unrolled_manifest(0, 1) }
        , verifier_transcript{ honk::StandardHonk::create_unrolled_manifest(0, 1) }
        , prover_challenges{ &prover_transcript }
        , verifier_challenges{ &verifier_transcript }
        , engine{ &numeric::random::get_debug_engine() }
    {}

    CK* ck() { return commitment_key; }
    VK* vk() { return verification_key; }

    Commitment commit(const Polynomial& polynomial) { return commitment_key->commit(polynomial); }

    Polynomial random_polynomial(const size_t n)
    {
        Polynomial p(n, n);
        for (size_t i = 0; i < n; ++i) {
            p[i] = Fr::random_element(engine);
        }
        return p;
    }

    Fr random_element() { return Fr::random_element(engine); }

    std::pair<Fr, Fr> random_eval(const Polynomial& polynomial)
    {
        Fr x{ random_element() };
        Fr y{ polynomial.evaluate(x) };
        return { x, y };
    }

    std::pair<OpeningClaim<Params>, Polynomial> random_claim(const size_t n)
    {
        auto p = random_polynomial(n);
        auto [x, y] = random_eval(p);
        auto c = commit(p);
        return { { c, x, y }, p };
    };

    std::vector<Fr> random_evaluation_point(const size_t num_variables)
    {
        std::vector<Fr> u(num_variables);
        for (size_t l = 0; l < num_variables; ++l) {
            u[l] = random_element();
        }
        return u;
    }

    void verify_opening_claim(const OpeningClaim<Params>& claim, const Polynomial& witness)
    {
        auto& [c, x, y] = claim;
        Fr y_expected = witness.evaluate(x);
        EXPECT_EQ(y, y_expected) << "OpeningClaim: evaluations mismatch";
        Commitment c_expected = commit(witness);
        EXPECT_EQ(c, c_expected) << "OpeningClaim: commitment mismatch";
    }

    /**
     * @brief Ensures that a 'BatchOpeningClaim' is correct by checking that
     * - all evaluations are correct by recomputing them from each witness polynomial.
     * - commitments are correct by recomputing a commitment from each witness polynomial.
     * - each 'queries' is a subset of 'all_queries' and 'all_queries' is the union of all 'queries'
     * - each 'commitment' of each 'SubClaim' appears only once.
     */
    void verify_batch_opening_claim(std::span<const MultiOpeningClaim<Params>> multi_claims,
                                    std::span<const Polynomial> witnesses)
    {
        size_t idx = 0;

        for (const auto& [queries, openings] : multi_claims) {
            const size_t num_queries = queries.size();

            for (const auto& [commitment, evals] : openings) {
                // compare commitment against recomputed commitment from witness
                Commitment commitment_expected = commit(witnesses[idx]);
                EXPECT_EQ(commitment, commitment_expected)
                    << "BatchOpeningClaim idx=" << idx << ": commitment mismatch";
                EXPECT_EQ(evals.size(), num_queries)
                    << "BatchOpeningClaim idx=" << idx << ": evaluation/query size mismatch";

                // check evaluations for each point in queries
                for (size_t i = 0; i < num_queries; ++i) {

                    // check evaluation
                    Fr eval_expected = witnesses[idx].evaluate(queries[i]);
                    EXPECT_EQ(evals[i], eval_expected)
                        << "BatchOpeningClaim idx=" << idx << ": evaluation " << i << " mismatch";
                }

                ++idx;
            }
        }
    }

    /**
     * @brief Ensures that a 'BatchOpeningClaim' is correct by checking that
     * - all evaluations are correct by recomputing them from each witness polynomial.
     * - commitments are correct by recomputing a commitment from each witness polynomial.
     * - each 'queries' is a subset of 'all_queries' and 'all_queries' is the union of all 'queries'
     * - each 'commitment' of each 'SubClaim' appears only once.
     */
    void verify_batch_opening_claim(std::span<const OpeningClaim<Params>> multi_claims,
                                    std::span<const Polynomial> witnesses)
    {
        const size_t num_claims = multi_claims.size();
        ASSERT_EQ(witnesses.size(), num_claims);

        for (size_t j = 0; j < num_claims; ++j) {
            this->verify_opening_claim(multi_claims[j], witnesses[j]);
        }
    }

    template <typename... T> void consume(const T&... args)
    {
        (prover_challenges.consume(args), ...);
        (verifier_challenges.consume(args), ...);
    }

    // Mock all prover transcript interactions up to the Gemini round.
    // TODO(luke): it might be useful for Transcript to own a method like this, e.g.
    // mock_interactions_up_to_challenge(name). It would use its manifest to directly perform these operations. That
    // would avoid the need to hard code the manifest details in a function like this.
    static void mock_transcript_interactions_up_to_gemini(auto& transcript, size_t log_n)
    {
        // Mock the rounds preceding Gemini
        const size_t LENGTH = honk::StandardHonk::MAX_RELATION_LENGTH;
        using Univariate = honk::sumcheck::Univariate<Fr, LENGTH>;

        // Define mock data to be added to transcript
        std::vector<uint8_t> g1_buf(64);
        std::vector<uint8_t> fr_buf(32);
        std::array<Fr, LENGTH> evaluations;
        for (size_t i = 0; i < g1_buf.size(); ++i) {
            g1_buf[i] = 1;
        }
        for (size_t i = 0; i < fr_buf.size(); ++i) {
            fr_buf[i] = 1;
        }
        for (size_t i = 0; i < LENGTH; ++i) {
            evaluations[i] = Fr::random_element();
        }

        transcript->add_element("circuit_size", { 1, 2, 3, 4 });
        transcript->add_element("public_input_size", { 0, 0, 0, 0 });

        transcript->apply_fiat_shamir("init");

        transcript->apply_fiat_shamir("eta");

        // Mock wire commitments
        transcript->add_element("public_inputs", {});
        transcript->add_element("W_1", g1_buf);
        transcript->add_element("W_2", g1_buf);
        transcript->add_element("W_3", g1_buf);
        transcript->apply_fiat_shamir("beta");

        // Mock permutation grand product commitment
        transcript->add_element("Z_PERM", g1_buf);
        transcript->apply_fiat_shamir("alpha");

        // Mock sumcheck prover interactions
        auto univariate = Univariate(evaluations);
        for (size_t round_idx = 0; round_idx < log_n; round_idx++) {
            transcript->add_element("univariate_" + std::to_string(log_n - round_idx), univariate.to_buffer());
            transcript->apply_fiat_shamir("u_" + std::to_string(log_n - round_idx));
        }
        transcript->add_element("w_1", fr_buf);
        transcript->add_element("w_2", fr_buf);
        transcript->add_element("w_3", fr_buf);
        transcript->add_element("sigma_1", fr_buf);
        transcript->add_element("sigma_2", fr_buf);
        transcript->add_element("sigma_3", fr_buf);
        transcript->add_element("q_1", fr_buf);
        transcript->add_element("q_2", fr_buf);
        transcript->add_element("q_3", fr_buf);
        transcript->add_element("q_m", fr_buf);
        transcript->add_element("q_c", fr_buf);
        transcript->add_element("z_perm", fr_buf);
        transcript->add_element("z_perm_omega", fr_buf);
    }

    // Mock all prover transcript interactions up to the Gemini round.
    static void mock_transcript_interactions_up_to_shplonk(auto& transcript, size_t log_n)
    {
        mock_transcript_interactions_up_to_gemini(transcript, log_n);

        transcript->apply_fiat_shamir("rho");
        for (size_t round_idx = 1; round_idx < log_n; round_idx++) {
            transcript->add_element("FOLD_" + std::to_string(round_idx), barretenberg::g1::affine_one.to_buffer());
        }

        transcript->apply_fiat_shamir("r");
        for (size_t round_idx = 0; round_idx < log_n; round_idx++) {
            transcript->add_element("a_" + std::to_string(round_idx), barretenberg::fr(round_idx + 1).to_buffer());
        }
    }

    // Transcript<Fr> prover_transcript;
    // Transcript<Fr> verifier_transcript;
    // Oracle<Transcript<Fr>> prover_challenges;
    // Oracle<Transcript<Fr>> verifier_challenges;
    Transcript prover_transcript;
    Transcript verifier_transcript;
    Oracle<Transcript> prover_challenges;
    Oracle<Transcript> verifier_challenges;
    numeric::random::Engine* engine;

    // Per-test-suite set-up.
    // Called before the first test in this test suite.
    // Can be omitted if not needed.
    static void SetUpTestSuite()
    {
        // Avoid reallocating static objects if called in subclasses of FooTest.
        if (commitment_key == nullptr) {
            commitment_key = CreateCommitmentKey<CK>();
        }
        if (verification_key == nullptr) {
            verification_key = CreateVerificationKey<VK>();
        }
    }

    // Per-test-suite tear-down.
    // Called after the last test in this test suite.
    // Can be omitted if not needed.
    static void TearDownTestSuite()
    {
        delete commitment_key;
        commitment_key = nullptr;
        delete verification_key;
        verification_key = nullptr;
    }

    static typename Params::CK* commitment_key;
    static typename Params::VK* verification_key;
};

template <typename Params> typename Params::CK* CommitmentTest<Params>::commitment_key = nullptr;
template <typename Params> typename Params::VK* CommitmentTest<Params>::verification_key = nullptr;

// using CommitmentSchemeParams =
//     ::testing::Types<fake::Params<barretenberg::g1>, fake::Params<grumpkin::g1>, kzg::Params>;
using CommitmentSchemeParams = ::testing::Types<kzg::Params>;

} // namespace honk::pcs