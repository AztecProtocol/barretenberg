#include "prover.hpp"
#include "prover_library.hpp"
#include "barretenberg/honk/composer/standard_honk_composer.hpp"
#include "barretenberg/polynomials/polynomial.hpp"

#include "barretenberg/srs/reference_string/file_reference_string.hpp"
#include <array>
#include <string>
#include <vector>
#include <cstddef>
#include <gtest/gtest.h>

using namespace honk;
namespace prover_library_tests {

// field is named Fscalar here because of clash with the Fr
template <class FF> class ProverLibraryTests : public testing::Test {

    using Polynomial = barretenberg::Polynomial<FF>;

  public:
    /**
     * @brief Get a random polynomial
     *
     * @param size
     * @return Polynomial
     */
    static constexpr Polynomial get_random_polynomial(size_t size)
    {
        Polynomial random_polynomial{ size };
        for (auto& coeff : random_polynomial) {
            coeff = FF::random_element();
        }
        return random_polynomial;
    }

    /**
     * @brief Test the correctness of the computation of the permutation grand product polynomial z_permutation
     * @details This test compares a simple, unoptimized, easily readable calculation of the grand product z_permutation
     * to the optimized implementation used by the prover. It's purpose is to provide confidence that some optimization
     * introduced into the calculation has not changed the result.
     * @note This test does confirm the correctness of z_permutation, only that the two implementations yield an
     * identical result.
     */
    static void test_permutation_grand_product_construction()
    {
        // Define some mock inputs for proving key constructor
        static const size_t num_gates = 8;
        static const size_t num_public_inputs = 0;
        auto reference_string = std::make_shared<bonk::FileReferenceString>(num_gates + 1, "../srs_db/ignition");

        // Instatiate a proving_key and make a pointer to it. This will be used to instantiate a Prover.
        auto proving_key = std::make_shared<bonk::proving_key>(
            num_gates, num_public_inputs, reference_string, plonk::ComposerType::STANDARD_HONK);

        static const size_t program_width = StandardProver::settings_::program_width;

        // Construct mock wire and permutation polynomials.
        // Note: for the purpose of checking the consistency between two methods of computing z_perm, these polynomials
        // can simply be random. We're not interested in the particular properties of the result.
        std::vector<Polynomial> wires;
        std::vector<Polynomial> sigmas;
        for (size_t i = 0; i < program_width; ++i) {
            wires.emplace_back(get_random_polynomial(num_gates));
            sigmas.emplace_back(get_random_polynomial(num_gates));

            // Add sigma polys to proving_key; to be used by the prover in constructing it's own z_perm
            std::string sigma_id = "sigma_" + std::to_string(i + 1) + "_lagrange";
            proving_key->polynomial_store.put(sigma_id, Polynomial{ sigmas[i] });
        }

        // Get random challenges
        auto beta = FF::random_element();
        auto gamma = FF::random_element();

        // Method 1: Compute z_perm using 'compute_grand_product_polynomial' as the prover would in practice
        Polynomial z_permutation =
            prover_library::compute_permutation_grand_product<program_width>(proving_key, wires, beta, gamma);

        // Method 2: Compute z_perm locally using the simplest non-optimized syntax possible. The comment below,
        // which describes the computation in 4 steps, is adapted from a similar comment in
        // compute_grand_product_polynomial.
        /*
         * Assume program_width 3. Z_perm may be defined in terms of its values
         * on X_i = 0,1,...,n-1 as Z_perm[0] = 0 and for i = 1:n-1
         *
         *                  (w_1(j) + β⋅id_1(j) + γ) ⋅ (w_2(j) + β⋅id_2(j) + γ) ⋅ (w_3(j) + β⋅id_3(j) + γ)
         * Z_perm[i] = ∏ --------------------------------------------------------------------------------
         *                  (w_1(j) + β⋅σ_1(j) + γ) ⋅ (w_2(j) + β⋅σ_2(j) + γ) ⋅ (w_3(j) + β⋅σ_3(j) + γ)
         *
         * where ∏ := ∏_{j=0:i-1} and id_i(X) = id(X) + n*(i-1). These evaluations are constructed over the
         * course of three steps. For expositional simplicity, write Z_perm[i] as
         *
         *                A_1(j) ⋅ A_2(j) ⋅ A_3(j)
         * Z_perm[i] = ∏ --------------------------
         *                B_1(j) ⋅ B_2(j) ⋅ B_3(j)
         *
         * Step 1) Compute the 2*program_width length-n polynomials A_i and B_i
         * Step 2) Compute the 2*program_width length-n polynomials ∏ A_i(j) and ∏ B_i(j)
         * Step 3) Compute the two length-n polynomials defined by
         *          numer[i] = ∏ A_1(j)⋅A_2(j)⋅A_3(j), and denom[i] = ∏ B_1(j)⋅B_2(j)⋅B_3(j)
         * Step 4) Compute Z_perm[i+1] = numer[i]/denom[i] (recall: Z_perm[0] = 1)
         */

        // Make scratch space for the numerator and denominator accumulators.
        std::array<std::array<FF, num_gates>, program_width> numererator_accum;
        std::array<std::array<FF, num_gates>, program_width> denominator_accum;

        // Step (1)
        for (size_t i = 0; i < proving_key->circuit_size; ++i) {
            for (size_t k = 0; k < program_width; ++k) {
                FF idx = k * proving_key->circuit_size + i;                            // id_k[i]
                numererator_accum[k][i] = wires[k][i] + (idx * beta) + gamma;          // w_k(i) + β.(k*n+i) + γ
                denominator_accum[k][i] = wires[k][i] + (sigmas[k][i] * beta) + gamma; // w_k(i) + β.σ_k(i) + γ
            }
        }

        // Step (2)
        for (size_t k = 0; k < program_width; ++k) {
            for (size_t i = 0; i < proving_key->circuit_size - 1; ++i) {
                numererator_accum[k][i + 1] *= numererator_accum[k][i];
                denominator_accum[k][i + 1] *= denominator_accum[k][i];
            }
        }

        // Step (3)
        for (size_t i = 0; i < proving_key->circuit_size; ++i) {
            for (size_t k = 1; k < program_width; ++k) {
                numererator_accum[0][i] *= numererator_accum[k][i];
                denominator_accum[0][i] *= denominator_accum[k][i];
            }
        }

        // Step (4)
        Polynomial z_permutation_expected(proving_key->circuit_size);
        z_permutation_expected[0] = FF::zero(); // Z_0 = 1
        // Note: in practice, we replace this expensive element-wise division with Montgomery batch inversion
        for (size_t i = 0; i < proving_key->circuit_size - 1; ++i) {
            z_permutation_expected[i + 1] = numererator_accum[0][i] / denominator_accum[0][i];
        }

        // Check consistency between locally computed z_perm and the one computed by the prover library
        EXPECT_EQ(z_permutation, z_permutation_expected);
    };

    /**
     * @brief Test the correctness of the computation of the lookup grand product polynomial z_lookup
     * @details This test compares a simple, unoptimized, easily readable calculation of the grand product z_lookup
     * to the optimized implementation used by the prover. It's purpose is to provide confidence that some optimization
     * introduced into the calculation has not changed the result.
     * @note This test does confirm the correctness of z_lookup, only that the two implementations yield an
     * identical result.
     */
    static void test_lookup_grand_product_construction()
    {
        // Define some mock inputs for proving key constructor
        static const size_t circuit_size = 8;
        static const size_t num_public_inputs = 0;
        auto reference_string = std::make_shared<bonk::FileReferenceString>(circuit_size + 1, "../srs_db/ignition");

        // Instatiate a proving_key and make a pointer to it. This will be used to instantiate a Prover.
        auto proving_key = std::make_shared<bonk::proving_key>(
            circuit_size, num_public_inputs, reference_string, plonk::ComposerType::STANDARD_HONK);

        static const size_t program_width = StandardProver::settings_::program_width;

        // Construct mock wire and permutation polynomials.
        // Note: for the purpose of checking the consistency between two methods of computing z_perm, these polynomials
        // can simply be random. We're not interested in the particular properties of the result.
        std::vector<Polynomial> wires;
        for (size_t i = 0; i < 3; ++i) {
            wires.emplace_back(get_random_polynomial(circuit_size));
        }
        std::vector<Polynomial> tables;
        for (size_t i = 0; i < 4; ++i) {
            tables.emplace_back(get_random_polynomial(circuit_size));
            std::string label = "table_value_" + std::to_string(i + 1) + "_lagrange";
            proving_key->polynomial_store.put(label, Polynomial{ tables[i] });
        }

        auto s_lagrange = get_random_polynomial(circuit_size);
        auto column_1_step_size = get_random_polynomial(circuit_size);
        auto column_2_step_size = get_random_polynomial(circuit_size);
        auto column_3_step_size = get_random_polynomial(circuit_size);
        auto lookup_index_selector = get_random_polynomial(circuit_size);
        auto lookup_selector = get_random_polynomial(circuit_size);

        proving_key->polynomial_store.put("s_lagrange", Polynomial{ s_lagrange });
        proving_key->polynomial_store.put("q_2_lagrange", Polynomial{ column_1_step_size });
        proving_key->polynomial_store.put("q_m_lagrange", Polynomial{ column_2_step_size });
        proving_key->polynomial_store.put("q_c_lagrange", Polynomial{ column_3_step_size });
        proving_key->polynomial_store.put("q_3_lagrange", Polynomial{ lookup_index_selector });
        proving_key->polynomial_store.put("table_type_lagrange", Polynomial{ lookup_selector });

        // Get random challenges
        auto beta = FF::random_element();
        auto gamma = FF::random_element();
        auto eta = FF::random_element();

        // Method 1: Compute z_perm using 'compute_grand_product_polynomial' as the prover would in practice
        Polynomial z_lookup = prover_library::compute_lookup_grand_product<program_width>(
            proving_key, wires, s_lagrange, eta, beta, gamma);

        // ----------------------------------------------------------------

        const Fr beta_constant = beta + Fr(1);                // (1 + β)
        const Fr gamma_beta_constant = gamma * beta_constant; // γ(1 + β)
        const Fr eta_sqr = eta.sqr();
        const Fr eta_cube = eta_sqr * eta;

        std::array<Polynomial, 4> accumulators;
        for (size_t i = 0; i < 4; ++i) {
            accumulators[i] = Polynomial{ circuit_size };
        }

        // Step 1: Compute polynomials f, t and s and incorporate them into terms that are ultimately needed
        // to construct the grand product polynomial Z_lookup(X):
        // Note 1: In what follows, 't' is associated with table values (and is not to be confused with the
        // quotient polynomial, also refered to as 't' elsewhere). Polynomial 's' is the sorted  concatenation
        // of the witnesses and the table values.
        // Note 2: Evaluation at Xω is indicated explicitly, e.g. 'p(Xω)'; evaluation at X is simply omitted, e.g. 'p'
        //
        // 1a.   Compute f, then set accumulators[0] = (q_lookup*f + γ), where
        //
        //         f = (w_1 + q_2*w_1(Xω)) + η(w_2 + q_m*w_2(Xω)) + η²(w_3 + q_c*w_3(Xω)) + η³q_index.
        //      Note that q_2, q_m, and q_c are just the selectors from Standard Plonk that have been repurposed
        //      in the context of the plookup gate to represent 'shift' values. For example, setting each of the
        //      q_* in f to 2^8 facilitates operations on 32-bit values via four operations on 8-bit values. See
        //      Ultra documentation for details.
        //
        // 1b.   Compute t, then set accumulators[1] = (t + βt(Xω) + γ(1 + β)), where t = t_1 + ηt_2 + η²t_3 + η³t_4
        //
        // 1c.   Set accumulators[2] = (1 + β)
        //
        // 1d.   Compute s, then set accumulators[3] = (s + βs(Xω) + γ(1 + β)), where s = s_1 + ηs_2 + η²s_3 + η³s_4
        //

        Fr T0;
        // Note: block_mask is used for efficient modulus, i.e. i % N := i & (N-1), for N = 2^k
        const size_t block_mask = circuit_size - 1;
        // Initialize 't(X)' to be used in an expression of the form t(X) + β*t(Xω)
        Fr next_table = tables[0][0] + tables[1][0] * eta + tables[2][0] * eta_sqr + tables[3][0] * eta_cube;

        for (size_t i = 0; i < circuit_size; ++i) {

            // Compute i'th element of f via Horner (see definition of f above)
            T0 = lookup_index_selector[i];
            T0 *= eta;
            T0 += wires[2][(i + 1) & block_mask] * column_3_step_size[i];
            T0 += wires[2][i];
            T0 *= eta;
            T0 += wires[1][(i + 1) & block_mask] * column_2_step_size[i];
            T0 += wires[1][i];
            T0 *= eta;
            T0 += wires[0][(i + 1) & block_mask] * column_1_step_size[i];
            T0 += wires[0][i];
            T0 *= lookup_selector[i];

            // Set i'th element of polynomial q_lookup*f + γ
            accumulators[0][i] = T0;
            accumulators[0][i] += gamma;

            // Compute i'th element of t via Horner
            T0 = tables[3][(i + 1) & block_mask];
            T0 *= eta;
            T0 += tables[2][(i + 1) & block_mask];
            T0 *= eta;
            T0 += tables[1][(i + 1) & block_mask];
            T0 *= eta;
            T0 += tables[0][(i + 1) & block_mask];

            // Set i'th element of polynomial (t + βt(Xω) + γ(1 + β))
            accumulators[1][i] = T0 * beta + next_table;
            next_table = T0;
            accumulators[1][i] += gamma_beta_constant;

            // Set value of this accumulator to (1 + β)
            accumulators[2][i] = beta_constant;

            // Set i'th element of polynomial (s + βs(Xω) + γ(1 + β))
            accumulators[3][i] = s_lagrange[(i + 1) & block_mask];
            accumulators[3][i] *= beta;
            accumulators[3][i] += s_lagrange[i];
            accumulators[3][i] += gamma_beta_constant;
        }

        // Step 2: Compute the constituent product components of Z_lookup(X).
        // Let ∏ := Prod_{k<j}. Let f_k, t_k and s_k now represent the k'th component of the polynomials f,t and s
        // defined above. We compute the following four product polynomials needed to construct the grand product
        // Z_lookup(X).
        // 1.   accumulators[0][j] = ∏ (q_lookup*f_k + γ)
        // 2.   accumulators[1][j] = ∏ (t_k + βt_{k+1} + γ(1 + β))
        // 3.   accumulators[2][j] = ∏ (1 + β)
        // 4.   accumulators[3][j] = ∏ (s_k + βs_{k+1} + γ(1 + β))
        // Note: This is a small multithreading bottleneck, as we have only 4 parallelizable processes.
        for (auto& accum : accumulators) {
            for (size_t i = 0; i < circuit_size - 1; ++i) {
                accum[i + 1] *= accum[i];
            }
        }

        // Step 3: Combine the accumulator product elements to construct Z_lookup(X).
        //
        //                      ∏ (1 + β) ⋅ ∏ (q_lookup*f_k + γ) ⋅ ∏ (t_k + βt_{k+1} + γ(1 + β))
        //  Z_lookup(g^j) = --------------------------------------------------------------------------
        //                                      ∏ (s_k + βs_{k+1} + γ(1 + β))
        //
        // Note: Montgomery batch inversion is used to efficiently compute the coefficients of Z_lookup
        // rather than peforming n individual inversions. I.e. we first compute the double product P_n:
        //
        // P_n := ∏_{j<n} ∏_{k<j} S_k, where S_k = (s_k + βs_{k+1} + γ(1 + β))
        //
        // and then compute the inverse on P_n. Then we work back to front to obtain terms of the form
        // 1/∏_{k<i} S_i that appear in Z_lookup, using the fact that P_i/P_{i+1} = 1/∏_{k<i} S_i. (Note
        // that once we have 1/P_n, we can compute 1/P_{n-1} as (1/P_n) * ∏_{k<n} S_i, and
        // so on).

        // Compute Z_lookup using Montgomery batch inversion
        // Note: This loop sets the values of z_lookup[i] for i = 1,...,(n-1), (Recall accumulators[0][i] = z_lookup[i +
        // 1])

        // Compute <Z_lookup numerator> * ∏_{j<i}∏_{k<j}S_k
        Fr inversion_accumulator = Fr::one();
        for (size_t i = 0; i < circuit_size - 1; ++i) {
            accumulators[0][i] *= accumulators[2][i];
            accumulators[0][i] *= accumulators[1][i];
            accumulators[0][i] *= inversion_accumulator;
            inversion_accumulator *= accumulators[3][i];
        }
        inversion_accumulator = inversion_accumulator.invert(); // invert

        // Compute [Z_lookup numerator] * ∏_{j<i}∏_{k<j}S_k / ∏_{j<i+1}∏_{k<j}S_k = <Z_lookup numerator> /
        // ∏_{k<i}S_k
        for (size_t i = circuit_size - 2; i != std::numeric_limits<size_t>::max(); --i) {
            // N.B. accumulators[0][i] = z_lookup[i + 1]
            // We can avoid fully reducing z_lookup[i + 1] as the inverse fft will take care of that for us
            accumulators[0][i] *= inversion_accumulator;
            inversion_accumulator *= accumulators[3][i];
        }

        Polynomial z_lookup_expected(circuit_size);
        // Initialize 0th coefficient to 0 to ensure z_perm is left-shiftable via division by X in gemini
        z_lookup_expected[0] = 0;
        barretenberg::polynomial_arithmetic::copy_polynomial(
            accumulators[0].data(), &z_lookup_expected[1], circuit_size - 1, circuit_size - 1);

        EXPECT_EQ(z_lookup, z_lookup_expected);
    };
};

typedef testing::Types<barretenberg::fr> FieldTypes;
TYPED_TEST_SUITE(ProverLibraryTests, FieldTypes);

TYPED_TEST(ProverLibraryTests, PermutationGrandProduct)
{
    TestFixture::test_permutation_grand_product_construction();
}

TYPED_TEST(ProverLibraryTests, LookupGrandProduct)
{
    TestFixture::test_lookup_grand_product_construction();
}

} // namespace prover_library_tests
