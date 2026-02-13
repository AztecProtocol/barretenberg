---
title: Generate Solana Verifier Artifacts
description:
  Generate Solana-oriented UltraHonk verifier artifacts from a verification key, including metadata,
  chunk manifests, and a staged-session client helper.
sidebar_position: 1
---

This guide shows how to generate Solana verifier artifacts with `bb`.

## Step 1 - Generate a verification key

```bash
bb write_vk --scheme ultra_honk -b ./target/<noir_artifact_name>.json -o ./target
```

## Step 2 - Generate Solana artifacts

```bash
bb write_solana_verifier --scheme ultra_honk -k ./target/vk -o ./target/solana
```

If your proof/VK flow uses non-default options, pass matching flags:

```bash
bb write_solana_verifier \
  --scheme ultra_honk \
  --oracle_hash keccak \
  --disable_zk \
  --chunk_size 900 \
  -k ./target/vk \
  -o ./target/solana
```

## Outputs

`bb write_solana_verifier` writes:

- `vk.bin`: serialized verification key bytes.
- `vk_meta.json`: circuit/proof settings metadata and `vk_hash`.
- `vk_chunks.json`: deterministic chunk layout for VK account initialization.
- `session_abi.json`: canonical staged-session ABI spec including:
  - typed account layouts
  - instruction discriminants and payload types
  - binary instruction encoding layout (discriminant + LE fields)
  - verify stage index map (`hash_checks`, `decode_inputs`, `native_verify`)
  - state machine transitions/status enum
  - hash boundary and canonical encoding rules
- `client_helper.ts`: helper functions for payload chunking, staged payload construction, and binary instruction encoding.
- `solana_verifier_program/`: generated Rust scaffold package:
  - `Cargo.toml`
  - `src/lib.rs`
  - `src/generated_vk.rs`
  - `src/generated_config.rs`
  - `README.md`

You can control chunking with `--chunk_size`.

## Step 2.1 - Validate generated Rust scaffold compiles

```bash
cargo check --offline --manifest-path ./target/solana/solana_verifier_program/Cargo.toml
```

Optional SBF build validation (when Solana toolchain is installed):

```bash
PATH=/Users/Americansprit/.local/share/solana/install/active_release/bin:$PATH \
  cargo build-sbf --offline --tools-version v1.52 \
  --sbf-out-dir ./target/solana/sbf_out \
  --manifest-path ./target/solana/solana_verifier_program/Cargo.toml
```

## Step 2.2 - Build staged instruction data with `client_helper.ts`

The generated helper can produce deterministic staged instruction payloads for a full verification session.

```ts
import {
  buildStagedSessionInstructionPlan,
  flattenStagedSessionInstructionPlan,
  estimateStagedSessionTransactionCount,
} from './target/solana/client_helper';

const proofBytes = /* Uint8Array proof bytes */;
const publicInputsBytes = /* Uint8Array public inputs bytes */;

const plan = buildStagedSessionInstructionPlan({
  vkHashHex,
  proofHashHex,
  publicInputsHashHex,
  sessionBindHashHex,
  proofBytes,
  publicInputsBytes,
  chunkSize: 900,
});

const instructionData = flattenStagedSessionInstructionPlan(plan);
const estimatedTxCount = estimateStagedSessionTransactionCount(
  proofBytes.length,
  publicInputsBytes.length,
  900,
);
```

This gives you encoded instruction data for:
- `InitSession`
- `AppendProofChunk` (chunked)
- `AppendPublicInputsChunk` (chunked)
- `VerifyStep` (stages `0..2`)
- `Finalize`

The generated on-chain package enforces that `InitSession.vk_hash` matches `src/generated_config.rs::VK_HASH` and compile-locks ABI/stage/discriminant constants (`SESSION_ABI_VERSION`, `TOTAL_VERIFY_STAGES`, instruction discriminants) against the shared runtime crate constants.

## Step 3 - Execute minimal staged verification locally

Use the generated proof/public-input artifacts to run a local staged-session flow:

```bash
bb solana_staged_verify \
  --scheme ultra_honk \
  --chunk_size 900 \
  -k ./target/vk \
  -p ./target/proof \
  -i ./target/public_inputs
```

This command executes:
- `InitSession` (session binding + lengths/hash commitments)
- chunked proof/public-input ingestion
- `VerifyStep` stage `0` (`hash_checks`)
- `VerifyStep` stage `1` (`decode_inputs`, including commitment and special-public-input pairing-point on-curve prevalidation)
- `VerifyStep` stage `2` (`native_verify`)
- `Finalize` with boolean result

## Program Scaffold

A standalone on-chain state-machine scaffold lives at:

- `/Users/Americansprit/blockchain/barretenberg/solana/ultrahonk_verifier_program`

It currently implements instruction decoding, session transition checks, and unit tests, and is intended to be the
base for full `solana-program` account wiring.

## Notes

- Output path is treated as a directory.
- `-o -` (stdout) is not supported for this command.
- These artifacts are intended for a universal staged Solana verifier program that consumes VK data at runtime.
