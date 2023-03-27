#include "logic_constraint.hpp"

namespace acir_format {

void create_logic_gate(Composer& composer,
                       const uint32_t a,
                       const uint32_t b,
                       const uint32_t result,
                       const size_t num_bits,
                       const bool is_xor_gate)
{
    // auto accumulators = composer.create_logic_constraint(a, b, num_bits, is_xor_gate);
    // composer.assert_equal(accumulators.out.back(), result);
    if (is_xor_gate) {
        xor_gate(composer, a, b, result);
    } else {
        and_gate(composer, a, b, result);
    }

    const size_t num_chunks = ((num_bits / 4) + (num_bits % 4 == 0)) ? 0 : 1;

    uint256_t left = composer.get_variable(a);
    uint256_t right = composer.get_variable(b);

    field_ct res(&composer);
    for (size_t i = 0; i < num_chunks; ++i)
    {

        uint256_t left_chunk = left & ((uint256_t(1) << 32) - 1);
        uint256_t right_chunk = right & ((uint256_t(1) << 32) - 1);

        const field_ct a(&composer, left_chunk);
        const field_ct b(&composer, right_chunk);

        field_ct result_chunk = 0;
        if (is_xor_gate) {
            result_chunk = plookup_read_ct::read_from_2_to_1_table(plookup::MultiTableId::UINT32_XOR, a, b);
        } else {
            result_chunk = plookup_read_ct::read_from_2_to_1_table(plookup::MultiTableId::UINT32_AND, a, b);
        }


        uint256_t scaling_factor = uint256_t(1) << (32 * i);
        res += result_chunk * scaling_factor;

        if (i == num_chunks - 1)
        {
            const size_t final_num_bits = num_bits - (i * 32);
            if (final_num_bits != 32)
            {
                composer.create_range_constraint(a.witness_index, final_num_bits, "");
                composer.create_range_constraint(b.witness_index, final_num_bits, "");
            }
        }

        left = left >> 32;
        right = right >> 32;
    }

    res.assert_equal(field_ct::from_witness_index(&composer, result));
}

} // namespace acir_format
