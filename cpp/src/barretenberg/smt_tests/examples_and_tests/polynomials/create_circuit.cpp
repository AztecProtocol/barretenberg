#include "create_circuit.hpp"
namespace {
auto& engine = numeric::random::get_debug_engine();
}

void create_circuit(const std::string& fname, size_t n, bool pub_coeffs)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();
    std::vector<fr> coeffs;
    std::vector<uint32_t> idxs;
    for (size_t i = 0; i < n; i++) {
        fr tmp_coeff = fr::random_element();
        uint32_t idx;
        if (pub_coeffs) {
            idx = builder.add_public_variable(tmp_coeff);
        } else {
            idx = builder.add_variable(tmp_coeff);
        }
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

    info("evaluation at point ", z, ": ", res);
    info("gates: ", builder.num_gates);
    info("variables: ", builder.get_num_variables());
    info("public inputs: ", builder.get_num_public_inputs());

    std::ofstream myfile;

    myfile.open(fname + ".json", std::ios::out | std::ios::trunc | std::ios::binary);
    builder.export_circuit_json(myfile);
    myfile.close();

    myfile.open(fname + ".pack", std::ios::out | std::ios::trunc | std::ios::binary);
    builder.export_circuit(myfile);
    myfile.close();
}