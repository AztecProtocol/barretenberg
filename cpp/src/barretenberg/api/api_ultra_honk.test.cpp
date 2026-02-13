#include "api_ultra_honk.hpp"
#include "barretenberg/api/file_io.hpp"
#include "barretenberg/bbapi/bbapi_ultra_honk.hpp"
#include "barretenberg/chonk/acir_bincode_mocks.hpp"
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/dsl/acir_format/acir_format.hpp"
#include "barretenberg/dsl/acir_format/acir_to_constraint_buf.hpp"
#include "barretenberg/dsl/acir_format/proof_surgeon.hpp"
#include "barretenberg/flavor/ultra_flavor.hpp"
#include "barretenberg/flavor/ultra_rollup_flavor.hpp"
#include "barretenberg/ultra_honk/prover_instance.hpp"
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <math.h>
#include <sstream>
#include <string_view>

namespace bb {
std::vector<uint8_t> compress(const std::vector<uint8_t>& input);
std::vector<uint8_t> decompress(const void* bytes, size_t size);
} // namespace bb

using namespace bb;

namespace {
// Create a unique temporary directory for each test run
// Uniqueness needed because tests are run in parallel and write to same file names.
std::filesystem::path get_test_dir(const std::string_view& test_name)
{
    std::filesystem::path temp_dir = "tmp_api_ultra_honk_test";
    std::filesystem::create_directories(temp_dir);
    std::filesystem::create_directories(temp_dir / test_name);
    return temp_dir / test_name;
}

// Create test data
std::pair<std::filesystem::path, std::filesystem::path> create_test_circuit_files(const std::filesystem::path& test_dir)
{
    auto [bytecode, witness] = acir_bincode_mocks::create_simple_circuit_bytecode();

    auto bytecode_path = test_dir / "circuit.gz";
    auto witness_path = test_dir / "witness.gz";

    write_file(bytecode_path, bb::compress(bytecode));
    write_file(witness_path, bb::compress(witness));

    return { bytecode_path, witness_path };
}

bool is_command_available(const std::string& command)
{
    const std::string check_cmd = "command -v " + command + " >/dev/null 2>&1";
    return std::system(check_cmd.c_str()) == 0;
}

bool is_command_available_with_path_prefix(const std::string& command, const std::string& path_prefix)
{
    const std::string check_cmd =
        "PATH=\"" + path_prefix + ":$PATH\" command -v " + command + " >/dev/null 2>&1";
    return std::system(check_cmd.c_str()) == 0;
}

int run_shell_command(const std::string& command)
{
    return std::system(command.c_str());
}

std::filesystem::path find_repo_root()
{
    auto cursor = std::filesystem::absolute(std::filesystem::path(__FILE__)).parent_path();
    for (size_t i = 0; i < 16; ++i) {
        if (std::filesystem::exists(cursor / "ultrahonk_solana_universal_generator_plan.md")) {
            return cursor;
        }
        if (cursor == cursor.parent_path()) {
            break;
        }
        cursor = cursor.parent_path();
    }
    return std::filesystem::current_path();
}

} // namespace

class ApiUltraHonkTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite() { bb::srs::init_file_crs_factory(bb::srs::bb_crs_path()); }

    void SetUp() override
    {
        const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
        test_dir = get_test_dir(info->name());
    }

    void TearDown() override
    {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    std::filesystem::path test_dir;
};

TEST_F(ApiUltraHonkTest, ProveAndVerify)
{
    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);

    API::Flags flags;
    flags.oracle_hash_type = "poseidon2"; // Set default oracle hash type

    UltraHonkAPI api;

    // Generate VK first
    auto vk_output_path = test_dir / "vk";
    std::filesystem::create_directories(vk_output_path);
    api.write_vk(flags, bytecode_path, vk_output_path);
    EXPECT_TRUE(std::filesystem::exists(vk_output_path / "vk"));

    // Generate proof
    auto proof_output_dir = test_dir / "proof";
    std::filesystem::create_directories(proof_output_dir);
    api.prove(flags, bytecode_path, witness_path, vk_output_path / "vk", proof_output_dir);

    // Check that proof files were created
    EXPECT_TRUE(std::filesystem::exists(proof_output_dir / "proof"));
    EXPECT_TRUE(std::filesystem::exists(proof_output_dir / "public_inputs"));

    // Verify the proof
    bool verified =
        api.verify(flags, proof_output_dir / "public_inputs", proof_output_dir / "proof", vk_output_path / "vk");
    EXPECT_TRUE(verified);
}

