#include "sumcheck.hpp"
#include "proof_system/flavor/flavor.hpp"
#include "transcript/transcript_wrappers.hpp"
#include "polynomials/multivariates.hpp"
#include "relations/arithmetic_relation.hpp"
#include "relations/grand_product_computation_relation.hpp"
#include "relations/grand_product_initialization_relation.hpp"
#include "transcript/manifest.hpp"
#include <array>
#include <cstddef>
#include <ecc/curves/bn254/fr.hpp>
#include <gtest/internal/gtest-internal.h>
#include <numeric/random/engine.hpp>

#include <initializer_list>
#include <gtest/gtest.h>

#pragma GCC diagnostic ignored "-Wunused-variable"

using namespace honk;
using namespace honk::sumcheck;

namespace test_sumcheck_round {

using Transcript = transcript::StandardTranscript;
using FF = barretenberg::fr;

TEST(Sumcheck, Prover)
{
    const size_t num_polys(proving_system::StandardArithmetization::NUM_POLYNOMIALS);
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

    FF round_challenge_1 = 1 /* FF::serialize_from_buffer(transcript.get_challenge("alpha").begin()) */;
    std::vector<FF> expected_values;
    for (auto& polynomial : full_polynomials) {
        FF expected = polynomial[1]; // the second value, 2 or 1
                                     // in general, polynomial[0] * (FF(1) - round_challenge_1) + polynomial[1] *
                                     // round_challenge_1;
        expected_values.emplace_back(expected);
    }

    multivariates.fold(multivariates.full_polynomials, multivariate_n, round_challenge_1);

    for (size_t poly_idx = 0; poly_idx < num_polys; poly_idx++) {
        EXPECT_EQ(multivariates.folded_polynomials[poly_idx][0], expected_values[poly_idx]);
    }
    // TODO(Cody) This does not constitute a test.
}

// TEST(Sumcheck, Verifier)
// {
//     GTEST_SKIP_("IDK if this worked before; subsumed by ProverAndVerifier(?).");
//     const size_t num_polys(proving_system::StandardArithmetization::NUM_POLYNOMIALS);
//     const size_t multivariate_d(1);
//     const size_t multivariate_n(1 << multivariate_d);

//     // const size_t max_relation_length = 4;
//     constexpr size_t fr_size = 32;

//     using Multivariates = ::Multivariates<FF, num_polys>;

//     std::vector<transcript::Manifest::RoundManifest> manifest_rounds;
//     manifest_rounds.emplace_back(transcript::Manifest::RoundManifest({ /* this is a noop */ },
//                                                                      /* challenge_name = */ "alpha",
//                                                                      /* num_challenges_in = */ 1));
//     for (size_t i = 0; i < multivariate_d; i++) {
//         auto label = std::to_string(multivariate_d - i);
//         manifest_rounds.emplace_back(transcript::Manifest::RoundManifest(
//             { { .name = "univariate_" + label,
//                 .num_bytes = fr_size * 4 /* honk::StandardHonk::MAX_RELATION_LENGTH */,
//                 .derived_by_verifier = false } },
//             /* challenge_name = */ "u_" + label,
//             /* num_challenges_in = */ 1));
//     }

//     auto transcript = Transcript(transcript::Manifest(manifest_rounds));

//     auto mock_transcript = [](Transcript& transcript) {
//         transcript.add_element("circuit_size", FF(1 << multivariate_d).to_buffer());

//         // Write d-many arbitrary round univariates to the transcript
//         // for (size_t round_idx = 0; round_idx < multivariate_d; round_idx++) {
//         FF value_at_0(0xe85d42e4bd185767, 0xe0c56bb20e0bc4da, 0x40d94d71a3696d50, 0x1213accc78bde576);
//         FF value_at_1(0x62bfc14ae6d0aacd, 0x8a4ffef970383298, 0x5ab9874ba283df25, 0x1ad9ca0db662daed);
//         FF value_at_2(0x21a2f874e3bd72e0, 0x69352740d08eda1e, 0x59204635125a04a2, 0x2934744e9f777e65);
//         FF value_at_3(0x272d6e5c8ced83e1, 0x7da554787bc376ca, 0xddff7645afb39242, 0x17606d7137222b1e);
//         FF value_at_4(0xfd4994239b6fb213, 0x1838c721b1fca51c, 0xfbe98f023a5aed3b, 0x2063143d42ec7cad);
//         value_at_0.self_to_montgomery_form();
//         value_at_1.self_to_montgomery_form();
//         value_at_2.self_to_montgomery_form();
//         value_at_3.self_to_montgomery_form();
//         value_at_4.self_to_montgomery_form();
//         auto round_univariate =
//             Univariate<FF, 4>(std::array<FF, 4>({ value_at_0, value_at_1, value_at_2, value_at_3/* , value_at_4 */
//             }));
//         transcript.add_element("univariate_" + std::to_string(multivariate_d), round_univariate.to_buffer());
//         // transcript.add_element("univariate_" + std::to_string(round_idx), round_univariate.to_buffer());
//         // }

//         transcript.apply_fiat_shamir("u_" + std::to_string(multivariate_d));
//         // Write array of arbitrary multivariate evaluations to trascript
//         std::array<FF, num_polys> multivariate_evaluations;
//         transcript.add_element("multivariate_evaluations", to_buffer(multivariate_evaluations));
//     };

//     transcript.apply_fiat_shamir("alpha");
//     mock_transcript(transcript);

//     auto sumcheck = Sumcheck<Multivariates,
//                              Transcript,
//                              ArithmeticRelation/* ,
//                              GrandProductComputationRelation,
//                              GrandProductInitializationRelation */>(transcript);

//     bool verified = sumcheck.execute_verifier();
//     ASSERT_TRUE(verified);
//     // TODO(Cody) This does not constitute a test.
// }

TEST(Sumcheck, ProverAndVerifier)
{
    const size_t num_polys(proving_system::StandardArithmetization::NUM_POLYNOMIALS);
    const size_t multivariate_d(1);
    const size_t multivariate_n(1 << multivariate_d);
    // const size_t max_relation_length = 5;

    const size_t max_relation_length = 4 /* honk::StandardHonk::MAX_RELATION_LENGTH */;
    constexpr size_t fr_size = 32;

    using Multivariates = ::Multivariates<FF, num_polys>;

    std::array<FF, 2> w_l = { 1, 2 };
    std::array<FF, 2> w_r = { 1, 2 };
    std::array<FF, 2> w_o = { 2, 4 };
    std::array<FF, 2> z_perm = { 1, 0 };       // NOTE: Not set up to be valid.
    std::array<FF, 2> z_perm_shift = { 0, 1 }; // NOTE: Not set up to be valid.
    std::array<FF, 2> q_m = { 0, 1 };
    std::array<FF, 2> q_l = { 1, 0 };
    std::array<FF, 2> q_r = { 1, 0 };
    std::array<FF, 2> q_o = { -1, -1 };
    std::array<FF, 2> q_c = { 0, 0 };
    std::array<FF, 2> sigma_1 = { 1, 2 };        // NOTE: Not set up to be valid.
    std::array<FF, 2> sigma_2 = { 1, 2 };        // NOTE: Not set up to be valid.
    std::array<FF, 2> sigma_3 = { 1, 2 };        // NOTE: Not set up to be valid.
    std::array<FF, 2> id_1 = { 1, 2 };           // NOTE: Not set up to be valid.
    std::array<FF, 2> id_2 = { 1, 2 };           // NOTE: Not set up to be valid.
    std::array<FF, 2> id_3 = { 1, 2 };           // NOTE: Not set up to be valid.
    std::array<FF, 2> lagrange_first = { 1, 0 }; // NOTE: Not set up to be valid.
    std::array<FF, 2> lagrange_last = { 1, 2 };  // NOTE: Not set up to be valid.

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
        transcript.add_element("circuit_size", FF(1 << multivariate_d).to_buffer());
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
    const size_t num_polys(proving_system::StandardArithmetization::NUM_POLYNOMIALS);
    const size_t multivariate_d(2);
    const size_t multivariate_n(1 << multivariate_d);
    // const size_t max_relation_length = 5;

    const size_t max_relation_length = 4 /* honk::StandardHonk::MAX_RELATION_LENGTH */;
    constexpr size_t fr_size = 32;

    using Multivariates = ::Multivariates<FF, num_polys>;

    // clang-format off
    std::array<FF, multivariate_n> w_l            = { 0,  1,  0, 0 };
    std::array<FF, multivariate_n> w_r            = { 0,  1,  0, 0 };
    std::array<FF, multivariate_n> w_o            = { 0,  2,  0, 0 };
    std::array<FF, multivariate_n> z_perm         = { 0,  0,  0, 0 };       // NOTE: Not set up to be valid.
    std::array<FF, multivariate_n> z_perm_shift   = { 0,  0,  0, 0 }; // NOTE: Not set up to be valid.
    std::array<FF, multivariate_n> q_m            = { 0,  0,  0, 0 };
    std::array<FF, multivariate_n> q_l            = { 1,  1,  0, 0 };
    std::array<FF, multivariate_n> q_r            = { 0,  1,  0, 0 };
    std::array<FF, multivariate_n> q_o            = { 0, -1,  0, 0 };
    std::array<FF, multivariate_n> q_c            = { 0,  0,  0, 0 };
    std::array<FF, multivariate_n> sigma_1        = { 0,  0,  0, 0 };        // NOTE: Not set up to be valid.
    std::array<FF, multivariate_n> sigma_2        = { 0,  0,  0, 0 };        // NOTE: Not set up to be valid.
    std::array<FF, multivariate_n> sigma_3        = { 0,  0,  0, 0 };        // NOTE: Not set up to be valid.
    std::array<FF, multivariate_n> id_1           = { 0,  0,  0, 0 };           // NOTE: Not set up to be valid.
    std::array<FF, multivariate_n> id_2           = { 0,  0,  0, 0 };           // NOTE: Not set up to be valid.
    std::array<FF, multivariate_n> id_3           = { 0,  0,  0, 0 };           // NOTE: Not set up to be valid.
    std::array<FF, multivariate_n> lagrange_first = { 0,  0,  0, 0 }; // NOTE: Not set up to be valid.
    std::array<FF, multivariate_n> lagrange_last  = { 0,  0,  0, 0 };  // NOTE: Not set up to be valid.
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
        manifest_rounds.emplace_back(transcript::Manifest::RoundManifest({ { .name = "univariate_" + label,
                                                                             .num_bytes = fr_size * max_relation_length,
                                                                             .derived_by_verifier = false } },
                                                                         /* challenge_name = */ "u_" + label,
                                                                         /* num_challenges_in = */ 1));
    }

    auto transcript = Transcript(transcript::Manifest(manifest_rounds));
    auto mock_transcript = [](Transcript& transcript) {
        transcript.add_element("circuit_size", FF(1 << multivariate_d).to_buffer());
    };

    mock_transcript(transcript);
    transcript.apply_fiat_shamir("alpha");

    auto multivariates = Multivariates(full_polynomials);

    auto sumcheck_prover = Sumcheck<Multivariates,
                                    Transcript,
                                    ArithmeticRelation/* ,
                                    GrandProductComputationRelation,
                                    GrandProductInitializationRelation */>(multivariates, transcript);

    sumcheck_prover.execute_prover();

    auto sumcheck_verifier = Sumcheck<Multivariates,
                                      Transcript,
                                      ArithmeticRelation/* ,
                                      GrandProductComputationRelation,
                                      GrandProductInitializationRelation */>(transcript);

    bool verified = sumcheck_verifier.execute_verifier();
    ASSERT_TRUE(verified);
}

} // namespace test_sumcheck_round
