#include "barretenberg/crypto/generators/generator_data.hpp"
#include "barretenberg/crypto/pedersen_commitment/pedersen.hpp"
#include "standard_circuit_builder.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "barretenberg/stdlib/primitives/field/field.hpp"

using namespace barretenberg;
using namespace proof_system;

namespace {
auto& engine = numeric::random::get_debug_engine();
}

namespace standard_circuit_constructor_tests {

TEST(standard_circuit_constructor, base_case)
{
    StandardCircuitBuilder circuit_constructor = StandardCircuitBuilder();
    fr a = fr::one();
    circuit_constructor.add_public_variable(a);

    bool result = circuit_constructor.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(standard_circuit_constructor, grumpkin_base_case)
{
    StandardGrumpkinCircuitBuilder composer = StandardGrumpkinCircuitBuilder();
    grumpkin::fr a = grumpkin::fr::one();
    composer.add_public_variable(a);

    bool result = composer.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(standard_circuit_constructor, test_add_gate)
{
    StandardCircuitBuilder circuit_constructor = StandardCircuitBuilder();
    fr a = fr::one();
    uint32_t a_idx = circuit_constructor.add_public_variable(a);
    fr b = fr::one();
    fr c = a + b;
    fr d = a + c;
    // uint32_t a_idx = circuit_constructor.add_variable(a);
    uint32_t b_idx = circuit_constructor.add_variable(b);
    uint32_t c_idx = circuit_constructor.add_variable(c);
    uint32_t d_idx = circuit_constructor.add_variable(d);
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });

    circuit_constructor.create_add_gate({ d_idx, c_idx, a_idx, fr::one(), fr::neg_one(), fr::neg_one(), fr::zero() });

    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ b_idx, a_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });

    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });

    bool result = circuit_constructor.check_circuit(); // instance, prover.reference_string.SRS_T2);
    EXPECT_EQ(result, true);
}

TEST(standard_circuit_constructor, test_mul_gate_proofs)
{
    StandardCircuitBuilder circuit_constructor = StandardCircuitBuilder();
    fr q[7]{ fr::random_element(), fr::random_element(), fr::random_element(), fr::random_element(),
             fr::random_element(), fr::random_element(), fr::random_element() };
    fr q_inv[7]{
        q[0].invert(), q[1].invert(), q[2].invert(), q[3].invert(), q[4].invert(), q[5].invert(), q[6].invert(),
    };

    fr a = fr::random_element();
    fr b = fr::random_element();
    fr c = -((((q[0] * a) + (q[1] * b)) + q[3]) * q_inv[2]);
    fr d = -((((q[4] * (a * b)) + q[6]) * q_inv[5]));

    uint32_t a_idx = circuit_constructor.add_variable(a);
    uint32_t b_idx = circuit_constructor.add_variable(b);
    uint32_t c_idx = circuit_constructor.add_variable(c);
    uint32_t d_idx = circuit_constructor.add_variable(d);

    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });

    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    circuit_constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });

    bool result = circuit_constructor.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(standard_circuit_constructor, range_constraint)
{
    StandardCircuitBuilder circuit_constructor = StandardCircuitBuilder();

    for (size_t i = 0; i < 10; ++i) {

        uint32_t value = engine.get_random_uint32();
        fr witness_value = fr{ value, 0, 0, 0 }.to_montgomery_form();
        uint32_t witness_index = circuit_constructor.add_variable(witness_value);

        // include non-nice numbers of bits, that will bleed over gate boundaries
        size_t extra_bits = 2 * (i % 4);

        std::vector<uint32_t> accumulators =
            circuit_constructor.decompose_into_base4_accumulators(witness_index, 32 + extra_bits);

        for (uint32_t j = 0; j < 16; ++j) {
            uint32_t result = (value >> (30U - (2 * j)));
            fr source = circuit_constructor.get_variable(accumulators[j + (extra_bits >> 1)]).from_montgomery_form();
            uint32_t expected = static_cast<uint32_t>(source.data[0]);
            EXPECT_EQ(result, expected);
        }
        for (uint32_t j = 1; j < 16; ++j) {
            uint32_t left = (value >> (30U - (2 * j)));
            uint32_t right = (value >> (30U - (2 * (j - 1))));
            EXPECT_EQ(left - 4 * right < 4, true);
        }
    }

    uint32_t zero_idx = circuit_constructor.add_variable(fr::zero());
    uint32_t one_idx = circuit_constructor.add_variable(fr::one());
    circuit_constructor.create_big_add_gate(
        { zero_idx, zero_idx, zero_idx, one_idx, fr::one(), fr::one(), fr::one(), fr::one(), fr::neg_one() });

    bool result = circuit_constructor.check_circuit();

    EXPECT_EQ(result, true);
}

