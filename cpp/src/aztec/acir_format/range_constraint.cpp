#include "range_constraint.hpp"

namespace acir_format {

template <typename B> inline void read(B& buf, RangeConstraint& constraint)
{
    using serialize::read;
    read(buf, constraint.witness);
    read(buf, constraint.num_bits);
}

template <typename B> inline void write(B& buf, RangeConstraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.witness);
    write(buf, constraint.num_bits);
}

} // namespace acir_format
