// #include "sumcheck.hpp"
// #include "proof_system/flavor/flavor.hpp"
#include "polynomials/polynomial.hpp"
#include "transcript/transcript_wrappers.hpp"
// #include "polynomials/multivariates.hpp"
// #include "relations/arithmetic_relation.hpp"
// #include "relations/grand_product_computation_relation.hpp"
// #include "relations/grand_product_initialization_relation.hpp"
// #include "transcript/manifest.hpp"
// #include <array>
// #include <cstddef>
// #include <cstdint>
// #include <ecc/curves/bn254/fr.hpp>
// #include <gtest/internal/gtest-internal.h>
// #include <numeric/random/engine.hpp>
#include "./gemini/gemini.hpp"
#include "./shplonk/shplonk.hpp"
#include "./kzg/kzg.hpp"
// #include "standard_honk_composer.hpp"
#include <cstddef>
#include <honk/composer/standard_honk_composer.hpp>
#include <honk/proof_system/prover.hpp>

#include <initializer_list>
#include <gtest/gtest.h>
#include <string>
#include <sys/types.h>
#include <vector>

#pragma GCC diagnostic ignored "-Wunused-variable"

using namespace honk;
using namespace honk::pcs;

namespace test_sumcheck_round {

using Transcript = transcript::StandardTranscript;
using FF = barretenberg::fr;
using Polynomial = barretenberg::Polynomial<FF>;

TEST(PolynomialCommitmentScheme, Simple)
{
    auto composer = StandardHonkComposer();

    // 1 + 1 - 2 = 0
    uint32_t w_l_1_idx = composer.circuit_constructor.add_variable(1);
    uint32_t w_r_1_idx = composer.circuit_constructor.add_variable(1);
    uint32_t w_o_1_idx = composer.circuit_constructor.add_variable(2);
    composer.create_add_gate({ w_l_1_idx, w_r_1_idx, w_o_1_idx, 1, 1, -1, 0 });

    // 2 * 2 - 4 = 0
    uint32_t w_l_2_idx = composer.circuit_constructor.add_variable(2);
    uint32_t w_r_2_idx = composer.circuit_constructor.add_variable(2);
    uint32_t w_o_2_idx = composer.circuit_constructor.add_variable(4);
    composer.create_mul_gate({ w_l_2_idx, w_r_2_idx, w_o_2_idx, 1, -1, 0 });

    auto prover = composer.create_unrolled_prover();

    const size_t num_gates = prover.key->n;

    // construct first wire poly explicitly for comparison
    Polynomial w_1_expected(num_gates, num_gates);
    w_1_expected[0] = 0;
    w_1_expected[1] = 1;
    w_1_expected[2] = 2;

    // check that the wire poly in the cache matches the one we've constructed explicitly
    auto w_1_lagrange = prover.key->polynomial_cache.get("w_1_lagrange");
    EXPECT_EQ(w_1_lagrange, w_1_expected);

    // construct proof (i.e. add sumcheck-produced evaluations to transcript)
    waffle::plonk_proof proof = prover.construct_proof();

    // get sumcheck-produced evaluations from transcript
    auto multivariate_evaluations = prover.transcript.get_field_element_vector("multivariate_evaluations");
    auto w_1_eval_sumcheck = multivariate_evaluations[0];

    // get the MLE point
    std::vector<FF> evaluation_point;
    for (size_t i = 0; i < prover.key->log_n; i++) {
        std::string label = "u_" + std::to_string(i + 1);
        info(label);
        evaluation_point.emplace_back(prover.transcript.get_challenge_field_element(label));
    }

    auto w_1_eval_expected = w_1_lagrange.evaluate_mle(evaluation_point);

    EXPECT_EQ(w_1_eval_sumcheck, w_1_eval_expected);
}

} // namespace test_sumcheck_round
