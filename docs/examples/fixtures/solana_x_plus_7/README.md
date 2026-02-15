# Solana UltraHonk Demo: `x + 7 = 13`

This Noir circuit proves the statement:

`x + 7 = 13`

So the only valid input is `x = 6`.

## Run the full local flow

```bash
cd docs/examples/fixtures/solana_x_plus_7
./run_solana_flow.sh
```

This command does:

1. `nargo compile`
2. `nargo execute witness_ok`
3. `bb prove --scheme ultra_honk --oracle_hash keccak --disable_zk --write_vk`
4. `bb write_solana_verifier --scheme ultra_honk --oracle_hash keccak --disable_zk`
5. `bb solana_staged_verify --scheme ultra_honk --oracle_hash keccak --disable_zk`
6. `cargo build-sbf --features sbf-entrypoint` for generated `solana_verifier_program`

## Deploy to Solana devnet

After the script succeeds:

```bash
solana config set --url https://api.devnet.solana.com
solana-keygen new --outfile ~/.config/solana/devnet-zk-keypair.json
solana config set --keypair ~/.config/solana/devnet-zk-keypair.json
solana airdrop 2
solana program deploy ./target/solana/sbf_out/ultrahonk_solana_generated_verifier.so
```

## If you really want `x = 5`

`x + 7 = 13` cannot be satisfied by `5`. Use either:

- `assert(x + 8 == 13);` or
- `assert(x + 7 == 12);`
