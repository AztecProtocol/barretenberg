# BB

### Why is this needed?

Barretenberg is a library that allows one to create and verify proofs. One way to specify the circuit that one will use to create and verify
proofs over is to use the Barretenberg standard library. Another way, which pertains to this module is to supply the circuit description using an IR called [ACIR](https://github.com/noir-lang/acvm).

This binary will take as input ACIR and witness values described in the IR to create proofs.

### Installation

Follow the installation instructions described [here](../../../../../barretenberg/bbup/README.md#installation).

### Usage prerequisites

Certain `bb` commands will expect the tool `jq` to already be installed. If `jq -V` doesn't return a version number, install it from [here](https://jqlang.github.io/jq/download/).

### Version compatibility with Noir

TODO: https://github.com/AztecProtocol/aztec-packages/issues/7511

For quick reference:

- Noir v0.38.0 <> BB v0.61.0
- Noir v0.37.0 <> BB v0.61.0
- Noir v0.36.0 <> BB v0.58.0
- Noir v0.35.0 <> BB v0.56.0
- Noir v0.34.0 <> BB v0.55.0
- Noir v0.33.0 <> BB v0.47.1
- Noir v0.32.0 <> BB v0.46.1
- Noir v0.31.0 <> BB v0.41.0

### Usage

TODO: https://github.com/AztecProtocol/aztec-packages/issues/7600

All available `bb` commands:
https://github.com/AztecProtocol/aztec-packages/blob/barretenberg-v0.55.0/barretenberg/cpp/src/barretenberg/bb/main.cpp#L654-L734

#### FilePath vs Stdout

For commands which allow you to send the output to a file using `-o {filePath}`, there is also the option to send the output to stdout by using `-o -`.

#### Usage with UltraHonk

##### Proving and verifying

1. Follow [the Noir docs](https://noir-lang.org/docs/getting_started/quick_start) to compile and generate witness of your Noir program

2. Prove the valid execution of your Noir program running:

   ```bash
   bb prove --scheme ultra_honk -b ./target/hello_world.json -w ./target/witness-name.gz -o ./target/proof
   ```

3. Compute the verification key for your Noir program running:

   ```bash
   bb write_vk --scheme ultra_honk -b ./target/hello_world.json -o ./target/vk
   ```

4. Verify your proof running:

   ```bash
   bb verify --scheme ultra_honk -k ./target/vk -p ./target/proof
   ```

   If successful, the verification will complete in silence; if unsuccessful, the command will trigger logging of the corresponding error.

Refer to all available `bb` commands linked above for full list of functionality.

##### Generating proofs for verifying in Solidity

Barretenberg UltraHonk comes with the capability to verify proofs in Solidity, i.e. in smart contracts on EVM chains.

1. Prove the valid execution of your Noir program running:

   ```bash
   bb prove --scheme ultra_honk --oracle-hash keccak -b ./target/hello_world.json -w ./target/witness-name.gz -o ./target/proof
   ```

   > **Note:** `--oracle-hash keccak` flag is used to generate UltraHonk proofs with Keccak hashes, as it is what the Solidity verifier is designed to be compatible with given the better gas efficiency when verifying onchain; The default `--oracle-hash poseidon` in comparison generates proofs with Poseidon hashes, which is more efficient in recursions but not for onchain verifications.

2. Compute the verification key for your Noir program running:

   ```bash
   bb write_vk --scheme ultra_honk -b ./target/hello_world.json -o ./target/vk
   ```

3. Generate Solidity verifier
   **WARNING:** Contract incomplete, do not use in production!

   ```bash
   bb write_solidity_verifier --scheme ultra_honk -k ./target/vk -o ./target/Verifier.sol
   ```

##### Generating artifacts for Solana verification sessions

Barretenberg UltraHonk can emit Solana-oriented verifier artifacts from a precomputed VK.

1. Compute the verification key for your Noir program:

   ```bash
   bb write_vk --scheme ultra_honk -b ./target/hello_world.json -o ./target
   ```

2. Generate Solana artifacts:

   ```bash
   bb write_solana_verifier --scheme ultra_honk --chunk_size 900 -k ./target/vk -o ./target/solana
   ```

3. Run a local minimal staged-session verification (Phase 2 bootstrap):

   ```bash
   bb solana_staged_verify \
     --scheme ultra_honk \
     --chunk_size 900 \
     -k ./target/vk \
     -p ./target/proof \
     -i ./target/public_inputs
   ```

4. Output files:
   - `./target/solana/vk.bin`: serialized verification key bytes
   - `./target/solana/vk_meta.json`: VK metadata (`vk_hash`, `log_n`, `num_public_inputs`, etc.)
   - `./target/solana/vk_chunks.json`: deterministic VK chunk manifest
   - `./target/solana/session_abi.json`: canonical staged-session ABI (typed account layouts, instruction discriminants, binary encoding layout, verify stage map, state transitions, hash boundary rules)
   - `./target/solana/client_helper.ts`: helper for deterministic chunking, canonical hash preimage construction, staged payloads, and binary instruction encoding
   - `./target/solana/solana_verifier_program/Cargo.toml`
   - `./target/solana/solana_verifier_program/src/lib.rs`
   - `./target/solana/solana_verifier_program/src/generated_vk.rs`
   - `./target/solana/solana_verifier_program/src/generated_config.rs`
   - `./target/solana/solana_verifier_program/README.md`

5. Validate generated scaffold compile:

   ```bash
   cargo check --offline --manifest-path ./target/solana/solana_verifier_program/Cargo.toml
   ```

   Optional SBF build check:

   ```bash
   PATH=/Users/Americansprit/.local/share/solana/install/active_release/bin:$PATH \
     cargo build-sbf --offline --tools-version v1.52 \
     --sbf-out-dir ./target/solana/sbf_out \
     --manifest-path ./target/solana/solana_verifier_program/Cargo.toml
   ```

#### Usage with MegaHonk

Use `bb <command>_mega_honk`.

Refer to all available `bb` commands linked above for full list of functionality.

Note that MegaHonk:

- Generates insecure recursion circuits when Goblin recursive verifiers are not present
- Will not have a Solidity verifier, as the proving system is intended for use with apps deploying on Aztec only

### Maximum circuit size

Currently the binary downloads an SRS that can be used to prove the maximum circuit size. This maximum circuit size parameter is a constant in the code and has been set to $2^{23}$ as of writing. This maximum circuit size differs from the maximum circuit size that one can prove in the browser, due to WASM limits.
