# UltraHonk Solana Verifier Program Scaffold

This crate is a standalone scaffold for the staged UltraHonk verifier session state machine and
the generated `session_abi.json` / `client_helper.ts` artifacts emitted by
`bb write_solana_verifier`.

## What is implemented

- canonical instruction discriminants and binary decoding/encoding
- session account data model (`InitSession`, chunk ingest, `VerifyStep`, `Finalize`)
- Solana runtime wiring:
  - `entrypoint` + account-based processor
  - session account serialization/deserialization in account data
  - signer, writable, and program-owner validation
  - session PDA derivation/checks (`["session", authority, session_bind_hash]`)
  - optional VK account hash-binding check on `InitSession`
- strict transition/invariant checks:
  - authority binding
  - contiguous chunk indices
  - offset monotonicity
  - stage monotonicity
  - finalize gating
- multi-stage verify scaffold:
  - stage 0: production hash checks (`proof_hash`, `public_inputs_hash`, `session_bind_hash`)
  - stage 1: canonical decode/sanity checks (`public_inputs` as canonical BN254 field elements)
  - stage 2: Rust native-verify kernel module (`src/native_verify_kernel.rs`) with deterministic
    commitment-consistency checks, transcript digesting, BN254 backend preflight checks, and
    structured arithmetic verification (`src/bn254_arithmetic.rs`):
    - G1 MSM consistency check from decoded proof witness fields
    - pairing product check over decoded witness points
- CU harness:
  - ignored integration test at `tests/cu_bench.rs` for per-instruction and full-session CU
  - emits `cu_*` lines for each stage and total session flow

## What is intentionally pending

- full UltraHonk arithmetic kernel in-program (stage 2 now executes real BN254 MSM/pairing
  operations, but still uses a transitional structured witness layout rather than full UH decode)

## Local check

```bash
cd solana/ultrahonk_verifier_program
cargo test
```

To include the exported Solana `entrypoint!` symbol in builds, enable:

```bash
cargo test --features sbf-entrypoint
```

Run SBF CU benchmark harness:

```bash
PATH="$HOME/.local/share/solana/install/active_release/bin:$PATH" \
  cargo-test-sbf \
  --tools-version v1.52 --features sbf-entrypoint --test cu_bench -- --ignored --nocapture
```
