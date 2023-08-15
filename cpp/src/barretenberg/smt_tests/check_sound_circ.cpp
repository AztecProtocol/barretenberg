#include "check_sound_circ.hpp"

cvc5::Term const_equal(cvc5::Term& var, std::string& t, cvc5::Solver& s, cvc5::Sort& fp, uint32_t base)
{
    cvc5::Term tmp = s.mkFiniteFieldElem(t, fp, base);
    return s.mkTerm(cvc5::Kind::EQUAL, { var, tmp });
}

void init_circ(std::vector<std::string>& variables,
               std::vector<uint32_t>& pub_inps,
               std::unordered_map<uint32_t, std::string>& vars_of_interest,
               cvc5::Solver& s,
               cvc5::Sort& fp,
               std::vector<cvc5::Term>& vars,
               std::vector<cvc5::Term>& inputs)
{
    size_t num_vars = variables.size();
    // size_t num_pub_vars = pub_inps.size();

    for (size_t i = 0; i < num_vars; i++) {
        if (vars_of_interest.contains(static_cast<uint32_t>(i))) {
            vars.push_back(s.mkConst(fp, vars_of_interest[static_cast<uint32_t>(i)]));
            inputs.push_back(vars[i]);
        } else {
            vars.push_back(s.mkConst(fp, "var_" + std::to_string(i)));
        }
    }

    std::string tmp = "0";
    s.assertFormula(const_equal(vars[0], tmp, s, fp));
    tmp = "1";
    s.assertFormula(const_equal(vars[1], tmp, s, fp));

    for (auto i : pub_inps) {
        s.assertFormula(const_equal(vars[i], variables[i], s, fp));
    }
}

void add_gates(std::vector<std::vector<std::string>>& selectors,
               std::vector<std::vector<uint32_t>>& wits,
               std::vector<cvc5::Term>& vars,
               cvc5::Solver& s,
               cvc5::Sort& fp)
{
    // assert(selectors.size() == wits.size()); TODO info

    for (size_t i = 0; i < selectors.size(); i++) {
        cvc5::Term q_m = s.mkFiniteFieldElem(selectors[i][0], fp, 16);
        cvc5::Term q_1 = s.mkFiniteFieldElem(selectors[i][1], fp, 16);
        cvc5::Term q_2 = s.mkFiniteFieldElem(selectors[i][2], fp, 16);
        cvc5::Term q_3 = s.mkFiniteFieldElem(selectors[i][3], fp, 16);
        cvc5::Term q_c = s.mkFiniteFieldElem(selectors[i][4], fp, 16);

        uint32_t w_l = wits[i][0];
        uint32_t w_r = wits[i][1];
        uint32_t w_o = wits[i][2];

        cvc5::Term tmp;
        cvc5::Term eq = s.mkFiniteFieldElem("0", fp);

        // mult selector
        if (static_cast<std::string>(q_m.getFiniteFieldValue()) != "0") {
            tmp = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { vars[w_l], vars[w_r] });
            tmp = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { tmp, q_m });
            eq = s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { eq, tmp });
        }

        // w_l selector
        if (static_cast<std::string>(q_1.getFiniteFieldValue()) != "0") {
            tmp = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { vars[w_l], q_1 });
            eq = s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { eq, tmp });
        }

        // w_r selector
        if (static_cast<std::string>(q_2.getFiniteFieldValue()) != "0") {
            tmp = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { vars[w_r], q_2 });
            eq = s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { eq, tmp });
        }

        // w_o selector
        if (static_cast<std::string>(q_3.getFiniteFieldValue()) != "0") {
            tmp = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { vars[w_o], q_3 });
            eq = s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { eq, tmp });
        }

        // w_c selector
        if (static_cast<std::string>(q_c.getFiniteFieldValue()) != "0") {
            eq = s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { eq, q_c });
        }

        eq = s.mkTerm(cvc5::Kind::EQUAL, { eq, s.mkFiniteFieldElem("0", fp) });
        s.assertFormula(eq);
    }
}

void check(std::vector<std::string>& variables,
           std::vector<uint32_t>& pub_inps,
           std::unordered_map<uint32_t, std::string>& vars_of_interest,
           std::vector<std::vector<std::string>>& selectors,
           std::vector<std::vector<uint32_t>>& wits,
           std::function<cvc5::Term(std::vector<cvc5::Term>&, cvc5::Solver&, cvc5::Sort&)> func
           // TODO need to think how to order this thing to match the results and everything
)
{
    cvc5::Solver s;
    s.setOption("produce-models", "true"); // TODO debug option if sat
    cvc5::Sort fp = s.mkFiniteFieldSort(r);
    std::vector<cvc5::Term> vars;
    std::vector<cvc5::Term> inputs;

    init_circ(variables, pub_inps, vars_of_interest, s, fp, vars, inputs);
    add_gates(selectors, wits, vars, s, fp);
    auto ev = func(inputs, s, fp); // TODO Debug

    auto start = std::chrono::high_resolution_clock::now();
    cvc5::Result r = s.checkSat();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Time elapsed: " << static_cast<double>(duration.count()) / 1e6 << " sec" << std::endl;
    std::cout << "Gates: " << selectors.size() << std::endl;
    std::cout << "Result: " << r << std::endl;

    if (r.isSat()) {
        for (auto x : inputs) {
            std::cout << x << " = " << s.getValue(x).getFiniteFieldValue() << std::endl;
        }
        std::cout << "ev = " << s.getValue(ev) << std::endl;
    }
}

// nlohmann::json open(const std::string& filename){
//     std::ifstream fin;
//     fin.open(filename);
//     if(!fin.is_open()){
//         throw std::invalid_argument("no such a file");
//     }
//     nlohmann::json res;
//     fin >> res;
//     return res;
// }

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