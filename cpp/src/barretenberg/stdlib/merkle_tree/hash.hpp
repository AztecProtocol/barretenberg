#pragma once
#include "barretenberg/common/net.hpp"
#include "barretenberg/crypto/blake2s/blake2s.hpp"
#include "barretenberg/crypto/pedersen_commitment/pedersen.hpp"
#include "barretenberg/crypto/pedersen_hash/pedersen_lookup.hpp"
#include "barretenberg/crypto/pedersen_commitment/convert_buffer_to_field.hpp"
#include "barretenberg/stdlib/hash/blake2s/blake2s.hpp"
#include "barretenberg/stdlib/hash/pedersen/pedersen.hpp"
#include "barretenberg/stdlib/primitives/field/field.hpp"
#include <vector>

namespace proof_system::plonk {
namespace stdlib {
namespace merkle_tree {

inline barretenberg::fr compress_native(barretenberg::fr const& lhs, barretenberg::fr const& rhs)
{
    if (plonk::SYSTEM_COMPOSER == plonk::PLOOKUP) {
        return crypto::pedersen_hash::lookup::hash_multiple({ lhs, rhs });
    } else {
        return crypto::pedersen_hash::hash_multiple({ lhs, rhs });
    }
}

} // namespace merkle_tree
} // namespace stdlib
} // namespace proof_system::plonk
