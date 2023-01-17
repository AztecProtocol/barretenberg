#pragma once
#include <common/net.hpp>
#include <crypto/blake2s/blake2s.hpp>
#include <crypto/pedersen_commitment/pedersen.hpp>
#include <crypto/pedersen_commitment/convert_buffer_to_field.hpp>
#include <stdlib/hash/blake2s/blake2s.hpp>
#include <stdlib/hash/pedersen/pedersen.hpp>
#include <stdlib/primitives/field/field.hpp>
#include <vector>

namespace plonk {
namespace stdlib {
namespace merkle_tree {

// template <typename ComposerContext> inline field_t<ComposerContext> hash_value(byte_array<ComposerContext> const&
// input)
// {
//     return plonk::stdlib::pedersen_hash<ComposerContext>::hash_multiple(input);
// }

// inline barretenberg::fr hash_value_native(std::vector<uint8_t> const& input)
// {
//     const auto elements = crypto::pedersen_commitment::convert_buffer_to_field(input);
//     return crypto::pedersen_hash::hash_multiple(elements);
// }

inline barretenberg::fr compress_native(barretenberg::fr const& lhs, barretenberg::fr const& rhs)
{
    return crypto::pedersen_hash::hash_multiple({ lhs, rhs });
}

} // namespace merkle_tree
} // namespace stdlib
} // namespace plonk