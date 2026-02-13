Generated fixture bundle for Solana UltraHonk staged-session tests.

Generated on: 2026-02-10

Toolchain:
- `nargo version = 1.0.0-beta.18`
- `bb = /Users/Americansprit/blockchain/barretenberg/cpp/build/bin/bb`

Source Noir project:
- `/Users/Americansprit/blockchain/barretenberg/acir_tests/internal_test_programs/ecdsa_regression`

Generation commands:
```bash
cd /Users/Americansprit/blockchain/barretenberg/acir_tests/internal_test_programs/ecdsa_regression
~/.nargo/bin/nargo compile
~/.nargo/bin/nargo execute

/Users/Americansprit/blockchain/barretenberg/cpp/build/bin/bb prove \
  --scheme ultra_honk \
  --oracle_hash keccak \
  --disable_zk \
  --write_vk \
  -b /Users/Americansprit/blockchain/barretenberg/acir_tests/internal_test_programs/ecdsa_regression/target/ecdsa_regression.json \
  -w /Users/Americansprit/blockchain/barretenberg/acir_tests/internal_test_programs/ecdsa_regression/target/ecdsa_regression.gz \
  -o /tmp/ecdsa_regression_proof_beta18
```

Copied artifacts:
- `proof`
- `public_inputs`
- `vk`
