// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#pragma once
#include "../circuit_builders/circuit_builders_fwd.hpp"
#include "../field/field.hpp"
#include "barretenberg/transcript/origin_tag.hpp"

namespace bb::stdlib {

// A runtime-defined read-only memory table. Table entries must be initialized in the constructor.
// Each entry contains a pair of values
// N.B. Only works with the UltraBuilder at the moment!
template <typename Builder> class twin_rom_table {
  private:
    typedef field_t<Builder> field_pt;
    typedef std::array<field_pt, 2> field_pair_pt;

  public:
    twin_rom_table(){};
    twin_rom_table(const std::vector<field_pair_pt>& table_entries);
    twin_rom_table(const twin_rom_table& other);
    twin_rom_table(twin_rom_table&& other);

    void initialize_table() const;

    twin_rom_table& operator=(const twin_rom_table& other);
    twin_rom_table& operator=(twin_rom_table&& other);

    // read from table with a constant index value. Does not add any gates
    field_pair_pt operator[](const size_t index) const;

    // read from table with a witness index value. Adds 2 gates
    field_pair_pt operator[](const field_pt& index) const;

    size_t size() const { return length; }

    Builder* get_context() const { return context; }

  private:
    std::vector<field_pair_pt> raw_entries;
    mutable std::vector<field_pair_pt> entries;

    // Origin Tags used for tracking dangerous interactions in stdlib primtives
    mutable std::vector<std::array<OriginTag, 2>> tags;
    size_t length = 0;
    mutable size_t rom_id = 0; // Builder identifier for this ROM table
    mutable bool initialized = false;
    mutable Builder* context = nullptr;
};
} // namespace bb::stdlib
