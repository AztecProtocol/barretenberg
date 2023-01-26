#include "sumcheck.hpp"
#include "polynomials/polynomial.hpp"
#include "proof_system/flavor/flavor.hpp"
#include "transcript/transcript_wrappers.hpp"
#include "polynomials/multivariates.hpp"
#include "relations/arithmetic_relation.hpp"
#include "relations/grand_product_computation_relation.hpp"
#include "relations/grand_product_initialization_relation.hpp"
#include "transcript/manifest.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <ecc/curves/bn254/fr.hpp>
#include <gtest/internal/gtest-internal.h>
#include <numeric/random/engine.hpp>

#include <initializer_list>
#include <gtest/gtest.h>
#include <string>
#include <sys/types.h>
#include <vector>

#pragma GCC diagnostic ignored "-Wunused-variable"

using namespace honk;
using namespace honk::sumcheck;

namespace test_sumcheck_round {

using Transcript = transcript::StandardTranscript;
using FF = barretenberg::fr;

TEST(Sumcheck, Prover)
{
    const size_t num_polys(bonk::StandardArithmetization::NUM_POLYNOMIALS);
    const size_t multivariate_d(1);
    const size_t multivariate_n(1 << multivariate_d);

    // const size_t max_relation_length = 4;
    constexpr size_t fr_size = 32;

    using Multivariates = ::Multivariates<FF, num_polys>;

    std::array<FF, 2> w_l = { 1, 2 };
    std::array<FF, 2> w_r = { 1, 2 };
    std::array<FF, 2> w_o = { 1, 2 };
    std::array<FF, 2> z_perm = { 1, 2 };
    std::array<FF, 2> z_perm_shift = { 0, 1 };
    std::array<FF, 2> q_m = { 1, 2 };
    std::array<FF, 2> q_l = { 1, 2 };
    std::array<FF, 2> q_r = { 1, 2 };
    std::array<FF, 2> q_o = { 1, 2 };
    std::array<FF, 2> q_c = { 1, 2 };
    std::array<FF, 2> sigma_1 = { 1, 2 };
    std::array<FF, 2> sigma_2 = { 1, 2 };
    std::array<FF, 2> sigma_3 = { 1, 2 };
    std::array<FF, 2> id_1 = { 1, 2 };
    std::array<FF, 2> id_2 = { 1, 2 };
    std::array<FF, 2> id_3 = { 1, 2 };
    std::array<FF, 2> lagrange_first = { 1, 2 };
    std::array<FF, 2> lagrange_last = { 1, 2 };

    // These will be owned outside the class, probably by the composer.
    std::array<std::span<FF>, Multivariates::num> full_polynomials = {
        w_l,     w_r,  w_o,  z_perm, z_perm_shift,   q_m,          q_l, q_r, q_o, q_c, sigma_1, sigma_2,
        sigma_3, id_1, id_2, id_3,   lagrange_first, lagrange_last
    };

    std::vector<transcript::Manifest::RoundManifest> manifest_rounds;
    manifest_rounds.emplace_back(transcript::Manifest::RoundManifest({ /* this is a noop */ },
                                                                     /* challenge_name = */ "alpha",
                                                                     /* num_challenges_in = */ 1));
    for (size_t i = 0; i < multivariate_d; i++) {
        auto label = std::to_string(multivariate_d - i);
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest(
            { { .name = "univariate_" + label,
                .num_bytes = fr_size * 5 /* honk::StandardHonk::MAX_RELATION_LENGTH */,
                .derived_by_verifier = false } },
            /* challenge_name = */ "u_" + label,
            /* num_challenges_in = */ 1));
    }

    auto transcript = Transcript(transcript::Manifest(manifest_rounds));
    // transcript.mock_inputs_prior_to_challenge("alpha");
    transcript.apply_fiat_shamir("alpha");

    auto multivariates = Multivariates(full_polynomials);

    auto sumcheck = Sumcheck<Multivariates,
                             Transcript,
                             ArithmeticRelation,
                             GrandProductComputationRelation,
                             GrandProductInitializationRelation>(multivariates, transcript);

    sumcheck.execute_prover();

    // FF round_challenge_1 = 1 /* FF::serialize_from_buffer(transcript.get_challenge("alpha").begin()) */;
    FF u_1 = transcript.get_challenge_field_element("u_1");
    std::vector<FF> expected_values;
    for (auto& polynomial : full_polynomials) {
        FF expected = polynomial[0] * (FF(1) - u_1) + polynomial[1] * u_1;
        expected_values.emplace_back(expected);
    }

