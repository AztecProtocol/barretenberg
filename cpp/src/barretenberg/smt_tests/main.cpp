#include "sound_circuit.hpp"

//#include <boost/multiprecision/cpp_int.hpp>

// void unjson_circuit(nlohmann::json& circuit_info,
//                     std::vector<std::string>& variables,
//                     std::vector<uint32_t>& public_inps,
//                     std::unordered_map<std::string, std::string>& vars_of_interest,
//                     std::vector<std::vector<std::string>>& selectors,
//                     std::vector<std::vector<uint32_t>>& wit_idxs
//                     ){
//     variables = circuit_info["variables"].get<std::vector<std::string>>();

//     for(size_t i = 0; i < variables.size(); i++){
//         variables[i][1] = '0'; // to avoid x in 0x part
//     }

//     public_inps = circuit_info["public_inps"].get<std::vector<uint32_t>>();
//     vars_of_interest = circuit_info["vars_of_interest"].get<std::unordered_map<std::string, std::string>>();

//     for(size_t i = 0; i < circuit_info["gates"].size(); i++){
//         auto tmp = circuit_info["gates"][i];

//         std::vector<std::string> tmp_sel;
//         for(size_t j = 0; j < 5; j++){
//             std::string q_i = tmp[j].get<std::string>();
//             q_i[1] = '0'; // to avoid x in 0x part
//             tmp_sel.push_back(q_i);
//         }
//         selectors.push_back(tmp_sel);

//         std::vector<uint32_t> tmp_wit_idxs;
//         tmp_wit_idxs.push_back(tmp[5]);
//         tmp_wit_idxs.push_back(tmp[6]);
//         tmp_wit_idxs.push_back(tmp[7]);
//         wit_idxs.push_back(tmp_wit_idxs);
//     }
// }

void unpack_circuit(CircuitSchema& circuit_info,
                    std::vector<std::string>& variables,
                    std::vector<uint32_t>& public_inps,
                    std::unordered_map<uint32_t, std::string>& vars_of_interest,
                    std::vector<std::vector<std::string>>& selectors,
                    std::vector<std::vector<uint32_t>>& wit_idxs)
{
    for (auto var : circuit_info.variables) { // TODO use just string repr if possible?
        std::stringstream buf;
        buf << var;
        std::string tmp = buf.str();
        tmp[1] = '0'; // to avoid x in 0x part
        variables.push_back(tmp);
    }

    public_inps = circuit_info.public_inps;
    vars_of_interest = circuit_info.vars_of_interest;
    wit_idxs = circuit_info.wits;

    for (size_t i = 0; i < circuit_info.selectors.size(); i++) {
        std::vector<std::string> tmp_sel;
        for (size_t j = 0; j < 5; j++) {
            std::stringstream buf;
            buf << circuit_info.selectors[i][j];
            std::string q_i = buf.str();

            q_i[1] = '0'; // to avoid x in 0x part
            tmp_sel.push_back(q_i);
        }
        selectors.push_back(tmp_sel);
    }
}

int main(int, char** argv)
{
    const std::string fname = argv[1];
    //    nlohmann::json circuit_info = open(fname);
    CircuitSchema circuit_info = unpack(fname);

    std::vector<std::string> variables;
    std::vector<uint32_t> public_inps;
    std::unordered_map<uint32_t, std::string> vars_of_interest;
    std::vector<std::vector<std::string>> selectors;
    std::vector<std::vector<uint32_t>> wit_idxs;

    unpack_circuit(circuit_info, variables, public_inps, vars_of_interest, selectors, wit_idxs);
    check(variables, public_inps, vars_of_interest, selectors, wit_idxs, func);
}
