#pragma once

#include <array>
#include <vector>

#include "../../primitives/composers/composers_fwd.hpp"

#include "../../primitives/field/field.hpp"
#include "../../primitives/witness/witness.hpp"

namespace plonk {
namespace stdlib {

namespace aes128 {

std::vector<field_t<waffle::PlookupComposer>> encrypt_buffer_cbc(
    const std::vector<field_t<waffle::PlookupComposer>>& input,
    const field_t<waffle::PlookupComposer>& iv,
    const field_t<waffle::PlookupComposer>& key);

} // namespace aes128
} // namespace stdlib
} // namespace plonk
