#include "prover_library.hpp"

namespace honk::prover_library {

using Fr = barretenberg::fr;
using Polynomial = barretenberg::Polynomial<Fr>;

/**
 * @brief Compute the permutation grand product polynomial Z_perm(X)
 * *
 * @detail (This description assumes program_width 3). Z_perm may be defined in terms of its values
 * on X_i = 0,1,...,n-1 as Z_perm[0] = 1 and for i = 1:n-1
 *
 *                  (w_1(j) + β⋅id_1(j) + γ) ⋅ (w_2(j) + β⋅id_2(j) + γ) ⋅ (w_3(j) + β⋅id_3(j) + γ)
 * Z_perm[i] = ∏ --------------------------------------------------------------------------------
 *                  (w_1(j) + β⋅σ_1(j) + γ) ⋅ (w_2(j) + β⋅σ_2(j) + γ) ⋅ (w_3(j) + β⋅σ_3(j) + γ)
 *
 * where ∏ := ∏_{j=0:i-1} and id_i(X) = id(X) + n*(i-1). These evaluations are constructed over the
 * course of four steps. For expositional simplicity, write Z_perm[i] as
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
 *
 * Note: Step (4) utilizes Montgomery batch inversion to replace n-many inversions with
 * one batch inversion (at the expense of more multiplications)
 */
// TODO(#222)(luke): Parallelize
template <size_t program_width>
Polynomial compute_permutation_grand_product(std::shared_ptr<bonk::proving_key>& key,
                                             std::vector<Polynomial>& wire_polynomials,
                                             Fr beta,
                                             Fr gamma)
{
    // (void)key;
    // (void)wire_polynomials;
    // (void)beta;
    // (void)gamma;
    using barretenberg::polynomial_arithmetic::copy_polynomial;
    // static const size_t program_width = settings::program_width;
    // static const size_t program_width = wire_polynomials.size();

    // TODO(luke): instantiate z_perm here then make the 0th accum a span of it? avoid extra memory.

    // Allocate accumulator polynomials that will serve as scratch space
    std::array<Polynomial, program_width> numerator_accumulator;
    std::array<Polynomial, program_width> denominator_accumulator;
    for (size_t i = 0; i < program_width; ++i) {
        numerator_accumulator[i] = Polynomial{ key->circuit_size };
        denominator_accumulator[i] = Polynomial{ key->circuit_size };
    }

    // Populate wire and permutation polynomials
    std::array<std::span<const Fr>, program_width> wires;
    std::array<std::span<const Fr>, program_width> sigmas;
    for (size_t i = 0; i < program_width; ++i) {
        std::string sigma_id = "sigma_" + std::to_string(i + 1) + "_lagrange";
        wires[i] = wire_polynomials[i];
        sigmas[i] = key->polynomial_store.get(sigma_id);
    }

    // Step (1)
    // TODO(#222)(kesha): Change the order to engage automatic prefetching and get rid of redundant computation
    for (size_t i = 0; i < key->circuit_size; ++i) {
        for (size_t k = 0; k < program_width; ++k) {
            // Note(luke): this idx could be replaced by proper ID polys if desired
            Fr idx = k * key->circuit_size + i;
            numerator_accumulator[k][i] = wires[k][i] + (idx * beta) + gamma;            // w_k(i) + β.(k*n+i) + γ
            denominator_accumulator[k][i] = wires[k][i] + (sigmas[k][i] * beta) + gamma; // w_k(i) + β.σ_k(i) + γ
        }
    }

    // Step (2)
    for (size_t k = 0; k < program_width; ++k) {
        for (size_t i = 0; i < key->circuit_size - 1; ++i) {
            numerator_accumulator[k][i + 1] *= numerator_accumulator[k][i];
            denominator_accumulator[k][i + 1] *= denominator_accumulator[k][i];
        }
    }

    // Step (3)
    for (size_t i = 0; i < key->circuit_size; ++i) {
        for (size_t k = 1; k < program_width; ++k) {
            numerator_accumulator[0][i] *= numerator_accumulator[k][i];
            denominator_accumulator[0][i] *= denominator_accumulator[k][i];
        }
    }

    // Step (4)
    // Use Montgomery batch inversion to compute z_perm[i+1] = numerator_accumulator[0][i] /
    // denominator_accumulator[0][i]. At the end of this computation, the quotient numerator_accumulator[0] /
    // denominator_accumulator[0] is stored in numerator_accumulator[0].
    // Note: Since numerator_accumulator[0][i] corresponds to z_lookup[i+1], we only iterate up to i = (n - 2).
    Fr* inversion_coefficients = &denominator_accumulator[1][0]; // arbitrary scratch space
    Fr inversion_accumulator = Fr::one();
    for (size_t i = 0; i < key->circuit_size - 1; ++i) {
        inversion_coefficients[i] = numerator_accumulator[0][i] * inversion_accumulator;
        inversion_accumulator *= denominator_accumulator[0][i];
    }
    inversion_accumulator = inversion_accumulator.invert(); // perform single inversion per thread
    for (size_t i = key->circuit_size - 2; i != std::numeric_limits<size_t>::max(); --i) {
        numerator_accumulator[0][i] = inversion_accumulator * inversion_coefficients[i];
        inversion_accumulator *= denominator_accumulator[0][i];
    }

    // Construct permutation polynomial 'z_perm' in lagrange form as:
    // z_perm = [0 numerator_accumulator[0][0] numerator_accumulator[0][1] ... numerator_accumulator[0][n-2] 0]
    Polynomial z_perm(key->circuit_size);
    // Initialize 0th coefficient to 0 to ensure z_perm is left-shiftable via division by X in gemini
    z_perm[0] = 0;
    copy_polynomial(numerator_accumulator[0].data(), &z_perm[1], key->circuit_size - 1, key->circuit_size - 1);

    return z_perm;
}

// Polynomial compute_lookup_grand_product_polynomial(auto& key, auto& wire_polynomials, Fr eta, Fr beta, Fr gamma)
// {
//     (void)wire_polynomials;

//     fr eta_sqr = eta.sqr();
//     fr eta_cube = eta_sqr * eta;

//     const size_t circuit_size = key->circuit_size;

//     // Allocate 4 length n 'accumulator' polynomials. accumulators[0] will be used to construct
//     // z_lookup (lagrange base) in place. The
//     // remaining 3 are needed only locally in the construction of z_lookup.
//     // Note: The magic number 4 here comes from the structure of the grand product and is not related to the program
//     // width.
//     std::array<Polynomial, 4> accumulators;
//     for (size_t i = 0; i < 4; ++i) {
//         accumulators[i] = Polynomial{ key->circuit_size };
//     }

//     polynomial s_lagrange = key->polynomial_store.get("s_lagrange");

//     const fr* column_1_step_size = key->polynomial_store.get("q_2_lagrange").get_coefficients();
//     const fr* column_2_step_size = key->polynomial_store.get("q_m_lagrange").get_coefficients();
//     const fr* column_3_step_size = key->polynomial_store.get("q_c_lagrange").get_coefficients();

//     std::array<const fr*, 3> lagrange_base_wires;
//     std::array<const fr*, 4> lagrange_base_tables{
//         key->polynomial_store.get("table_value_1_lagrange").get_coefficients(),
//         key->polynomial_store.get("table_value_2_lagrange").get_coefficients(),
//         key->polynomial_store.get("table_value_3_lagrange").get_coefficients(),
//         key->polynomial_store.get("table_value_4_lagrange").get_coefficients(),
//     };

//     const fr* lookup_selector = key->polynomial_store.get("table_type_lagrange").get_coefficients();
//     const fr* lookup_index_selector = key->polynomial_store.get("q_3_lagrange").get_coefficients();
//     for (size_t i = 0; i < 3; ++i) {
//         lagrange_base_wires[i] =
//             key->polynomial_store.get("w_" + std::to_string(i + 1) + "_lagrange").get_coefficients();
//     }

//     const fr beta_constant = beta + fr(1);                // (1 + β)
//     const fr gamma_beta_constant = gamma * beta_constant; // γ(1 + β)

//     // Step 1: Compute polynomials f, t and s and incorporate them into terms that are ultimately needed
//     // to construct the grand product polynomial Z_lookup(X):
//     // Note 1: In what follows, 't' is associated with table values (and is not to be confused with the
//     // quotient polynomial, also refered to as 't' elsewhere). Polynomial 's' is the sorted  concatenation
//     // of the witnesses and the table values.
//     // Note 2: Evaluation at Xω is indicated explicitly, e.g. 'p(Xω)'; evaluation at X is simply omitted, e.g. 'p'
//     //
//     // 1a.   Compute f, then set accumulators[0] = (q_lookup*f + γ), where
//     //
//     //         f = (w_1 + q_2*w_1(Xω)) + η(w_2 + q_m*w_2(Xω)) + η²(w_3 + q_c*w_3(Xω)) + η³q_index.
//     //      Note that q_2, q_m, and q_c are just the selectors from Standard Plonk that have been repurposed
//     //      in the context of the plookup gate to represent 'shift' values. For example, setting each of the
//     //      q_* in f to 2^8 facilitates operations on 32-bit values via four operations on 8-bit values. See
//     //      Ultra documentation for details.
//     //
//     // 1b.   Compute t, then set accumulators[1] = (t + βt(Xω) + γ(1 + β)), where t = t_1 + ηt_2 + η²t_3 + η³t_4
//     //
//     // 1c.   Set accumulators[2] = (1 + β)
//     //
//     // 1d.   Compute s, then set accumulators[3] = (s + βs(Xω) + γ(1 + β)), where s = s_1 + ηs_2 + η²s_3 + η³s_4
//     //

//     fr T0;
//     // Note: block_mask is used for efficient modulus, i.e. i % N := i & (N-1), for N = 2^k
//     const size_t block_mask = key->small_domain.size - 1;
//     // Initialize 't(X)' to be used in an expression of the form t(X) + β*t(Xω)
//     fr next_table = lagrange_base_tables[0][0] + lagrange_base_tables[1][0] * eta +
//                     lagrange_base_tables[2][0] * eta_sqr + lagrange_base_tables[3][0] * eta_cube;

//     for (size_t i = 0; i < circuit_size; ++i) {

//         // Compute i'th element of f via Horner (see definition of f above)
//         T0 = lookup_index_selector[i];
//         T0 *= eta;
//         T0 += lagrange_base_wires[2][(i + 1) & block_mask] * column_3_step_size[i];
//         T0 += lagrange_base_wires[2][i];
//         T0 *= eta;
//         T0 += lagrange_base_wires[1][(i + 1) & block_mask] * column_2_step_size[i];
//         T0 += lagrange_base_wires[1][i];
//         T0 *= eta;
//         T0 += lagrange_base_wires[0][(i + 1) & block_mask] * column_1_step_size[i];
//         T0 += lagrange_base_wires[0][i];
//         T0 *= lookup_selector[i];

//         // Set i'th element of polynomial q_lookup*f + γ
//         accumulators[0][i] = T0;
//         accumulators[0][i] += gamma;

//         // Compute i'th element of t via Horner
//         T0 = lagrange_base_tables[3][(i + 1) & block_mask];
//         T0 *= eta;
//         T0 += lagrange_base_tables[2][(i + 1) & block_mask];
//         T0 *= eta;
//         T0 += lagrange_base_tables[1][(i + 1) & block_mask];
//         T0 *= eta;
//         T0 += lagrange_base_tables[0][(i + 1) & block_mask];

//         // Set i'th element of polynomial (t + βt(Xω) + γ(1 + β))
//         accumulators[1][i] = T0 * beta + next_table;
//         next_table = T0;
//         accumulators[1][i] += gamma_beta_constant;

//         // Set value of this accumulator to (1 + β)
//         accumulators[2][i] = beta_constant;

//         // Set i'th element of polynomial (s + βs(Xω) + γ(1 + β))
//         accumulators[3][i] = s_lagrange[(i + 1) & block_mask];
//         accumulators[3][i] *= beta;
//         accumulators[3][i] += s_lagrange[i];
//         accumulators[3][i] += gamma_beta_constant;
//     }

//     // Step 2: Compute the constituent product components of Z_lookup(X).
//     // Let ∏ := Prod_{k<j}. Let f_k, t_k and s_k now represent the k'th component of the polynomials f,t and s
//     // defined above. We compute the following four product polynomials needed to construct the grand product
//     // Z_lookup(X).
//     // 1.   accumulators[0][j] = ∏ (q_lookup*f_k + γ)
//     // 2.   accumulators[1][j] = ∏ (t_k + βt_{k+1} + γ(1 + β))
//     // 3.   accumulators[2][j] = ∏ (1 + β)
//     // 4.   accumulators[3][j] = ∏ (s_k + βs_{k+1} + γ(1 + β))
//     // Note: This is a small multithreading bottleneck, as we have only 4 parallelizable processes.
//     for (auto& accum : accumulators) {
//         for (size_t i = 0; i < circuit_size - 1; ++i) {
//             accum[i + 1] *= accum[i];
//         }
//     }

//     // Step 3: Combine the accumulator product elements to construct Z_lookup(X).
//     //
//     //                      ∏ (1 + β) ⋅ ∏ (q_lookup*f_k + γ) ⋅ ∏ (t_k + βt_{k+1} + γ(1 + β))
//     //  Z_lookup(g^j) = --------------------------------------------------------------------------
//     //                                      ∏ (s_k + βs_{k+1} + γ(1 + β))
//     //
//     // Note: Montgomery batch inversion is used to efficiently compute the coefficients of Z_lookup
//     // rather than peforming n individual inversions. I.e. we first compute the double product P_n:
//     //
//     // P_n := ∏_{j<n} ∏_{k<j} S_k, where S_k = (s_k + βs_{k+1} + γ(1 + β))
//     //
//     // and then compute the inverse on P_n. Then we work back to front to obtain terms of the form
//     // 1/∏_{k<i} S_i that appear in Z_lookup, using the fact that P_i/P_{i+1} = 1/∏_{k<i} S_i. (Note
//     // that once we have 1/P_n, we can compute 1/P_{n-1} as (1/P_n) * ∏_{k<n} S_i, and
//     // so on).

//     // Compute Z_lookup using Montgomery batch inversion
//     // Note: This loop sets the values of z_lookup[i] for i = 1,...,(n-1), (Recall accumulators[0][i] = z_lookup[i +
//     // 1])

//     // Compute <Z_lookup numerator> * ∏_{j<i}∏_{k<j}S_k
//     fr inversion_accumulator = fr::one();
//     for (size_t i = 0; i < circuit_size - 1; ++i) {
//         accumulators[0][i] *= accumulators[2][i];
//         accumulators[0][i] *= accumulators[1][i];
//         accumulators[0][i] *= inversion_accumulator;
//         inversion_accumulator *= accumulators[3][i];
//     }
//     inversion_accumulator = inversion_accumulator.invert(); // invert

//     // Compute [Z_lookup numerator] * ∏_{j<i}∏_{k<j}S_k / ∏_{j<i+1}∏_{k<j}S_k = <Z_lookup numerator> /
//     // ∏_{k<i}S_k
//     for (size_t i = circuit_size - 2; i != std::numeric_limits<size_t>::max(); --i) {
//         // N.B. accumulators[0][i] = z_lookup[i + 1]
//         // We can avoid fully reducing z_lookup[i + 1] as the inverse fft will take care of that for us
//         accumulators[0][i] *= inversion_accumulator;
//         inversion_accumulator *= accumulators[3][i];
//     }

//     Polynomial z_lookup(key->circuit_size);
//     // Initialize 0th coefficient to 0 to ensure z_perm is left-shiftable via division by X in gemini
//     z_lookup[0] = 0;
//     barretenberg::polynomial_arithmetic::copy_polynomial(
//         accumulators[0].data(), &z_lookup[1], key->circuit_size - 1, key->circuit_size - 1);

//     return z_lookup;
// }

template Polynomial compute_permutation_grand_product<plonk::standard_settings::program_width>(
    std::shared_ptr<bonk::proving_key>&, std::vector<Polynomial>&, Fr, Fr);

} // namespace honk::prover_library
