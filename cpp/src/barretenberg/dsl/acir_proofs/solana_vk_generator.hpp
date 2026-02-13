// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#pragma once

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>

namespace bb::solana {

inline constexpr uint32_t SOLANA_VK_META_VERSION = 1;
inline constexpr uint32_t SOLANA_SESSION_ABI_VERSION = 2;
inline constexpr uint32_t SOLANA_TOTAL_VERIFY_STAGES = 3;
inline constexpr size_t SOLANA_DEFAULT_CHUNK_SIZE_BYTES = 900;
inline constexpr uint8_t SOLANA_INIT_SESSION_DISCRIMINANT = 0;
inline constexpr uint8_t SOLANA_APPEND_PROOF_CHUNK_DISCRIMINANT = 1;
inline constexpr uint8_t SOLANA_APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT = 2;
inline constexpr uint8_t SOLANA_VERIFY_STEP_DISCRIMINANT = 3;
inline constexpr uint8_t SOLANA_FINALIZE_DISCRIMINANT = 4;

template <typename VerificationKey>
inline std::string build_vk_meta_json(const VerificationKey& vk,
                                      const std::string& scheme,
                                      const std::string& oracle_hash_type,
                                      bool disable_zk,
                                      bool ipa_accumulation,
                                      size_t vk_bytes)
{
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"version\": " << SOLANA_VK_META_VERSION << ",\n";
    ss << "  \"vk_encoding\": \"barretenberg\",\n";
    ss << "  \"scheme\": \"" << scheme << "\",\n";
    ss << "  \"oracle_hash\": \"" << oracle_hash_type << "\",\n";
    ss << "  \"disable_zk\": " << (disable_zk ? "true" : "false") << ",\n";
    ss << "  \"ipa_accumulation\": " << (ipa_accumulation ? "true" : "false") << ",\n";
    ss << "  \"log_n\": " << vk.log_circuit_size << ",\n";
    ss << "  \"num_public_inputs\": " << vk.num_public_inputs << ",\n";
    ss << "  \"public_inputs_offset\": " << vk.pub_inputs_offset << ",\n";
    ss << "  \"vk_hash\": \"" << vk.hash() << "\",\n";
    ss << "  \"vk_bytes\": " << vk_bytes << "\n";
    ss << "}\n";
    return ss.str();
}

inline std::string build_vk_chunks_json(size_t total_bytes, size_t chunk_size = SOLANA_DEFAULT_CHUNK_SIZE_BYTES)
{
    const size_t safe_chunk_size = chunk_size == 0 ? SOLANA_DEFAULT_CHUNK_SIZE_BYTES : chunk_size;
    const size_t num_chunks = (total_bytes + safe_chunk_size - 1) / safe_chunk_size;

    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"chunk_size\": " << safe_chunk_size << ",\n";
    ss << "  \"total_bytes\": " << total_bytes << ",\n";
    ss << "  \"num_chunks\": " << num_chunks << ",\n";
    ss << "  \"chunks\": [\n";
    for (size_t i = 0; i < num_chunks; ++i) {
        const size_t offset = i * safe_chunk_size;
        const size_t remaining = total_bytes - offset;
        const size_t len = remaining < safe_chunk_size ? remaining : safe_chunk_size;
        ss << "    {\"index\": " << i << ", \"offset\": " << offset << ", \"length\": " << len << "}";
        if (i + 1 < num_chunks) {
            ss << ",";
        }
        ss << "\n";
    }
    ss << "  ]\n";
    ss << "}\n";
    return ss.str();
}

inline std::string build_session_abi_json()
{
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"version\": " << SOLANA_SESSION_ABI_VERSION << ",\n";
    ss << "  \"program_model\": \"universal_ultrahonk_verifier\",\n";
    ss << "  \"encoding\": {\n";
    ss << "    \"integers\": \"little_endian_fixed_width\",\n";
    ss << "    \"hashes\": \"bytes32_raw\",\n";
    ss << "    \"pubkeys\": \"bytes32_raw\",\n";
    ss << "    \"variable_length\": \"length_prefixed_u32_le\"\n";
    ss << "  },\n";
    ss << "  \"hashing\": {\n";
    ss << "    \"function\": \"sha256\",\n";
    ss << "    \"domain_tags\": {\n";
    ss << "      \"proof\": \"bb.solana.ultrahonk.proof.v1\",\n";
    ss << "      \"public_inputs\": \"bb.solana.ultrahonk.public_inputs.v1\",\n";
    ss << "      \"session_bind\": \"bb.solana.ultrahonk.session_bind.v1\"\n";
    ss << "    },\n";
    ss << "    \"proof_hash\": \"sha256(domain_tag.proof || proof_bytes)\",\n";
    ss << "    \"public_inputs_hash\": \"sha256(domain_tag.public_inputs || public_inputs_bytes)\",\n";
    ss << "    \"session_bind_hash\": \"sha256(domain_tag.session_bind || vk_hash || proof_hash || "
          "public_inputs_hash || authority || proof_total_len_u64_le || public_inputs_total_len_u64_le)\"\n";
    ss << "  },\n";
    ss << "  \"accounts\": {\n";
    ss << "    \"vk\": {\n";
    ss << "      \"fields\": [\n";
    ss << "        {\"name\": \"vk_hash\", \"type\": \"bytes32\", \"size\": 32},\n";
    ss << "        {\"name\": \"log_n\", \"type\": \"u32\", \"size\": 4},\n";
    ss << "        {\"name\": \"num_public_inputs\", \"type\": \"u32\", \"size\": 4},\n";
    ss << "        {\"name\": \"public_inputs_offset\", \"type\": \"u32\", \"size\": 4},\n";
    ss << "        {\"name\": \"vk_bytes_len\", \"type\": \"u32\", \"size\": 4},\n";
    ss << "        {\"name\": \"vk_bytes\", \"type\": \"bytes\", \"size\": \"variable\"}\n";
    ss << "      ]\n";
    ss << "    },\n";
    ss << "    \"session\": {\n";
    ss << "      \"fields\": [\n";
    ss << "        {\"name\": \"authority\", \"type\": \"pubkey\", \"size\": 32},\n";
    ss << "        {\"name\": \"vk_hash\", \"type\": \"bytes32\", \"size\": 32},\n";
    ss << "        {\"name\": \"proof_hash\", \"type\": \"bytes32\", \"size\": 32},\n";
    ss << "        {\"name\": \"public_inputs_hash\", \"type\": \"bytes32\", \"size\": 32},\n";
    ss << "        {\"name\": \"session_bind_hash\", \"type\": \"bytes32\", \"size\": 32},\n";
    ss << "        {\"name\": \"proof_total_len\", \"type\": \"u64\", \"size\": 8},\n";
    ss << "        {\"name\": \"public_inputs_total_len\", \"type\": \"u64\", \"size\": 8},\n";
    ss << "        {\"name\": \"proof_received_len\", \"type\": \"u64\", \"size\": 8},\n";
    ss << "        {\"name\": \"public_inputs_received_len\", \"type\": \"u64\", \"size\": 8},\n";
    ss << "        {\"name\": \"next_proof_chunk_index\", \"type\": \"u32\", \"size\": 4},\n";
    ss << "        {\"name\": \"next_public_inputs_chunk_index\", \"type\": \"u32\", \"size\": 4},\n";
    ss << "        {\"name\": \"stage_index\", \"type\": \"u32\", \"size\": 4},\n";
    ss << "        {\"name\": \"status\", \"type\": \"u8\", \"size\": 1}\n";
    ss << "      ],\n";
    ss << "      \"status_enum\": {\n";
    ss << "        \"0\": \"CollectingProof\",\n";
    ss << "        \"1\": \"CollectingPublicInputs\",\n";
    ss << "        \"2\": \"ReadyToVerify\",\n";
    ss << "        \"3\": \"Verifying\",\n";
    ss << "        \"4\": \"Verified\",\n";
    ss << "        \"5\": \"Failed\"\n";
    ss << "      }\n";
    ss << "    }\n";
    ss << "  },\n";
    ss << "  \"instruction_discriminants\": {\n";
    ss << "    \"InitSession\": " << static_cast<uint32_t>(SOLANA_INIT_SESSION_DISCRIMINANT) << ",\n";
    ss << "    \"AppendProofChunk\": " << static_cast<uint32_t>(SOLANA_APPEND_PROOF_CHUNK_DISCRIMINANT) << ",\n";
    ss << "    \"AppendPublicInputsChunk\": " << static_cast<uint32_t>(SOLANA_APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT)
       << ",\n";
    ss << "    \"VerifyStep\": " << static_cast<uint32_t>(SOLANA_VERIFY_STEP_DISCRIMINANT) << ",\n";
    ss << "    \"Finalize\": " << static_cast<uint32_t>(SOLANA_FINALIZE_DISCRIMINANT) << "\n";
    ss << "  },\n";
    ss << "  \"instruction_encoding\": {\n";
    ss << "    \"layout\": \"u8_discriminant_then_le_fields\",\n";
    ss << "    \"bytes_fields\": \"u32_le_length_prefixed\"\n";
    ss << "  },\n";
    ss << "  \"verify_stages\": [\n";
    ss << "    {\"index\": 0, \"name\": \"hash_checks\"},\n";
    ss << "    {\"index\": 1, \"name\": \"decode_inputs\"},\n";
    ss << "    {\"index\": 2, \"name\": \"native_verify\"}\n";
    ss << "  ],\n";
    ss << "  \"instructions\": [\n";
    ss << "    {\n";
    ss << "      \"name\": \"InitSession\",\n";
    ss << "      \"discriminant\": " << static_cast<uint32_t>(SOLANA_INIT_SESSION_DISCRIMINANT) << ",\n";
    ss << "      \"fields\": [\n";
    ss << "        {\"name\": \"vk_hash\", \"type\": \"bytes32\"},\n";
    ss << "        {\"name\": \"proof_total_len\", \"type\": \"u64\"},\n";
    ss << "        {\"name\": \"public_inputs_total_len\", \"type\": \"u64\"},\n";
    ss << "        {\"name\": \"proof_hash\", \"type\": \"bytes32\"},\n";
    ss << "        {\"name\": \"public_inputs_hash\", \"type\": \"bytes32\"},\n";
    ss << "        {\"name\": \"session_bind_hash\", \"type\": \"bytes32\"}\n";
    ss << "      ]\n";
    ss << "    },\n";
    ss << "    {\n";
    ss << "      \"name\": \"AppendProofChunk\",\n";
    ss << "      \"discriminant\": " << static_cast<uint32_t>(SOLANA_APPEND_PROOF_CHUNK_DISCRIMINANT) << ",\n";
    ss << "      \"fields\": [\n";
    ss << "        {\"name\": \"chunk_index\", \"type\": \"u32\"},\n";
    ss << "        {\"name\": \"offset\", \"type\": \"u32\"},\n";
    ss << "        {\"name\": \"chunk_bytes\", \"type\": \"bytes\"}\n";
    ss << "      ]\n";
    ss << "    },\n";
    ss << "    {\n";
    ss << "      \"name\": \"AppendPublicInputsChunk\",\n";
    ss << "      \"discriminant\": " << static_cast<uint32_t>(SOLANA_APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT)
       << ",\n";
    ss << "      \"fields\": [\n";
    ss << "        {\"name\": \"chunk_index\", \"type\": \"u32\"},\n";
    ss << "        {\"name\": \"offset\", \"type\": \"u32\"},\n";
    ss << "        {\"name\": \"chunk_bytes\", \"type\": \"bytes\"}\n";
    ss << "      ]\n";
    ss << "    },\n";
    ss << "    {\n";
    ss << "      \"name\": \"VerifyStep\",\n";
    ss << "      \"discriminant\": " << static_cast<uint32_t>(SOLANA_VERIFY_STEP_DISCRIMINANT) << ",\n";
    ss << "      \"fields\": [{\"name\": \"expected_stage_index\", \"type\": \"u32\"}]\n";
    ss << "    },\n";
    ss << "    {\n";
    ss << "      \"name\": \"Finalize\",\n";
    ss << "      \"discriminant\": " << static_cast<uint32_t>(SOLANA_FINALIZE_DISCRIMINANT) << ",\n";
    ss << "      \"fields\": []\n";
    ss << "    }\n";
    ss << "  ],\n";
    ss << "  \"state_machine\": {\n";
    ss << "    \"transitions\": [\n";
    ss << "      {\"from\": \"CollectingProof\", \"instruction\": \"AppendProofChunk\", \"to\": \"CollectingProof\"},\n";
    ss << "      {\"from\": \"CollectingProof\", \"instruction\": \"AppendPublicInputsChunk\", \"to\": "
          "\"CollectingPublicInputs\", \"guard\": \"proof_received_len == proof_total_len\"},\n";
    ss << "      {\"from\": \"CollectingPublicInputs\", \"instruction\": \"AppendPublicInputsChunk\", \"to\": "
          "\"CollectingPublicInputs\"},\n";
    ss << "      {\"from\": \"CollectingPublicInputs\", \"instruction\": \"VerifyStep\", \"to\": \"Verifying\", "
          "\"guard\": \"public_inputs_received_len == public_inputs_total_len\"},\n";
    ss << "      {\"from\": \"Verifying\", \"instruction\": \"VerifyStep\", \"to\": \"Verifying\"},\n";
    ss << "      {\"from\": \"Verifying\", \"instruction\": \"Finalize\", \"to\": \"Verified\", \"guard\": "
          "\"all_verify_stages_complete == true\"}\n";
    ss << "    ]\n";
    ss << "  },\n";
    ss << "  \"invariants\": [\n";
    ss << "    \"chunk_index must be contiguous for each stream\",\n";
    ss << "    \"offset must match bytes already received\",\n";
    ss << "    \"stage_index is monotonic and can only increment by one\",\n";
    ss << "    \"Finalize is allowed only when proof and public inputs are fully ingested\",\n";
    ss << "    \"proof_hash/public_inputs_hash must match sha256 over canonical chunk reassembly\",\n";
    ss << "    \"session_bind_hash must match canonical InitSession tuple binding\"\n";
    ss << "  ]\n";
    ss << "}\n";
    return ss.str();
}

inline std::string build_client_helper_ts(size_t default_chunk_size = SOLANA_DEFAULT_CHUNK_SIZE_BYTES)
{
    std::ostringstream ss;
    ss << "// Auto-generated by bb write_solana_verifier.\n"
          "// This helper only builds deterministic payloads/chunks for a staged verifier flow.\n"
          "// Wire these payloads into your Solana SDK instruction constructors.\n\n"
          "export type Chunk = { index: number; offset: number; bytes: Uint8Array };\n\n"
          "export const SESSION_ABI_VERSION = "
       << SOLANA_SESSION_ABI_VERSION
       << ";\n\n"
          "export enum SessionStatus {\n"
          "  CollectingProof = 0,\n"
          "  CollectingPublicInputs = 1,\n"
          "  ReadyToVerify = 2,\n"
          "  Verifying = 3,\n"
          "  Verified = 4,\n"
          "  Failed = 5,\n"
          "}\n\n"
          "export const INSTRUCTION_DISCRIMINANT = {\n"
          "  InitSession: "
       << static_cast<uint32_t>(SOLANA_INIT_SESSION_DISCRIMINANT)
       << ",\n"
          "  AppendProofChunk: "
       << static_cast<uint32_t>(SOLANA_APPEND_PROOF_CHUNK_DISCRIMINANT)
       << ",\n"
          "  AppendPublicInputsChunk: "
       << static_cast<uint32_t>(SOLANA_APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT)
       << ",\n"
          "  VerifyStep: "
       << static_cast<uint32_t>(SOLANA_VERIFY_STEP_DISCRIMINANT)
       << ",\n"
          "  Finalize: "
       << static_cast<uint32_t>(SOLANA_FINALIZE_DISCRIMINANT)
       << ",\n"
          "} as const;\n\n"
          "export const VERIFY_STAGE = {\n"
          "  HashChecks: 0,\n"
          "  DecodeInputs: 1,\n"
          "  NativeVerify: 2,\n"
          "} as const;\n\n"
          "export const TOTAL_VERIFY_STAGES = "
       << SOLANA_TOTAL_VERIFY_STAGES
       << ";\n\n"
          "const DOMAIN_PROOF = 'bb.solana.ultrahonk.proof.v1';\n"
          "const DOMAIN_PUBLIC_INPUTS = 'bb.solana.ultrahonk.public_inputs.v1';\n"
          "const DOMAIN_SESSION_BIND = 'bb.solana.ultrahonk.session_bind.v1';\n\n"
          "export const DEFAULT_CHUNK_SIZE = "
       << default_chunk_size
       << ";\n\n"
          "export function chunkBytes(input: Uint8Array, chunkSize: number = DEFAULT_CHUNK_SIZE): Chunk[] {\n"
          "  if (chunkSize <= 0) {\n"
          "    throw new Error('chunkSize must be > 0');\n"
          "  }\n"
          "  const chunks: Chunk[] = [];\n"
          "  for (let offset = 0, index = 0; offset < input.length; offset += chunkSize, index += 1) {\n"
          "    const end = Math.min(offset + chunkSize, input.length);\n"
          "    chunks.push({ index, offset, bytes: input.slice(offset, end) });\n"
          "  }\n"
          "  return chunks;\n"
          "}\n\n"
          "export function assertContiguousChunks(chunks: Chunk[]): void {\n"
          "  for (let i = 0; i < chunks.length; i += 1) {\n"
          "    if (chunks[i].index !== i) {\n"
          "      throw new Error(`chunk index mismatch at position ${i}`);\n"
          "    }\n"
          "    if (i === 0) {\n"
          "      if (chunks[i].offset !== 0) {\n"
          "        throw new Error('first chunk offset must be 0');\n"
          "      }\n"
          "      continue;\n"
          "    }\n"
          "    const prev = chunks[i - 1];\n"
          "    const expectedOffset = prev.offset + prev.bytes.length;\n"
          "    if (chunks[i].offset !== expectedOffset) {\n"
          "      throw new Error(`chunk offset mismatch at index ${i}`);\n"
          "    }\n"
          "  }\n"
          "}\n\n"
          "function utf8Bytes(input: string): Uint8Array {\n"
          "  return new TextEncoder().encode(input);\n"
          "}\n\n"
          "function hexToBytes(input: string): Uint8Array {\n"
          "  const normalized = input.startsWith('0x') ? input.slice(2) : input;\n"
          "  if (normalized.length % 2 !== 0) {\n"
          "    throw new Error('hex string must have even length');\n"
          "  }\n"
          "  const out = new Uint8Array(normalized.length / 2);\n"
          "  for (let i = 0; i < out.length; i += 1) {\n"
          "    const byte = normalized.slice(i * 2, i * 2 + 2);\n"
          "    out[i] = Number.parseInt(byte, 16);\n"
          "    if (Number.isNaN(out[i])) {\n"
          "      throw new Error('invalid hex string');\n"
          "    }\n"
          "  }\n"
          "  return out;\n"
          "}\n\n"
          "function u64ToLeBytes(input: number | bigint): Uint8Array {\n"
          "  const value = BigInt(input);\n"
          "  if (value < 0n) {\n"
          "    throw new Error('u64 value must be non-negative');\n"
          "  }\n"
          "  const out = new Uint8Array(8);\n"
          "  let v = value;\n"
          "  for (let i = 0; i < 8; i += 1) {\n"
          "    out[i] = Number(v & 0xffn);\n"
          "    v >>= 8n;\n"
          "  }\n"
          "  return out;\n"
          "}\n\n"
          "function u32ToLeBytes(input: number): Uint8Array {\n"
          "  if (!Number.isInteger(input) || input < 0 || input > 0xffffffff) {\n"
          "    throw new Error('u32 value out of range');\n"
          "  }\n"
          "  const out = new Uint8Array(4);\n"
          "  out[0] = input & 0xff;\n"
          "  out[1] = (input >>> 8) & 0xff;\n"
          "  out[2] = (input >>> 16) & 0xff;\n"
          "  out[3] = (input >>> 24) & 0xff;\n"
          "  return out;\n"
          "}\n\n"
          "function concatBytes(parts: Uint8Array[]): Uint8Array {\n"
          "  const total = parts.reduce((sum, part) => sum + part.length, 0);\n"
          "  const out = new Uint8Array(total);\n"
          "  let offset = 0;\n"
          "  for (const part of parts) {\n"
          "    out.set(part, offset);\n"
          "    offset += part.length;\n"
          "  }\n"
          "  return out;\n"
          "}\n\n"
          "function asBytes32(input: string): Uint8Array {\n"
          "  const bytes = hexToBytes(input);\n"
          "  if (bytes.length !== 32) {\n"
          "    throw new Error('expected 32-byte hex value');\n"
          "  }\n"
          "  return bytes;\n"
          "}\n\n"
          "function bytesWithLenPrefix(input: Uint8Array): Uint8Array {\n"
          "  return concatBytes([u32ToLeBytes(input.length), input]);\n"
          "}\n\n"
          "export function buildProofHashPreimage(proofBytes: Uint8Array): Uint8Array {\n"
          "  return concatBytes([utf8Bytes(DOMAIN_PROOF), proofBytes]);\n"
          "}\n\n"
          "export function buildPublicInputsHashPreimage(publicInputsBytes: Uint8Array): Uint8Array {\n"
          "  return concatBytes([utf8Bytes(DOMAIN_PUBLIC_INPUTS), publicInputsBytes]);\n"
          "}\n\n"
          "export function buildSessionBindHashPreimage(params: {\n"
          "  vkHashHex: string;\n"
          "  proofHashHex: string;\n"
          "  publicInputsHashHex: string;\n"
          "  authorityPubkey: Uint8Array;\n"
          "  proofTotalLen: number | bigint;\n"
          "  publicInputsTotalLen: number | bigint;\n"
          "}): Uint8Array {\n"
          "  if (params.authorityPubkey.length !== 32) {\n"
          "    throw new Error('authorityPubkey must be 32 bytes');\n"
          "  }\n"
          "  return concatBytes([\n"
          "    utf8Bytes(DOMAIN_SESSION_BIND),\n"
          "    asBytes32(params.vkHashHex),\n"
          "    asBytes32(params.proofHashHex),\n"
          "    asBytes32(params.publicInputsHashHex),\n"
          "    params.authorityPubkey,\n"
          "    u64ToLeBytes(params.proofTotalLen),\n"
          "    u64ToLeBytes(params.publicInputsTotalLen),\n"
          "  ]);\n"
          "}\n\n"
          "export function buildInitSessionPayload(params: {\n"
          "  vkHashHex: string;\n"
          "  proofHashHex: string;\n"
          "  publicInputsHashHex: string;\n"
          "  sessionBindHashHex: string;\n"
          "  proofTotalLen: number;\n"
          "  publicInputsTotalLen: number;\n"
          "}) {\n"
          "  return {\n"
          "    instruction: 'InitSession',\n"
          "    ...params,\n"
          "  };\n"
          "}\n\n"
          "export function buildAppendProofChunkPayload(chunk: Chunk) {\n"
          "  return {\n"
          "    instruction: 'AppendProofChunk',\n"
          "    chunkIndex: chunk.index,\n"
          "    offset: chunk.offset,\n"
          "    bytes: chunk.bytes,\n"
          "  };\n"
          "}\n\n"
          "export function buildAppendPublicInputsChunkPayload(chunk: Chunk) {\n"
          "  return {\n"
          "    instruction: 'AppendPublicInputsChunk',\n"
          "    chunkIndex: chunk.index,\n"
          "    offset: chunk.offset,\n"
          "    bytes: chunk.bytes,\n"
          "  };\n"
          "}\n\n"
          "export function buildVerifyStepPayload(expectedStageIndex: number) {\n"
          "  return {\n"
          "    instruction: 'VerifyStep',\n"
          "    expectedStageIndex,\n"
          "  };\n"
          "}\n\n"
          "export function buildFinalizePayload() {\n"
          "  return {\n"
          "    instruction: 'Finalize',\n"
          "  };\n"
          "}\n\n"
          "export function encodeInitSessionInstruction(params: {\n"
          "  vkHashHex: string;\n"
          "  proofHashHex: string;\n"
          "  publicInputsHashHex: string;\n"
          "  sessionBindHashHex: string;\n"
          "  proofTotalLen: number | bigint;\n"
          "  publicInputsTotalLen: number | bigint;\n"
          "}): Uint8Array {\n"
          "  return concatBytes([\n"
          "    new Uint8Array([INSTRUCTION_DISCRIMINANT.InitSession]),\n"
          "    asBytes32(params.vkHashHex),\n"
          "    u64ToLeBytes(params.proofTotalLen),\n"
          "    u64ToLeBytes(params.publicInputsTotalLen),\n"
          "    asBytes32(params.proofHashHex),\n"
          "    asBytes32(params.publicInputsHashHex),\n"
          "    asBytes32(params.sessionBindHashHex),\n"
          "  ]);\n"
          "}\n\n"
          "export function encodeAppendProofChunkInstruction(chunk: Chunk): Uint8Array {\n"
          "  return concatBytes([\n"
          "    new Uint8Array([INSTRUCTION_DISCRIMINANT.AppendProofChunk]),\n"
          "    u32ToLeBytes(chunk.index),\n"
          "    u32ToLeBytes(chunk.offset),\n"
          "    bytesWithLenPrefix(chunk.bytes),\n"
          "  ]);\n"
          "}\n\n"
          "export function encodeAppendPublicInputsChunkInstruction(chunk: Chunk): Uint8Array {\n"
          "  return concatBytes([\n"
          "    new Uint8Array([INSTRUCTION_DISCRIMINANT.AppendPublicInputsChunk]),\n"
          "    u32ToLeBytes(chunk.index),\n"
          "    u32ToLeBytes(chunk.offset),\n"
          "    bytesWithLenPrefix(chunk.bytes),\n"
          "  ]);\n"
          "}\n\n"
          "export function encodeVerifyStepInstruction(expectedStageIndex: number): Uint8Array {\n"
          "  return concatBytes([\n"
          "    new Uint8Array([INSTRUCTION_DISCRIMINANT.VerifyStep]),\n"
          "    u32ToLeBytes(expectedStageIndex),\n"
          "  ]);\n"
          "}\n\n"
          "export function encodeFinalizeInstruction(): Uint8Array {\n"
          "  return new Uint8Array([INSTRUCTION_DISCRIMINANT.Finalize]);\n"
          "}\n\n"
          "export type SessionInstructionPlan = {\n"
          "  initSession: Uint8Array;\n"
          "  appendProofChunks: Uint8Array[];\n"
          "  appendPublicInputsChunks: Uint8Array[];\n"
          "  verifySteps: Uint8Array[];\n"
          "  finalize: Uint8Array;\n"
          "};\n\n"
          "export function buildStagedSessionInstructionPlan(params: {\n"
          "  vkHashHex: string;\n"
          "  proofHashHex: string;\n"
          "  publicInputsHashHex: string;\n"
          "  sessionBindHashHex: string;\n"
          "  proofBytes: Uint8Array;\n"
          "  publicInputsBytes: Uint8Array;\n"
          "  chunkSize?: number;\n"
          "}): SessionInstructionPlan {\n"
          "  const chunkSize = params.chunkSize ?? DEFAULT_CHUNK_SIZE;\n"
          "  const proofChunks = chunkBytes(params.proofBytes, chunkSize);\n"
          "  const publicInputsChunks = chunkBytes(params.publicInputsBytes, chunkSize);\n"
          "  assertContiguousChunks(proofChunks);\n"
          "  assertContiguousChunks(publicInputsChunks);\n\n"
          "  return {\n"
          "    initSession: encodeInitSessionInstruction({\n"
          "      vkHashHex: params.vkHashHex,\n"
          "      proofHashHex: params.proofHashHex,\n"
          "      publicInputsHashHex: params.publicInputsHashHex,\n"
          "      sessionBindHashHex: params.sessionBindHashHex,\n"
          "      proofTotalLen: params.proofBytes.length,\n"
          "      publicInputsTotalLen: params.publicInputsBytes.length,\n"
          "    }),\n"
          "    appendProofChunks: proofChunks.map((chunk) => encodeAppendProofChunkInstruction(chunk)),\n"
          "    appendPublicInputsChunks: publicInputsChunks.map((chunk) =>\n"
          "      encodeAppendPublicInputsChunkInstruction(chunk),\n"
          "    ),\n"
          "    verifySteps: Array.from({ length: TOTAL_VERIFY_STAGES }, (_, stageIndex) =>\n"
          "      encodeVerifyStepInstruction(stageIndex),\n"
          "    ),\n"
          "    finalize: encodeFinalizeInstruction(),\n"
          "  };\n"
          "}\n\n"
          "export function flattenStagedSessionInstructionPlan(plan: SessionInstructionPlan): Uint8Array[] {\n"
          "  return [\n"
          "    plan.initSession,\n"
          "    ...plan.appendProofChunks,\n"
          "    ...plan.appendPublicInputsChunks,\n"
          "    ...plan.verifySteps,\n"
          "    plan.finalize,\n"
          "  ];\n"
          "}\n\n"
          "export function estimateStagedSessionTransactionCount(\n"
          "  proofBytesLength: number,\n"
          "  publicInputsBytesLength: number,\n"
          "  chunkSize: number = DEFAULT_CHUNK_SIZE,\n"
          "): number {\n"
          "  if (!Number.isInteger(proofBytesLength) || proofBytesLength < 0) {\n"
          "    throw new Error('proofBytesLength must be a non-negative integer');\n"
          "  }\n"
          "  if (!Number.isInteger(publicInputsBytesLength) || publicInputsBytesLength < 0) {\n"
          "    throw new Error('publicInputsBytesLength must be a non-negative integer');\n"
          "  }\n"
          "  if (!Number.isInteger(chunkSize) || chunkSize <= 0) {\n"
          "    throw new Error('chunkSize must be a positive integer');\n"
          "  }\n"
          "  const proofChunkCount = Math.ceil(proofBytesLength / chunkSize);\n"
          "  const publicInputsChunkCount = Math.ceil(publicInputsBytesLength / chunkSize);\n"
          "  return 1 + proofChunkCount + publicInputsChunkCount + TOTAL_VERIFY_STAGES + 1;\n"
          "}\n";
    return ss.str();
}

inline std::string build_generated_program_cargo_toml(const std::string& runtime_crate_path)
{
    std::ostringstream ss;
    ss << "[package]\n";
    ss << "name = \"ultrahonk_solana_generated_verifier\"\n";
    ss << "version = \"0.1.0\"\n";
    ss << "edition = \"2021\"\n";
    ss << "license = \"MIT\"\n";
    ss << "description = \"Auto-generated Solana verifier package emitted by bb write_solana_verifier\"\n\n";
    ss << "[lib]\n";
    ss << "path = \"src/lib.rs\"\n";
    ss << "crate-type = [\"cdylib\", \"lib\"]\n\n";
    ss << "[features]\n";
    ss << "default = []\n";
    ss << "sbf-entrypoint = []\n\n";
    ss << "[dependencies]\n";
    ss << "solana-program = \"3.0.0\"\n";
    ss << "ultrahonk_verifier_runtime = { package = \"ultrahonk_verifier_program\", path = \""
       << runtime_crate_path << "\", default-features = false }\n";
    return ss.str();
}

inline std::string build_generated_program_lib_rs()
{
    return R"(#![forbid(unsafe_code)]

pub mod generated_config;
pub mod generated_vk;

use solana_program::{account_info::AccountInfo, entrypoint::ProgramResult, program_error::ProgramError, pubkey::Pubkey};
#[cfg(feature = "sbf-entrypoint")]
use solana_program::entrypoint;

pub use ultrahonk_verifier_runtime::{
    APPEND_PROOF_CHUNK_DISCRIMINANT, APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT, FINALIZE_DISCRIMINANT,
    INIT_SESSION_DISCRIMINANT, SESSION_ABI_VERSION, TOTAL_VERIFY_STAGES, VERIFY_STEP_DISCRIMINANT,
};

const _: [(); generated_config::SESSION_ABI_VERSION as usize] = [(); SESSION_ABI_VERSION as usize];
const _: [(); generated_config::TOTAL_VERIFY_STAGES as usize] = [(); TOTAL_VERIFY_STAGES as usize];
const _: [(); generated_config::INIT_SESSION_DISCRIMINANT as usize] = [(); INIT_SESSION_DISCRIMINANT as usize];
const _: [(); generated_config::APPEND_PROOF_CHUNK_DISCRIMINANT as usize] =
    [(); APPEND_PROOF_CHUNK_DISCRIMINANT as usize];
const _: [(); generated_config::APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT as usize] =
    [(); APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT as usize];
const _: [(); generated_config::VERIFY_STEP_DISCRIMINANT as usize] = [(); VERIFY_STEP_DISCRIMINANT as usize];
const _: [(); generated_config::FINALIZE_DISCRIMINANT as usize] = [(); FINALIZE_DISCRIMINANT as usize];

pub fn vk_bytes() -> &'static [u8] {
    generated_vk::VK_BYTES
}

#[cfg(feature = "sbf-entrypoint")]
entrypoint!(entrypoint_process_instruction);

pub fn entrypoint_process_instruction(
    program_id: &Pubkey,
    accounts: &[AccountInfo],
    instruction_data: &[u8],
) -> ProgramResult {
    process_instruction(program_id, accounts, instruction_data)
}

fn hex_nibble(value: u8) -> Option<u8> {
    match value {
        b'0'..=b'9' => Some(value - b'0'),
        b'a'..=b'f' => Some(value - b'a' + 10),
        b'A'..=b'F' => Some(value - b'A' + 10),
        _ => None,
    }
}

fn decode_generated_vk_hash() -> Option<[u8; 32]> {
    let mut hex = generated_config::VK_HASH.as_bytes();
    if hex.len() >= 2 && hex[0] == b'0' && (hex[1] == b'x' || hex[1] == b'X') {
        hex = &hex[2..];
    }
    if hex.len() != 64 {
        return None;
    }
    let mut out = [0u8; 32];
    let mut index = 0usize;
    while index < out.len() {
        let hi = hex_nibble(hex[index * 2])?;
        let lo = hex_nibble(hex[index * 2 + 1])?;
        out[index] = (hi << 4) | lo;
        index += 1;
    }
    Some(out)
}

fn enforce_generated_vk_hash_on_init(instruction_data: &[u8]) -> ProgramResult {
    if instruction_data.first().copied() != Some(INIT_SESSION_DISCRIMINANT) {
        return Ok(());
    }
    if instruction_data.len() < 33 {
        return Ok(());
    }
    let expected_hash = decode_generated_vk_hash().ok_or(ProgramError::InvalidInstructionData)?;
    if instruction_data[1..33] != expected_hash[..] {
        return Err(ProgramError::InvalidInstructionData);
    }
    Ok(())
}

pub fn process_instruction(
    program_id: &Pubkey,
    accounts: &[AccountInfo],
    instruction_data: &[u8],
) -> ProgramResult {
    enforce_generated_vk_hash_on_init(instruction_data)?;
    ultrahonk_verifier_runtime::process_instruction_accounts(program_id, accounts, instruction_data)
}

#[cfg(test)]
mod tests {
    use super::*;
    use solana_program::{program_error::ProgramError, pubkey::Pubkey};
    use ultrahonk_verifier_runtime as runtime;

