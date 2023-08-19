#include <gtest/gtest.h>

#include "barretenberg/numeric/random/engine.hpp"

#include "barretenberg/ecc/curves/bn254/fq.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"

#include "barretenberg/stdlib/primitives/bigfield/bigfield.hpp"
#include "barretenberg/stdlib/primitives/bool/bool.hpp"
#include "barretenberg/stdlib/primitives/byte_array/byte_array.hpp"
#include "barretenberg/stdlib/primitives/field/field.hpp"

#include "barretenberg/plonk/proof_system/constants.hpp"
#include "barretenberg/plonk/proof_system/prover/prover.hpp"
#include "barretenberg/plonk/proof_system/verifier/verifier.hpp"

#include "barretenberg/stdlib/primitives/circuit_builders/circuit_builders.hpp"
#include "barretenberg/stdlib/primitives/curves/bn254.hpp"

#include "barretenberg/polynomials/polynomial_arithmetic.hpp"
#include <fstream>
#include <memory>
#include <utility>

using namespace barretenberg;
using namespace proof_system::plonk;

using field_ct = proof_system::plonk::stdlib::field_t<StandardCircuitBuilder>;
using witness_t = proof_system::plonk::stdlib::witness_t<StandardCircuitBuilder>;
using pub_witness_t = proof_system::plonk::stdlib::public_witness_t<StandardCircuitBuilder>;

using bn254 = stdlib::bn254<StandardCircuitBuilder>;

using fr_ct = bn254::ScalarField;
using fq_ct = bn254::BaseField;
using public_witness_ct = bn254::public_witness_ct;
using witness_ct = bn254::witness_ct;

auto NUM_LIMB_BITS = NUM_LIMB_BITS_IN_FIELD_SIMULATION;

TEST(bigifield, test_mul)
{
    info(fr::neg_one());
    info(fq::neg_one());
    StandardCircuitBuilder builder = StandardCircuitBuilder();
    fq inputs[2]{ fq::random_element(), fq::random_element() };
    fq_ct a(public_witness_ct(&builder, fr(uint256_t(inputs[0]).slice(0, fq_ct::NUM_LIMB_BITS * 2))),
            public_witness_ct(&builder,
                              fr(uint256_t(inputs[0]).slice(fq_ct::NUM_LIMB_BITS * 2, fq_ct::NUM_LIMB_BITS * 4))));
    fq_ct b(public_witness_ct(&builder, fr(uint256_t(inputs[1]).slice(0, fq_ct::NUM_LIMB_BITS * 2))),
            public_witness_ct(&builder,
                              fr(uint256_t(inputs[1]).slice(fq_ct::NUM_LIMB_BITS * 2, fq_ct::NUM_LIMB_BITS * 4))));

    // info(b);
    // info(b.binary_basis_limbs[0]);
    // info(b.binary_basis_limbs[1]);
    // info(b.binary_basis_limbs[2]);
    // info(b.binary_basis_limbs[3]);
    // info(b.binary_basis_limbs[3].element.witness_index);

    info("a = ", a.get_value());
    info("b = ", b.get_value());
    builder.set_variable_name(a.binary_basis_limbs[0].element.witness_index, "a_limb_0");
    builder.set_variable_name(a.binary_basis_limbs[1].element.witness_index, "a_limb_1");
    builder.set_variable_name(a.binary_basis_limbs[2].element.witness_index, "a_limb_2");
    builder.set_variable_name(a.binary_basis_limbs[3].element.witness_index, "a_limb_3");

    builder.set_variable_name(b.binary_basis_limbs[0].element.witness_index, "b_limb_0");
    builder.set_variable_name(b.binary_basis_limbs[1].element.witness_index, "b_limb_1");
    builder.set_variable_name(b.binary_basis_limbs[2].element.witness_index, "b_limb_2");
    builder.set_variable_name(b.binary_basis_limbs[3].element.witness_index, "b_limb_3");

    // uint64_t before = builder.get_num_gates();
    fq_ct c = a * b;
    info("c = ", c.get_value());

    builder.set_variable_name(c.binary_basis_limbs[0].element.witness_index, "c_limb_0");
    builder.set_variable_name(c.binary_basis_limbs[1].element.witness_index, "c_limb_1");
    builder.set_variable_name(c.binary_basis_limbs[2].element.witness_index, "c_limb_2");
    builder.set_variable_name(c.binary_basis_limbs[3].element.witness_index, "c_limb_3");

    info(builder.variable_names[c.binary_basis_limbs[3].element.witness_index]);

    // builder.set_variable_name("c_" + std::to_string(i), c.get_witness_index());
    uint64_t after = builder.get_num_gates();
    info(after);
    info(builder.get_num_variables());

    // uint256_t modulus{ Bn254FqParams::modulus_0,
    //                    Bn254FqParams::modulus_1,
    //                    Bn254FqParams::modulus_2,
    //                    Bn254FqParams::modulus_3 };

    fq expected = (inputs[0] * inputs[1]);
    expected = expected.from_montgomery_form();
    uint512_t result = c.get_value();
    info();

    EXPECT_EQ(result.lo.data[0], expected.data[0]);
    EXPECT_EQ(result.lo.data[1], expected.data[1]);
    EXPECT_EQ(result.lo.data[2], expected.data[2]);
    EXPECT_EQ(result.lo.data[3], expected.data[3]);
    EXPECT_EQ(result.hi.data[0], 0ULL);
    EXPECT_EQ(result.hi.data[1], 0ULL);
    EXPECT_EQ(result.hi.data[2], 0ULL);
    EXPECT_EQ(result.hi.data[3], 0ULL);

    bool circ_result = builder.check_circuit();
    EXPECT_EQ(circ_result, true);

    std::ofstream myfile;
    myfile.open("bigfield_mul.pack", std::ios::out | std::ios::trunc | std::ios::binary);
    builder.export_circuit(myfile);
    myfile.close();

    myfile.open("bigfield_mul.json", std::ios::out | std::ios::trunc | std::ios::binary);
    builder.export_circuit_json(myfile);
    myfile.close();
}

TEST(a, b)
{
    fr a = fr(uint256_t("192c02920a781f356941d00b513a1be4a4a5bc9856681cb51b4e763369f69ce8"));
    fr b = fr(uint256_t("291cc3373297a87ea5fb34b37798e8f195e16e4a7866bdac5316b770b82f325d"));
    info(a);
    info(b);
    info(a * b);
}