    // The 'multivariates' local to this test is not getting modified by sumcheck so somewhere there
    // is a copy or something. But if we pull the sumcheck-produced multivariate evals out of the transcript
    // directly (i.e. how its done in practice) then the test passes.
    auto sumcheck_evaluations = transcript.get_field_element_vector("multivariate_evaluations");
    for (size_t poly_idx = 0; poly_idx < num_polys; poly_idx++) {

        info("poly_idx = ", poly_idx);
        EXPECT_EQ(sumcheck_evaluations[poly_idx], expected_values[poly_idx]);
    }
    // TODO(Cody) Improve this test.
}

// TODO(Cody): write standalone test of the verifier.

TEST(Sumcheck, ProverAndVerifier)
{
    const size_t num_polys(bonk::StandardArithmetization::NUM_POLYNOMIALS);
    const size_t multivariate_d(1);
    const size_t multivariate_n(1 << multivariate_d);
    // const size_t max_relation_length = 5;

    const size_t max_relation_length = 4 /* honk::StandardHonk::MAX_RELATION_LENGTH */;
    constexpr size_t fr_size = 32;

    using Multivariates = ::Multivariates<FF, num_polys>;

    std::array<FF, 2> w_l = { 1, 2 };
    std::array<FF, 2> w_r = { 1, 2 };
    std::array<FF, 2> w_o = { 2, 4 };
    std::array<FF, 2> z_perm = { 0, 1 };
    std::array<FF, 2> z_perm_shift = { 1, 0 }; // NOTE: Not set up to be valid.
    std::array<FF, 2> q_m = { 0, 1 };
    std::array<FF, 2> q_l = { 1, 0 };
    std::array<FF, 2> q_r = { 1, 0 };
    std::array<FF, 2> q_o = { -1, -1 };
    std::array<FF, 2> q_c = { 0, 0 };
    std::array<FF, 2> sigma_1 = { 1, 2 }; // NOTE: Not set up to be valid.
    std::array<FF, 2> sigma_2 = { 1, 2 }; // NOTE: Not set up to be valid.
    std::array<FF, 2> sigma_3 = { 1, 2 }; // NOTE: Not set up to be valid.
    std::array<FF, 2> id_1 = { 1, 2 };    // NOTE: Not set up to be valid.
    std::array<FF, 2> id_2 = { 1, 2 };    // NOTE: Not set up to be valid.
    std::array<FF, 2> id_3 = { 1, 2 };    // NOTE: Not set up to be valid.
    std::array<FF, 2> lagrange_first = { 1, 0 };
    std::array<FF, 2> lagrange_last = { 0, 1 }; // NOTE: Not set up to be valid.

    // These will be owned outside the class, probably by the composer.
    std::array<std::span<FF>, Multivariates::num> full_polynomials = {
        w_l,     w_r,  w_o,  z_perm, z_perm_shift,   q_m,          q_l, q_r, q_o, q_c, sigma_1, sigma_2,
        sigma_3, id_1, id_2, id_3,   lagrange_first, lagrange_last
    };

    std::vector<transcript::Manifest::RoundManifest> manifest_rounds;
    manifest_rounds.emplace_back(transcript::Manifest::RoundManifest({ /* this is a noop */ },
                                                                     /* challenge_name = */ "alpha",
                                                                     /* num_challenges_in = */ 1));
    for (size_t i = 0; i < multivariate_d; i++) {
        auto label = std::to_string(multivariate_d - i);
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest({ { .name = "univariate_" + label,
                                                                             .num_bytes = fr_size * max_relation_length,
                                                                             .derived_by_verifier = false } },
                                                                         /* challenge_name = */ "u_" + label,
                                                                         /* num_challenges_in = */ 1));
    }

    auto transcript = Transcript(transcript::Manifest(manifest_rounds));
    auto mock_transcript = [](Transcript& transcript) {
        static_assert(multivariate_d < 64);
        uint64_t multivariate_n = 1 << multivariate_d;
        transcript.add_element("circuit_size",
                               { static_cast<uint8_t>(multivariate_n >> 24),
                                 static_cast<uint8_t>(multivariate_n >> 16),
                                 static_cast<uint8_t>(multivariate_n >> 8),
                                 static_cast<uint8_t>(multivariate_n) });
    };

    mock_transcript(transcript);
    transcript.apply_fiat_shamir("alpha");

    auto multivariates = Multivariates(full_polynomials);

    auto sumcheck_prover = Sumcheck<Multivariates,
                                    Transcript,
                                    ArithmeticRelation,
                                    // GrandProductComputationRelation,
                                    GrandProductInitializationRelation>(multivariates, transcript);

    sumcheck_prover.execute_prover();

    auto sumcheck_verifier = Sumcheck<Multivariates,
                                      Transcript,
                                      ArithmeticRelation,
                                      //   GrandProductComputationRelation,
                                      GrandProductInitializationRelation>(transcript);

    bool verified = sumcheck_verifier.execute_verifier();
    ASSERT_TRUE(verified);
}