    fn init_instruction_with_hash(hash: [u8; 32]) -> Vec<u8> {
        let mut out = Vec::with_capacity(33);
        out.push(INIT_SESSION_DISCRIMINANT);
        out.extend_from_slice(&hash);
        out
    }

    fn encode_init_with_lengths(
        authority: [u8; 32],
        proof_bytes: &[u8],
        public_inputs_bytes: &[u8],
    ) -> Vec<u8> {
        let proof_hash = runtime::compute_proof_hash_bytes(proof_bytes);
        let public_inputs_hash = runtime::compute_public_inputs_hash_bytes(public_inputs_bytes);
        let session_bind_hash = runtime::compute_session_bind_hash_bytes(
            &decode_generated_vk_hash().expect("generated VK hash must decode"),
            &proof_hash,
            &public_inputs_hash,
            &authority,
            proof_bytes.len() as u64,
            public_inputs_bytes.len() as u64,
        );
        runtime::encode_instruction(&runtime::VerifierInstruction::InitSession(
            runtime::InitSessionPayload {
                vk_hash: decode_generated_vk_hash().expect("generated VK hash must decode"),
                proof_total_len: proof_bytes.len() as u64,
                public_inputs_total_len: public_inputs_bytes.len() as u64,
                proof_hash,
                public_inputs_hash,
                session_bind_hash,
            },
        ))
    }

