#include <common/serialize.hpp>
#include <common/assert.hpp>
#include <plonk/composer/composer_base.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include <stdlib/hash/pedersen/pedersen.hpp>
#include <stdlib/types/types.hpp>

using namespace plonk::stdlib::types;
using namespace barretenberg;
using namespace plonk;

namespace acir_format {

// P = xG + bH
struct PedersenConstraint {
    std::vector<uint32_t> scalars;
    uint32_t result_x;
    uint32_t result_y;

    friend bool operator==(PedersenConstraint const& lhs, PedersenConstraint const& rhs) = default;
};

void create_pedersen_constraint(plonk::TurboComposer& composer, const PedersenConstraint& input);

template <typename B> inline void read(B& buf, PedersenConstraint& constraint);

template <typename B> inline void write(B& buf, PedersenConstraint const& constraint);

} // namespace acir_format
