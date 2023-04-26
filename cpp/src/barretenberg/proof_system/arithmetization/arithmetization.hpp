#pragma once
#include <array>
#include <cstddef>
#include <vector>

namespace arithmetization {

/**
 * @brief Specify the structure of a CircuitConstructor
 *
 * @details This is typically passed as a template argument specifying the structure of a circuit constructor. It
 * should only ever contain circuit constructor data--it should not contain data that is particular to any
 * proving system.
 *
 * @remark It may make sense to say this is only partial arithmetization data, with the full data being
 * contained in the circuit constructor. We could change the name of this class if it conflicts with common usage.
 *
 * @tparam _num_wires
 * @tparam _num_selectors
 */
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
template <typename FF> class Standard : public Arithmetization</*num_wires =*/3, /*num_selectors =*/5> {
  public:
    struct Selectors {
        using DataType = std::array<std::vector<FF>, num_selectors>;

        DataType _data;
        std::vector<FF>& q_m = std::get<0>(_data);
        std::vector<FF>& q_1 = std::get<1>(_data);
        std::vector<FF>& q_2 = std::get<2>(_data);
        std::vector<FF>& q_3 = std::get<3>(_data);
        std::vector<FF>& q_c = std::get<4>(_data);
        size_t size() { return _data.size(); };
        typename DataType::const_iterator begin() const { return _data.begin(); };
        typename DataType::iterator begin() { return _data.begin(); };
        typename DataType::const_iterator end() const { return _data.end(); };
        typename DataType::iterator end() { return _data.end(); };
    };
};

template <typename FF> class Turbo : public Arithmetization</*num_wires =*/4, /*num_selectors =*/11> {
  public:
    struct Selectors {
        using DataType = std::array<std::vector<FF>, num_selectors>;

        DataType _data;
        std::vector<FF>& q_m = std::get<0>(_data);
        std::vector<FF>& q_c = std::get<1>(_data);
        std::vector<FF>& q_1 = std::get<2>(_data);
        std::vector<FF>& q_2 = std::get<3>(_data);
        std::vector<FF>& q_3 = std::get<4>(_data);
        std::vector<FF>& q_4 = std::get<5>(_data);
        std::vector<FF>& q_5 = std::get<6>(_data);
        std::vector<FF>& q_arith = std::get<7>(_data);
        std::vector<FF>& q_fixed_base = std::get<8>(_data);
        std::vector<FF>& q_range = std::get<9>(_data);
        std::vector<FF>& q_logic = std::get<10>(_data);

        size_t size() { return _data.size(); };
        typename DataType::const_iterator begin() const { return _data.begin(); };
        typename DataType::iterator begin() { return _data.begin(); };
        typename DataType::const_iterator end() const { return _data.end(); };
        typename DataType::iterator end() { return _data.end(); };
    };
};

template <typename FF> class Ultra : public Arithmetization</*num_wires =*/4, /*num_selectors =*/11> {
  public:
    struct Selectors {
        using DataType = std::array<std::vector<FF>, num_selectors>;

        DataType _data;
        std::vector<FF>& q_m = std::get<0>(_data);
        std::vector<FF>& q_c = std::get<1>(_data);
        std::vector<FF>& q_1 = std::get<2>(_data);
        std::vector<FF>& q_2 = std::get<3>(_data);
        std::vector<FF>& q_3 = std::get<4>(_data);
        std::vector<FF>& q_4 = std::get<5>(_data);
        std::vector<FF>& q_arith = std::get<6>(_data);
        std::vector<FF>& q_sort = std::get<7>(_data);
        std::vector<FF>& q_elliptic = std::get<8>(_data);
        std::vector<FF>& q_aux = std::get<9>(_data);
        std::vector<FF>& q_lookup_type = std::get<10>(_data);

        size_t size() { return _data.size(); };
        typename DataType::const_iterator begin() const { return _data.begin(); };
        typename DataType::iterator begin() { return _data.begin(); };
        typename DataType::const_iterator end() const { return _data.end(); };
        typename DataType::iterator end() { return _data.end(); };
    };
};

} // namespace arithmetization