TEST(Sumcheck, ProverAndVerifierLonger)
{
    auto run_test = [](bool expect_verified) {
        const size_t num_polys(bonk::StandardArithmetization::NUM_POLYNOMIALS);
        const size_t multivariate_d(2);
        const size_t multivariate_n(1 << multivariate_d);
        // const size_t max_relation_length = 5;

        const size_t max_relation_length = 4 /* honk::StandardHonk::MAX_RELATION_LENGTH */;
        constexpr size_t fr_size = 32;

        using Multivariates = ::Multivariates<FF, num_polys>;

        // clang-format off
    std::array<FF, multivariate_n> w_l;
    if (expect_verified) {         w_l =            { 0,  1,  0, 0 };
    } else {                       w_l =            { 0,  0,  0, 0 };
    }
    std::array<FF, multivariate_n> w_r            = { 0,  1,  0, 0 };
    std::array<FF, multivariate_n> w_o            = { 0,  2,  0, 0 };
    std::array<FF, multivariate_n> z_perm         = { 0,  0,  0, 0 }; 
    std::array<FF, multivariate_n> z_perm_shift   = { 0,  0,  0, 0 }; 
    std::array<FF, multivariate_n> q_m            = { 0,  0,  0, 0 };
    std::array<FF, multivariate_n> q_l            = { 1,  1,  0, 0 };
    std::array<FF, multivariate_n> q_r            = { 0,  1,  0, 0 };
    std::array<FF, multivariate_n> q_o            = { 0, -1,  0, 0 };
    std::array<FF, multivariate_n> q_c            = { 0,  0,  0, 0 };
    std::array<FF, multivariate_n> sigma_1        = { 0,  0,  0, 0 };
    std::array<FF, multivariate_n> sigma_2        = { 0,  0,  0, 0 };
    std::array<FF, multivariate_n> sigma_3        = { 0,  0,  0, 0 };
    std::array<FF, multivariate_n> id_1           = { 0,  0,  0, 0 };
    std::array<FF, multivariate_n> id_2           = { 0,  0,  0, 0 };
    std::array<FF, multivariate_n> id_3           = { 0,  0,  0, 0 };
    std::array<FF, multivariate_n> lagrange_first = { 0,  0,  0, 0 };
    std::array<FF, multivariate_n> lagrange_last  = { 0,  0,  0, 0 };
        // clang-format on

        // These will be owned outside the class, probably by the composer.
        std::array<std::span<FF>, Multivariates::num> full_polynomials = {
            w_l,     w_r,  w_o,  z_perm, z_perm_shift,   q_m,          q_l, q_r, q_o, q_c, sigma_1, sigma_2,
            sigma_3, id_1, id_2, id_3,   lagrange_first, lagrange_last
        };

        std::vector<transcript::Manifest::RoundManifest> manifest_rounds;
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest({ /* this is a noop */ },
                                                                         /* challenge_name = */ "alpha",
                                                                         /* num_challenges_in = */ 1));
        for (size_t i = 0; i < multivariate_d; i++) {
            auto label = std::to_string(multivariate_d - i);
            manifest_rounds.emplace_back(
                transcript::Manifest::RoundManifest({ { .name = "univariate_" + label,
                                                        .num_bytes = fr_size * max_relation_length,
                                                        .derived_by_verifier = false } },
                                                    /* challenge_name = */ "u_" + label,
                                                    /* num_challenges_in = */ 1));
        }

        auto transcript = Transcript(transcript::Manifest(manifest_rounds));
        auto mock_transcript = [](Transcript& transcript) {
            static_assert(multivariate_d < 64);
            uint64_t multivariate_n = 1 << multivariate_d;
            transcript.add_element("circuit_size",
                                   { static_cast<uint8_t>(multivariate_n >> 24),
                                     static_cast<uint8_t>(multivariate_n >> 16),
                                     static_cast<uint8_t>(multivariate_n >> 8),
                                     static_cast<uint8_t>(multivariate_n) });
        };

        mock_transcript(transcript);
        transcript.apply_fiat_shamir("alpha");

        auto multivariates = Multivariates(full_polynomials);

        auto sumcheck_prover = Sumcheck<Multivariates,
                                        Transcript,
                                        ArithmeticRelation,
                                        // GrandProductComputationRelation,
                                        GrandProductInitializationRelation>(multivariates, transcript);

        sumcheck_prover.execute_prover();

        auto sumcheck_verifier = Sumcheck<Multivariates,
                                          Transcript,
                                          ArithmeticRelation,
                                          //   GrandProductComputationRelation,
                                          GrandProductInitializationRelation>(transcript);

        bool verified = sumcheck_verifier.execute_verifier();
        EXPECT_EQ(verified, expect_verified);
    };

    run_test(/* expect_verified=*/true);
    run_test(/* expect_verified=*/false);
}

