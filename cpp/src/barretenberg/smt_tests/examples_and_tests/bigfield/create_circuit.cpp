#include "create_circuit.hpp"

void create_circuit(const std::string& fname, bool pub_ab)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();
    fq inputs[2]{ fq::random_element(), fq::random_element() };
    fq_ct a, b;
    if (pub_ab) {
        a = fq_ct(public_witness_ct(&builder, fr(uint256_t(inputs[0]).slice(0, fq_ct::NUM_LIMB_BITS * 2))),
                  public_witness_ct(
                      &builder, fr(uint256_t(inputs[0]).slice(fq_ct::NUM_LIMB_BITS * 2, fq_ct::NUM_LIMB_BITS * 4))));
        b = fq_ct(public_witness_ct(&builder, fr(uint256_t(inputs[1]).slice(0, fq_ct::NUM_LIMB_BITS * 2))),
                  public_witness_ct(
                      &builder, fr(uint256_t(inputs[1]).slice(fq_ct::NUM_LIMB_BITS * 2, fq_ct::NUM_LIMB_BITS * 4))));
    } else {
        a = fq_ct(
            witness_ct(&builder, fr(uint256_t(inputs[0]).slice(0, fq_ct::NUM_LIMB_BITS * 2))),
            witness_ct(&builder, fr(uint256_t(inputs[0]).slice(fq_ct::NUM_LIMB_BITS * 2, fq_ct::NUM_LIMB_BITS * 4))));
        b = fq_ct(
            witness_ct(&builder, fr(uint256_t(inputs[1]).slice(0, fq_ct::NUM_LIMB_BITS * 2))),
            witness_ct(&builder, fr(uint256_t(inputs[1]).slice(fq_ct::NUM_LIMB_BITS * 2, fq_ct::NUM_LIMB_BITS * 4))));
    }

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
    // uint512_t result = c.get_value();

    bool circ_result = builder.check_circuit();
    info(circ_result);

    std::ofstream myfile;
    myfile.open(fname + ".pack", std::ios::out | std::ios::trunc | std::ios::binary);
    builder.export_circuit(myfile);
    myfile.close();

    myfile.open(fname + ".json", std::ios::out | std::ios::trunc | std::ios::binary);
    builder.export_circuit_json(myfile);
    myfile.close();
}