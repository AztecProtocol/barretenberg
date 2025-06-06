#include "circuit_base.hpp"

namespace smt_circuit {

CircuitBase::CircuitBase(std::unordered_map<uint32_t, std::string>& variable_names,
                         std::vector<bb::fr>& variables,
                         std::vector<uint32_t>& public_inps,
                         std::vector<uint32_t>& real_variable_index,
                         std::vector<uint32_t>& real_variable_tags,
                         Solver* solver,
                         TermType type,
                         const std::string& tag,
                         bool enable_optimizations)
    : variables(variables)
    , variable_names(variable_names)
    , real_variable_index(real_variable_index)
    , real_variable_tags(real_variable_tags)
    , enable_optimizations(enable_optimizations)
    , solver(solver)
    , type(type)
    , tag(tag)
{
    if (!this->tag.empty() && tag[0] != '_') {
        this->tag = "_" + this->tag;
    }

    for (auto& x : variable_names) {
        variable_names_inverse.insert({ x.second, x.first });
    }

    this->init();

    for (const auto& i : public_inps) {
        this->symbolic_vars[this->real_variable_index[i]] == this->variables[i];
        this->public_inps.insert(i);
    }
}

/**
 * Creates all the needed symbolic variables and constants
 * which are used in circuit.
 *
 */
void CircuitBase::init()
{
    size_t num_vars = this->variables.size();

    for (uint32_t i = 0; i < num_vars; i++) {
        uint32_t real_idx = this->real_variable_index[i];
        if (this->symbolic_vars.contains(real_idx)) {
            continue;
        }

        std::string name = variable_names.contains(real_idx) ? variable_names[real_idx] : "var_" + std::to_string(i);
        name += this->tag;
        symbolic_vars.insert({ real_idx, STerm::Var(name, this->solver, this->type) });

        optimized.insert({ real_idx, true });
    }

    this->symbolic_vars[this->variable_names_inverse["zero"]] == bb::fr::zero();
}

/**
 * @brief Returns a previously named symbolic variable.
 *
 * @param name
 * @return STerm
 */
STerm CircuitBase::operator[](const std::string& name)
{
    if (!this->variable_names_inverse.contains(name)) {
        throw std::invalid_argument("No such an item `" + name + "` in vars or it vas not declared as interesting");
    }
    uint32_t idx = this->variable_names_inverse[name];
    return this->symbolic_vars[idx];
}

} // namespace smt_circuit