TEST_F(ApiUltraHonkTest, ProveWithWriteVk)
{
    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);

    API::Flags flags;
    flags.oracle_hash_type = "poseidon2";
    flags.write_vk = true;

    UltraHonkAPI api;

    // Generate proof with write_vk flag (will compute and write VK)
    auto proof_output_dir = test_dir / "proof";
    std::filesystem::create_directories(proof_output_dir);
    api.prove(flags, bytecode_path, witness_path, "", proof_output_dir);

    // Check that proof and VK files were created
    EXPECT_TRUE(std::filesystem::exists(proof_output_dir / "proof"));
    EXPECT_TRUE(std::filesystem::exists(proof_output_dir / "public_inputs"));
    EXPECT_TRUE(std::filesystem::exists(proof_output_dir / "vk"));
    EXPECT_TRUE(std::filesystem::exists(proof_output_dir / "vk_hash"));

    // Verify the proof
    bool verified =
        api.verify(flags, proof_output_dir / "public_inputs", proof_output_dir / "proof", proof_output_dir / "vk");
    EXPECT_TRUE(verified);
}

TEST_F(ApiUltraHonkTest, ProveAndVerifyWithFields)
{
    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);

    // First generate VK for the prove step
    API::Flags vk_flags;
    vk_flags.oracle_hash_type = "poseidon2";

    UltraHonkAPI api;

    auto vk_output_path = test_dir / "vk";
    std::filesystem::create_directories(vk_output_path);
    api.write_vk(vk_flags, bytecode_path, vk_output_path);
    EXPECT_TRUE(std::filesystem::exists(vk_output_path / "vk"));

    // Now test proof generation
    API::Flags flags;
    flags.oracle_hash_type = "poseidon2";

    // Generate proof
    auto proof_output_dir = test_dir / "proof";
    std::filesystem::create_directories(proof_output_dir);
    api.prove(flags, bytecode_path, witness_path, vk_output_path / "vk", proof_output_dir);

    // Check that proof files were created
    EXPECT_TRUE(std::filesystem::exists(proof_output_dir / "proof"));
    EXPECT_TRUE(std::filesystem::exists(proof_output_dir / "public_inputs"));
}

TEST_F(ApiUltraHonkTest, ProveWithDifferentSettings)
{
    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);

    // Test different oracle hash types
    const std::vector<std::pair<std::string, bool>> test_cases = { { "poseidon2",
                                                                     false }, // oracle_hash_type, disable_zk
                                                                   { "poseidon2", true },
                                                                   { "keccak", false },
                                                                   { "keccak", true } };

    for (const auto& [oracle_hash_type, disable_zk] : test_cases) {
        API::Flags flags;
        flags.oracle_hash_type = oracle_hash_type;
        flags.disable_zk = disable_zk;
        flags.write_vk = true;

        auto case_dir = test_dir / (oracle_hash_type + "_" + (disable_zk ? "no_zk" : "zk"));
        std::filesystem::create_directories(case_dir);

        UltraHonkAPI api;

        // Generate proof
        api.prove(flags, bytecode_path, witness_path, "", case_dir);

        // Verify the proof
        bool verified = api.verify(flags, case_dir / "public_inputs", case_dir / "proof", case_dir / "vk");
        EXPECT_TRUE(verified) << "Failed with oracle_hash_type=" << oracle_hash_type << ", disable_zk=" << disable_zk;
    }
}

TEST_F(ApiUltraHonkTest, WriteVk)
{
    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);
    API::Flags flags;
    flags.oracle_hash_type = "poseidon2";

    UltraHonkAPI api;
    api.write_vk(flags, bytecode_path, test_dir);

    // Test against bbapi::CircuitComputeVk
    auto bytecode = read_file(bytecode_path);
    auto expected_vk =
        bbapi::CircuitComputeVk({ .circuit = { .bytecode = bb::decompress(bytecode.data(), bytecode.size()) },
                                  .settings = { .oracle_hash_type = flags.oracle_hash_type } })
            .execute();

    info("after write_vk, expected_vk size: {}", expected_vk.bytes.size());
    EXPECT_EQ(expected_vk.bytes, read_file(test_dir / "vk"));
    EXPECT_EQ(expected_vk.hash, read_file(test_dir / "vk_hash"));

    // Verify round-trip: decode the VK and check that to_field_elements() matches
    auto vk_from_bytes = from_buffer<UltraFlavor::VerificationKey>(expected_vk.bytes);
    auto vk_from_file = from_buffer<UltraFlavor::VerificationKey>(read_file(test_dir / "vk"));
    EXPECT_EQ(vk_from_bytes.to_field_elements(), vk_from_file.to_field_elements());
}

