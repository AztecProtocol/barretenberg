#include <chrono>
#include <iostream>

#include "barretenberg/smt_tests/circuit/circuit.hpp"
#include "create_circuit.hpp"

using namespace smt_circuit;

const std::string r = "21888242871839275222246405745257275088548364400416034343698204186575808495617";
const std::string q = "21888242871839275222246405745257275088696311157297823662689037894645226208583";
const std::string r_hex = "30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001";
const std::string q_hex = "30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd47";

std::vector<FFTerm> bigfield_multiplication(Circuit& c, Solver* s)
{
    FFTerm a_limb0 = c["a_limb_0"];
    FFTerm a_limb1 = c["a_limb_1"];
    FFTerm a_limb2 = c["a_limb_2"];
    FFTerm a_limb3 = c["a_limb_3"];

    FFTerm b_limb0 = c["b_limb_0"];
    FFTerm b_limb1 = c["b_limb_1"];
    FFTerm b_limb2 = c["b_limb_2"];
    FFTerm b_limb3 = c["b_limb_3"];

    FFTerm c_limb0 = c["c_limb_0"];
    FFTerm c_limb1 = c["c_limb_1"];
    FFTerm c_limb2 = c["c_limb_2"];
    FFTerm c_limb3 = c["c_limb_3"];

    FFTerm two68 = Const("100000000000000000", s);
    FFTerm two136 = two68 * two68;
    FFTerm two204 = two136 * two68;

    FFTerm a = a_limb0 + two68 * a_limb1 + two136 * a_limb2 + two204 * a_limb3;
    FFTerm b = b_limb0 + two68 * b_limb1 + two136 * b_limb2 + two204 * b_limb3;
    FFTerm cr = c_limb0 + two68 * c_limb1 + two136 * c_limb2 + two204 * c_limb3;
    FFTerm n = Var("n", s);
    FFTerm q_ = Const(q, s, 10); // Const(q_hex, s)
    a* b != cr + n* q_;
    return { cr, n };
}

void model_variables(Circuit& c, Solver* s, std::vector<FFTerm>& evaluation)
{
    std::unordered_map<std::string, cvc5::Term> terms;
    for (size_t i = 0; i < 4; i++) {
        terms.insert({ "a_limb_" + std::to_string(i), c["a_limb_" + std::to_string(i)] });
        terms.insert({ "b_limb_" + std::to_string(i), c["b_limb_" + std::to_string(i)] });
        terms.insert({ "c_limb_" + std::to_string(i), c["c_limb_" + std::to_string(i)] });
    }
    terms.insert({ "cr", evaluation[0] });
    terms.insert({ "n", evaluation[1] });

    auto values = s->model(terms);

    for (size_t i = 0; i < 4; i++) {
        std::string tmp = "a_limb_" + std::to_string(i);
        info(tmp, " = ", values[tmp]);
    }
    for (size_t i = 0; i < 4; i++) {
        std::string tmp = "b_limb_" + std::to_string(i);
        info(tmp, " = ", values[tmp]);
    }
    for (size_t i = 0; i < 4; i++) {
        std::string tmp = "c_limb_" + std::to_string(i);
        info(tmp, " = ", values[tmp]);
    }
    info("cr = ", values["cr"]);
    info("n = ", values["n"]);
}

int main(int, char** argv)
{
    const std::string fname = argv[1];
    create_circuit(fname, true);

    CircuitSchema circuit_info = unpack(fname + ".pack");

    Solver s(r, true, 10);
    Circuit circuit(circuit_info, &s);

    std::vector<FFTerm> ev = bigfield_multiplication(circuit, &s);

    auto start = std::chrono::high_resolution_clock::now();
    bool res = s.check();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    info();
    info("Gates: ", circuit.get_num_gates());
    info("Result: ", s.getResult());
    info("Time elapsed: ", static_cast<double>(duration.count()) / 1e6, " sec");

    if (res) {
        model_variables(circuit, &s, ev);
    }
}
