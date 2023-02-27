#include <common/serialize.hpp>
#include <common/assert.hpp>
#include <plonk/composer/composer_base.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include <stdlib/hash/pedersen/pedersen.hpp>
#include <stdlib/types/types.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>
#include <stdlib/primitives/byte_array/byte_array.hpp>

using namespace plonk::stdlib::types;
using namespace barretenberg;
using namespace plonk;

namespace acir_format {

typedef plonk::stdlib::field_t<plonk::TurboComposer> field_t;
typedef plonk::stdlib::byte_array<plonk::TurboComposer> byte_array;

struct FixedBaseScalarMul {
    uint32_t scalar;
    uint32_t pub_key_x;
    uint32_t pub_key_y;

    friend bool operator==(FixedBaseScalarMul const& lhs, FixedBaseScalarMul const& rhs) = default;
};

void create_fixed_base_constraint(plonk::TurboComposer& composer, const FixedBaseScalarMul& input);

template <typename B> inline void read(B& buf, FixedBaseScalarMul& constraint);

template <typename B> inline void write(B& buf, FixedBaseScalarMul const& constraint);

} // namespace acir_format