    #[test]
    fn generated_vk_hash_guard_accepts_matching_init_hash() {
        let expected = decode_generated_vk_hash().expect("generated VK hash must decode");
        let instruction = init_instruction_with_hash(expected);
        assert!(enforce_generated_vk_hash_on_init(&instruction).is_ok());
    }

    #[test]
    fn generated_vk_hash_guard_rejects_mismatched_init_hash() {
        let mut expected = decode_generated_vk_hash().expect("generated VK hash must decode");
        expected[0] ^= 1;
        let instruction = init_instruction_with_hash(expected);
        assert_eq!(
            enforce_generated_vk_hash_on_init(&instruction),
            Err(ProgramError::InvalidInstructionData)
        );
    }

    #[test]
    fn generated_vk_hash_guard_ignores_non_init_discriminants() {
        let instruction = [FINALIZE_DISCRIMINANT];
        assert!(enforce_generated_vk_hash_on_init(&instruction).is_ok());
    }

    #[test]
    fn runtime_smoke_rejects_verify_step_before_init() {
        let authority = [7u8; 32];
        let mut session = runtime::SessionAccount::default();
        let verify_ix = runtime::encode_instruction(&runtime::VerifierInstruction::VerifyStep {
            expected_stage_index: 0,
        });
        let err = runtime::process_instruction(&mut session, authority, &verify_ix).unwrap_err();
        assert_eq!(err, runtime::SessionError::SessionNotInitialized);
    }

