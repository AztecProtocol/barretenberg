#include "solver.hpp"
#include <iostream>

namespace smt_solver {

bool Solver::check()
{
    cvc5::Result result = this->s.checkSat();
    this->res = result.isSat();
    this->checked = true;
    return this->res;
}

std::unordered_map<std::string, std::string> Solver::model(std::unordered_map<std::string, cvc5::Term>& terms) const
{
    if (!this->res) {
        throw std::length_error("There's no solution");
    }
    std::unordered_map<std::string, std::string> resulting_model;
    for (auto& term : terms) {
        std::string str_val = this->s.getValue(term.second).getFiniteFieldValue();
        resulting_model.insert({ term.first, str_val });
    }
    return resulting_model;
}

}; // namespace smt_solver
