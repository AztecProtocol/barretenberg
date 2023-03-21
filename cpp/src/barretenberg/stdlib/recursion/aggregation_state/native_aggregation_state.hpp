#pragma once
#include "barretenberg/ecc/groups/affine_element.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"

namespace plonk {
namespace stdlib {
namespace recursion {

struct native_aggregation_state {
    typename barretenberg::g1::affine_element P0 = barretenberg::g1::affine_one;
    typename barretenberg::g1::affine_element P1 = barretenberg::g1::affine_one;
    bool has_data = false;
};

inline void read(uint8_t const*& it, native_aggregation_state& state)
{
    using serialize::read;

    read(it, state.P0);
    read(it, state.P1);
    read(it, state.has_data);
};

inline std::ostream& operator<<(std::ostream& os, native_aggregation_state const& obj)
{
    os << "P0: " << obj.P0 << "\n"
       << "P1: " << obj.P1 << "\n"
       << "has_data: " << obj.has_data << "\n";
    return os;
};

} // namespace recursion
} // namespace stdlib
} // namespace plonk