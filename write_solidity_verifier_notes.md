# `write_solidity_verifier` Notes

## What it does

`bb write_solidity_verifier` generates a Solidity verifier contract from an existing verification key (`vk`) file.

It does not compile or inspect the Noir circuit directly at this stage. The circuit-specific data is already encoded in the VK.

## End-to-end flow

1. Build a circuit VK first (typically with `bb write_vk` from Noir ACIR bytecode).
2. Run `bb write_solidity_verifier -k <vk_path> -o <output.sol>`.
3. The command reads the VK bytes and emits Solidity source with:
- a VK loader library containing circuit-specific constants/commitments
- a verifier contract template implementing Honk verification logic

## CLI wiring

Command registration and dispatch:
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/bb/cli.cpp:429`
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/bb/cli.cpp:637`

The CLI subcommand takes:
- `--scheme/-s` (default `ultra_honk`)
- `--vk_path/-k`
- `--output_path/-o`
- `--disable_zk`
- `--optimized`

## API path in code

Main implementation:
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/api/api_ultra_honk.cpp:207`
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/bbapi/bbapi_ultra_honk.cpp:380`

Behavior:
- reads VK bytes from disk
- builds `ProofSystemSettings`
- executes `CircuitWriteSolidityVerifier`
- writes Solidity text to file (or stdout if `-o -`)

## How Solidity is assembled

`CircuitWriteSolidityVerifier::execute()`:
- deserializes VK as `UltraKeccakFlavor::VerificationKey`
- selects one contract generator:
  - non-zk: `get_honk_solidity_verifier(vk)`
  - zk: `get_honk_zk_solidity_verifier(vk)`
  - optimized non-zk (native only): `get_optimized_honk_solidity_verifier(vk)`

References:
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/bbapi/bbapi_ultra_honk.cpp:383`
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/bbapi/bbapi_ultra_honk.cpp:386`
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/bbapi/bbapi_ultra_honk.cpp:391`

VK embedding:
- `output_vk_sol_ultra_honk(...)` writes constants like `N`, `LOG_N`, `VK_HASH`, public input count, and all precomputed commitments into a Solidity library.
- then the full verifier template source is appended.

Reference:
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/honk/utils/honk_key_gen.hpp:19`
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/dsl/acir_proofs/honk_contract.hpp:2174`
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/dsl/acir_proofs/honk_zk_contract.hpp:2335`
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/dsl/acir_proofs/honk_optimized_contract.hpp:3300`

## Practical compatibility notes

`write_solidity_verifier` itself does not accept `--oracle_hash`, but generated verifier logic uses Keccak transcript challenges in Solidity.

So, in practice:
- generate/prove with Keccak settings for onchain compatibility
- use matching proof flavor and VK flavor

Docs showing this expectation:
- `/Users/Americansprit/blockchain/barretenberg/docs/docs/how_to_guides/how-to-solidity-verifier.md:39`

One docs caveat:
- `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/bb/readme.md:94` shows `-b` with `write_solidity_verifier`, but this subcommand does not define a bytecode option in current CLI wiring.

## Is it truly "universal"?

Accurate statement:
- universal across UltraHonk-compatible circuits, because it parameterizes verifier code with the circuit VK.

Not universal across all schemes:
- `ChonkAPI::write_solidity_verifier` is not implemented and throws.
  - `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/api/api_chonk.cpp:140`
- `avm` is an allowed scheme flag generally, but this subcommand has no AVM handler path.
  - `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/bb/cli.cpp:741`
  - `/Users/Americansprit/blockchain/barretenberg/cpp/src/barretenberg/bb/cli.cpp:795`

## Useful command sequence

```bash
bb write_vk --scheme ultra_honk --oracle_hash keccak -b ./target/program.json -o ./target
bb write_solidity_verifier --scheme ultra_honk -k ./target/vk -o ./target/Verifier.sol
```
