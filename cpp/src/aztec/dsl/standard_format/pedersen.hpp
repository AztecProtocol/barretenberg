#include <common/serialize.hpp>
#include <common/assert.hpp>
#include <plonk/composer/composer_base.hpp>
#include <plonk/composer/ultra_composer.hpp>
#include <stdlib/hash/pedersen/pedersen.hpp>
#include <stdlib/types/types.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>
#include <stdlib/primitives/byte_array/byte_array.hpp>

using namespace plonk::stdlib::types;

using namespace barretenberg;
using namespace plonk;

// P = xG + bH
struct PedersenConstraint {
    std::vector<uint32_t> scalars;
    uint32_t result_x;
    uint32_t result_y;
};

void create_pedersen_constraint(waffle::UltraComposer& composer, const PedersenConstraint& input)
{
    std::vector<field_ct> scalars;

    for (const auto& scalar : input.scalars) {
        // convert input indices to field_ct
        field_ct scalar_as_field = field_ct::from_witness_index(&composer, scalar);
        scalars.push_back(scalar_as_field);
    }
    auto point = pedersen::commit(scalars);

    composer.assert_equal(point.x.witness_index, input.result_x);
    composer.assert_equal(point.y.witness_index, input.result_y);
}

template <typename B> inline void read(B& buf, PedersenConstraint& constraint)
{
    using serialize::read;
    read(buf, constraint.scalars);
    read(buf, constraint.result_x);
    read(buf, constraint.result_y);
}

template <typename B> inline void write(B& buf, PedersenConstraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.scalars);
    read(buf, constraint.result_x);
    read(buf, constraint.result_y);
}

inline bool operator==(PedersenConstraint const& lhs, PedersenConstraint const& rhs)
{
    // clang-format off
    return
        lhs.scalars == rhs.scalars &&
        lhs.result_x == rhs.result_x &&
        lhs.result_y == rhs.result_y;
    // clang-format on
}
