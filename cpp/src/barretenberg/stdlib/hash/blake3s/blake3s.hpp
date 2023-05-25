#pragma once
#include "barretenberg/stdlib/primitives/byte_array/byte_array.hpp"

namespace proof_system::plonk {
class TurboPlonkComposer;
class UltraPlonkComposer;
} // namespace proof_system::plonk

namespace proof_system::plonk {
namespace stdlib {

template <typename Composer> byte_array<Composer> blake3s(const byte_array<Composer>& input);

extern template byte_array<plonk::StandardPlonkComposer> blake3s(const byte_array<plonk::StandardPlonkComposer>& input);
extern template byte_array<plonk::TurboPlonkComposer> blake3s(const byte_array<plonk::TurboPlonkComposer>& input);
extern template byte_array<plonk::UltraPlonkComposer> blake3s(const byte_array<plonk::UltraPlonkComposer>& input);

extern template byte_array<proof_system::StandardCircuitConstructor> blake3s(
    const byte_array<proof_system::StandardCircuitConstructor>& input);
extern template byte_array<proof_system::TurboCircuitConstructor> blake3s(
    const byte_array<proof_system::TurboCircuitConstructor>& input);
extern template byte_array<proof_system::UltraCircuitConstructor> blake3s(
    const byte_array<proof_system::UltraCircuitConstructor>& input);

} // namespace stdlib
} // namespace proof_system::plonk