TEST(standard_circuit_constructor, range_constraint_fail)
{
    StandardCircuitBuilder circuit_constructor = StandardCircuitBuilder();

    uint64_t value = 0xffffff;
    uint32_t witness_index = circuit_constructor.add_variable(fr(value));

    circuit_constructor.decompose_into_base4_accumulators(witness_index, 23);

    bool result = circuit_constructor.check_circuit();

    EXPECT_EQ(result, false);
}

TEST(standard_circuit_constructor, and_constraint)
{
    StandardCircuitBuilder circuit_constructor = StandardCircuitBuilder();

    for (size_t i = 0; i < /*10*/ 1; ++i) {
        uint32_t left_value = engine.get_random_uint32();

        fr left_witness_value = fr{ left_value, 0, 0, 0 }.to_montgomery_form();
        uint32_t left_witness_index = circuit_constructor.add_variable(left_witness_value);

        uint32_t right_value = engine.get_random_uint32();
        fr right_witness_value = fr{ right_value, 0, 0, 0 }.to_montgomery_form();
        uint32_t right_witness_index = circuit_constructor.add_variable(right_witness_value);

        uint32_t out_value = left_value & right_value;
        // include non-nice numbers of bits, that will bleed over gate boundaries
        size_t extra_bits = 2 * (i % 4);

        accumulator_triple accumulators =
            circuit_constructor.create_and_constraint(left_witness_index, right_witness_index, 32 + extra_bits);
        // circuit_constructor.create_and_constraint(left_witness_index, right_witness_index, 32 + extra_bits);

        for (uint32_t j = 0; j < 16; ++j) {
            uint32_t left_expected = (left_value >> (30U - (2 * j)));
            uint32_t right_expected = (right_value >> (30U - (2 * j)));
            uint32_t out_expected = left_expected & right_expected;

            fr left_source =
                circuit_constructor.get_variable(accumulators.left[j + (extra_bits >> 1)]).from_montgomery_form();
            uint32_t left_result = static_cast<uint32_t>(left_source.data[0]);

            fr right_source =
                circuit_constructor.get_variable(accumulators.right[j + (extra_bits >> 1)]).from_montgomery_form();
            uint32_t right_result = static_cast<uint32_t>(right_source.data[0]);

            fr out_source =
                circuit_constructor.get_variable(accumulators.out[j + (extra_bits >> 1)]).from_montgomery_form();
            uint32_t out_result = static_cast<uint32_t>(out_source.data[0]);

            EXPECT_EQ(left_result, left_expected);
            EXPECT_EQ(right_result, right_expected);
            EXPECT_EQ(out_result, out_expected);
        }
        for (uint32_t j = 1; j < 16; ++j) {
            uint32_t left = (left_value >> (30U - (2 * j)));
            uint32_t right = (left_value >> (30U - (2 * (j - 1))));
            EXPECT_EQ(left - 4 * right < 4, true);

            left = (right_value >> (30U - (2 * j)));
            right = (right_value >> (30U - (2 * (j - 1))));
            EXPECT_EQ(left - 4 * right < 4, true);

            left = (out_value >> (30U - (2 * j)));
            right = (out_value >> (30U - (2 * (j - 1))));
            EXPECT_EQ(left - 4 * right < 4, true);
        }
    }

    uint32_t zero_idx = circuit_constructor.add_variable(fr::zero());
    uint32_t one_idx = circuit_constructor.add_variable(fr::one());
    circuit_constructor.create_big_add_gate(
        { zero_idx, zero_idx, zero_idx, one_idx, fr::one(), fr::one(), fr::one(), fr::one(), fr::neg_one() });

    bool result = circuit_constructor.check_circuit();

    EXPECT_EQ(result, true);
}

