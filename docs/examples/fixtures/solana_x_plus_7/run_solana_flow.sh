#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

REPO_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"
SOLANA_BIN_DEFAULT="$HOME/.local/share/solana/install/active_release/bin"
SOLANA_BIN_DIR="${SOLANA_BIN_DIR:-$SOLANA_BIN_DEFAULT}"
export PATH="$SOLANA_BIN_DIR:$PATH"
BB_DEFAULT="$REPO_ROOT/cpp/build/bin/bb"
BB_BIN="${BB_BIN:-$BB_DEFAULT}"

command -v nargo >/dev/null || { echo "nargo is required"; exit 1; }
command -v cargo >/dev/null || { echo "cargo is required"; exit 1; }
command -v cargo-build-sbf >/dev/null || { echo "cargo-build-sbf is required in PATH"; exit 1; }
[[ -x "$BB_BIN" ]] || { echo "bb binary not found at $BB_BIN (set BB_BIN)"; exit 1; }

echo "==> Compiling Noir circuit"
nargo compile

echo "==> Executing witness (Prover.toml)"
nargo execute witness_ok

echo "==> Generating UltraHonk proof/vk/public_inputs"
"$BB_BIN" prove \
  --scheme ultra_honk \
  --oracle_hash keccak \
  --disable_zk \
  --write_vk \
  -b ./target/solana_x_plus_7.json \
  -w ./target/witness_ok.gz \
  -o ./target

echo "==> Emitting Solana verifier artifacts"
"$BB_BIN" write_solana_verifier \
  --scheme ultra_honk \
  --oracle_hash keccak \
  --disable_zk \
  --chunk_size 900 \
  -k ./target/vk \
  -o ./target/solana

echo "==> Local staged verification sanity check"
"$BB_BIN" solana_staged_verify \
  --scheme ultra_honk \
  --oracle_hash keccak \
  --disable_zk \
  --chunk_size 900 \
  -k ./target/vk \
  -p ./target/proof \
  -i ./target/public_inputs

echo "==> Building Solana verifier SBF program"
cargo build-sbf \
  --offline \
  --tools-version v1.52 \
  --features sbf-entrypoint \
  --sbf-out-dir ./target/solana/sbf_out \
  --manifest-path ./target/solana/solana_verifier_program/Cargo.toml

PROGRAM_SO_PATH="$(find "$SCRIPT_DIR/target/solana/sbf_out" -maxdepth 1 -name '*.so' -type f | head -n 1)"
if [[ -z "${PROGRAM_SO_PATH:-}" ]]; then
  echo "No .so produced under $SCRIPT_DIR/target/solana/sbf_out"
  exit 1
fi

echo
echo "Artifacts ready:"
echo "  - Proof:         $SCRIPT_DIR/target/proof"
echo "  - Public inputs: $SCRIPT_DIR/target/public_inputs"
echo "  - VK:            $SCRIPT_DIR/target/vk"
echo "  - Solana output: $SCRIPT_DIR/target/solana"
echo "  - Program .so:   $PROGRAM_SO_PATH"
echo
echo "To deploy on devnet:"
echo "  solana config set --url https://api.devnet.solana.com"
echo "  solana-keygen new --outfile ~/.config/solana/devnet-zk-keypair.json"
echo "  solana config set --keypair ~/.config/solana/devnet-zk-keypair.json"
echo "  solana airdrop 2"
echo "  solana program deploy $PROGRAM_SO_PATH"
