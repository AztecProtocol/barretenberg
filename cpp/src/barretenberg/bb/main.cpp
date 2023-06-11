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

using namespace barretenberg;

uint32_t MAX_CIRCUIT_SIZE = 1 << 19;
auto CRS_PATH = "./crs";
bool verbose = false;

void init(const std::string& jsonPath)
{
    // Must +1!
    auto g1_data = get_g1_data(CRS_PATH, MAX_CIRCUIT_SIZE + 1);
    auto g2_data = get_g2_data(CRS_PATH);
    srs::init_crs_factory(g1_data, g2_data);
}

acir_format::WitnessVector get_witness(std::string const& witness_path)
{
    auto witness_data = read_file(witness_path);
    // We need to prefix the number of fields to comply with serialization format.
    // TODO: Make noir side output witness data prefixed.
    return from_buffer<acir_format::WitnessVector>(
        join({ to_buffer((uint32_t)witness_data.size() / 32), witness_data }));
}

acir_format::acir_format get_contraint_system(std::string const& json_path)
{
    auto bytecode = get_bytecode(json_path);
    return from_buffer<acir_format::acir_format>(bytecode.data());
}

void proveAndVerify(const std::string& jsonPath, const std::string& witnessPath, bool recursive)
{
    auto acir_composer = new acir_proofs::AcirComposer(MAX_CIRCUIT_SIZE, verbose);
    auto constraint_system = get_contraint_system(jsonPath);
    auto witness = get_witness(witnessPath);
    auto proof = acir_composer->create_proof(srs::get_crs_factory(), constraint_system, witness, recursive);
    auto verified = acir_composer->verify_proof(proof, recursive);
    info("verified: ", verified);
}

void prove(const std::string& jsonPath, const std::string& witnessPath, bool recursive, const std::string& outputPath)
{
    auto acir_composer = new acir_proofs::AcirComposer(MAX_CIRCUIT_SIZE, verbose);
    auto constraint_system = get_contraint_system(jsonPath);
    auto witness = get_witness(witnessPath);
    auto proof = acir_composer->create_proof(srs::get_crs_factory(), constraint_system, witness, recursive);
    write_file(outputPath, proof);
    info("proof written to: ", outputPath);
}

void gateCount(const std::string& jsonPath)
{
    auto acir_composer = new acir_proofs::AcirComposer(MAX_CIRCUIT_SIZE, verbose);
    auto constraint_system = get_contraint_system(jsonPath);
    acir_composer->create_circuit(constraint_system);
    info("gates: ", acir_composer->get_total_circuit_size());
}

bool verify(const std::string& proof_path, bool recursive, const std::string& vk_path)
{
    auto acir_composer = new acir_proofs::AcirComposer(MAX_CIRCUIT_SIZE, verbose);
    auto vk_data = from_buffer<plonk::verification_key_data>(read_file(vk_path));
    acir_composer->load_verification_key(barretenberg::srs::get_crs_factory(), std::move(vk_data));
    auto verified = acir_composer->verify_proof(read_file(proof_path), recursive);
    info("verified: ", verified);
    return verified;
}

void writeVk(const std::string& jsonPath, const std::string& outputPath)
{
    auto acir_composer = new acir_proofs::AcirComposer(MAX_CIRCUIT_SIZE, verbose);
    auto constraint_system = get_contraint_system(jsonPath);
    acir_composer->init_proving_key(srs::get_crs_factory(), constraint_system);
    auto vk = acir_composer->init_verification_key();
    write_file(outputPath, to_buffer(*vk));
    info("vk written to: ", outputPath);
}

void contract(const std::string& outputPath, const std::string& vk);
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

std::string getRequiredOption(std::vector<std::string>& args, const std::string& option)
{
    auto itr = std::find(args.begin(), args.end(), option);
    if (itr != args.end() && std::next(itr) != args.end()) {
        return *(std::next(itr));
    }
    throw std::runtime_error(format("Required option missing ", option));
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

    std::string jsonPath = getOption(args, "-j", "./target/main.json");
    init(jsonPath);

    if (command == "prove_and_verify") {
        std::string witnessPath = getOption(args, "-w", "./target/witness.tr");
        bool recursive = flagPresent(args, "-r") || flagPresent(args, "--recursive");
        proveAndVerify(jsonPath, witnessPath, recursive);
    } else if (command == "prove") {
        std::string witness_path = getOption(args, "-w", "./target/witness.tr");
        std::string output_path = getOption(args, "-o", "./proofs/proof");
        bool recursive = flagPresent(args, "-r") || flagPresent(args, "--recursive");
        prove(jsonPath, witness_path, recursive, output_path);
    } else if (command == "gates") {
        gateCount(jsonPath);
    } else if (command == "verify") {
        std::string proof_path = getOption(args, "-p", "./proofs/proof");
        bool recursive = flagPresent(args, "-r") || flagPresent(args, "--recursive");
        std::string vk_path = getOption(args, "-k", "./target/vk");
        verify(proof_path, recursive, vk_path);
    } else if (command == "contract") {
        // Further implementation ...
    } else if (command == "write_vk") {
        std::string proof_path = getOption(args, "-p", "./proofs/proof");
        std::string output_path = getOption(args, "-o", "./target/vk");
        writeVk(jsonPath, output_path);
    } else if (command == "proof_as_fields") {
        // Further implementation ...
    } else if (command == "vk_as_fields") {
        // Further implementation ...
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        return -1;
    }
}