TEST(standard_circuit_constructor, xor_constraint)
{
    StandardCircuitBuilder circuit_constructor = StandardCircuitBuilder();

    for (size_t i = 0; i < /*10*/ 1; ++i) {
        uint32_t left_value = engine.get_random_uint32();

        fr left_witness_value = fr{ left_value, 0, 0, 0 }.to_montgomery_form();
        uint32_t left_witness_index = circuit_constructor.add_variable(left_witness_value);

        uint32_t right_value = engine.get_random_uint32();
        fr right_witness_value = fr{ right_value, 0, 0, 0 }.to_montgomery_form();
        uint32_t right_witness_index = circuit_constructor.add_variable(right_witness_value);

        uint32_t out_value = left_value ^ right_value;
        // include non-nice numbers of bits, that will bleed over gate boundaries
        size_t extra_bits = 2 * (i % 4);

        accumulator_triple accumulators =
            circuit_constructor.create_xor_constraint(left_witness_index, right_witness_index, 32 + extra_bits);

        for (uint32_t j = 0; j < 16; ++j) {
            uint32_t left_expected = (left_value >> (30U - (2 * j)));
            uint32_t right_expected = (right_value >> (30U - (2 * j)));
            uint32_t out_expected = left_expected ^ right_expected;

            fr left_source =
                circuit_constructor.get_variable(accumulators.left[j + (extra_bits >> 1)]).from_montgomery_form();
            uint32_t left_result = static_cast<uint32_t>(left_source.data[0]);

            fr right_source =
                circuit_constructor.get_variable(accumulators.right[j + (extra_bits >> 1)]).from_montgomery_form();
            uint32_t right_result = static_cast<uint32_t>(right_source.data[0]);

            fr out_source =
                circuit_constructor.get_variable(accumulators.out[j + (extra_bits >> 1)]).from_montgomery_form();
            uint32_t out_result = static_cast<uint32_t>(out_source.data[0]);

            EXPECT_EQ(left_result, left_expected);
            EXPECT_EQ(right_result, right_expected);
            EXPECT_EQ(out_result, out_expected);
        }
        for (uint32_t j = 1; j < 16; ++j) {
            uint32_t left = (left_value >> (30U - (2 * j)));
            uint32_t right = (left_value >> (30U - (2 * (j - 1))));
            EXPECT_EQ(left - 4 * right < 4, true);

            left = (right_value >> (30U - (2 * j)));
            right = (right_value >> (30U - (2 * (j - 1))));
            EXPECT_EQ(left - 4 * right < 4, true);

            left = (out_value >> (30U - (2 * j)));
            right = (out_value >> (30U - (2 * (j - 1))));
            EXPECT_EQ(left - 4 * right < 4, true);
        }
    }

    uint32_t zero_idx = circuit_constructor.add_variable(fr::zero());
    uint32_t one_idx = circuit_constructor.add_variable(fr::one());
    circuit_constructor.create_big_add_gate(
        { zero_idx, zero_idx, zero_idx, one_idx, fr::one(), fr::one(), fr::one(), fr::one(), fr::neg_one() });

    bool result = circuit_constructor.check_circuit();

    EXPECT_EQ(result, true);
}

