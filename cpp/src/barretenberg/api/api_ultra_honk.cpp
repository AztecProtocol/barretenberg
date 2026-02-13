#include "api_ultra_honk.hpp"

#include "barretenberg/api/file_io.hpp"
#include "barretenberg/bbapi/bbapi_ultra_honk.hpp"
#include "barretenberg/common/bb_bench.hpp"
#include "barretenberg/common/get_bytecode.hpp"
#include "barretenberg/common/map.hpp"
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/throw_or_abort.hpp"
#include "barretenberg/dsl/acir_format/acir_to_constraint_buf.hpp"
#include "barretenberg/dsl/acir_format/proof_surgeon.hpp"
#include "barretenberg/dsl/acir_proofs/honk_contract.hpp"
#include "barretenberg/dsl/acir_proofs/honk_optimized_contract.hpp"
#include "barretenberg/dsl/acir_proofs/honk_zk_contract.hpp"
#include "barretenberg/dsl/acir_proofs/solana_vk_generator.hpp"
#include "barretenberg/crypto/sha256/sha256.hpp"
#include "barretenberg/flavor/ultra_keccak_flavor.hpp"
#include "barretenberg/flavor/ultra_keccak_zk_flavor.hpp"
#include "barretenberg/honk/proof_system/types/proof.hpp"
#include "barretenberg/numeric/uint256/uint256.hpp"
#include "barretenberg/special_public_inputs/special_public_inputs.hpp"
#include "barretenberg/constants.hpp"
#include "barretenberg/srs/global_crs.hpp"
#include <algorithm>
#include <array>
#include <exception>
#include <iomanip>
#include <optional>
#include <span>
#include <sstream>
#include <string_view>

#ifdef STARKNET_GARAGA_FLAVORS
#include "barretenberg/ext/starknet/flavor/ultra_starknet_flavor.hpp"
#include "barretenberg/ext/starknet/flavor/ultra_starknet_zk_flavor.hpp"
#endif

