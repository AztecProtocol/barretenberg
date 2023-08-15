#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>

#include "barretenberg/serialize/cbind.hpp"
#include "barretenberg/serialize/msgpack.hpp"
#include <cvc5/cvc5.h>

const std::string r = "21888242871839275222246405745257275088548364400416034343698204186575808495617";
const std::string q = "21888242871839275222246405745257275088696311157297823662689037894645226208583";

struct CircuitSchema {
    std::vector<uint32_t> public_inps;
    std::unordered_map<uint32_t, std::string> vars_of_interest;
    std::vector<barretenberg::fr> variables;
    std::vector<std::vector<barretenberg::fr>> selectors;
    std::vector<std::vector<uint32_t>> wits;
    MSGPACK_FIELDS(public_inps, vars_of_interest, variables, selectors, wits);
};
CircuitSchema unpack(const std::string&);
//// nlohmann::json open(const std::string& filename);

cvc5::Term const_equal(cvc5::Term&, std::string&, cvc5::Solver&, cvc5::Sort&, uint32_t base = 16);

void init_circ(std::vector<std::string>& variables,
               std::vector<uint32_t>& pub_inps,
               std::unordered_map<uint32_t, std::string>& vars_of_interest,
               cvc5::Solver& s,
               cvc5::Sort& fp,
               std::vector<cvc5::Term>& vars,
               std::vector<cvc5::Term>& inputs);

void add_gates(std::vector<std::vector<std::string>>& selectors,
               std::vector<std::vector<uint32_t>>& wits,
               std::vector<cvc5::Term>& vars,
               cvc5::Solver& s,
               cvc5::Sort& fp);

void check(std::vector<std::string>& variables,
           std::vector<uint32_t>& pub_inps,
           std::unordered_map<uint32_t, std::string>& vars_of_interst,
           std::vector<std::vector<std::string>>& selectors,
           std::vector<std::vector<uint32_t>>& wits,
           std::function<cvc5::Term(std::vector<cvc5::Term>&, cvc5::Solver&, cvc5::Sort&)> func);

cvc5::Term func(std::vector<cvc5::Term>& inputs, cvc5::Solver& s, cvc5::Sort& fp);
