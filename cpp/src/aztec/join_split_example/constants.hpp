#pragma once
#include <stddef.h>
#include <stdint.h>
#include <numeric/uint256/uint256.hpp>
#include <ecc/curves/grumpkin/grumpkin.hpp>

namespace join_split_example {

constexpr size_t DATA_TREE_DEPTH = 32;

constexpr size_t MAX_NO_WRAP_INTEGER_BIT_LENGTH = grumpkin::MAX_NO_WRAP_INTEGER_BIT_LENGTH;
constexpr size_t MAX_TXS_BIT_LENGTH = 10;
constexpr size_t TX_FEE_BIT_LENGTH = MAX_NO_WRAP_INTEGER_BIT_LENGTH - MAX_TXS_BIT_LENGTH;

constexpr size_t NUM_ASSETS_BIT_LENGTH = 4;
constexpr size_t NUM_ASSETS = 1 << NUM_ASSETS_BIT_LENGTH;
constexpr size_t ASSET_ID_BIT_LENGTH = 30;
constexpr size_t MAX_NUM_ASSETS_BIT_LENGTH = 30;
constexpr size_t MAX_NUM_ASSETS = 1 << MAX_NUM_ASSETS_BIT_LENGTH;
constexpr size_t ALIAS_HASH_BIT_LENGTH = 224;

namespace circuit_gate_count {

/*
The boolean is_circuit_change_expected should be set to 0 by default. When there is an expected circuit change, the
developer can quickly check whether the circuit gate counts are in allowed range i.e., below the next power of two
limit, by setting it to one. However, while merging the corresponding PR, the developer should set
is_circuit_change_expected to zero and change the modified circuit gate counts accordingly.
*/
constexpr bool is_circuit_change_expected = 0;
/* The below constants are only used for regression testing; to identify accidental changes to circuit
 constraints. They need to be changed when there is a circuit change. */
constexpr uint32_t JOIN_SPLIT = 64047;
}; // namespace circuit_gate_count

namespace circuit_gate_next_power_of_two {
/* The below constants are used in tests to detect undesirable circuit changes. They should not be changed unless we
want to exceed the next power of two limit. */
constexpr uint32_t JOIN_SPLIT = 65536;
}; // namespace circuit_gate_next_power_of_two

namespace circuit_vk_hash {
/* These below constants are only used for regression testing; to identify accidental changes to circuit
 constraints. They need to be changed when there is a circuit change. Note that they are written in the reverse order
 to comply with the from_buffer<>() method. */
constexpr auto JOIN_SPLIT = uint256_t(0xb23c7772f47bc823, 0x5493625d4f08603c, 0x21ac50a5929576f9, 0xb7b3113c131460e5);
}; // namespace circuit_vk_hash

namespace ProofIds {
enum { PADDING = 0, DEPOSIT = 1, WITHDRAW = 2, SEND = 3, ACCOUNT = 4, DEFI_DEPOSIT = 5, DEFI_CLAIM = 6 };
};

} // namespace join_split_example