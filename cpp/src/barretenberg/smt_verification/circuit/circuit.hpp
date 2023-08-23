#pragma once
#include <fstream>
#include <limits>
#include <sstream>
//#include <format>
#include "barretenberg/serialize/cbind.hpp"
#include "barretenberg/serialize/msgpack.hpp"
#include <string>
#include <unordered_map>

#include "barretenberg/smt_verification/terms/ffterm.hpp"
//#include "barretenberg/smt_tests/terms/bool.hpp"

namespace smt_circuit {
using namespace smt_solver;
using namespace smt_terms;

const std::string p = "21888242871839275222246405745257275088548364400416034343698204186575808495617";

struct CircuitSchema {
    std::vector<uint32_t> public_inps;
    std::unordered_map<uint32_t, std::string> vars_of_interest;
    std::vector<barretenberg::fr> variables;
    std::vector<std::vector<barretenberg::fr>> selectors;
    std::vector<std::vector<uint32_t>> wits;
    MSGPACK_FIELDS(public_inps, vars_of_interest, variables, selectors, wits);
};

class Circuit { // SymCircuit?
  private:
    std::vector<std::string> variables;
    std::vector<uint32_t> public_inps;
    std::unordered_map<uint32_t, std::string> vars_of_interest;
    std::unordered_map<std::string, uint32_t> terms;
    std::vector<std::vector<std::string>> selectors;
    std::vector<std::vector<uint32_t>> wit_idxs;
    std::vector<FFTerm> vars;

    Solver* solver;
    std::string tag;

    void init();
    void add_gates();

  public:
    explicit Circuit(CircuitSchema& circuit_info, Solver* solver, const std::string& tag = "");

    FFTerm operator[](const std::string& name);
    inline uint32_t get_num_gates() const { return static_cast<uint32_t>(selectors.size()); };
    inline uint32_t get_num_vars() const { return static_cast<uint32_t>(vars.size()); };
};

CircuitSchema unpack_from_buffer(const msgpack::sbuffer& buf);
CircuitSchema unpack_from_file(const std::string& fname);

std::pair<Circuit, Circuit> unique_witness(CircuitSchema& circuit_info,
                                           Solver* s,
                                           const std::vector<std::string>& inputs,
                                           const std::vector<std::string>& outputs);

}; // namespace smt_circuit