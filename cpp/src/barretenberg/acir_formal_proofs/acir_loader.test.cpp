#include "barretenberg/boomerang_value_detection/graph.hpp"
#include "barretenberg/crypto/aes128/aes128.hpp"
#include "barretenberg/crypto/generators/generator_data.hpp"
#include "barretenberg/crypto/pedersen_commitment/pedersen.hpp"
#include "barretenberg/stdlib/encryption/aes128/aes128.hpp"
#include "barretenberg/stdlib_circuit_builders/mock_circuits.hpp"
#include "barretenberg/stdlib_circuit_builders/plookup_tables/fixed_base/fixed_base.hpp"
#include "barretenberg/stdlib_circuit_builders/ultra_circuit_builder.hpp"
#include "barretenberg/common/test.hpp"

TEST(acir_formal_proofs, loader) {
    info("amogus");
    EXPECT_EQ(1 + 1, 3);
} 