TEST(Sumcheck, ProverDebug)
{
    const size_t num_polys(bonk::StandardArithmetization::NUM_POLYNOMIALS);
    const size_t multivariate_d(2);
    const size_t multivariate_n(1 << multivariate_d);

    // const size_t max_relation_length = 4;
    constexpr size_t fr_size = 32;

    using Multivariates = ::Multivariates<FF, num_polys>;

    std::array<FF, multivariate_n> w_l = { 1, 2, 0, 0 };
    // std::array<FF, multivariate_n> w_l = { 0, 0, 2, 1 };
    std::array<FF, multivariate_n> w_r = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> w_o = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> z_perm = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> z_perm_shift = { 0, 1, 0, 0 };
    std::array<FF, multivariate_n> q_m = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> q_l = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> q_r = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> q_o = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> q_c = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> sigma_1 = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> sigma_2 = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> sigma_3 = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> id_1 = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> id_2 = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> id_3 = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> lagrange_first = { 1, 2, 0, 0 };
    std::array<FF, multivariate_n> lagrange_last = { 1, 2, 0, 0 };

    // These will be owned outside the class, probably by the composer.
    std::array<std::span<FF>, Multivariates::num> full_polynomials = {
        w_l,     w_r,  w_o,  z_perm, z_perm_shift,   q_m,          q_l, q_r, q_o, q_c, sigma_1, sigma_2,
        sigma_3, id_1, id_2, id_3,   lagrange_first, lagrange_last
    };

    std::vector<transcript::Manifest::RoundManifest> manifest_rounds;
    manifest_rounds.emplace_back(transcript::Manifest::RoundManifest({ /* this is a noop */ },
                                                                     /* challenge_name = */ "alpha",
                                                                     /* num_challenges_in = */ 1));
    for (size_t i = 0; i < multivariate_d; i++) {
        auto label = std::to_string(multivariate_d - i);
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest(
            { { .name = "univariate_" + label,
                .num_bytes = fr_size * 5 /* honk::StandardHonk::MAX_RELATION_LENGTH */,
                .derived_by_verifier = false } },
            /* challenge_name = */ "u_" + label,
            /* num_challenges_in = */ 1));
    }

    auto transcript = Transcript(transcript::Manifest(manifest_rounds));
    // transcript.mock_inputs_prior_to_challenge("alpha");
    transcript.apply_fiat_shamir("alpha");

    auto multivariates = Multivariates(full_polynomials);

    auto sumcheck = Sumcheck<Multivariates,
                             Transcript,
                             ArithmeticRelation,
                             GrandProductComputationRelation,
                             GrandProductInitializationRelation>(multivariates, transcript);

    sumcheck.execute_prover();

    // FF round_challenge_1 = 1 /* FF::serialize_from_buffer(transcript.get_challenge("alpha").begin()) */;
    FF u_1 = transcript.get_challenge_field_element("u_1");
    FF u_2 = transcript.get_challenge_field_element("u_2");
    std::vector<FF> expected_values;
    for (auto& polynomial : full_polynomials) {
        FF expected = polynomial[0] * (FF(1) - u_2) + polynomial[1] * u_2;
        expected *= (FF(1) - u_1);
        expected_values.emplace_back(expected);
    }

    barretenberg::polynomial w_1(multivariate_n, multivariate_n);
    w_1[0] = 1;
    w_1[1] = 2;
    w_1[2] = 0;
    w_1[3] = 0;
    // std::vector<FF> u_vect{u_1, u_2};
    std::vector<FF> u_vect{ u_2, u_1 };

    auto w_1_eval_expected = w_1.evaluate_mle(u_vect);

    // The 'multivariates' local to this test is not getting modified by sumcheck so somewhere there
    // is a copy or something. But if we pull the sumcheck-produced multivariate evals out of the transcript
    // directly (i.e. how its done in practice) then the test passes.
    auto sumcheck_evaluations = transcript.get_field_element_vector("multivariate_evaluations");
    for (size_t poly_idx = 0; poly_idx < num_polys; poly_idx++) {

        info("poly_idx = ", poly_idx);
        EXPECT_EQ(sumcheck_evaluations[poly_idx], expected_values[poly_idx]);
    }
    // TODO(Cody) Improve this test.

    auto w_1_eval_sumcheck = sumcheck_evaluations[0];
    EXPECT_EQ(w_1_eval_sumcheck, w_1_eval_expected);
}

} // namespace test_sumcheck_round
