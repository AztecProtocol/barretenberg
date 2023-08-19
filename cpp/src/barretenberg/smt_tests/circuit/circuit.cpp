#include "circuit.hpp"
namespace smt_circuit {

Circuit::Circuit(CircuitSchema& circuit_info, Solver* solver)
    : public_inps(circuit_info.public_inps)
    , vars_of_interest(circuit_info.vars_of_interest)
    , wit_idxs(circuit_info.wits)
    , solver(solver)
{
    for (auto var : circuit_info.variables) {
        std::stringstream buf; // TODO(alex): looks bad. Would be great to create tostring() converter
        buf << var;
        std::string tmp = buf.str();
        tmp[1] = '0'; // avoiding x in 0x part
        variables.push_back(tmp);
    }

    for (auto& x : vars_of_interest) {
        terms.insert({ x.second, x.first });
    }

    // I hope they are still at these idxs
    vars_of_interest.insert({ 0, "zero" });
    vars_of_interest.insert({ 1, "one" });
    terms.insert({ "zero", 0 });
    terms.insert({ "one", 1 });

    for (auto sel : circuit_info.selectors) {
        std::vector<std::string> tmp_sel;
        for (size_t j = 0; j < 5; j++) {
            std::stringstream buf; // TODO(alex): #2
            buf << sel[j];
            std::string q_i = buf.str();
            q_i[1] = '0'; // avoiding x in 0x part
            tmp_sel.push_back(q_i);
        }
        selectors.push_back(tmp_sel);
    }

    this->init();
    this->add_gates();
}

void Circuit::init()
{
    size_t num_vars = variables.size();

    // I believe noone will name zero and one otherwise...
    vars.push_back(Var("zero", this->solver));
    vars.push_back(Var("one", this->solver));

    // TODO(alex): maybe public variables should be Consts
    for (size_t i = 2; i < num_vars; i++) {
        if (vars_of_interest.contains(static_cast<uint32_t>(i))) {
            std::string name = vars_of_interest[static_cast<uint32_t>(i)];
            vars.push_back(Var(name, this->solver));
        } else {
            vars.push_back(Var("var_" + std::to_string(i), this->solver));
        }
    }

    vars[0] == Const("0", this->solver);
    vars[1] == Const("1", this->solver);

    for (auto i : public_inps) {
        vars[i] == Const(variables[i], this->solver);
    }
}

void Circuit::add_gates()
{
    for (size_t i = 0; i < get_num_gates(); i++) {
        FFTerm q_m = Const(selectors[i][0], this->solver);
        FFTerm q_1 = Const(selectors[i][1], this->solver);
        FFTerm q_2 = Const(selectors[i][2], this->solver);
        FFTerm q_3 = Const(selectors[i][3], this->solver);
        FFTerm q_c = Const(selectors[i][4], this->solver);

        uint32_t w_l = wit_idxs[i][0];
        uint32_t w_r = wit_idxs[i][1];
        uint32_t w_o = wit_idxs[i][2];

        FFTerm eq = vars[0];

        // mult selector
        if (std::string(q_m) != "0") {
            eq += q_m * vars[w_l] * vars[w_r];
        }
        // w_l selector
        if (std::string(q_1) != "0") {
            eq += q_1 * vars[w_l];
        }
        // w_r selector
        if (std::string(q_2) != "0") {
            eq += q_2 * vars[w_r];
        }
        // w_o selector
        if (std::string(q_3) != "0") {
            eq += q_3 * vars[w_o];
        }
        // w_c selector
        if (std::string(q_c) != "0") {
            eq += q_c;
        }
        eq == vars[0];
    }
}

FFTerm Circuit::operator[](const std::string& name)
{
    if (!this->terms.contains(name)) {
        throw std::length_error("No such an item " + name + " in vars or it vas not declared as interesting");
    }
    uint32_t idx = this->terms[name];
    return this->vars[idx];
}

CircuitSchema unpack(const std::string& filename)
{
    std::ifstream fin;
    fin.open(filename, std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        throw std::invalid_argument("file not found");
    }
    if (fin.tellg() == -1) {
        throw std::invalid_argument("something went wrong");
    }

    fin.ignore(std::numeric_limits<std::streamsize>::max()); // ohboy
    std::streamsize fsize = fin.gcount();
    fin.clear();
    fin.seekg(0, std::ios_base::beg);
    info("File size: ", fsize);

    CircuitSchema cir;
    char* encoded_data = (char*)aligned_alloc(64, static_cast<size_t>(fsize));
    fin.read(encoded_data, fsize);
    msgpack::unpack((const char*)encoded_data, static_cast<size_t>(fsize)).get().convert(cir);
    return cir;
}
}; // namespace smt_circuit