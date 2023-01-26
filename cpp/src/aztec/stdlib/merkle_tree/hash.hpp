#pragma once
#include <common/net.hpp>
#include <crypto/blake2s/blake2s.hpp>
#include <crypto/pedersen_commitment/pedersen.hpp>
#include <crypto/pedersen_hash/pedersen_lookup.hpp>
#include <crypto/pedersen_commitment/convert_buffer_to_field.hpp>
#include <stdlib/hash/blake2s/blake2s.hpp>
#include <stdlib/hash/pedersen/pedersen.hpp>
#include <stdlib/primitives/field/field.hpp>
#include <vector>

namespace plonk {
namespace stdlib {
namespace merkle_tree {

inline barretenberg::fr compress_native(barretenberg::fr const& lhs, barretenberg::fr const& rhs)
{
    return crypto::pedersen_hash::lookup::hash_multiple({ lhs, rhs });
}

} // namespace merkle_tree
} // namespace stdlib
} // namespace plonk