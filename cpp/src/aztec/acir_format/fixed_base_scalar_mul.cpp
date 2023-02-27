#include "fixed_base_scalar_mul.hpp"

using namespace plonk::stdlib::types;
using namespace barretenberg;
using namespace plonk;

namespace acir_format {

void create_fixed_base_constraint(plonk::TurboComposer& composer, const FixedBaseScalarMul& input)
{

    field_t scalar_as_field = field_t::from_witness_index(&composer, input.scalar);
    auto public_key = group_ct::fixed_base_scalar_mul_g1<254>(scalar_as_field);

    composer.copy_from_to(public_key.x.witness_index, input.pub_key_x);
    composer.copy_from_to(public_key.y.witness_index, input.pub_key_y);
}

template <typename B> inline void read(B& buf, FixedBaseScalarMul& constraint)
{
    using serialize::read;
    read(buf, constraint.scalar);
    read(buf, constraint.pub_key_x);
    read(buf, constraint.pub_key_y);
}

template <typename B> inline void write(B& buf, FixedBaseScalarMul const& constraint)
{
    using serialize::write;
    write(buf, constraint.scalar);
    write(buf, constraint.pub_key_x);
    write(buf, constraint.pub_key_y);
}

} // namespace acir_format