namespace bb {

namespace {

void write_vk_outputs(const bbapi::CircuitComputeVk::Response& vk_response, const std::filesystem::path& output_dir)
{
    write_file(output_dir / "vk", vk_response.bytes);
    info("VK saved to ", output_dir / "vk");
    write_file(output_dir / "vk_hash", vk_response.hash);
    info("VK Hash saved to ", output_dir / "vk_hash");
}

void write_proof_outputs(const bbapi::CircuitProve::Response& prove_response, const std::filesystem::path& output_dir)
{
    auto public_inputs_buf = to_buffer(prove_response.public_inputs);
    auto proof_buf = to_buffer(prove_response.proof);

    write_file(output_dir / "public_inputs", public_inputs_buf);
    write_file(output_dir / "proof", proof_buf);
    info("Public inputs saved to ", output_dir / "public_inputs");
    info("Proof saved to ", output_dir / "proof");
}

std::string vk_hash_from_vk_bytes(const API::Flags& flags, const std::vector<uint8_t>& vk_bytes)
{
    auto build_hash = [&]<typename Flavor>() {
        using VK = typename Flavor::VerificationKey;
        auto vk = from_buffer<VK>(vk_bytes);
        std::ostringstream hash_stream;
        hash_stream << vk.hash();
        return hash_stream.str();
    };

    if (flags.ipa_accumulation) {
        return build_hash.template operator()<UltraRollupFlavor>();
    }
    if (flags.oracle_hash_type == "poseidon2" && !flags.disable_zk) {
        return build_hash.template operator()<UltraZKFlavor>();
    }
    if (flags.oracle_hash_type == "poseidon2" && flags.disable_zk) {
        return build_hash.template operator()<UltraFlavor>();
    }
    if (flags.oracle_hash_type == "keccak" && !flags.disable_zk) {
        return build_hash.template operator()<UltraKeccakZKFlavor>();
    }
    if (flags.oracle_hash_type == "keccak" && flags.disable_zk) {
        return build_hash.template operator()<UltraKeccakFlavor>();
    }
#ifdef STARKNET_GARAGA_FLAVORS
    if (flags.oracle_hash_type == "starknet" && !flags.disable_zk) {
        return build_hash.template operator()<UltraStarknetZKFlavor>();
    }
    if (flags.oracle_hash_type == "starknet" && flags.disable_zk) {
        return build_hash.template operator()<UltraStarknetFlavor>();
    }
#endif
    throw_or_abort("invalid proof type while decoding vk hash");
    return "";
}

std::filesystem::path resolve_solana_runtime_crate_dir()
{
    auto current = std::filesystem::path(__FILE__).parent_path();
    while (!current.empty()) {
        const auto candidate = current / "solana" / "ultrahonk_verifier_program" / "Cargo.toml";
        if (std::filesystem::exists(candidate)) {
            return candidate.parent_path();
        }
        if (!current.has_parent_path() || current.parent_path() == current) {
            break;
        }
        current = current.parent_path();
    }
    throw_or_abort("unable to locate shared Solana runtime crate (expected solana/ultrahonk_verifier_program)");
    return {};
}

template <typename Flavor>
bool validate_honk_proof_commitments(const std::vector<uint256_t>& public_inputs,
                                     const std::vector<uint256_t>& proof,
                                     const std::vector<uint8_t>& vk_bytes,
                                     std::string& error)
{
    using VerificationKey = typename Flavor::VerificationKey;
    using Transcript = typename Flavor::Transcript;
    using DataType = typename Transcript::DataType;
    using Codec = typename Transcript::Codec;
    using Commitment = typename Flavor::Commitment;
    using BaseField = typename Commitment::Fq;

    constexpr bool is_fr_codec = std::is_same_v<DataType, fr>;
    constexpr bool has_ipa_accumulator = HasIPAAccumulator<Flavor>;
    constexpr size_t num_frs_fr = Codec::template calc_num_fields<typename Flavor::FF>();

    auto vk = from_buffer<VerificationKey>(vk_bytes);
    const size_t num_public_inputs = static_cast<size_t>(vk.num_public_inputs);
    const size_t num_inner_public_inputs = [&]() {
        constexpr size_t default_io_public_inputs =
            has_ipa_accumulator ? RollupIO::PUBLIC_INPUTS_SIZE : DefaultIO::PUBLIC_INPUTS_SIZE;
        if (num_public_inputs < default_io_public_inputs) {
            return static_cast<size_t>(0);
        }
        return num_public_inputs - default_io_public_inputs;
    }();
    const size_t virtual_log_n = Flavor::USE_PADDING ? CONST_PROOF_SIZE_LOG_N : static_cast<size_t>(vk.log_circuit_size);

    if (public_inputs.size() != num_inner_public_inputs) {
        error =
            format("public input count mismatch: expected ", num_inner_public_inputs, ", got ", public_inputs.size());
        return false;
    }

    std::vector<DataType> complete_proof;
    complete_proof.reserve(public_inputs.size() + proof.size());
    complete_proof.insert(complete_proof.end(), public_inputs.begin(), public_inputs.end());
    complete_proof.insert(complete_proof.end(), proof.begin(), proof.end());

    if constexpr (is_fr_codec) {
        for (size_t idx = 0; idx < complete_proof.size(); ++idx) {
            if (static_cast<uint256_t>(complete_proof[idx]) >= fr::modulus) {
                error = format("proof element at index ", idx, " is not a canonical fr encoding");
                return false;
            }
        }
    }

    size_t expected_honk_without_pub_inputs = Flavor::PROOF_LENGTH_WITHOUT_PUB_INPUTS(virtual_log_n);
    if constexpr (has_ipa_accumulator) {
        expected_honk_without_pub_inputs -= IPA_PROOF_LENGTH;
    }
    const size_t expected_honk_total = num_public_inputs + expected_honk_without_pub_inputs;
    if (complete_proof.size() < expected_honk_total) {
        error = format("proof too short: expected at least ", expected_honk_total, " elements, got ", complete_proof.size());
        return false;
    }

    // Prevalidate BN254 pairing points propagated in the "special public inputs" region.
    // This avoids assertion-heavy failure paths in downstream reconstruction on malformed data.
    {
        constexpr size_t pairing_public_inputs_size = PairingPoints<curve::BN254>::PUBLIC_INPUTS_SIZE;
        constexpr size_t point_public_inputs_size = curve::BN254::AffineElement::PUBLIC_INPUTS_SIZE;
        constexpr uint64_t limb_bits = bb::stdlib::NUM_LIMB_BITS_IN_FIELD_SIMULATION;
        constexpr uint256_t limb_bound = uint256_t(1) << limb_bits;
        const size_t pairing_start = num_inner_public_inputs;
        if (pairing_start + pairing_public_inputs_size > expected_honk_total) {
            error = "proof truncated while decoding pairing public inputs";
            return false;
        }

        std::array<fr, pairing_public_inputs_size> pairing_inputs{};
        for (size_t idx = 0; idx < pairing_public_inputs_size; ++idx) {
            const auto element = complete_proof[pairing_start + idx];
            const auto element_u256 = static_cast<uint256_t>(element);
            if (element_u256 >= fr::modulus) {
                error = format("special public input at index ", idx, " is not a canonical fr encoding");
                return false;
            }
            pairing_inputs[idx] = static_cast<fr>(element_u256);
        }

        auto decode_fq_from_limbs = [&](size_t offset) -> std::optional<curve::BN254::BaseField> {
            const uint256_t l0 = static_cast<uint256_t>(pairing_inputs[offset + 0]);
            const uint256_t l1 = static_cast<uint256_t>(pairing_inputs[offset + 1]);
            const uint256_t l2 = static_cast<uint256_t>(pairing_inputs[offset + 2]);
            const uint256_t l3 = static_cast<uint256_t>(pairing_inputs[offset + 3]);
            if (l0 >= limb_bound || l1 >= limb_bound || l2 >= limb_bound || l3 >= limb_bound) {
                return std::nullopt;
            }
            return curve::BN254::BaseField(l0 + (l1 << limb_bits) + (l2 << (2 * limb_bits)) + (l3 << (3 * limb_bits)));
        };

        for (size_t point_idx = 0; point_idx < pairing_public_inputs_size / point_public_inputs_size; ++point_idx) {
            const size_t point_offset = point_idx * point_public_inputs_size;
            auto x = decode_fq_from_limbs(point_offset);
            auto y = decode_fq_from_limbs(point_offset + curve::BN254::BaseField::PUBLIC_INPUTS_SIZE);
            if (!x.has_value() || !y.has_value()) {
                error = format("malformed pairing point limbs at point index ", point_idx);
                return false;
            }
            curve::BN254::AffineElement point;
            point.x = x.value();
            point.y = y.value();
            if (!point.on_curve()) {
                error = format("malformed pairing point at point index ", point_idx);
                return false;
            }
        }
    }

    size_t cursor = num_public_inputs;
    const auto ensure_available = [&](size_t count, std::string_view label) {
        if (cursor + count > expected_honk_total) {
            error = format("proof truncated while decoding ", label);
            return false;
        }
        return true;
    };

    const auto consume_scalars = [&](size_t count, std::string_view label) {
        if (!ensure_available(count, label)) {
            return false;
        }
        cursor += count;
        return true;
    };

    const auto consume_commitment = [&](std::string_view label) {
        constexpr size_t base_field_size = Codec::template calc_num_fields<BaseField>();
        constexpr size_t commitment_size = 2 * base_field_size;
        if (!ensure_available(commitment_size, label)) {
            return false;
        }

        std::span<const DataType> proof_span(complete_proof);
        const auto x_fields = proof_span.subspan(cursor, base_field_size);
        const auto y_fields = proof_span.subspan(cursor + base_field_size, base_field_size);
        const auto x = Codec::template deserialize_from_fields<BaseField>(x_fields);
        const auto y = Codec::template deserialize_from_fields<BaseField>(y_fields);
        cursor += commitment_size;

        Commitment commitment;
        commitment.x = x;
        commitment.y = y;
        if (commitment.x == BaseField::zero() && commitment.y == BaseField::zero()) {
            commitment.self_set_infinity();
        }
        if (!commitment.on_curve()) {
            error = format("malformed commitment encountered while decoding ", label);
            return false;
        }
        return true;
    };

    if constexpr (Flavor::HasZK) {
        if (!consume_commitment("hiding_polynomial_commitment")) {
            return false;
        }
    }
    if (!consume_commitment("W_L")) {
        return false;
    }
    if (!consume_commitment("W_R")) {
        return false;
    }
    if (!consume_commitment("W_O")) {
        return false;
    }
    if (!consume_commitment("LOOKUP_READ_COUNTS")) {
        return false;
    }
    if (!consume_commitment("LOOKUP_READ_TAGS")) {
        return false;
    }
    if (!consume_commitment("W_4")) {
        return false;
    }
    if (!consume_commitment("LOOKUP_INVERSES")) {
        return false;
    }
    if (!consume_commitment("Z_PERM")) {
        return false;
    }
    if constexpr (Flavor::HasZK) {
        if (!consume_commitment("Libra:concatenation_commitment")) {
            return false;
        }
        if (!consume_scalars(num_frs_fr, "Libra:Sum")) {
            return false;
        }
    }
    if (!consume_scalars(virtual_log_n * Flavor::BATCHED_RELATION_PARTIAL_LENGTH * num_frs_fr, "sumcheck_univariates")) {
        return false;
    }
    if constexpr (Flavor::HasZK) {
        if (!consume_scalars(num_frs_fr, "Libra:claimed_evaluation")) {
            return false;
        }
    }
    if (!consume_scalars(Flavor::NUM_ALL_ENTITIES * num_frs_fr, "sumcheck_evaluations")) {
        return false;
    }
    if constexpr (Flavor::HasZK) {
        if (!consume_commitment("Libra:grand_sum_commitment")) {
            return false;
        }
        if (!consume_commitment("Libra:quotient_commitment")) {
            return false;
        }
    }
    for (size_t idx = 1; idx < virtual_log_n; ++idx) {
        if (!consume_commitment(format("Gemini:FOLD_", idx))) {
            return false;
        }
    }
    if (!consume_scalars(virtual_log_n * num_frs_fr, "gemini_fold_evaluations")) {
        return false;
    }
    if constexpr (Flavor::HasZK) {
        if (!consume_scalars(NUM_SMALL_IPA_EVALUATIONS * num_frs_fr, "libra_evaluations")) {
            return false;
        }
    }
    if (!consume_commitment("Shplonk:Q")) {
        return false;
    }
    if (!consume_commitment("KZG:W")) {
        return false;
    }

    if (cursor != expected_honk_total) {
        error = format("honk proof structure mismatch: decoded cursor ", cursor, ", expected ", expected_honk_total);
        return false;
    }
    return true;
}

} // anonymous namespace

bool UltraHonkAPI::check([[maybe_unused]] const Flags& flags,
                         [[maybe_unused]] const std::filesystem::path& bytecode_path,
                         [[maybe_unused]] const std::filesystem::path& witness_path)
{
    throw_or_abort("API function check_witness not implemented");
    return false;
}

void UltraHonkAPI::prove(const Flags& flags,
                         const std::filesystem::path& bytecode_path,
                         const std::filesystem::path& witness_path,
                         const std::filesystem::path& vk_path,
                         const std::filesystem::path& output_dir)
{
    BB_BENCH_NAME("UltraHonkAPI::prove");
    // Validate output directory
    if (output_dir == "-") {
        throw_or_abort("Stdout output is not supported. Please specify an output directory.");
    }

    // Convert flags to ProofSystemSettings
    bbapi::ProofSystemSettings settings{ .ipa_accumulation = flags.ipa_accumulation,
                                         .oracle_hash_type = flags.oracle_hash_type,
                                         .disable_zk = flags.disable_zk };

    // Read input files
    auto bytecode = get_bytecode(bytecode_path);
    auto witness = get_bytecode(witness_path);

    // Handle VK
    std::vector<uint8_t> vk_bytes;

    if (!vk_path.empty() && !flags.write_vk) {
        vk_bytes = read_file(vk_path);
    }

    // Prove
    auto response = bbapi::CircuitProve{ .circuit = { .name = "circuit",
                                                      .bytecode = std::move(bytecode),
                                                      .verification_key = std::move(vk_bytes) },
                                         .witness = std::move(witness),
                                         .settings = std::move(settings) }
                        .execute();
    write_proof_outputs(response, output_dir);
    if (flags.write_vk) {
        write_vk_outputs(response.vk, output_dir);
    }
}

bool UltraHonkAPI::verify(const Flags& flags,
                          const std::filesystem::path& public_inputs_path,
                          const std::filesystem::path& proof_path,
                          const std::filesystem::path& vk_path)
{
    BB_BENCH_NAME("UltraHonkAPI::verify");
    // Read input files
    auto public_inputs = many_from_buffer<uint256_t>(read_file(public_inputs_path));
    auto proof = many_from_buffer<uint256_t>(read_file(proof_path));
    auto vk_bytes = read_file(vk_path);

    // Convert flags to ProofSystemSettings
    bbapi::ProofSystemSettings settings{ .ipa_accumulation = flags.ipa_accumulation,
                                         .oracle_hash_type = flags.oracle_hash_type,
                                         .disable_zk = flags.disable_zk };

    // Execute verify command
    auto response = bbapi::CircuitVerify{ .verification_key = std::move(vk_bytes),
                                          .public_inputs = std::move(public_inputs),
                                          .proof = std::move(proof),
                                          .settings = settings }
                        .execute();

    return response.verified;
}

bool UltraHonkAPI::verify_solana_staged(const Flags& flags,
                                        const std::filesystem::path& public_inputs_path,
                                        const std::filesystem::path& proof_path,
                                        const std::filesystem::path& vk_path)
{
    BB_BENCH_NAME("UltraHonkAPI::verify_solana_staged");
    constexpr uint32_t TOTAL_VERIFY_STAGES = 3;
    enum class VerifyStage : uint32_t { HashChecks = 0, DecodeInputs = 1, NativeVerify = 2 };

    enum class SessionStatus : uint8_t {
        CollectingProof = 0,
        CollectingPublicInputs = 1,
        ReadyToVerify = 2,
        Verifying = 3,
        Verified = 4,
        Failed = 5
    };

    struct SolanaSessionState {
        std::array<uint8_t, 32> authority{};
        std::string vk_hash;
        crypto::Sha256Hash proof_hash{};
        crypto::Sha256Hash public_inputs_hash{};
        crypto::Sha256Hash session_bind_hash{};
        uint64_t proof_total_len = 0;
        uint64_t public_inputs_total_len = 0;
        uint64_t proof_received_len = 0;
        uint64_t public_inputs_received_len = 0;
        uint32_t next_proof_chunk_index = 0;
        uint32_t next_public_inputs_chunk_index = 0;
        uint32_t stage_index = 0;
        SessionStatus status = SessionStatus::CollectingProof;
        bool stage_failed = false;
        bool verify_result = false;
        std::vector<uint8_t> proof_bytes;
        std::vector<uint8_t> public_inputs_bytes;
        std::vector<uint256_t> decoded_public_inputs;
        std::vector<uint256_t> decoded_proof;
    };

    constexpr std::string_view proof_domain = "bb.solana.ultrahonk.proof.v1";
    constexpr std::string_view public_inputs_domain = "bb.solana.ultrahonk.public_inputs.v1";
    constexpr std::string_view session_bind_domain = "bb.solana.ultrahonk.session_bind.v1";

    auto append_u64_le = [](std::vector<uint8_t>& out, uint64_t value) {
        for (size_t i = 0; i < 8; ++i) {
            out.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xffU));
        }
    };

    auto domain_hash = [](std::string_view domain, const std::vector<uint8_t>& payload) {
        std::vector<uint8_t> preimage;
        preimage.reserve(domain.size() + payload.size());
        preimage.insert(preimage.end(), domain.begin(), domain.end());
        preimage.insert(preimage.end(), payload.begin(), payload.end());
        return crypto::sha256(preimage);
    };

    auto vk_bytes = read_file(vk_path);
    auto proof_bytes = read_file(proof_path);
    auto public_inputs_bytes = read_file(public_inputs_path);

    SolanaSessionState session;
    session.vk_hash = vk_hash_from_vk_bytes(flags, vk_bytes);
    session.proof_hash = domain_hash(proof_domain, proof_bytes);
    session.public_inputs_hash = domain_hash(public_inputs_domain, public_inputs_bytes);
    session.proof_total_len = proof_bytes.size();
    session.public_inputs_total_len = public_inputs_bytes.size();
    session.proof_bytes.resize(proof_bytes.size());
    session.public_inputs_bytes.resize(public_inputs_bytes.size());

    {
        std::vector<uint8_t> bind_preimage;
        bind_preimage.reserve(session_bind_domain.size() + session.vk_hash.size() + 32 + 32 + 32 + 8 + 8);
        bind_preimage.insert(bind_preimage.end(), session_bind_domain.begin(), session_bind_domain.end());
        bind_preimage.insert(bind_preimage.end(), session.vk_hash.begin(), session.vk_hash.end());
        bind_preimage.insert(bind_preimage.end(), session.proof_hash.begin(), session.proof_hash.end());
        bind_preimage.insert(bind_preimage.end(), session.public_inputs_hash.begin(), session.public_inputs_hash.end());
        bind_preimage.insert(bind_preimage.end(), session.authority.begin(), session.authority.end());
        append_u64_le(bind_preimage, session.proof_total_len);
        append_u64_le(bind_preimage, session.public_inputs_total_len);
        session.session_bind_hash = crypto::sha256(bind_preimage);
    }

    if (session.proof_total_len == 0 && session.public_inputs_total_len == 0) {
        session.status = SessionStatus::ReadyToVerify;
    } else if (session.proof_total_len == 0) {
        session.status = SessionStatus::CollectingPublicInputs;
    }

    auto append_proof_chunk = [&](uint32_t chunk_index, uint32_t offset, const std::vector<uint8_t>& chunk_bytes) {
        if (session.status != SessionStatus::CollectingProof) {
            throw_or_abort("AppendProofChunk called in invalid session state");
        }
        if (chunk_index != session.next_proof_chunk_index) {
            throw_or_abort("AppendProofChunk chunk_index is not contiguous");
        }
        if (offset != session.proof_received_len) {
            throw_or_abort("AppendProofChunk offset mismatch");
        }
        if (session.proof_received_len + chunk_bytes.size() > session.proof_total_len) {
            throw_or_abort("AppendProofChunk length overflow");
        }

        std::copy(chunk_bytes.begin(), chunk_bytes.end(), session.proof_bytes.begin() + static_cast<ptrdiff_t>(offset));
        session.proof_received_len += chunk_bytes.size();
        session.next_proof_chunk_index++;
        if (session.proof_received_len == session.proof_total_len) {
            session.status = session.public_inputs_total_len == 0 ? SessionStatus::ReadyToVerify
                                                                  : SessionStatus::CollectingPublicInputs;
        }
    };

    auto append_public_inputs_chunk = [&](uint32_t chunk_index, uint32_t offset, const std::vector<uint8_t>& chunk_bytes) {
        if (session.status != SessionStatus::CollectingPublicInputs) {
            throw_or_abort("AppendPublicInputsChunk called in invalid session state");
        }
        if (chunk_index != session.next_public_inputs_chunk_index) {
            throw_or_abort("AppendPublicInputsChunk chunk_index is not contiguous");
        }
        if (offset != session.public_inputs_received_len) {
            throw_or_abort("AppendPublicInputsChunk offset mismatch");
        }
        if (session.public_inputs_received_len + chunk_bytes.size() > session.public_inputs_total_len) {
            throw_or_abort("AppendPublicInputsChunk length overflow");
        }

        std::copy(chunk_bytes.begin(),
                  chunk_bytes.end(),
                  session.public_inputs_bytes.begin() + static_cast<ptrdiff_t>(offset));
        session.public_inputs_received_len += chunk_bytes.size();
        session.next_public_inputs_chunk_index++;
        if (session.public_inputs_received_len == session.public_inputs_total_len) {
            session.status = SessionStatus::ReadyToVerify;
        }
    };

    const size_t chunk_size = flags.solana_chunk_size == 0 ? solana::SOLANA_DEFAULT_CHUNK_SIZE_BYTES : flags.solana_chunk_size;
    for (size_t offset = 0, index = 0; offset < proof_bytes.size(); offset += chunk_size, ++index) {
        const size_t end = std::min(offset + chunk_size, proof_bytes.size());
        std::vector<uint8_t> chunk(proof_bytes.begin() + static_cast<ptrdiff_t>(offset),
                                   proof_bytes.begin() + static_cast<ptrdiff_t>(end));
        append_proof_chunk(static_cast<uint32_t>(index), static_cast<uint32_t>(offset), chunk);
    }
    for (size_t offset = 0, index = 0; offset < public_inputs_bytes.size(); offset += chunk_size, ++index) {
        const size_t end = std::min(offset + chunk_size, public_inputs_bytes.size());
        std::vector<uint8_t> chunk(public_inputs_bytes.begin() + static_cast<ptrdiff_t>(offset),
                                   public_inputs_bytes.begin() + static_cast<ptrdiff_t>(end));
        append_public_inputs_chunk(static_cast<uint32_t>(index), static_cast<uint32_t>(offset), chunk);
    }

    auto verify_step = [&](uint32_t expected_stage_index) {
        if (session.status != SessionStatus::ReadyToVerify && session.status != SessionStatus::Verifying) {
            throw_or_abort("VerifyStep called in invalid session state");
        }
        if (expected_stage_index != session.stage_index) {
            throw_or_abort("VerifyStep expected_stage_index mismatch");
        }

        bbapi::ProofSystemSettings settings{ .ipa_accumulation = flags.ipa_accumulation,
                                             .oracle_hash_type = flags.oracle_hash_type,
                                             .disable_zk = flags.disable_zk };
        const auto stage = static_cast<VerifyStage>(session.stage_index);

        if (stage == VerifyStage::HashChecks) {
            if (domain_hash(proof_domain, session.proof_bytes) != session.proof_hash ||
                domain_hash(public_inputs_domain, session.public_inputs_bytes) != session.public_inputs_hash) {
                vinfo("solana_staged_verify hash check stage failed");
                session.stage_failed = true;
            }
        } else if (stage == VerifyStage::DecodeInputs) {
            if (!session.stage_failed) {
                try {
                    session.decoded_public_inputs = many_from_buffer<uint256_t>(session.public_inputs_bytes);
                    session.decoded_proof = many_from_buffer<uint256_t>(session.proof_bytes);

                    auto validate_commitments = [&]<typename Flavor>() {
                        std::string validation_error;
                        if (!validate_honk_proof_commitments<Flavor>(
                                session.decoded_public_inputs, session.decoded_proof, vk_bytes, validation_error)) {
                            info("solana_staged_verify decode stage rejected malformed commitment data: ",
                                 validation_error);
                            session.stage_failed = true;
                        }
                    };
                    if (flags.ipa_accumulation) {
                        validate_commitments.template operator()<UltraRollupFlavor>();
                    } else if (flags.oracle_hash_type == "poseidon2" && !flags.disable_zk) {
                        validate_commitments.template operator()<UltraZKFlavor>();
                    } else if (flags.oracle_hash_type == "poseidon2" && flags.disable_zk) {
                        validate_commitments.template operator()<UltraFlavor>();
                    } else if (flags.oracle_hash_type == "keccak" && !flags.disable_zk) {
                        validate_commitments.template operator()<UltraKeccakZKFlavor>();
                    } else if (flags.oracle_hash_type == "keccak" && flags.disable_zk) {
                        validate_commitments.template operator()<UltraKeccakFlavor>();
#ifdef STARKNET_GARAGA_FLAVORS
                    } else if (flags.oracle_hash_type == "starknet" && !flags.disable_zk) {
                        validate_commitments.template operator()<UltraStarknetZKFlavor>();
                    } else if (flags.oracle_hash_type == "starknet" && flags.disable_zk) {
                        validate_commitments.template operator()<UltraStarknetFlavor>();
#endif
                    } else {
                        vinfo("solana_staged_verify decode stage: unsupported proof settings");
                        session.stage_failed = true;
                    }
                } catch (const std::exception& err) {
                    vinfo("solana_staged_verify decode stage rejected malformed input: ", err.what());
                    session.stage_failed = true;
                } catch (...) {
                    vinfo("solana_staged_verify decode stage rejected malformed input");
                    session.stage_failed = true;
                }
            }
        } else if (stage == VerifyStage::NativeVerify) {
            if (!session.stage_failed) {
                try {
                    auto response = bbapi::CircuitVerify{ .verification_key = vk_bytes,
                                                          .public_inputs = session.decoded_public_inputs,
                                                          .proof = session.decoded_proof,
                                                          .settings = settings }
                                        .execute();
                    session.verify_result = response.verified;
                } catch (const std::exception& err) {
                    vinfo("solana_staged_verify native verify stage rejected malformed input: ", err.what());
                    session.stage_failed = true;
                    session.verify_result = false;
                } catch (...) {
                    vinfo("solana_staged_verify native verify stage rejected malformed input");
                    session.stage_failed = true;
                    session.verify_result = false;
                }
            }
        } else {
            throw_or_abort("VerifyStep stage index out of range");
        }

        session.stage_index++;
        session.status = SessionStatus::Verifying;
    };

    auto finalize = [&]() -> bool {
        if (session.status != SessionStatus::Verifying) {
            throw_or_abort("Finalize called before VerifyStep");
        }
        if (session.proof_received_len != session.proof_total_len ||
            session.public_inputs_received_len != session.public_inputs_total_len) {
            throw_or_abort("Finalize called before full ingest");
        }
        if (session.stage_index < TOTAL_VERIFY_STAGES) {
            throw_or_abort("Finalize called before any verification stage");
        }
        const bool verified = !session.stage_failed && session.verify_result;
        session.status = verified ? SessionStatus::Verified : SessionStatus::Failed;
        return verified;
    };

    for (uint32_t stage = 0; stage < TOTAL_VERIFY_STAGES; ++stage) {
        verify_step(stage);
    }
    const bool verified = finalize();
    vinfo("solana_staged_verify result: ", verified);
    return verified;
}

