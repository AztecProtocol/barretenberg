#include "barretenberg/crypto/generators/generator_data.hpp"
#include "barretenberg/crypto/pedersen_commitment/pedersen.hpp"
#include "barretenberg/proof_system/circuit_builder/standard_circuit_builder.hpp"
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

using field_ct = proof_system::plonk::stdlib::field_t<StandardCircuitBuilder>;
using witness_t = proof_system::plonk::stdlib::witness_t<StandardCircuitBuilder>;
using pub_witness_t = proof_system::plonk::stdlib::public_witness_t<StandardCircuitBuilder>;

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
    EXPECT_FALSE(builder._failed);
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
    EXPECT_TRUE(builder._failed);
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
    size_t n = 35;
    // std::vector<uint32_t> coeffs = { 17, 20, 1, 10, 1, 12, 18, 6, 0, 4, 2, 14, 9, 19, 16, 11, 2, 13, 18, 6 };
    std::vector<fr> coeffs;
    std::vector<uint32_t> idxs;
    for (size_t i = 0; i < n; i++) {
        fr tmp_coeff = fr::random_element();
        uint32_t idx = builder.add_public_variable(tmp_coeff);
        idxs.push_back(idx);
        coeffs.push_back(tmp_coeff);
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
    builder.export_circuit_json(myfile);
    myfile.close();

    myfile.open("strict.pack", std::ios::out | std::ios::trunc | std::ios::binary);
    builder.export_circuit(myfile);
    myfile.close();
}

// TEST(standard, test_circuit_field) // TODO check how to add variable names here
// {
//     StandardCircuitBuilder builder = StandardCircuitBuilder();

//     size_t n = 20;
//     std::vector<uint32_t> cfs = { 17, 20, 1, 10, 1, 12, 18, 6, 0, 4, 2, 14, 9, 19, 16, 11, 2, 13, 18, 6 };
//     std::vector<field_ct> coeffs;
//     for (size_t i = 0; i < n; i++) {
//         coeffs.emplace_back(field_ct(pub_witness_t(&builder, cfs[i])));
//     }
//     field_ct z(witness_t(&builder, 10));
//     field_ct res(witness_t(&builder, fr::zero()));

//     for (size_t i = 0; i < n; i++) {
//         res = res * z + coeffs[i];
//     }
//     EXPECT_TRUE(builder.check_circuit());
//     info(res);
//     info(builder.num_gates);
//     info(builder.get_num_variables());
//     info(builder.get_num_public_inputs());

//     std::ofstream myfile;
//     myfile.open("field.json", std::ios::out | std::ios::trunc | std::ios::binary);

//     builder.export_circuit(myfile);
// }

} // namespace standard_circuit_constructor_tests