    #[test]
    fn runtime_smoke_rejects_wrong_stage_index_after_init() {
        let authority = [8u8; 32];
        let mut session = runtime::SessionAccount::default();

        let init_ix = encode_init_with_lengths(authority, &[], &[]);
        runtime::process_instruction(&mut session, authority, &init_ix).expect("init must succeed");

        let verify_ix = runtime::encode_instruction(&runtime::VerifierInstruction::VerifyStep {
            expected_stage_index: 1,
        });
        let err = runtime::process_instruction(&mut session, authority, &verify_ix).unwrap_err();
        assert_eq!(
            err,
            runtime::SessionError::VerifyStageMismatch {
                expected: 0,
                got: 1
            }
        );
    }

    #[test]
    fn runtime_smoke_progresses_stages_and_finalizes_failed_for_empty_payload() {
        let authority = [9u8; 32];
        let mut session = runtime::SessionAccount::default();

        let init_ix = encode_init_with_lengths(authority, &[], &[]);
        runtime::process_instruction(&mut session, authority, &init_ix).expect("init must succeed");
        assert_eq!(session.status, runtime::SessionStatus::ReadyToVerify);

        for stage in 0..runtime::TOTAL_VERIFY_STAGES {
            let verify_ix = runtime::encode_instruction(&runtime::VerifierInstruction::VerifyStep {
                expected_stage_index: stage,
            });
            runtime::process_instruction(&mut session, authority, &verify_ix)
                .expect("verify stage must execute");
        }

        let finalize_ix = runtime::encode_instruction(&runtime::VerifierInstruction::Finalize);
        let finalized = runtime::process_instruction(&mut session, authority, &finalize_ix)
            .expect("finalize must execute");
        assert_eq!(finalized, Some(false));
        assert_eq!(session.status, runtime::SessionStatus::Failed);
    }