bool UltraHonkAPI::prove_and_verify([[maybe_unused]] const Flags& flags,
                                    [[maybe_unused]] const std::filesystem::path& bytecode_path,
                                    [[maybe_unused]] const std::filesystem::path& witness_path)
{
    throw_or_abort("API function prove_and_verify not implemented");
    return false;
}

void UltraHonkAPI::write_vk(const Flags& flags,
                            const std::filesystem::path& bytecode_path,
                            const std::filesystem::path& output_dir)
{
    BB_BENCH_NAME("UltraHonkAPI::write_vk");
    // Validate output directory
    if (output_dir == "-") {
        throw_or_abort("Stdout output is not supported. Please specify an output directory.");
    }

    // Read bytecode
    auto bytecode = get_bytecode(bytecode_path);

    // Convert flags to ProofSystemSettings
    bbapi::ProofSystemSettings settings{ .ipa_accumulation = flags.ipa_accumulation,
                                         .oracle_hash_type = flags.oracle_hash_type,
                                         .disable_zk = flags.disable_zk };

    auto response = bbapi::CircuitComputeVk{ .circuit = { .name = "circuit", .bytecode = std::move(bytecode) },
                                             .settings = settings }
                        .execute();

    write_vk_outputs(response, output_dir);
}

void UltraHonkAPI::gates([[maybe_unused]] const Flags& flags,
                         [[maybe_unused]] const std::filesystem::path& bytecode_path)
{
    BB_BENCH_NAME("UltraHonkAPI::gates");
    // Get the bytecode directly
    auto bytecode = get_bytecode(bytecode_path);

    // All circuit reports will be built into the string below
    std::string functions_string = "{\"functions\": [\n  ";

    // For now, treat the entire bytecode as a single circuit
    // TODO(https://github.com/AztecProtocol/barretenberg/issues/1074): Handle multi-circuit programs properly
    // Convert flags to ProofSystemSettings
    bbapi::ProofSystemSettings settings{ .ipa_accumulation = flags.ipa_accumulation,
                                         .oracle_hash_type = flags.oracle_hash_type,
                                         .disable_zk = flags.disable_zk };

    // Execute CircuitStats command
    auto response = bbapi::CircuitStats{ .circuit = { .name = "circuit", .bytecode = bytecode, .verification_key = {} },
                                         .include_gates_per_opcode = flags.include_gates_per_opcode,
                                         .settings = settings }
                        .execute();

    vinfo("Calculated circuit size in gate_count: ", response.num_gates);

    // Build individual circuit report to match original gate_count output
    std::string gates_per_opcode_str;
    if (flags.include_gates_per_opcode) {
        size_t i = 0;
        for (size_t count : response.gates_per_opcode) {
            if (i != 0) {
                gates_per_opcode_str += ",";
            }
            gates_per_opcode_str += std::to_string(count);
            i++;
        }
    }

    // For now, we'll use the CircuitStats response which includes circuit statistics
    // The num_acir_opcodes is not directly available from bytecode alone
    auto result_string = format(
        "{\n        \"acir_opcodes\": ",
        response.num_acir_opcodes,
        ",\n        \"circuit_size\": ",
        response.num_gates,
        (flags.include_gates_per_opcode ? format(",\n        \"gates_per_opcode\": [", gates_per_opcode_str, "]") : ""),
        "\n  }");

    functions_string = format(functions_string, result_string);
    std::cout << format(functions_string, "\n]}");
}

