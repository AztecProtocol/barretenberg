#include <chrono>
#include <iostream>

#include "barretenberg/smt_tests/circuit/circuit.hpp"
#include "create_circuit.hpp"

using namespace smt_circuit;

const std::string r = "21888242871839275222246405745257275088548364400416034343698204186575808495617";
const std::string q = "21888242871839275222246405745257275088696311157297823662689037894645226208583";
const std::string r_hex = "30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001";
const std::string q_hex = "30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd47";

uint32_t num_coeffs = 35;

FFTerm polynomial_evaluation(Circuit& c)
{
    std::vector<smt_terms::FFTerm> coeffs(num_coeffs);
    for (size_t i = 0; i < num_coeffs; i++) {
        coeffs[i] = c["coeff_" + std::to_string(i)];
    }

    FFTerm point = c["point"];
    FFTerm result = c["result"];

    // FFTerm ev = c["one"];
    FFTerm ev = c["zero"];
    for (size_t i = 0; i < num_coeffs; i++) {
        ev = ev * point + coeffs[i];
    }

    result != ev;
    return ev;
}

void model_variables(Circuit& c, Solver* s, FFTerm& evaluation)
{
    std::unordered_map<std::string, cvc5::Term> terms;
    terms.insert({ "point", c["point"] });
    terms.insert({ "result", c["result"] });
    terms.insert({ "evaluation", evaluation });

    auto values = s->model(terms);

    std::cout << "point = " << values["point"] << std::endl;
    std::cout << "circuit_result = " << values["result"] << std::endl;
    std::cout << "function_evaluation = " << values["evaluation"] << std::endl;
}

int main(int, char** argv)
{
    const std::string fname = argv[1];
    size_t n = static_cast<size_t>(std::stoi(argv[2]));
    create_circuit(fname, n, true);

    CircuitSchema circuit_info = unpack(fname + ".pack");

    Solver s(r, true, 10);
    Circuit circuit(circuit_info, &s);

    FFTerm ev = polynomial_evaluation(circuit);

    auto start = std::chrono::high_resolution_clock::now();
    bool res = s.check();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "Gates: " << circuit.get_num_gates() << std::endl;
    std::cout << "Result: " << s.getResult() << std::endl;
    std::cout << "Time elapsed: " << static_cast<double>(duration.count()) / 1e6 << " sec" << std::endl;

    if (res) {
        model_variables(circuit, &s, ev);
    }
}
