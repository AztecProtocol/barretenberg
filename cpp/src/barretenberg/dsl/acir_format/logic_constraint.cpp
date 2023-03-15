#include "logic_constraint.hpp"

namespace acir_format {

void create_logic_gate(Composer& composer,
                       const uint32_t a,
                       const uint32_t b,
                       const uint32_t result,
                       const size_t num_bits,
                       const bool is_xor_gate)
{
    (void)num_bits;
    // auto accumulators = composer.create_logic_constraint(a, b, num_bits, is_xor_gate);
    // composer.assert_equal(accumulators.out.back(), result);
    if (is_xor_gate) {
        xor_gate(composer, a, b, result);
    } else {
        and_gate(composer, a, b, result);
    }
}
void xor_gate(Composer& composer, const uint32_t a, const uint32_t b, const uint32_t result)
{
    // create_logic_gate(composer, a, b, result, num_bits, true);
    auto plookup_res = a ^ b;
    composer.assert_equal(plookup_res, result);
}
void and_gate(Composer& composer, const uint32_t a, const uint32_t b, const uint32_t result)
{
    // create_logic_gate(composer, a, b, result, num_bits, false);
    auto plookup_res = a & b;
    composer.assert_equal(plookup_res, result);
}

} // namespace acir_format