    #[test]
    fn account_path_smoke_forwards_non_init_instruction_to_runtime() {
        let program_id = Pubkey::new_unique();
        let finalize_ix = runtime::encode_instruction(&runtime::VerifierInstruction::Finalize);
        let err = process_instruction(&program_id, &[], &finalize_ix).unwrap_err();
        assert_eq!(err, ProgramError::Custom(8));
    }

    #[test]
    fn account_path_smoke_matching_init_hash_reaches_runtime_checks() {
        let program_id = Pubkey::new_unique();
        let authority = [10u8; 32];
        let init_ix = encode_init_with_lengths(authority, &[], &[]);
        let err = process_instruction(&program_id, &[], &init_ix).unwrap_err();
        assert_eq!(err, ProgramError::Custom(8));
    }
}

pub fn summary() -> String {
    format!(
        "scheme={} oracle_hash={} disable_zk={} ipa_accumulation={} vk_hash={} log_n={} num_public_inputs={} public_inputs_offset={} session_abi_version={} runtime_session_abi_version={} vk_meta_version={} default_chunk_size={}",
        generated_config::SCHEME,
        generated_config::ORACLE_HASH,
        generated_config::DISABLE_ZK,
        generated_config::IPA_ACCUMULATION,
        generated_config::VK_HASH,
        generated_config::LOG_N,
        generated_config::NUM_PUBLIC_INPUTS,
        generated_config::PUBLIC_INPUTS_OFFSET,
        generated_config::SESSION_ABI_VERSION,
        SESSION_ABI_VERSION,
        generated_config::VK_META_VERSION,
        generated_config::DEFAULT_CHUNK_SIZE
    )
}
)";
}

