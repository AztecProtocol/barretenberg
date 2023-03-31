#include "barretenberg/proof_system/flavor/flavor.hpp"
#include <cstddef>
#include <gtest/gtest.h>

#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wunused-variable"

namespace test_flavor {
TEST(Flavor, Standard)
{
    using Flavor = proof_system::honk::flavor::Standard;
    using FF = Flavor::FF;
    Flavor::ProvingKey proving_key;
    Flavor::VerificationKey verification_key;
    Flavor::ProverPolynomials prover_polynomials;
    Flavor::VerifierCommitments verifier_commitments;
    Flavor::ExtendedEdges edges;
    Flavor::PurportedEvaluations evals;
    Flavor::CommitmentLabels commitment_labels;

    EXPECT_EQ(prover_polynomials.size(), 18);
    EXPECT_EQ(prover_polynomials.size(), prover_polynomials.get_in_order().size());
    EXPECT_EQ(commitment_labels.w_l, "w_l");

    auto get_test_polynomial = [](size_t& idx) {
        Flavor::Polynomial poly(4);
        for (size_t i = 0; i < 4; i++) {
            poly[i] = idx++;
        };
        return poly;
    };

    size_t idx = 0;
    auto w_l = get_test_polynomial(idx);
    auto w_r = get_test_polynomial(idx);
    auto w_o = get_test_polynomial(idx);
    auto z_perm = get_test_polynomial(idx);
    auto z_perm_shift = get_test_polynomial(idx);
    auto q_m = get_test_polynomial(idx);
    auto q_l = get_test_polynomial(idx);
    auto q_r = get_test_polynomial(idx);
    auto q_o = get_test_polynomial(idx);
    auto q_c = get_test_polynomial(idx);
    auto sigma_1 = get_test_polynomial(idx);
    auto sigma_2 = get_test_polynomial(idx);
    auto sigma_3 = get_test_polynomial(idx);
    auto id_1 = get_test_polynomial(idx);
    auto id_2 = get_test_polynomial(idx);
    auto id_3 = get_test_polynomial(idx);
    auto lagrange_first = get_test_polynomial(idx);
    auto lagrange_last = get_test_polynomial(idx);

    prover_polynomials.w_l = w_l;
    prover_polynomials.w_r = w_r;
    prover_polynomials.w_o = w_o;
    prover_polynomials.z_perm = z_perm;
    prover_polynomials.z_perm_shift = z_perm_shift;
    prover_polynomials.q_m = q_m;
    prover_polynomials.q_l = q_l;
    prover_polynomials.q_r = q_r;
    prover_polynomials.q_o = q_o;
    prover_polynomials.q_c = q_c;
    prover_polynomials.sigma_1 = sigma_1;
    prover_polynomials.sigma_2 = sigma_2;
    prover_polynomials.sigma_3 = sigma_3;
    prover_polynomials.id_1 = id_1;
    prover_polynomials.id_2 = id_2;
    prover_polynomials.id_3 = id_3;
    prover_polynomials.lagrange_first = lagrange_first;
    prover_polynomials.lagrange_last = lagrange_last;

    idx = 0;
    for (auto& poly : prover_polynomials) {
        EXPECT_EQ(poly[0], 4 * idx);
        EXPECT_EQ(poly[1], 4 * idx + 1);
        EXPECT_EQ(poly[2], 4 * idx + 2);
        EXPECT_EQ(poly[3], 4 * idx + 3);
        idx++;
    };
}

} // namespace test_flavor
