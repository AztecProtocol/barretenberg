#pragma once

#include <array>
#include <vector>

#include "../../primitives/composers/composers_fwd.hpp"

#include "../../primitives/field/field.hpp"
#include "../../primitives/witness/witness.hpp"

namespace proof_system::plonk {
namespace stdlib {

namespace aes128 {

std::vector<field_t<plonk::UltraPlonkComposer>> encrypt_buffer_cbc(
    const std::vector<field_t<plonk::UltraPlonkComposer>>& input,
    const field_t<plonk::UltraPlonkComposer>& iv,
    const field_t<plonk::UltraPlonkComposer>& key);

} // namespace aes128
} // namespace stdlib
} // namespace proof_system::plonk