inline std::string build_generated_program_vk_rs()
{
    return R"(// Auto-generated by bb write_solana_verifier.
// This points to the artifact emitted next to the generated package.
pub const VK_BYTES: &[u8] = include_bytes!("../../vk.bin");
)";
}

inline std::string build_generated_program_config_rs(const std::string& scheme,
                                                     const std::string& oracle_hash_type,
                                                     bool disable_zk,
                                                     bool ipa_accumulation,
                                                     uint32_t log_n,
                                                     uint32_t num_public_inputs,
                                                     uint32_t public_inputs_offset,
                                                     const std::string& vk_hash,
                                                     size_t default_chunk_size,
                                                     uint32_t session_abi_version,
                                                     uint32_t vk_meta_version)
{
    std::ostringstream ss;
    ss << "// Auto-generated by bb write_solana_verifier.\n";
    ss << "pub const SCHEME: &str = \"" << scheme << "\";\n";
    ss << "pub const ORACLE_HASH: &str = \"" << oracle_hash_type << "\";\n";
    ss << "pub const DISABLE_ZK: bool = " << (disable_zk ? "true" : "false") << ";\n";
    ss << "pub const IPA_ACCUMULATION: bool = " << (ipa_accumulation ? "true" : "false") << ";\n";
    ss << "pub const LOG_N: u32 = " << log_n << ";\n";
    ss << "pub const NUM_PUBLIC_INPUTS: u32 = " << num_public_inputs << ";\n";
    ss << "pub const PUBLIC_INPUTS_OFFSET: u32 = " << public_inputs_offset << ";\n";
    ss << "pub const VK_HASH: &str = \"" << vk_hash << "\";\n";
    ss << "pub const DEFAULT_CHUNK_SIZE: usize = " << default_chunk_size << ";\n";
    ss << "pub const SESSION_ABI_VERSION: u32 = " << session_abi_version << ";\n";
    ss << "pub const TOTAL_VERIFY_STAGES: u32 = " << SOLANA_TOTAL_VERIFY_STAGES << ";\n";
    ss << "pub const INIT_SESSION_DISCRIMINANT: u8 = " << static_cast<uint32_t>(SOLANA_INIT_SESSION_DISCRIMINANT)
       << ";\n";
    ss << "pub const APPEND_PROOF_CHUNK_DISCRIMINANT: u8 = "
       << static_cast<uint32_t>(SOLANA_APPEND_PROOF_CHUNK_DISCRIMINANT) << ";\n";
    ss << "pub const APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT: u8 = "
       << static_cast<uint32_t>(SOLANA_APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT) << ";\n";
    ss << "pub const VERIFY_STEP_DISCRIMINANT: u8 = " << static_cast<uint32_t>(SOLANA_VERIFY_STEP_DISCRIMINANT)
       << ";\n";
    ss << "pub const FINALIZE_DISCRIMINANT: u8 = " << static_cast<uint32_t>(SOLANA_FINALIZE_DISCRIMINANT) << ";\n";
    ss << "pub const VK_META_VERSION: u32 = " << vk_meta_version << ";\n";
    return ss.str();
}