TEST(standard_circuit_constructor, big_add_gate_with_bit_extract)
{
    StandardCircuitBuilder circuit_constructor = StandardCircuitBuilder();

    const auto generate_constraints = [&circuit_constructor](uint32_t quad_value) {
        uint32_t quad_accumulator_left =
            (engine.get_random_uint32() & 0x3fffffff) - quad_value; // make sure this won't overflow
        uint32_t quad_accumulator_right = (4 * quad_accumulator_left) + quad_value;

        uint32_t left_idx = circuit_constructor.add_variable(uint256_t(quad_accumulator_left));
        uint32_t right_idx = circuit_constructor.add_variable(uint256_t(quad_accumulator_right));

        uint32_t input = engine.get_random_uint32();
        uint32_t output = input + (quad_value > 1 ? 1 : 0);

        add_quad gate{ circuit_constructor.add_variable(uint256_t(input)),
                       circuit_constructor.add_variable(uint256_t(output)),
                       right_idx,
                       left_idx,
                       fr(6),
                       -fr(6),
                       fr::zero(),
                       fr::zero(),
                       fr::zero() };

        circuit_constructor.create_big_add_gate_with_bit_extraction(gate);
    };

    generate_constraints(0);
    generate_constraints(1);
    generate_constraints(2);
    generate_constraints(3);

    bool result = circuit_constructor.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(standard_circuit_constructor, test_range_constraint_fail)
{
    StandardCircuitBuilder circuit_constructor = StandardCircuitBuilder();
    uint32_t witness_index = circuit_constructor.add_variable(fr::neg_one());
    circuit_constructor.decompose_into_base4_accumulators(witness_index, 32);

    bool result = circuit_constructor.check_circuit();

    EXPECT_EQ(result, false);
}

TEST(standard_circuit_constructor, test_check_circuit_correct)
{
    StandardCircuitBuilder circuit_constructor = StandardCircuitBuilder();
    fr a = fr::one();
    uint32_t a_idx = circuit_constructor.add_public_variable(a);
    fr b = fr::one();
    fr c = a + b;
    fr d = a + c;
    // uint32_t a_idx = circuit_constructor.add_variable(a);
    uint32_t b_idx = circuit_constructor.add_variable(b);
    uint32_t c_idx = circuit_constructor.add_variable(c);
    uint32_t d_idx = circuit_constructor.add_variable(d);
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });

    circuit_constructor.create_add_gate({ d_idx, c_idx, a_idx, fr::one(), fr::neg_one(), fr::neg_one(), fr::zero() });

    bool result = circuit_constructor.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(standard_circuit_constructor, test_check_circuit_broken)
{
    StandardCircuitBuilder circuit_constructor = StandardCircuitBuilder();
    fr a = fr::one();
    uint32_t a_idx = circuit_constructor.add_public_variable(a);
    fr b = fr::one();
    fr c = a + b;
    fr d = a + c + 1;
    // uint32_t a_idx = circuit_constructor.add_variable(a);
    uint32_t b_idx = circuit_constructor.add_variable(b);
    uint32_t c_idx = circuit_constructor.add_variable(c);
    uint32_t d_idx = circuit_constructor.add_variable(d);
    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });

    circuit_constructor.create_add_gate({ d_idx, c_idx, a_idx, fr::one(), fr::neg_one(), fr::neg_one(), fr::zero() });

    bool result = circuit_constructor.check_circuit();
    EXPECT_EQ(result, false);
}

TEST(standard_circuit_constructor, test_set_variable_name)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();
    fr a = fr::one();
    uint32_t a_idx = builder.add_public_variable(a);
    const std::string a_name = "a_in";
    builder.set_variable_name(a_idx, a_name);
    fr b = fr::one();
    fr c = a + b;
    uint32_t b_idx = builder.add_variable(b);
    uint32_t c_idx = builder.add_variable(c);
    builder.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    builder.assert_equal(a_idx, b_idx);
    EXPECT_TRUE(builder.check_circuit());
    const std::string b_name = "b_in";

    EXPECT_FALSE(builder._failed);
    builder.set_variable_name(b_idx, b_name);
    EXPECT_TRUE(builder._failed);
}

TEST(standard_circuit_constructor, test_set_variable_name_todo)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();
    fr a = fr::one();
    uint32_t a_idx = builder.add_public_variable(a);
    const std::string a_name = "a_in";
    builder.set_variable_name(a_idx, a_name);
    fr b = fr::one();
    fr c = a + b;
    uint32_t b_idx = builder.add_variable(b);
    uint32_t c_idx = builder.add_variable(c);
    builder.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });

    const std::string b_name = "b_in";

    EXPECT_FALSE(builder._failed);
    builder.set_variable_name(b_idx, b_name);

    builder.assert_equal(a_idx, b_idx);
    EXPECT_TRUE(builder.check_circuit());
    EXPECT_FALSE(builder._failed); // THIS IS NOT OK
}

