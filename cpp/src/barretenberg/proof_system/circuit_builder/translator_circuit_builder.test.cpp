#include "barretenberg/ecc/curves/bn254/bn254.hpp"
#include <gtest/gtest.h>

using namespace barretenberg;
namespace {
auto& engine = numeric::random::get_debug_engine();
}
namespace proof_system {
TEST(translator_circuit_builder, scoping_out_the_circuit)
{
    // Questions:
    // 1. Do we need 68-bit limbs at all?
    using Fr = ::curve::BN254::ScalarField;
    using Fp = ::curve::BN254::BaseField;

    constexpr size_t NUM_LIMB_BITS = 68;
    constexpr uint512_t modulus_u512 = uint512_t(Fp::modulus);

    constexpr Fr shift_1 = Fr(uint256_t(1) << (NUM_LIMB_BITS));
    constexpr Fr shift_2 = Fr(uint256_t(1) << (NUM_LIMB_BITS * 2));
    constexpr Fr shift_2_inverse = shift_2.invert();
    constexpr uint256_t DOUBLE_LIMB_SIZE = uint256_t(1) << (NUM_LIMB_BITS << 1);
    constexpr uint512_t BINARY_BASIS_MODULUS = uint512_t(1) << (NUM_LIMB_BITS << 2);
    constexpr uint512_t negative_prime_modulus = BINARY_BASIS_MODULUS - modulus_u512;
    constexpr std::array<Fr, 5> neg_modulus_limbs = {
        Fr(negative_prime_modulus.slice(0, NUM_LIMB_BITS).lo),
        Fr(negative_prime_modulus.slice(NUM_LIMB_BITS, NUM_LIMB_BITS * 2).lo),
        Fr(negative_prime_modulus.slice(NUM_LIMB_BITS * 2, NUM_LIMB_BITS * 3).lo),
        Fr(negative_prime_modulus.slice(NUM_LIMB_BITS * 3, NUM_LIMB_BITS * 4).lo),
        -Fr(modulus_u512.lo)
    };
    // x is the value (challenge) at which we are evaluating the polynomials
    // y is the end result of the whole combination (I don't know why we use y for domain and x for evalutation in
    // the pepe paper) v is the polynomial batching challenge

    // 2 rows:
    // OP | P.xₗₒ | P.xₕᵢ | P.yₗₒ
    // -  | P.yₕᵢ | z₁    | z₂

    // Rows written vertically:
    // 0	 |  -       |   OP      |
    // 1	 |  P.yₕᵢ   |   P.xₗₒ   |
    // 2	 |  z₁      |   P.xₕᵢ   |
    // 3	 |  z₂      |   P.yₗₒ   |
    // 4	 |  p_x_1   |   p_x_0   | 68-bit limbs
    // 5	 |  p_x_1_0 |   p_x_0_0 | 12 bit limbs
    // 6	 |  p_x_1_1 |   p_x_0_1 | 12 bit limbs
    // 7	 |  p_x_1_2 |   p_x_0_2 | 12 bit limbs
    // 8	 |  p_x_1_3 |   p_x_0_3 | 12 bit limbs
    // 9	 |  p_x_1_4 |   p_x_0_4 | 12 bit limbs
    // 10	 |  p_x_1_5 |   p_x_0_5 | 8 bit limns
    // 11	 |  p_x_3   |   p_x_2   | 68-bit limbs
    // 12	 |  p_x_3_0 |   p_x_2_0 | 12 bit limbs
    // 13	 |  p_x_3_1 |   p_x_2_1 | 12 bit limbs
    // 14	 |  p_x_3_2 |   p_x_2_2 | 12 bit limbs
    // 15	 |  p_x_3_3 |   p_x_2_3 | 12 bit limbs
    // 16	 |  p_x_3_4 |   p_x_2_4 | p_x_3_4 is 2 bits and enforced with a relation. p_x_2_4 is 12 bits
    // 17	 |  -       |   p_x_2_5 | 8 bit limb
    // 18	 |  p_y_1   |   p_y_0   | 68-bit limbs
    // 19	 |  p_y_1_0 |   p_y_0_0 | 12 bit limbs
    // 20	 |  p_y_1_1 |   p_y_0_1 | 12 bit limbs
    // 21	 |  p_y_1_2 |   p_y_0_2 | 12 bit limbs
    // 22	 |  p_y_1_3 |   p_y_0_3 | 12 bit limbs
    // 23	 |  p_y_1_4 |   p_y_0_4 | 12 bit limbs
    // 24	 |  p_y_1_5 |   p_y_0_5 | 8 bit limns
    // 25	 |  p_y_3   |   p_y_2   | 68-bit limbs
    // 26	 |  p_y_3_0 |   p_y_2_0 | 12 bit limbs
    // 27	 |  p_y_3_1 |   p_y_2_1 | 12 bit limbs
    // 28	 |  p_y_3_2 |   p_y_2_2 | 12 bit limbs
    // 29	 |  p_y_3_3 |   p_y_2_3 | 12 bit limbs
    // 30	 |  p_y_3_4 |   p_y_2_4 | p_y_3_4 is 2 bits and enforced with a relation. p_y_2_4 is 12 bits
    // 31	 |  -       |   p_y_2_5 | 8 bit limb
    // 32	 |  z_1_hi  |   z_1_lo  | 68 bit limbs
    // 33	 |  z_1_hi_0|   z_1_lo_0| 12 bit limbs
    // 34	 |  z_1_hi_1|   z_1_lo_1| 12 bit limbs
    // 35	 |  z_1_hi_2|   z_1_lo_2| 12 bit limbs
    // 36	 |  z_1_hi_3|   z_1_lo_3| 12 bit limbs
    // 37	 |  z_1_hi_4|   z_1_lo_4| 12 bit limbs
    // 38	 |  z_1_hi_5|   z_1_lo_5| 8 bit limbs
    // 39	 |  z_2_hi  |   z_2_lo  | 68 bit limbs
    // 40	 |  z_2_hi_0|   z_2_lo_0| 12 bit limbs
    // 41	 |  z_2_hi_1|   z_2_lo_1| 12 bit limbs
    // 42	 |  z_2_hi_2|   z_2_lo_2| 12 bit limbs
    // 43	 |  z_2_hi_3|   z_2_lo_3| 12 bit limbs
    // 44	 |  z_2_hi_4|   z_2_lo_4| 12 bit limbs
    // 45	 |  z_2_hi_5|   z_2_lo_5| 8 bit limbs
    // 46	 |  A₀      |   Aₚᵣₑᵥ_₀ | 68
    // 47	 |  A₁      |   Aₚᵣₑᵥ_₁ | 68
    // 48	 |  A₂      |   Aₚᵣₑᵥ_₂ | 68
    // 49	 |  A₃      |   Aₚᵣₑᵥ_₃ | 68
    // 50	 |  A_1_0   |   A_0_0   | 12
    // 51	 |  A_1_1   |   A_0_1   | 12
    // 52	 |  A_1_2   |   A_0_2   | 12
    // 53	 |  A_1_3   |   A_0_3   | 12
    // 54	 |  A_1_4   |   A_0_4   | 12
    // 55	 |  A_1_5   |   A_0_5   | 8
    // 56	 |  A_3_0   |   A_2_0   | 12
    // 57	 |  A_3_1   |   A_2_1   | 12
    // 58	 |  A_3_2   |   A_2_2   | 12
    // 59	 |  A_3_3   |   A_2_3   | 12
    // 60	 |  A_3_4   |   A_2_4   | 2/12
    // 61	 |  -       |   A_2_5   | 12

    Fr op;
    Fr p_x_lo;
    Fr p_x_hi(0);
    Fr p_y_lo(0);
    Fr p_y_hi(0);
    Fr z_1(0);
    Fr z_2(0);
    op = Fr::random_element();
    auto get_random_wide_limb = []() { return Fr(engine.get_random_uint256() >> (256 - NUM_LIMB_BITS * 2)); };
    auto get_random_shortened_wide_limb = []() { return uint256_t(Fp::random_element()) >> (NUM_LIMB_BITS * 2); };
    p_x_lo = get_random_wide_limb();
    p_x_hi = get_random_shortened_wide_limb();
    p_y_lo = get_random_wide_limb();
    p_y_hi = get_random_shortened_wide_limb();
    z_1 = get_random_wide_limb();
    z_2 = get_random_wide_limb();

    Fp accumulator;
    accumulator = Fp::random_element();
    // p_y_lo = get_random_wide_limb();
    //  Creating a bigfield representation from (binary_limb_0, binary_limb_1, binary_limb_2, binary_limb_3, prime_limb)
    auto base_element_to_bigfield = [](Fp& original) {
        uint256_t original_uint = original;
        return std::make_tuple(Fr(original_uint.slice(0, NUM_LIMB_BITS)),
                               Fr(original_uint.slice(NUM_LIMB_BITS, 2 * NUM_LIMB_BITS)),
                               Fr(original_uint.slice(2 * NUM_LIMB_BITS, 3 * NUM_LIMB_BITS)),
                               Fr(original_uint.slice(3 * NUM_LIMB_BITS, 4 * NUM_LIMB_BITS)),
                               Fr(original_uint));
    };
    auto uint512_t_to_bigfield = [&shift_2](uint512_t& original) {
        return std::make_tuple(Fr(original.slice(0, NUM_LIMB_BITS).lo),
                               Fr(original.slice(NUM_LIMB_BITS, 2 * NUM_LIMB_BITS).lo),
                               Fr(original.slice(2 * NUM_LIMB_BITS, 3 * NUM_LIMB_BITS).lo),
                               Fr(original.slice(3 * NUM_LIMB_BITS, 4 * NUM_LIMB_BITS).lo),
                               Fr(original.slice(0, NUM_LIMB_BITS * 2).lo) +
                                   Fr(original.slice(NUM_LIMB_BITS * 2, NUM_LIMB_BITS * 4).lo) * shift_2);
    };

    auto [accumulator_0, accumulator_1, accumulator_2, accumulator_3, accumulator_prime] =
        base_element_to_bigfield(accumulator);
    std::array<Fr, 5> accumulator_witnesses = {
        accumulator_0, accumulator_1, accumulator_2, accumulator_3, accumulator_prime
    };
    //  x and powers of v are given to use in challenge form, so the verifier has to deal with this :)
    Fp v;
    Fp v_squared;
    Fp v_cubed;
    Fp v_quarted;
    Fp x;
    v = Fp::random_element();
    v_squared = v * v;
    v_cubed = v_squared * v;
    v_quarted = v_cubed * v;
    x = Fp::random_element();

    auto [v_0, v_1, v_2, v_3, v_prime] = base_element_to_bigfield(v);
    std::array<Fr, 5> v_witnesses = { v_0, v_1, v_2, v_3, v_prime };
    auto [v_squared_0, v_squared_1, v_squared_2, v_squared_3, v_squared_prime] = base_element_to_bigfield(v_squared);
    std::array<Fr, 5> v_squared_witnesses = { v_squared_0, v_squared_1, v_squared_2, v_squared_3, v_squared_prime };
    auto [v_cubed_0, v_cubed_1, v_cubed_2, v_cubed_3, v_cubed_prime] = base_element_to_bigfield(v_cubed);
    std::array<Fr, 5> v_cubed_witnesses = { v_cubed_0, v_cubed_1, v_cubed_2, v_cubed_3, v_cubed_prime };
    auto [v_quarted_0, v_quarted_1, v_quarted_2, v_quarted_3, v_quarted_prime] = base_element_to_bigfield(v_quarted);
    std::array<Fr, 5> v_quarted_witnesses = { v_quarted_0, v_quarted_1, v_quarted_2, v_quarted_3, v_quarted_prime };
    auto [x_0, x_1, x_2, x_3, x_prime] = base_element_to_bigfield(x);
    std::array<Fr, 5> x_witnesses = { x_0, x_1, x_2, x_3, x_prime };

    // Each of these first needs to be converted to a bigfield value:
    // Range constrain op to 68 bits (1 limb). We can then simply treat it as 1 limb and add it at the endw
    auto split_wide_limb_into_2_limbs = [](Fr& wide_limb) {
        return std::make_tuple(Fr(uint256_t(wide_limb).slice(0, NUM_LIMB_BITS)),
                               Fr(uint256_t(wide_limb).slice(NUM_LIMB_BITS, 2 * NUM_LIMB_BITS)));
    };
    // Unsigned interger versions for use in witness computation
    auto uint_accumulator = uint512_t(accumulator);
    auto uint_x = uint512_t(x);
    auto uint_op = uint512_t(op);
    auto uint_p_x = uint512_t(uint256_t(p_x_lo) + (uint256_t(p_x_hi) << (NUM_LIMB_BITS << 1)));
    auto uint_p_y = uint512_t(uint256_t(p_y_lo) + (uint256_t(p_y_hi) << (NUM_LIMB_BITS << 1)));
    auto uint_z_1 = uint512_t(z_1);
    auto uint_z_2 = uint512_t(z_2);
    auto uint_v = uint512_t(v);
    auto uint_v_squared = uint512_t(v_squared);
    auto uint_v_cubed = uint512_t(v_cubed);
    auto uint_v_quarted = uint512_t(v_quarted);

    // Construct Fp for op, P.x, P.y, z_1, z_2 for use in witness computation
    Fp base_op = Fp(uint256_t(op));
    Fp base_p_x = Fp(uint256_t(p_x_lo) + (uint256_t(p_x_hi) << (NUM_LIMB_BITS << 1)));
    Fp base_p_y = Fp(uint256_t(p_y_lo) + (uint256_t(p_y_hi) << (NUM_LIMB_BITS << 1)));
    Fp base_z_1 = Fp(uint256_t(z_1));
    Fp base_z_2 = Fp(uint256_t(z_2));
    // Construct bigfield representations of P.x and P.y
    auto [p_x_0, p_x_1] = split_wide_limb_into_2_limbs(p_x_lo);
    auto [p_x_2, p_x_3] = split_wide_limb_into_2_limbs(p_x_hi);
    Fr p_x_prime = p_x_lo + p_x_hi * Fr(DOUBLE_LIMB_SIZE);
    Fr p_x_witnesses[5] = { p_x_0, p_x_1, p_x_2, p_x_3, p_x_prime };
    auto [p_y_0, p_y_1] = split_wide_limb_into_2_limbs(p_y_lo);
    auto [p_y_2, p_y_3] = split_wide_limb_into_2_limbs(p_y_hi);
    Fr p_y_prime = p_y_lo + p_y_hi * Fr(DOUBLE_LIMB_SIZE);
    Fr p_y_witnesses[5] = { p_y_0, p_y_1, p_y_2, p_y_3, p_y_prime };
    // Construct bigfield representations of z1 and z2 only using 2 limbs each
    // z_1 and z_2 are low enough to act as their own prime limbs
    auto [z_1_lo, z_1_hi] = split_wide_limb_into_2_limbs(z_1);
    auto [z_2_lo, z_2_hi] = split_wide_limb_into_2_limbs(z_2);
    // Range constrain all the individual limbs
    // The formula is `accumulator = accumulator⋅x + (op + v⋅p.x + v²⋅p.y + v³⋅z₁ + v⁴z₂)`. We need to compute ther
    // remainder

    Fp remainder =
        accumulator * x + base_z_2 * v_quarted + base_z_1 * v_cubed + base_p_y * v_squared + base_p_x * v + base_op;
    uint512_t quotient_by_modulus = uint_accumulator * uint_x + uint_z_2 * uint_v_quarted + uint_z_1 * uint_v_cubed +
                                    uint_p_y * uint_v_squared + uint_p_x * uint_v + uint_op - uint512_t(remainder);

    EXPECT_EQ(quotient_by_modulus % uint512_t(Fp::modulus), 0);

    uint512_t quotient = quotient_by_modulus / uint512_t(Fp::modulus);

    auto [remainder_0, remainder_1, remainder_2, remainder_3, remainder_prime] = base_element_to_bigfield(remainder);
    std::array<Fr, 5> remainder_witnesses = { remainder_0, remainder_1, remainder_2, remainder_3, remainder_prime };
    auto [quotient_0, quotient_1, quotient_2, quotient_3, quotient_prime] = uint512_t_to_bigfield(quotient);
    std::array<Fr, 5> quotient_witnesses = { quotient_0, quotient_1, quotient_2, quotient_3, quotient_prime };

    // We will divide by shift_2 instantly in the relation itself, but f
    Fr low_wide_relation_limb =
        accumulator_witnesses[0] * x_witnesses[0] + op + v_witnesses[0] * p_x_witnesses[0] +
        v_squared_witnesses[0] * p_y_witnesses[0] + v_cubed_witnesses[0] * z_1_lo + v_quarted_witnesses[0] * z_2_lo +
        quotient_witnesses[0] * neg_modulus_limbs[0] - remainder_witnesses[0] + // This covers the lowest limb
        (accumulator_witnesses[1] * x_witnesses[0] + accumulator_witnesses[0] * x_witnesses[1] +
         v_witnesses[1] * p_x_witnesses[0] + p_x_witnesses[1] * v_witnesses[0] +
         v_squared_witnesses[1] * p_y_witnesses[0] + v_squared_witnesses[0] * p_y_witnesses[1] +
         v_cubed_witnesses[1] * z_1_lo + z_1_hi * v_cubed_witnesses[0] + v_quarted_witnesses[1] * z_2_lo +
         v_quarted_witnesses[0] * z_2_hi + quotient_witnesses[0] * neg_modulus_limbs[1] +
         quotient_witnesses[1] * neg_modulus_limbs[0] - remainder_witnesses[1]) *
            shift_1; // And this covers the limb shifted by 68
    // Treating accumulator as 254-bit constrained value
    constexpr auto max_limb_size = (uint512_t(1) << NUM_LIMB_BITS) - 1;
    constexpr auto shift_1_u512 = uint512_t(shift_1);
    constexpr auto op_max_size = uint512_t(4);
    constexpr uint512_t lwl_maximum_value = op_max_size + (max_limb_size * max_limb_size) * ((shift_1_u512 * 12) + 6);
    constexpr uint512_t lwl_maximum_value_constraint =
        (lwl_maximum_value >> (2 * NUM_LIMB_BITS)).lo +
        uint256_t(uint64_t((lwl_maximum_value % uint512_t(1) << (2 * NUM_LIMB_BITS)) != 0));
    constexpr auto lwl_range_consraint_size = lwl_maximum_value_constraint.get_msb() + 1;
    (void)lwl_range_consraint_size;
    // Low bits have to be zero
    EXPECT_EQ(uint256_t(low_wide_relation_limb).slice(0, 2 * NUM_LIMB_BITS), 0);

    Fr low_wide_relation_limb_divided = low_wide_relation_limb * shift_2_inverse;
    // We need to range constrain the low_wide_relation_limb_divided
    constexpr size_t NUM_LAST_BN254_LIMB_BITS = modulus_u512.get_msb() + 1 - NUM_LIMB_BITS * 3;

    constexpr auto max_high_limb_size = (uint512_t(1) << NUM_LAST_BN254_LIMB_BITS) - 1;
    constexpr uint512_t hwl_maximum_value =
        lwl_maximum_value_constraint + (max_limb_size * max_limb_size) * 16 +
        (max_limb_size * max_limb_size * 10 + max_limb_size * max_high_limb_size * 10) * shift_1_u512;
    constexpr uint512_t hwl_maximum_value_constraint =
        (hwl_maximum_value >> (2 * NUM_LIMB_BITS)).lo +
        uint256_t(uint64_t((hwl_maximum_value % uint512_t(1) << (2 * NUM_LIMB_BITS)) != 0));
    constexpr auto hwl_range_consraint_size = hwl_maximum_value_constraint.get_msb() + 1;
    info(hwl_range_consraint_size);
    // 4 high combinations = 8 ml*ml + 8 ml*last_ml. 2 low combinations = 2*ml*ml + 2*ml*last_ml
    Fr high_wide_relation_limb =
        low_wide_relation_limb_divided + accumulator_witnesses[2] * x_witnesses[0] +
        accumulator_witnesses[1] * x_witnesses[1] + accumulator_witnesses[0] * x_witnesses[2] +
        v_witnesses[2] * p_x_witnesses[0] + v_witnesses[1] * p_x_witnesses[1] + v_witnesses[0] * p_x_witnesses[2] +
        v_squared_witnesses[2] * p_y_witnesses[0] + v_squared_witnesses[1] * p_y_witnesses[1] +
        v_squared_witnesses[0] * p_y_witnesses[2] + v_cubed_witnesses[2] * z_1_lo + v_cubed_witnesses[1] * z_1_hi +
        v_quarted_witnesses[2] * z_2_lo + v_quarted_witnesses[1] * z_2_hi +
        quotient_witnesses[2] * neg_modulus_limbs[0] + quotient_witnesses[1] * neg_modulus_limbs[1] +
        quotient_witnesses[0] * neg_modulus_limbs[2] - remainder_witnesses[2] +
        (accumulator_witnesses[3] * x_witnesses[0] + accumulator_witnesses[2] * x_witnesses[1] +
         accumulator_witnesses[1] * x_witnesses[2] + accumulator_witnesses[0] * x_witnesses[3] +
         v_witnesses[3] * p_x_witnesses[0] + v_witnesses[2] * p_x_witnesses[1] + v_witnesses[1] * p_x_witnesses[2] +
         v_witnesses[0] * p_x_witnesses[3] + v_squared_witnesses[3] * p_y_witnesses[0] +
         v_squared_witnesses[2] * p_y_witnesses[1] + v_squared_witnesses[1] * p_y_witnesses[2] +
         v_squared_witnesses[0] * p_y_witnesses[3] + v_cubed_witnesses[3] * z_1_lo + v_cubed_witnesses[2] * z_1_hi +
         v_quarted_witnesses[3] * z_2_lo + v_quarted_witnesses[2] * z_1_hi +
         quotient_witnesses[3] * neg_modulus_limbs[0] + quotient_witnesses[2] * neg_modulus_limbs[1] +
         quotient_witnesses[1] * neg_modulus_limbs[2] + quotient_witnesses[0] * neg_modulus_limbs[3] -
         remainder_witnesses[3]) *
            shift_1;

    // Low bits have to be zero
    EXPECT_EQ(uint256_t(high_wide_relation_limb).slice(0, NUM_LIMB_BITS), 0);
    // And we'll need to range constrain it
    // 68 can be treated as 12/12/12/12/12/8
    // 68 can be treated as 12/12/12/12/12/8

    // Prime relation
    Fr prime_relation = accumulator_witnesses[4] * x_witnesses[4] + op + v_witnesses[4] * p_x_witnesses[4] +
                        v_squared_witnesses[4] * p_y_witnesses[4] + v_cubed_witnesses[4] * z_1 +
                        v_quarted_witnesses[4] * z_2 + quotient_witnesses[4] * neg_modulus_limbs[4] -
                        remainder_witnesses[4];
    EXPECT_EQ(prime_relation, 0);
    EXPECT_EQ(Fp(1), Fp(1));
    EXPECT_EQ(Fr(1), Fr(1));
}
} // namespace proof_system