inline std::string build_generated_program_readme()
{
    return R"(# Generated Solana Verifier Scaffold

This package was auto-generated by `bb write_solana_verifier`.

## Files

- `src/generated_vk.rs`: points to `../../vk.bin` via `include_bytes!`.
- `src/generated_config.rs`: verifier configuration constants derived from the VK and flags.
- `src/lib.rs`: entrypoint + instruction processor wired to shared staged runtime.

## Notes

- The generated package is wired by default to the shared staged runtime crate.
- The generated package enforces `InitSession.vk_hash` against `generated_config::VK_HASH` before forwarding to runtime.
- The generated package compile-locks ABI/stage/discriminant constants against runtime constants.
- It is intended to be build/deploy/verify-ready within this repository layout.

## Client Helper (output root)

`../client_helper.ts` includes deterministic instruction-data builders for full staged-session orchestration:

- `buildStagedSessionInstructionPlan(...)`
- `flattenStagedSessionInstructionPlan(...)`
- `estimateStagedSessionTransactionCount(...)`

Typical integration flow:

1. Compute proof/public-input/session-bind hashes in your client.
2. Build a full staged instruction plan with `buildStagedSessionInstructionPlan(...)`.
3. Flatten encoded instruction data with `flattenStagedSessionInstructionPlan(...)`.
4. Submit those instruction payloads in order with your Solana SDK.
)";
}

} // namespace bb::solana
