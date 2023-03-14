#include "logic_constraint.hpp"

namespace ultra_acir_format {

void create_logic_gate(UltraComposer& composer,
                       const uint32_t a,
                       const uint32_t b,
                       const uint32_t result,
                       const size_t num_bits,
                       const bool is_xor_gate)
{
    auto accumulators = composer.create_logic_constraint(a, b, num_bits, is_xor_gate);
    composer.assert_equal(accumulators.out.back(), result);
}
void xor_gate(UltraComposer& composer, const uint32_t a, const uint32_t b, const uint32_t result, const size_t num_bits)
{
    create_logic_gate(composer, a, b, result, num_bits, true);
}
void and_gate(UltraComposer& composer, const uint32_t a, const uint32_t b, const uint32_t result, const size_t num_bits)
{
    create_logic_gate(composer, a, b, result, num_bits, false);
}

} // namespace ultra_acir_format
