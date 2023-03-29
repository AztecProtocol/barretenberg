#include "logic_constraint.hpp"

namespace acir_format {

// TODO(maxim): This doesn't work correctly
void create_logic_gate(Composer& composer,
                       const uint32_t a,
                       const uint32_t b,
                       const uint32_t result,
                       const size_t num_bits,
                       const bool is_xor_gate)
{
    (void)composer;
    (void)a;
    (void)b;
    (void)result;
    (void)num_bits;
    (void)is_xor_gate;
    // const size_t num_chunks = (num_bits / 32) + ((num_bits % 32 == 0) ? 0 : 1);
    // uint256_t left = composer.get_variable(a);
    // uint256_t right = composer.get_variable(b);

    // field_ct res(&composer);
    // for (size_t i = 0; i < num_chunks; ++i) {
    //     uint256_t left_chunk = left & ((uint256_t(1) << 32) - 1);
    //     uint256_t right_chunk = right & ((uint256_t(1) << 32) - 1);

    //     const field_ct a(&composer, left_chunk);
    //     const field_ct b(&composer, right_chunk);

    //     field_ct result_chunk = 0;
    //     if (is_xor_gate) {
    //         result_chunk = plookup_read_ct::read_from_2_to_1_table(plookup::MultiTableId::UINT32_AND, a, b);
    //     } else {
    //         result_chunk = plookup_read_ct::read_from_2_to_1_table(plookup::MultiTableId::UINT32_AND, a, b);
    //     }

    //     uint256_t scaling_factor = uint256_t(1) << (32 * i);
    //     res += result_chunk * scaling_factor;

    //     if (i == num_chunks - 1) {
    //         const size_t final_num_bits = num_bits - (i * 32);
    //         if (final_num_bits != 32) {
    //             composer.create_range_constraint(a.witness_index, final_num_bits, "bad range on a");
    //             composer.create_range_constraint(b.witness_index, final_num_bits, "bad range on b");
    //         }
    //     }

    //     left = left >> 32;
    //     right = right >> 32;
    // }

    // auto our_res = field_ct::from_witness_index(&composer, result);
    // (our_res + res);
}

} // namespace acir_format
