#pragma once
#include <common/net.hpp>
#include <crypto/pedersen/pedersen.hpp>
#include <vector>

namespace crypto {
namespace merkle_tree {

using namespace barretenberg;

typedef std::vector<std::pair<fr, fr>> fr_hash_path;

inline barretenberg::fr hash_value_native(std::vector<uint8_t> const& input)
{
    return crypto::pedersen::compress_native(input);
}

inline barretenberg::fr compress_native(barretenberg::fr const& lhs, barretenberg::fr const& rhs)
{
    return crypto::pedersen::compress_native({ lhs, rhs });
}

} // namespace merkle_tree
} // namespace crypto