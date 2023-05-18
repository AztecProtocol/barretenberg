#pragma once
#include <array>
#include "barretenberg/stdlib/primitives/uint/uint.hpp"
#include "barretenberg/stdlib/primitives/packed_byte_array/packed_byte_array.hpp"
#include "barretenberg/stdlib/primitives/byte_array/byte_array.hpp"

#include "sha256_plookup.hpp"

namespace proof_system::plonk {
class UltraComposer;
class StandardPlonkComposer;
class TurboPlonkComposer;
} // namespace proof_system::plonk

namespace proof_system::plonk {
namespace stdlib {
template <typename Composer> class bit_array;

template <typename Composer>
std::array<uint32<Composer>, 8> sha256_block(const std::array<uint32<Composer>, 8>& h_init,
                                             const std::array<uint32<Composer>, 16>& input);

template <typename Composer> byte_array<Composer> sha256_block(const byte_array<Composer>& input);
template <typename Composer> packed_byte_array<Composer> sha256(const packed_byte_array<Composer>& input);

template <typename Composer> field_t<Composer> sha256_to_field(const packed_byte_array<Composer>& input)
{
    std::vector<field_t<Composer>> slices = stdlib::sha256<Composer>(input).to_unverified_byte_slices(16);
    return slices[1] + (slices[0] * (uint256_t(1) << 128));
}

extern template byte_array<plonk::TurboPlonkComposer> sha256_block(const byte_array<plonk::TurboPlonkComposer>& input);

extern template packed_byte_array<plonk::TurboPlonkComposer> sha256(
    const packed_byte_array<plonk::TurboPlonkComposer>& input);

extern template byte_array<plonk::StandardPlonkComposer> sha256_block(
    const byte_array<plonk::StandardPlonkComposer>& input);

extern template packed_byte_array<plonk::StandardPlonkComposer> sha256(
    const packed_byte_array<plonk::StandardPlonkComposer>& input);

extern template packed_byte_array<plonk::UltraComposer> sha256(const packed_byte_array<plonk::UltraComposer>& input);

} // namespace stdlib
} // namespace proof_system::plonk