void UltraHonkAPI::write_solidity_verifier(const Flags& flags,
                                           const std::filesystem::path& output_path,
                                           const std::filesystem::path& vk_path)
{
    BB_BENCH_NAME("UltraHonkAPI::write_solidity_verifier");
    // Read VK file
    auto vk_bytes = read_file(vk_path);

    // Convert flags to ProofSystemSettings
    bbapi::ProofSystemSettings settings{ .ipa_accumulation = flags.ipa_accumulation,
                                         .oracle_hash_type = flags.oracle_hash_type,
                                         .disable_zk = flags.disable_zk,
                                         .optimized_solidity_verifier = flags.optimized_solidity_verifier };

    // Execute solidity verifier command
    auto response = bbapi::CircuitWriteSolidityVerifier{ .verification_key = vk_bytes, .settings = settings }.execute();

    // Write output
    if (output_path == "-") {
        std::cout << response.solidity_code;
    } else {
        write_file(output_path, { response.solidity_code.begin(), response.solidity_code.end() });
        if (flags.disable_zk) {
            info("Honk solidity verifier saved to ", output_path);
        } else {
            info("ZK Honk solidity verifier saved to ", output_path);
        }
    }
}

void UltraHonkAPI::write_solana_verifier(const Flags& flags,
                                         const std::filesystem::path& output_path,
                                         const std::filesystem::path& vk_path)
{
    BB_BENCH_NAME("UltraHonkAPI::write_solana_verifier");
    if (output_path == "-") {
        throw_or_abort("Stdout output is not supported. Please specify an output directory.");
    }

    auto vk_bytes = read_file(vk_path);

    struct GeneratedProgramConfig {
        std::string vk_hash;
        uint32_t log_n = 0;
        uint32_t num_public_inputs = 0;
        uint32_t public_inputs_offset = 0;
    };

    std::string meta_json;
    GeneratedProgramConfig generated_config;

    auto assign_meta_and_config = [&]<typename Flavor>() {
        using VK = typename Flavor::VerificationKey;
        auto vk = from_buffer<VK>(vk_bytes);
        std::ostringstream hash_stream;
        hash_stream << vk.hash();
        generated_config.vk_hash = hash_stream.str();
        generated_config.log_n = static_cast<uint32_t>(vk.log_circuit_size);
        generated_config.num_public_inputs = static_cast<uint32_t>(vk.num_public_inputs);
        generated_config.public_inputs_offset = static_cast<uint32_t>(vk.pub_inputs_offset);
        meta_json = solana::build_vk_meta_json(
            vk, flags.scheme, flags.oracle_hash_type, flags.disable_zk, flags.ipa_accumulation, vk_bytes.size());
    };

    if (flags.ipa_accumulation) {
        assign_meta_and_config.template operator()<UltraRollupFlavor>();
    } else if (flags.oracle_hash_type == "poseidon2" && !flags.disable_zk) {
        assign_meta_and_config.template operator()<UltraZKFlavor>();
    } else if (flags.oracle_hash_type == "poseidon2" && flags.disable_zk) {
        assign_meta_and_config.template operator()<UltraFlavor>();
    } else if (flags.oracle_hash_type == "keccak" && !flags.disable_zk) {
        assign_meta_and_config.template operator()<UltraKeccakZKFlavor>();
    } else if (flags.oracle_hash_type == "keccak" && flags.disable_zk) {
        assign_meta_and_config.template operator()<UltraKeccakFlavor>();
#ifdef STARKNET_GARAGA_FLAVORS
    } else if (flags.oracle_hash_type == "starknet" && !flags.disable_zk) {
        assign_meta_and_config.template operator()<UltraStarknetZKFlavor>();
    } else if (flags.oracle_hash_type == "starknet" && flags.disable_zk) {
        assign_meta_and_config.template operator()<UltraStarknetFlavor>();
#endif
    } else {
        throw_or_abort("invalid proof type in write_solana_verifier");
    }

    std::filesystem::create_directories(output_path);
    const size_t chunk_size = flags.solana_chunk_size == 0 ? solana::SOLANA_DEFAULT_CHUNK_SIZE_BYTES : flags.solana_chunk_size;
    const std::string vk_chunks_json = solana::build_vk_chunks_json(vk_bytes.size(), chunk_size);
    const std::string session_abi_json = solana::build_session_abi_json();
    const std::string client_helper_ts = solana::build_client_helper_ts(chunk_size);
    const auto generated_program_dir = output_path / "solana_verifier_program";
    const auto generated_program_src_dir = generated_program_dir / "src";
    std::filesystem::create_directories(generated_program_src_dir);

    const auto runtime_crate_dir = resolve_solana_runtime_crate_dir();
    std::error_code relative_ec;
    auto runtime_path_from_generated = std::filesystem::relative(runtime_crate_dir, generated_program_dir, relative_ec);
    if (relative_ec || runtime_path_from_generated.empty()) {
        runtime_path_from_generated = runtime_crate_dir;
    }
    const std::string generated_program_cargo_toml =
        solana::build_generated_program_cargo_toml(runtime_path_from_generated.generic_string());
    const std::string generated_program_lib_rs = solana::build_generated_program_lib_rs();
    const std::string generated_program_vk_rs = solana::build_generated_program_vk_rs();
    const std::string generated_program_config_rs =
        solana::build_generated_program_config_rs(flags.scheme,
                                                  flags.oracle_hash_type,
                                                  flags.disable_zk,
                                                  flags.ipa_accumulation,
                                                  generated_config.log_n,
                                                  generated_config.num_public_inputs,
                                                  generated_config.public_inputs_offset,
                                                  generated_config.vk_hash,
                                                  chunk_size,
                                                  solana::SOLANA_SESSION_ABI_VERSION,
                                                  solana::SOLANA_VK_META_VERSION);
    const std::string generated_program_readme = solana::build_generated_program_readme();

    std::vector<uint8_t> meta_buf(meta_json.begin(), meta_json.end());
    std::vector<uint8_t> vk_chunks_buf(vk_chunks_json.begin(), vk_chunks_json.end());
    std::vector<uint8_t> session_abi_buf(session_abi_json.begin(), session_abi_json.end());
    std::vector<uint8_t> client_helper_buf(client_helper_ts.begin(), client_helper_ts.end());
    std::vector<uint8_t> generated_program_cargo_toml_buf(generated_program_cargo_toml.begin(),
                                                          generated_program_cargo_toml.end());
    std::vector<uint8_t> generated_program_lib_rs_buf(generated_program_lib_rs.begin(), generated_program_lib_rs.end());
    std::vector<uint8_t> generated_program_vk_rs_buf(generated_program_vk_rs.begin(), generated_program_vk_rs.end());
    std::vector<uint8_t> generated_program_config_rs_buf(generated_program_config_rs.begin(),
                                                         generated_program_config_rs.end());
    std::vector<uint8_t> generated_program_readme_buf(generated_program_readme.begin(), generated_program_readme.end());
    write_file(output_path / "vk.bin", vk_bytes);
    write_file(output_path / "vk_meta.json", meta_buf);
    write_file(output_path / "vk_chunks.json", vk_chunks_buf);
    write_file(output_path / "session_abi.json", session_abi_buf);
    write_file(output_path / "client_helper.ts", client_helper_buf);
    write_file(generated_program_dir / "Cargo.toml", generated_program_cargo_toml_buf);
    write_file(generated_program_src_dir / "lib.rs", generated_program_lib_rs_buf);
    write_file(generated_program_src_dir / "generated_vk.rs", generated_program_vk_rs_buf);
    write_file(generated_program_src_dir / "generated_config.rs", generated_program_config_rs_buf);
    write_file(generated_program_dir / "README.md", generated_program_readme_buf);
    info("Solana VK saved to ", output_path / "vk.bin");
    info("Solana VK metadata saved to ", output_path / "vk_meta.json");
    info("Solana VK chunk manifest saved to ", output_path / "vk_chunks.json");
    info("Solana session ABI spec saved to ", output_path / "session_abi.json");
    info("Solana client helper saved to ", output_path / "client_helper.ts");
    info("Generated Solana verifier scaffold saved to ", generated_program_dir);
}
} // namespace bb