// NOTE: very light test
TEST_F(ApiUltraHonkTest, GatesWithOpcodesSmokeTest)
{
    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);

    // Capture stdout
    testing::internal::CaptureStdout();

    API::Flags flags;
    flags.oracle_hash_type = "poseidon2";
    flags.include_gates_per_opcode = true;
    UltraHonkAPI api;
    api.gates(flags, bytecode_path);

    std::string output = testing::internal::GetCapturedStdout();

    // Check that output contains per-opcode information
    EXPECT_TRUE(output.find("gates_per_opcode") != std::string::npos);
}

TEST_F(ApiUltraHonkTest, WriteSolanaVerifierArtifacts)
{
    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);
    (void)witness_path;

    API::Flags flags;
    flags.scheme = "ultra_honk";
    flags.oracle_hash_type = "poseidon2";
    flags.disable_zk = true;

    UltraHonkAPI api;

    const auto vk_dir = test_dir / "vk_output";
    std::filesystem::create_directories(vk_dir);
    api.write_vk(flags, bytecode_path, vk_dir);

    const auto vk_path = vk_dir / "vk";
    const auto solana_dir = test_dir / "solana";
    api.write_solana_verifier(flags, solana_dir, vk_path);

    EXPECT_TRUE(std::filesystem::exists(solana_dir / "vk.bin"));
    EXPECT_TRUE(std::filesystem::exists(solana_dir / "vk_meta.json"));
    EXPECT_TRUE(std::filesystem::exists(solana_dir / "vk_chunks.json"));
    EXPECT_TRUE(std::filesystem::exists(solana_dir / "session_abi.json"));
    EXPECT_TRUE(std::filesystem::exists(solana_dir / "client_helper.ts"));
    EXPECT_TRUE(std::filesystem::exists(solana_dir / "solana_verifier_program" / "Cargo.toml"));
    EXPECT_TRUE(std::filesystem::exists(solana_dir / "solana_verifier_program" / "src" / "lib.rs"));
    EXPECT_TRUE(std::filesystem::exists(solana_dir / "solana_verifier_program" / "src" / "generated_vk.rs"));
    EXPECT_TRUE(std::filesystem::exists(solana_dir / "solana_verifier_program" / "src" / "generated_config.rs"));
    EXPECT_TRUE(std::filesystem::exists(solana_dir / "solana_verifier_program" / "README.md"));
    const auto vk_bytes = read_file(vk_path);
    EXPECT_EQ(read_file(solana_dir / "vk.bin"), vk_bytes);

    const auto meta_json_buf = read_file(solana_dir / "vk_meta.json");
    const auto chunks_json_buf = read_file(solana_dir / "vk_chunks.json");
    const auto session_abi_buf = read_file(solana_dir / "session_abi.json");
    const auto client_helper_buf = read_file(solana_dir / "client_helper.ts");
    const auto generated_cargo_buf = read_file(solana_dir / "solana_verifier_program" / "Cargo.toml");
    const auto generated_lib_buf = read_file(solana_dir / "solana_verifier_program" / "src" / "lib.rs");
    const auto generated_vk_buf = read_file(solana_dir / "solana_verifier_program" / "src" / "generated_vk.rs");
    const auto generated_config_buf = read_file(solana_dir / "solana_verifier_program" / "src" / "generated_config.rs");
    const auto generated_readme_buf = read_file(solana_dir / "solana_verifier_program" / "README.md");

    const std::string meta_json(meta_json_buf.begin(), meta_json_buf.end());
    const std::string chunks_json(chunks_json_buf.begin(), chunks_json_buf.end());
    const std::string session_abi_json(session_abi_buf.begin(), session_abi_buf.end());
    const std::string client_helper_ts(client_helper_buf.begin(), client_helper_buf.end());
    const std::string generated_cargo(generated_cargo_buf.begin(), generated_cargo_buf.end());
    const std::string generated_lib(generated_lib_buf.begin(), generated_lib_buf.end());
    const std::string generated_vk(generated_vk_buf.begin(), generated_vk_buf.end());
    const std::string generated_config(generated_config_buf.begin(), generated_config_buf.end());
    const std::string generated_readme(generated_readme_buf.begin(), generated_readme_buf.end());

    const auto vk = from_buffer<UltraFlavor::VerificationKey>(vk_bytes);
    std::ostringstream hash_stream;
    hash_stream << vk.hash();
    const auto expected_hash = hash_stream.str();

    EXPECT_NE(meta_json.find("\"version\": 1"), std::string::npos);
    EXPECT_NE(meta_json.find("\"scheme\": \"ultra_honk\""), std::string::npos);
    EXPECT_NE(meta_json.find("\"oracle_hash\": \"poseidon2\""), std::string::npos);
    EXPECT_NE(meta_json.find("\"disable_zk\": true"), std::string::npos);
    EXPECT_NE(meta_json.find("\"vk_hash\": \"" + expected_hash + "\""), std::string::npos);

    EXPECT_NE(chunks_json.find("\"chunk_size\": 900"), std::string::npos);
    EXPECT_NE(chunks_json.find("\"total_bytes\": " + std::to_string(vk_bytes.size())), std::string::npos);

    EXPECT_NE(session_abi_json.find("\"version\": 2"), std::string::npos);
    EXPECT_NE(session_abi_json.find("\"name\": \"InitSession\""), std::string::npos);
    EXPECT_NE(session_abi_json.find("\"instruction_discriminants\""), std::string::npos);
    EXPECT_NE(session_abi_json.find("\"instruction_encoding\""), std::string::npos);
    EXPECT_NE(session_abi_json.find("\"verify_stages\""), std::string::npos);
    EXPECT_NE(session_abi_json.find("\"name\": \"hash_checks\""), std::string::npos);
    EXPECT_NE(session_abi_json.find("\"name\": \"decode_inputs\""), std::string::npos);
    EXPECT_NE(session_abi_json.find("\"name\": \"native_verify\""), std::string::npos);
    EXPECT_NE(session_abi_json.find("\"session_bind_hash\""), std::string::npos);
    EXPECT_NE(session_abi_json.find("\"state_machine\""), std::string::npos);
    EXPECT_NE(session_abi_json.find("\"name\": \"Finalize\""), std::string::npos);
    EXPECT_NE(client_helper_ts.find("export function chunkBytes"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("SESSION_ABI_VERSION = 2"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("TOTAL_VERIFY_STAGES = 3"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("export const VERIFY_STAGE"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("buildSessionBindHashPreimage"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("assertContiguousChunks"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("encodeInitSessionInstruction"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("encodeAppendProofChunkInstruction"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("encodeAppendPublicInputsChunkInstruction"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("encodeVerifyStepInstruction"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("encodeFinalizeInstruction"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("buildFinalizePayload"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("buildStagedSessionInstructionPlan"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("flattenStagedSessionInstructionPlan"), std::string::npos);
    EXPECT_NE(client_helper_ts.find("estimateStagedSessionTransactionCount"), std::string::npos);

    EXPECT_NE(generated_cargo.find("name = \"ultrahonk_solana_generated_verifier\""), std::string::npos);
    EXPECT_NE(generated_cargo.find("solana-program = \"3.0.0\""), std::string::npos);
    EXPECT_NE(generated_cargo.find("ultrahonk_verifier_runtime = {"), std::string::npos);
    EXPECT_NE(generated_cargo.find("package = \"ultrahonk_verifier_program\""), std::string::npos);
    EXPECT_NE(generated_lib.find("pub mod generated_config;"), std::string::npos);
    EXPECT_NE(generated_lib.find("pub fn process_instruction("), std::string::npos);
    EXPECT_NE(generated_lib.find("entrypoint!(entrypoint_process_instruction);"), std::string::npos);
    EXPECT_NE(generated_lib.find("ultrahonk_verifier_runtime::process_instruction"), std::string::npos);
    EXPECT_NE(generated_lib.find("enforce_generated_vk_hash_on_init"), std::string::npos);
    EXPECT_NE(generated_lib.find("generated_config::VK_HASH"), std::string::npos);
    EXPECT_NE(generated_lib.find("generated_config::SESSION_ABI_VERSION"), std::string::npos);
    EXPECT_NE(generated_lib.find("generated_config::TOTAL_VERIFY_STAGES"), std::string::npos);
    EXPECT_NE(generated_lib.find("generated_config::INIT_SESSION_DISCRIMINANT"), std::string::npos);
    EXPECT_NE(generated_lib.find("generated_config::FINALIZE_DISCRIMINANT"), std::string::npos);
    EXPECT_NE(generated_lib.find("generated_vk_hash_guard_rejects_mismatched_init_hash"), std::string::npos);
    EXPECT_NE(generated_lib.find("runtime_smoke_rejects_verify_step_before_init"), std::string::npos);
    EXPECT_NE(generated_lib.find("runtime_smoke_progresses_stages_and_finalizes_failed_for_empty_payload"),
              std::string::npos);
    EXPECT_NE(generated_lib.find("account_path_smoke_forwards_non_init_instruction_to_runtime"),
              std::string::npos);
    EXPECT_NE(generated_vk.find("include_bytes!(\"../../vk.bin\")"), std::string::npos);
    EXPECT_NE(generated_config.find("pub const SCHEME: &str = \"ultra_honk\";"), std::string::npos);
    EXPECT_NE(generated_config.find("pub const ORACLE_HASH: &str = \"poseidon2\";"), std::string::npos);
    EXPECT_NE(generated_config.find("pub const DISABLE_ZK: bool = true;"), std::string::npos);
    EXPECT_NE(generated_config.find("pub const IPA_ACCUMULATION: bool = false;"), std::string::npos);
    EXPECT_NE(generated_config.find("pub const DEFAULT_CHUNK_SIZE: usize = 900;"), std::string::npos);
    EXPECT_NE(generated_config.find("pub const TOTAL_VERIFY_STAGES: u32 = 3;"), std::string::npos);
    EXPECT_NE(generated_config.find("pub const INIT_SESSION_DISCRIMINANT: u8 = 0;"), std::string::npos);
    EXPECT_NE(generated_config.find("pub const APPEND_PROOF_CHUNK_DISCRIMINANT: u8 = 1;"), std::string::npos);
    EXPECT_NE(generated_config.find("pub const APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT: u8 = 2;"),
              std::string::npos);
    EXPECT_NE(generated_config.find("pub const VERIFY_STEP_DISCRIMINANT: u8 = 3;"), std::string::npos);
    EXPECT_NE(generated_config.find("pub const FINALIZE_DISCRIMINANT: u8 = 4;"), std::string::npos);
    EXPECT_NE(generated_readme.find("Generated Solana Verifier Scaffold"), std::string::npos);
    EXPECT_NE(generated_readme.find("buildStagedSessionInstructionPlan"), std::string::npos);
    EXPECT_NE(generated_readme.find("flattenStagedSessionInstructionPlan"), std::string::npos);
    EXPECT_NE(generated_readme.find("wired by default"), std::string::npos);
    EXPECT_NE(generated_readme.find("enforces `InitSession.vk_hash`"), std::string::npos);
    EXPECT_NE(generated_readme.find("compile-locks ABI/stage/discriminant constants"), std::string::npos);
}

TEST_F(ApiUltraHonkTest, WriteSolanaVerifierGeneratedProgramCompilesWithCargoCheck)
{
    if (!is_command_available("cargo")) {
        GTEST_SKIP() << "cargo is not available in PATH";
    }

    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);
    (void)witness_path;

    API::Flags flags;
    flags.scheme = "ultra_honk";
    flags.oracle_hash_type = "poseidon2";
    flags.disable_zk = true;

    UltraHonkAPI api;

    const auto vk_dir = test_dir / "vk_output";
    std::filesystem::create_directories(vk_dir);
    api.write_vk(flags, bytecode_path, vk_dir);

    const auto vk_path = vk_dir / "vk";
    const auto solana_dir = test_dir / "solana";
    api.write_solana_verifier(flags, solana_dir, vk_path);

    const auto generated_program_dir = solana_dir / "solana_verifier_program";
    ASSERT_TRUE(std::filesystem::exists(generated_program_dir / "Cargo.toml"));

    const std::string cargo_check_cmd =
        "cargo check --offline --manifest-path \"" + (generated_program_dir / "Cargo.toml").string() + "\"";
    EXPECT_EQ(run_shell_command(cargo_check_cmd), 0);
}

TEST_F(ApiUltraHonkTest, WriteSolanaVerifierGeneratedProgramRunsGuardUnitTests)
{
    if (!is_command_available("cargo")) {
        GTEST_SKIP() << "cargo is not available in PATH";
    }

    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);
    (void)witness_path;

    API::Flags flags;
    flags.scheme = "ultra_honk";
    flags.oracle_hash_type = "poseidon2";
    flags.disable_zk = true;

    UltraHonkAPI api;

    const auto vk_dir = test_dir / "vk_output";
    std::filesystem::create_directories(vk_dir);
    api.write_vk(flags, bytecode_path, vk_dir);

    const auto vk_path = vk_dir / "vk";
    const auto solana_dir = test_dir / "solana";
    api.write_solana_verifier(flags, solana_dir, vk_path);

    const auto generated_program_dir = solana_dir / "solana_verifier_program";
    ASSERT_TRUE(std::filesystem::exists(generated_program_dir / "Cargo.toml"));

    const std::string cargo_test_cmd =
        "cargo test --offline --manifest-path \"" + (generated_program_dir / "Cargo.toml").string() + "\" --lib";
    EXPECT_EQ(run_shell_command(cargo_test_cmd), 0);
}

TEST_F(ApiUltraHonkTest, WriteSolanaVerifierGeneratedProgramBuildsSbfWhenToolchainAvailable)
{
    constexpr const char* SOLANA_BIN_PATH = "/Users/Americansprit/.local/share/solana/install/active_release/bin";
    if (!is_command_available("cargo-build-sbf") &&
        !is_command_available_with_path_prefix("cargo-build-sbf", SOLANA_BIN_PATH)) {
        GTEST_SKIP() << "cargo-build-sbf is not available in PATH";
    }
    const auto solana_cache_dir = test_dir / "sbf_cache";
    const auto sbf_out_dir = test_dir / "sbf_out";
    std::error_code ec;
    std::filesystem::create_directories(solana_cache_dir, ec);
    if (ec) {
        GTEST_SKIP() << "Solana cache directory is not writable: " << solana_cache_dir;
    }
    std::filesystem::create_directories(sbf_out_dir, ec);
    if (ec) {
        GTEST_SKIP() << "SBF output directory is not writable: " << sbf_out_dir;
    }
    const auto probe_path = solana_cache_dir / ".bb_sbf_write_probe";
    std::ofstream probe_stream(probe_path, std::ios::out | std::ios::trunc);
    if (!probe_stream.is_open()) {
        GTEST_SKIP() << "Cannot write to Solana cache directory: " << solana_cache_dir;
    }
    probe_stream << "probe";
    probe_stream.close();
    std::filesystem::remove(probe_path, ec);

    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);
    (void)witness_path;

    API::Flags flags;
    flags.scheme = "ultra_honk";
    flags.oracle_hash_type = "poseidon2";
    flags.disable_zk = true;

    UltraHonkAPI api;

    const auto vk_dir = test_dir / "vk_output";
    std::filesystem::create_directories(vk_dir);
    api.write_vk(flags, bytecode_path, vk_dir);

    const auto vk_path = vk_dir / "vk";
    const auto solana_dir = test_dir / "solana";
    api.write_solana_verifier(flags, solana_dir, vk_path);

    const auto generated_program_dir = solana_dir / "solana_verifier_program";
    ASSERT_TRUE(std::filesystem::exists(generated_program_dir / "Cargo.toml"));

    const std::string sbf_build_cmd = "XDG_CACHE_HOME=\"" + solana_cache_dir.string() + "\" " +
                                      "PATH=\"" + std::string(SOLANA_BIN_PATH) + ":$PATH\" "
                                      "cargo build-sbf --offline --tools-version v1.52 --sbf-out-dir \"" +
                                      sbf_out_dir.string() + "\" --manifest-path \"" +
                                      (generated_program_dir / "Cargo.toml").string() + "\"";
    EXPECT_EQ(run_shell_command(sbf_build_cmd), 0);
}

TEST_F(ApiUltraHonkTest, WriteSolanaVerifierSessionAbiLocksToRuntimeConstants)
{
    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);
    (void)witness_path;

    API::Flags flags;
    flags.scheme = "ultra_honk";
    flags.oracle_hash_type = "poseidon2";
    flags.disable_zk = true;

    UltraHonkAPI api;
    const auto vk_dir = test_dir / "vk_output";
    std::filesystem::create_directories(vk_dir);
    api.write_vk(flags, bytecode_path, vk_dir);

    const auto solana_dir = test_dir / "solana";
    api.write_solana_verifier(flags, solana_dir, vk_dir / "vk");

    const auto session_abi_buf = read_file(solana_dir / "session_abi.json");
    const std::string session_abi_json(session_abi_buf.begin(), session_abi_buf.end());

    const auto runtime_source_path = find_repo_root() / "solana" / "ultrahonk_verifier_program" / "src" / "lib.rs";
    ASSERT_TRUE(std::filesystem::exists(runtime_source_path));
    const auto runtime_source_buf = read_file(runtime_source_path);
    const std::string runtime_source(runtime_source_buf.begin(), runtime_source_buf.end());

    const std::array<std::pair<std::string, std::string>, 14> lock_checks = {
        std::make_pair("\"version\": 2", "pub const SESSION_ABI_VERSION: u32 = 2;"),
        std::make_pair("\"InitSession\": 0", "pub const INIT_SESSION_DISCRIMINANT: u8 = 0;"),
        std::make_pair("\"AppendProofChunk\": 1", "pub const APPEND_PROOF_CHUNK_DISCRIMINANT: u8 = 1;"),
        std::make_pair("\"AppendPublicInputsChunk\": 2", "pub const APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT: u8 = 2;"),
        std::make_pair("\"VerifyStep\": 3", "pub const VERIFY_STEP_DISCRIMINANT: u8 = 3;"),
        std::make_pair("\"Finalize\": 4", "pub const FINALIZE_DISCRIMINANT: u8 = 4;"),
        std::make_pair("\"proof\": \"bb.solana.ultrahonk.proof.v1\"",
                       "pub const PROOF_HASH_DOMAIN_TAG: &[u8] = b\"bb.solana.ultrahonk.proof.v1\";"),
        std::make_pair("\"public_inputs\": \"bb.solana.ultrahonk.public_inputs.v1\"",
                       "pub const PUBLIC_INPUTS_HASH_DOMAIN_TAG: &[u8] = b\"bb.solana.ultrahonk.public_inputs.v1\";"),
        std::make_pair("\"session_bind\": \"bb.solana.ultrahonk.session_bind.v1\"",
                       "pub const SESSION_BIND_HASH_DOMAIN_TAG: &[u8] = b\"bb.solana.ultrahonk.session_bind.v1\";"),
        std::make_pair("\"0\": \"CollectingProof\"", "CollectingProof = 0,"),
        std::make_pair("\"5\": \"Failed\"", "Failed = 5,"),
        std::make_pair("{\"index\": 0, \"name\": \"hash_checks\"}", "HashChecks = 0,"),
        std::make_pair("{\"index\": 1, \"name\": \"decode_inputs\"}", "DecodeInputs = 1,"),
        std::make_pair("{\"index\": 2, \"name\": \"native_verify\"}", "NativeVerify = 2,"),
    };

    for (const auto& [abi_marker, runtime_marker] : lock_checks) {
        EXPECT_NE(session_abi_json.find(abi_marker), std::string::npos) << "Missing ABI marker: " << abi_marker;
        EXPECT_NE(runtime_source.find(runtime_marker), std::string::npos)
            << "Missing runtime marker: " << runtime_marker;
    }
}

TEST_F(ApiUltraHonkTest, VerifySolanaStaged)
{
    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);

    API::Flags flags;
    flags.scheme = "ultra_honk";
    flags.oracle_hash_type = "poseidon2";
    flags.disable_zk = true;
    flags.solana_chunk_size = 128;

    UltraHonkAPI api;

    const auto vk_dir = test_dir / "vk";
    std::filesystem::create_directories(vk_dir);
    api.write_vk(flags, bytecode_path, vk_dir);

    const auto proof_dir = test_dir / "proof";
    std::filesystem::create_directories(proof_dir);
    api.prove(flags, bytecode_path, witness_path, vk_dir / "vk", proof_dir);

    const bool verified =
        api.verify_solana_staged(flags, proof_dir / "public_inputs", proof_dir / "proof", vk_dir / "vk");
    EXPECT_TRUE(verified);
}

TEST_F(ApiUltraHonkTest, VerifySolanaStagedRejectsTamperedProof)
{
    auto [bytecode_path, witness_path] = create_test_circuit_files(test_dir);

    API::Flags flags;
    flags.scheme = "ultra_honk";
    flags.oracle_hash_type = "poseidon2";
    flags.disable_zk = true;
    flags.solana_chunk_size = 96;

    UltraHonkAPI api;

    const auto vk_dir = test_dir / "vk";
    std::filesystem::create_directories(vk_dir);
    api.write_vk(flags, bytecode_path, vk_dir);

    const auto proof_dir = test_dir / "proof";
    std::filesystem::create_directories(proof_dir);
    api.prove(flags, bytecode_path, witness_path, vk_dir / "vk", proof_dir);

    auto tampered_proof = read_file(proof_dir / "proof");
    ASSERT_FALSE(tampered_proof.empty());
    tampered_proof[0] ^= 0x01;
    const auto tampered_path = proof_dir / "proof_tampered";
    write_file(tampered_path, tampered_proof);

    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    const bool verified = api.verify_solana_staged(flags, proof_dir / "public_inputs", tampered_path, vk_dir / "vk");
    const std::string captured_stdout = testing::internal::GetCapturedStdout();
    const std::string captured_stderr = testing::internal::GetCapturedStderr();

    EXPECT_FALSE(verified);
    EXPECT_EQ(captured_stdout.find("Assertion failed: (val.on_curve())"), std::string::npos);
    EXPECT_EQ(captured_stdout.find("Assertion failed: (result.on_curve())"), std::string::npos);
    EXPECT_EQ(captured_stderr.find("Assertion failed: (val.on_curve())"), std::string::npos);
    EXPECT_EQ(captured_stderr.find("Assertion failed: (result.on_curve())"), std::string::npos);
}

TEST_F(ApiUltraHonkTest, VerifySolanaStagedLargeProofFixtureEcdsaRegression)
{
    const auto fixture_root = find_repo_root() / "solana" / "ultrahonk_verifier_program" / "tests" / "fixtures" /
                              "bb_generated" / "ecdsa_regression_keccak_nonzk";
    const auto proof_path = fixture_root / "proof";
    const auto public_inputs_path = fixture_root / "public_inputs";
    const auto vk_path = fixture_root / "vk";

    if (!std::filesystem::exists(proof_path) || !std::filesystem::exists(public_inputs_path) ||
        !std::filesystem::exists(vk_path)) {
        GTEST_SKIP() << "Large bb fixture is missing at " << fixture_root;
    }

    API::Flags flags;
    flags.scheme = "ultra_honk";
    flags.oracle_hash_type = "keccak";
    flags.disable_zk = true;
    flags.solana_chunk_size = 96;

    UltraHonkAPI api;
    const bool verified = api.verify_solana_staged(flags, public_inputs_path, proof_path, vk_path);
    EXPECT_TRUE(verified);
}

TEST_F(ApiUltraHonkTest, VerifySolanaStagedRejectsTamperedLargeProofFixtureEcdsaRegression)
{
    const auto fixture_root = find_repo_root() / "solana" / "ultrahonk_verifier_program" / "tests" / "fixtures" /
                              "bb_generated" / "ecdsa_regression_keccak_nonzk";
    const auto proof_path = fixture_root / "proof";
    const auto public_inputs_path = fixture_root / "public_inputs";
    const auto vk_path = fixture_root / "vk";

    if (!std::filesystem::exists(proof_path) || !std::filesystem::exists(public_inputs_path) ||
        !std::filesystem::exists(vk_path)) {
        GTEST_SKIP() << "Large bb fixture is missing at " << fixture_root;
    }

    auto tampered_proof = read_file(proof_path);
    ASSERT_GT(tampered_proof.size(), 64);
    tampered_proof[tampered_proof.size() / 2] ^= 0x01;
    const auto tampered_path = test_dir / "large_fixture_proof_tampered";
    write_file(tampered_path, tampered_proof);

    API::Flags flags;
    flags.scheme = "ultra_honk";
    flags.oracle_hash_type = "keccak";
    flags.disable_zk = true;
    flags.solana_chunk_size = 96;

    UltraHonkAPI api;
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    const bool verified = api.verify_solana_staged(flags, public_inputs_path, tampered_path, vk_path);
    const std::string captured_stdout = testing::internal::GetCapturedStdout();
    const std::string captured_stderr = testing::internal::GetCapturedStderr();

    EXPECT_FALSE(verified);
    EXPECT_EQ(captured_stdout.find("Assertion failed: (val.on_curve())"), std::string::npos);
    EXPECT_EQ(captured_stdout.find("Assertion failed: (result.on_curve())"), std::string::npos);
    EXPECT_EQ(captured_stderr.find("Assertion failed: (val.on_curve())"), std::string::npos);
    EXPECT_EQ(captured_stderr.find("Assertion failed: (result.on_curve())"), std::string::npos);
}
