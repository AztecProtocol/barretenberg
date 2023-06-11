#include <iostream>
#include <string>
#include <vector>
#include <barretenberg/common/container.hpp>
#include <barretenberg/dsl/acir_proofs/acir_composer.hpp>
#include <barretenberg/dsl/acir_format/acir_format.hpp>
#include <barretenberg/srs/global_crs.hpp>
#include "barretenberg/bb/get_crs.hpp"
#include "file_io.hpp"
#include "get_bytecode.hpp"

uint32_t MAX_CIRCUIT_SIZE = 1 << 19;
auto CRS_PATH = "./crs";
bool verbose = false;

// void init(const std::string& jsonPath) {}

void proveAndVerify(const std::string& jsonPath, const std::string& witnessPath, bool recursive)
{
    // Must +1!
    auto g1_data = get_g1_data(CRS_PATH, MAX_CIRCUIT_SIZE + 1);
    auto g2_data = get_g2_data(CRS_PATH);
    barretenberg::srs::init_crs_factory(g1_data, g2_data);

    auto bytecode = get_bytecode(jsonPath);
    auto witness_data = read_file(witnessPath);

    auto acir_composer = new acir_proofs::AcirComposer(1 << 19, verbose);
    auto constraint_system = from_buffer<acir_format::acir_format>(bytecode.data());
    // We need to prefix the number of fields to comply with serialization format.
    // TODO: Make noir side output witness data prefixed.
    auto witness =
        from_buffer<acir_format::WitnessVector>(join({ to_buffer((uint32_t)witness_data.size() / 32), witness_data }));
    auto proof =
        acir_composer->create_proof(barretenberg::srs::get_crs_factory(), constraint_system, witness, recursive);
    auto verified = acir_composer->verify_proof(proof, recursive);
    info("verified: ", verified);
}

void prove(const std::string& jsonPath, const std::string& witnessPath, bool recursive, const std::string& outputPath);
void gateCount(const std::string& jsonPath);
bool verify(const std::string& proofPath, bool recursive, const std::string& vk);
void contract(const std::string& outputPath, const std::string& vk);
void writeVk(const std::string& jsonPath, const std::string& outputPath);
void proofAsFields(const std::string& proofPath, int numPublicInputs, const std::string& outputPath);
void vkAsFields(const std::string& inputPath, const std::string& outputPath);

// Helper function to check if a flag is present
bool flagPresent(std::vector<std::string>& args, const std::string& flag)
{
    return std::find(args.begin(), args.end(), flag) != args.end();
}

// Helper function to get an option value
std::string getOption(std::vector<std::string>& args, const std::string& option, const std::string& defaultValue)
{
    auto itr = std::find(args.begin(), args.end(), option);
    return (itr != args.end() && std::next(itr) != args.end()) ? *(std::next(itr)) : defaultValue;
}

int main(int argc, char* argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc);
    verbose = flagPresent(args, "-v") || flagPresent(args, "--verbose");

    if (args.empty()) {
        std::cerr << "No command provided.\n";
        return 1;
    }

    std::string command = args[0];

    if (command == "prove_and_verify") {
        std::string jsonPath = getOption(args, "-b", "./target/main.json");
        std::string witnessPath = getOption(args, "-w", "./target/witness.tr");
        bool recursive = flagPresent(args, "-r") || flagPresent(args, "--recursive");
        proveAndVerify(jsonPath, witnessPath, recursive);
    } else if (command == "prove") {
        // Further implementation ...
    } else if (command == "gates") {
        // Further implementation ...
    } else if (command == "verify") {
        // Further implementation ...
    } else if (command == "contract") {
        // Further implementation ...
    } else if (command == "write_vk") {
        // Further implementation ...
    } else if (command == "proof_as_fields") {
        // Further implementation ...
    } else if (command == "vk_as_fields") {
        // Further implementation ...
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        return -1;
    }
}