TEST(standard, functionality_check)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();
    fr a = fr::one();
    uint32_t a_idx = builder.add_public_variable(a);
    uint32_t b_idx = builder.add_public_variable(fr::one());
    uint32_t c_idx = builder.add_variable(fr(2));

    builder.set_variable_name(a_idx, "a_in");
    builder.set_variable_name(b_idx, "b_in");
    builder.export_circuit(std::cout);
    builder.assert_equal(a_idx, b_idx);
    builder.update_variable_names(b_idx);
    builder.export_circuit(std::cout);

    builder.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    builder.set_variable_name(c_idx, "c_in");

    std::ofstream myfile;
    myfile.open("circuit.json", std::ios::out | std::ios::trunc | std::ios::binary);

    builder.export_circuit(myfile);
    builder.export_circuit(std::cout);
}

TEST(standard, test_circuit)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();
    size_t n = 3;
    std::vector<uint32_t> coeffs = { 17, 20, 1, 10, 1, 12, 18, 6, 0, 4, 2, 14, 9, 19, 16, 11, 2, 13, 18, 6 };
    std::vector<uint32_t> idxs;
    for (size_t i = 0; i < n; i++) {
        uint32_t idx = builder.add_public_variable(coeffs[i]);
        idxs.push_back(idx);
        builder.set_variable_name(idx, "coeff_" + std::to_string(i));
    }

    fr z(10);
    uint32_t z_idx = builder.add_variable(z);
    builder.set_variable_name(z_idx, "point");
    fr res = fr::zero();
    uint32_t res_idx = builder.zero_idx; // i think assert_equal was needed for zero initialization
    builder.assert_equal(res_idx, 0);

    for (size_t i = 0; i < n; i++) {
        res = res * z;
        uint32_t mul_idx = builder.add_variable(res);
        // builder.set_variable_name(mul_idx, "mul_" + std::to_string(i));
        builder.create_mul_gate({ res_idx, z_idx, mul_idx, fr::one(), fr::neg_one(), fr::zero() });

        res = res + coeffs[i];
        uint32_t add_idx = builder.add_variable(res);
        builder.create_add_gate({
            mul_idx,
            idxs[i],
            add_idx,
            fr::one(),
            fr::one(),
            fr::neg_one(),
            fr::zero(),
        });

        res_idx = add_idx;
    }
    builder.set_variable_name(res_idx, "result");
    EXPECT_TRUE(builder.check_circuit());
    info(res);
    info(builder.num_gates);
    info(builder.get_num_variables());
    info(builder.get_num_public_inputs());

    std::ofstream myfile;
    myfile.open("strict.json", std::ios::out | std::ios::trunc | std::ios::binary);

    builder.export_circuit(myfile);
}

TEST(standard, test_circuit_field) // TODO check how to add variable names here
{
    using field_ct = proof_system::plonk::stdlib::field_t<StandardCircuitBuilder>;
    using witness_t = proof_system::plonk::stdlib::witness_t<StandardCircuitBuilder>;
    using pub_witness_t = proof_system::plonk::stdlib::public_witness_t<StandardCircuitBuilder>;

    StandardCircuitBuilder builder = StandardCircuitBuilder();

    size_t n = 20;
    std::vector<uint32_t> cfs = { 17, 20, 1, 10, 1, 12, 18, 6, 0, 4, 2, 14, 9, 19, 16, 11, 2, 13, 18, 6 };
    std::vector<field_ct> coeffs;
    for (size_t i = 0; i < n; i++) {
        coeffs.emplace_back(field_ct(pub_witness_t(&builder, cfs[i])));
    }
    field_ct z(witness_t(&builder, 10));
    field_ct res(witness_t(&builder, fr::zero()));

    for (size_t i = 0; i < n; i++) {
        res = res * z + coeffs[i];
    }
    EXPECT_TRUE(builder.check_circuit());
    info(res);
    info(builder.num_gates);
    info(builder.get_num_variables());
    info(builder.get_num_public_inputs());

    std::ofstream myfile;
    myfile.open("field.json", std::ios::out | std::ios::trunc | std::ios::binary);

    builder.export_circuit(myfile);
}

} // namespace standard_circuit_constructor_tests