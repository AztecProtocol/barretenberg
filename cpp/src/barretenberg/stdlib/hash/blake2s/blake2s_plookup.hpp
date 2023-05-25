#pragma once
#include <array>
#include "barretenberg/proof_system/plookup_tables/plookup_tables.hpp"
#include "barretenberg/stdlib/primitives/uint/uint.hpp"

#include "barretenberg/numeric/bitop/sparse_form.hpp"

#include "../../primitives/field/field.hpp"
#include "../../primitives/composers/composers_fwd.hpp"
#include "../../primitives/packed_byte_array/packed_byte_array.hpp"

namespace proof_system::plonk {
namespace stdlib {

namespace blake2s_plookup {

template <typename Composer> byte_array<Composer> blake2s(const byte_array<Composer>& input);

extern template byte_array<UltraPlonkComposer> blake2s(const byte_array<plonk::UltraPlonkComposer>& input);
extern template byte_array<proof_system::UltraCircuitConstructor> blake2s(
    const byte_array<proof_system::UltraCircuitConstructor>& input);

} // namespace blake2s_plookup

} // namespace stdlib
} // namespace proof_system::plonk
