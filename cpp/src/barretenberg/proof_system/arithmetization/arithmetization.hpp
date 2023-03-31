#pragma once
#include <cstddef>

namespace arithmetization {

template <size_t _num_wires, size_t _num_selectors> struct Arithmetization {
    static constexpr size_t num_wires = _num_wires;
    static constexpr size_t num_selectors = _num_selectors;
    // Note: For even greater modularity, in each instantiation we could specify a list of components here, where a
    // component is a meaningful collection of functions for creating gates, as in:
    //
    // struct Component {
    //     using Arithmetic = component::Arithmetic3Wires;
    //     using RangeConstraints = component::Base4Accumulators or component::GenPerm or...
    //     using LooupTables = component::Plookup4Wire or component::CQ8Wire or...
    //     ...
    // };
    //
    // We should only do this if it becomes necessary or convenient.
};

// These are not magic numbers and they should not be written with global constants. These paraters are not accessible
// through clearly named static class members.
using Standard = Arithmetization<3, 5>;
using Turbo = Arithmetization<4, 11>;

} // namespace arithmetization