#pragma once
#include "../composers/composers_fwd.hpp"
#include "../field/field.hpp"

namespace plonk {
namespace stdlib {

// A runtime-defined read-only memory table. Table entries must be initialized in the constructor.
// N.B. Only works with the UltraComposer at the moment!
template <typename Composer> class logic {
  private:
    typedef field_t<Composer> field_pt;
    typedef witness_t<Composer> witness_pt;

  public:
    static field_pt create_logic_constraint(field_pt& a, field_pt& b, size_t num_bits, bool is_xor_gate);
};

EXTERN_STDLIB_TYPE(logic);

} // namespace stdlib
} // namespace plonk