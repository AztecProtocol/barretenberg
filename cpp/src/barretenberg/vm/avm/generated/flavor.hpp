// AUTOGENERATED FILE
#pragma once

#include "barretenberg/commitment_schemes/kzg/kzg.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/flavor/relation_definitions.hpp"
#include "barretenberg/polynomials/barycentric.hpp"
#include "barretenberg/polynomials/univariate.hpp"

#include "barretenberg/flavor/flavor.hpp"
#include "barretenberg/flavor/flavor_macros.hpp"
#include "barretenberg/polynomials/evaluation_domain.hpp"
#include "barretenberg/transcript/transcript.hpp"

#include "barretenberg/vm/avm/generated/flavor_settings.hpp"
#include "barretenberg/vm/aztec_constants.hpp"

// Relations
#include "barretenberg/vm/avm/generated/relations/alu.hpp"
#include "barretenberg/vm/avm/generated/relations/binary.hpp"
#include "barretenberg/vm/avm/generated/relations/cmp.hpp"
#include "barretenberg/vm/avm/generated/relations/conversion.hpp"
#include "barretenberg/vm/avm/generated/relations/gas.hpp"
#include "barretenberg/vm/avm/generated/relations/keccakf1600.hpp"
#include "barretenberg/vm/avm/generated/relations/kernel.hpp"
#include "barretenberg/vm/avm/generated/relations/main.hpp"
#include "barretenberg/vm/avm/generated/relations/mem.hpp"
#include "barretenberg/vm/avm/generated/relations/mem_slice.hpp"
#include "barretenberg/vm/avm/generated/relations/poseidon2.hpp"
#include "barretenberg/vm/avm/generated/relations/range_check.hpp"
#include "barretenberg/vm/avm/generated/relations/sha256.hpp"

// Lookup and permutation relations
#include "barretenberg/vm/avm/generated/relations/incl_main_tag_err.hpp"
#include "barretenberg/vm/avm/generated/relations/incl_mem_tag_err.hpp"
#include "barretenberg/vm/avm/generated/relations/kernel_output_lookup.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_byte_lengths.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_byte_operations.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_cd_value.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_into_kernel.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_opcode_gas.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_pow_2_0.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_pow_2_1.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_ret_value.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_rng_chk_0.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_rng_chk_1.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_rng_chk_2.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_rng_chk_3.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_rng_chk_4.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_rng_chk_5.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_rng_chk_6.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_rng_chk_7.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_rng_chk_diff.hpp"
#include "barretenberg/vm/avm/generated/relations/lookup_rng_chk_pow_2.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_cmp_alu.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_da_end_gas.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_da_start_gas.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_l2_end_gas.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_l2_start_gas.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_alu.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_bin.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_conv.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_mem_a.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_mem_b.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_mem_c.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_mem_d.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_mem_ind_addr_a.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_mem_ind_addr_b.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_mem_ind_addr_c.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_mem_ind_addr_d.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_pos2_perm.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_sha256.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_main_slice.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_pos_mem_read_a.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_pos_mem_read_b.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_pos_mem_read_c.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_pos_mem_read_d.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_pos_mem_write_a.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_pos_mem_write_b.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_pos_mem_write_c.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_pos_mem_write_d.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_rng_alu.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_rng_cmp_hi.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_rng_cmp_lo.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_rng_gas_da.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_rng_gas_l2.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_rng_mem.hpp"
#include "barretenberg/vm/avm/generated/relations/perm_slice_mem.hpp"

// Metaprogramming to concatenate tuple types.
template <typename... input_t> using tuple_cat_t = decltype(std::tuple_cat(std::declval<input_t>()...));

// The entities that will be used in the flavor.
// clang-format off
#define PRECOMPUTED_ENTITIES byte_lookup_sel_bin, byte_lookup_table_byte_lengths, byte_lookup_table_in_tags, byte_lookup_table_input_a, byte_lookup_table_input_b, byte_lookup_table_op_id, byte_lookup_table_output, gas_base_da_gas_fixed_table, gas_base_l2_gas_fixed_table, gas_dyn_da_gas_fixed_table, gas_dyn_l2_gas_fixed_table, gas_sel_gas_cost, main_clk, main_sel_da_end_gas_kernel_input, main_sel_da_start_gas_kernel_input, main_sel_first, main_sel_l2_end_gas_kernel_input, main_sel_l2_start_gas_kernel_input, main_sel_start_exec, main_zeroes, powers_power_of_2
#define WIRE_ENTITIES main_kernel_inputs, main_kernel_value_out, main_kernel_side_effect_out, main_kernel_metadata_out, main_calldata, main_returndata, alu_a_hi, alu_a_lo, alu_b_hi, alu_b_lo, alu_b_pow, alu_c_hi, alu_c_lo, alu_cf, alu_clk, alu_cmp_gadget_gt, alu_cmp_gadget_input_a, alu_cmp_gadget_input_b, alu_cmp_gadget_result, alu_cmp_gadget_sel, alu_ff_tag, alu_ia, alu_ib, alu_ic, alu_in_tag, alu_max_bits_sub_b_bits, alu_max_bits_sub_b_pow, alu_op_add, alu_op_cast, alu_op_div, alu_op_eq, alu_op_lt, alu_op_lte, alu_op_mul, alu_op_not, alu_op_shl, alu_op_shr, alu_op_sub, alu_partial_prod_hi, alu_partial_prod_lo, alu_range_check_input_value, alu_range_check_num_bits, alu_range_check_sel, alu_remainder, alu_sel_alu, alu_sel_cmp, alu_sel_shift_which, alu_u128_tag, alu_u16_tag, alu_u1_tag, alu_u32_tag, alu_u64_tag, alu_u8_tag, alu_zero_shift, binary_acc_ia, binary_acc_ib, binary_acc_ic, binary_clk, binary_ia_bytes, binary_ib_bytes, binary_ic_bytes, binary_in_tag, binary_mem_tag_ctr, binary_mem_tag_ctr_inv, binary_op_id, binary_sel_bin, binary_start, cmp_a_hi, cmp_a_lo, cmp_b_hi, cmp_b_lo, cmp_borrow, cmp_clk, cmp_cmp_rng_ctr, cmp_input_a, cmp_input_b, cmp_op_eq, cmp_op_eq_diff_inv, cmp_op_gt, cmp_p_a_borrow, cmp_p_b_borrow, cmp_p_sub_a_hi, cmp_p_sub_a_lo, cmp_p_sub_b_hi, cmp_p_sub_b_lo, cmp_range_chk_clk, cmp_res_hi, cmp_res_lo, cmp_result, cmp_sel_cmp, cmp_sel_rng_chk, cmp_shift_sel, conversion_clk, conversion_input, conversion_num_limbs, conversion_output_bits, conversion_radix, conversion_sel_to_radix_le, keccakf1600_clk, keccakf1600_input, keccakf1600_output, keccakf1600_sel_keccakf1600, main_abs_da_rem_gas, main_abs_l2_rem_gas, main_alu_in_tag, main_base_da_gas_op_cost, main_base_l2_gas_op_cost, main_bin_op_id, main_call_ptr, main_da_gas_remaining, main_da_out_of_gas, main_dyn_da_gas_op_cost, main_dyn_gas_multiplier, main_dyn_l2_gas_op_cost, main_emit_l2_to_l1_msg_write_offset, main_emit_note_hash_write_offset, main_emit_nullifier_write_offset, main_emit_unencrypted_log_write_offset, main_ia, main_ib, main_ic, main_id, main_id_zero, main_ind_addr_a, main_ind_addr_b, main_ind_addr_c, main_ind_addr_d, main_internal_return_ptr, main_inv, main_is_fake_row, main_is_gas_accounted, main_kernel_in_offset, main_kernel_out_offset, main_l1_to_l2_msg_exists_write_offset, main_l2_gas_remaining, main_l2_out_of_gas, main_mem_addr_a, main_mem_addr_b, main_mem_addr_c, main_mem_addr_d, main_note_hash_exist_write_offset, main_nullifier_exists_write_offset, main_nullifier_non_exists_write_offset, main_op_err, main_opcode_val, main_pc, main_r_in_tag, main_rwa, main_rwb, main_rwc, main_rwd, main_sel_alu, main_sel_bin, main_sel_calldata, main_sel_execution_end, main_sel_execution_row, main_sel_kernel_inputs, main_sel_kernel_out, main_sel_mem_op_a, main_sel_mem_op_b, main_sel_mem_op_c, main_sel_mem_op_d, main_sel_mov_ia_to_ic, main_sel_mov_ib_to_ic, main_sel_op_add, main_sel_op_address, main_sel_op_and, main_sel_op_block_number, main_sel_op_calldata_copy, main_sel_op_cast, main_sel_op_chain_id, main_sel_op_dagasleft, main_sel_op_div, main_sel_op_ecadd, main_sel_op_emit_l2_to_l1_msg, main_sel_op_emit_note_hash, main_sel_op_emit_nullifier, main_sel_op_emit_unencrypted_log, main_sel_op_eq, main_sel_op_external_call, main_sel_op_external_return, main_sel_op_external_revert, main_sel_op_fdiv, main_sel_op_fee_per_da_gas, main_sel_op_fee_per_l2_gas, main_sel_op_function_selector, main_sel_op_get_contract_instance, main_sel_op_internal_call, main_sel_op_internal_return, main_sel_op_is_static_call, main_sel_op_jump, main_sel_op_jumpi, main_sel_op_keccak, main_sel_op_l1_to_l2_msg_exists, main_sel_op_l2gasleft, main_sel_op_lt, main_sel_op_lte, main_sel_op_mov, main_sel_op_msm, main_sel_op_mul, main_sel_op_not, main_sel_op_note_hash_exists, main_sel_op_nullifier_exists, main_sel_op_or, main_sel_op_poseidon2, main_sel_op_radix_le, main_sel_op_sender, main_sel_op_set, main_sel_op_sha256, main_sel_op_shl, main_sel_op_shr, main_sel_op_sload, main_sel_op_sstore, main_sel_op_static_call, main_sel_op_storage_address, main_sel_op_sub, main_sel_op_timestamp, main_sel_op_transaction_fee, main_sel_op_version, main_sel_op_xor, main_sel_q_kernel_lookup, main_sel_q_kernel_output_lookup, main_sel_resolve_ind_addr_a, main_sel_resolve_ind_addr_b, main_sel_resolve_ind_addr_c, main_sel_resolve_ind_addr_d, main_sel_returndata, main_sel_rng_16, main_sel_rng_8, main_sel_slice_gadget, main_side_effect_counter, main_sload_write_offset, main_space_id, main_sstore_write_offset, main_tag_err, main_w_in_tag, mem_addr, mem_clk, mem_diff, mem_glob_addr, mem_last, mem_lastAccess, mem_one_min_inv, mem_r_in_tag, mem_rw, mem_sel_mem, mem_sel_mov_ia_to_ic, mem_sel_mov_ib_to_ic, mem_sel_op_a, mem_sel_op_b, mem_sel_op_c, mem_sel_op_d, mem_sel_op_poseidon_read_a, mem_sel_op_poseidon_read_b, mem_sel_op_poseidon_read_c, mem_sel_op_poseidon_read_d, mem_sel_op_poseidon_write_a, mem_sel_op_poseidon_write_b, mem_sel_op_poseidon_write_c, mem_sel_op_poseidon_write_d, mem_sel_op_slice, mem_sel_resolve_ind_addr_a, mem_sel_resolve_ind_addr_b, mem_sel_resolve_ind_addr_c, mem_sel_resolve_ind_addr_d, mem_sel_rng_chk, mem_skip_check_tag, mem_space_id, mem_tag, mem_tag_err, mem_tsp, mem_val, mem_w_in_tag, poseidon2_B_10_0, poseidon2_B_10_1, poseidon2_B_10_2, poseidon2_B_10_3, poseidon2_B_11_0, poseidon2_B_11_1, poseidon2_B_11_2, poseidon2_B_11_3, poseidon2_B_12_0, poseidon2_B_12_1, poseidon2_B_12_2, poseidon2_B_12_3, poseidon2_B_13_0, poseidon2_B_13_1, poseidon2_B_13_2, poseidon2_B_13_3, poseidon2_B_14_0, poseidon2_B_14_1, poseidon2_B_14_2, poseidon2_B_14_3, poseidon2_B_15_0, poseidon2_B_15_1, poseidon2_B_15_2, poseidon2_B_15_3, poseidon2_B_16_0, poseidon2_B_16_1, poseidon2_B_16_2, poseidon2_B_16_3, poseidon2_B_17_0, poseidon2_B_17_1, poseidon2_B_17_2, poseidon2_B_17_3, poseidon2_B_18_0, poseidon2_B_18_1, poseidon2_B_18_2, poseidon2_B_18_3, poseidon2_B_19_0, poseidon2_B_19_1, poseidon2_B_19_2, poseidon2_B_19_3, poseidon2_B_20_0, poseidon2_B_20_1, poseidon2_B_20_2, poseidon2_B_20_3, poseidon2_B_21_0, poseidon2_B_21_1, poseidon2_B_21_2, poseidon2_B_21_3, poseidon2_B_22_0, poseidon2_B_22_1, poseidon2_B_22_2, poseidon2_B_22_3, poseidon2_B_23_0, poseidon2_B_23_1, poseidon2_B_23_2, poseidon2_B_23_3, poseidon2_B_24_0, poseidon2_B_24_1, poseidon2_B_24_2, poseidon2_B_24_3, poseidon2_B_25_0, poseidon2_B_25_1, poseidon2_B_25_2, poseidon2_B_25_3, poseidon2_B_26_0, poseidon2_B_26_1, poseidon2_B_26_2, poseidon2_B_26_3, poseidon2_B_27_0, poseidon2_B_27_1, poseidon2_B_27_2, poseidon2_B_27_3, poseidon2_B_28_0, poseidon2_B_28_1, poseidon2_B_28_2, poseidon2_B_28_3, poseidon2_B_29_0, poseidon2_B_29_1, poseidon2_B_29_2, poseidon2_B_29_3, poseidon2_B_30_0, poseidon2_B_30_1, poseidon2_B_30_2, poseidon2_B_30_3, poseidon2_B_31_0, poseidon2_B_31_1, poseidon2_B_31_2, poseidon2_B_31_3, poseidon2_B_32_0, poseidon2_B_32_1, poseidon2_B_32_2, poseidon2_B_32_3, poseidon2_B_33_0, poseidon2_B_33_1, poseidon2_B_33_2, poseidon2_B_33_3, poseidon2_B_34_0, poseidon2_B_34_1, poseidon2_B_34_2, poseidon2_B_34_3, poseidon2_B_35_0, poseidon2_B_35_1, poseidon2_B_35_2, poseidon2_B_35_3, poseidon2_B_36_0, poseidon2_B_36_1, poseidon2_B_36_2, poseidon2_B_36_3, poseidon2_B_37_0, poseidon2_B_37_1, poseidon2_B_37_2, poseidon2_B_37_3, poseidon2_B_38_0, poseidon2_B_38_1, poseidon2_B_38_2, poseidon2_B_38_3, poseidon2_B_39_0, poseidon2_B_39_1, poseidon2_B_39_2, poseidon2_B_39_3, poseidon2_B_40_0, poseidon2_B_40_1, poseidon2_B_40_2, poseidon2_B_40_3, poseidon2_B_41_0, poseidon2_B_41_1, poseidon2_B_41_2, poseidon2_B_41_3, poseidon2_B_42_0, poseidon2_B_42_1, poseidon2_B_42_2, poseidon2_B_42_3, poseidon2_B_43_0, poseidon2_B_43_1, poseidon2_B_43_2, poseidon2_B_43_3, poseidon2_B_44_0, poseidon2_B_44_1, poseidon2_B_44_2, poseidon2_B_44_3, poseidon2_B_45_0, poseidon2_B_45_1, poseidon2_B_45_2, poseidon2_B_45_3, poseidon2_B_46_0, poseidon2_B_46_1, poseidon2_B_46_2, poseidon2_B_46_3, poseidon2_B_47_0, poseidon2_B_47_1, poseidon2_B_47_2, poseidon2_B_47_3, poseidon2_B_48_0, poseidon2_B_48_1, poseidon2_B_48_2, poseidon2_B_48_3, poseidon2_B_49_0, poseidon2_B_49_1, poseidon2_B_49_2, poseidon2_B_49_3, poseidon2_B_4_0, poseidon2_B_4_1, poseidon2_B_4_2, poseidon2_B_4_3, poseidon2_B_50_0, poseidon2_B_50_1, poseidon2_B_50_2, poseidon2_B_50_3, poseidon2_B_51_0, poseidon2_B_51_1, poseidon2_B_51_2, poseidon2_B_51_3, poseidon2_B_52_0, poseidon2_B_52_1, poseidon2_B_52_2, poseidon2_B_52_3, poseidon2_B_53_0, poseidon2_B_53_1, poseidon2_B_53_2, poseidon2_B_53_3, poseidon2_B_54_0, poseidon2_B_54_1, poseidon2_B_54_2, poseidon2_B_54_3, poseidon2_B_55_0, poseidon2_B_55_1, poseidon2_B_55_2, poseidon2_B_55_3, poseidon2_B_56_0, poseidon2_B_56_1, poseidon2_B_56_2, poseidon2_B_56_3, poseidon2_B_57_0, poseidon2_B_57_1, poseidon2_B_57_2, poseidon2_B_57_3, poseidon2_B_58_0, poseidon2_B_58_1, poseidon2_B_58_2, poseidon2_B_58_3, poseidon2_B_59_0, poseidon2_B_59_1, poseidon2_B_59_2, poseidon2_B_59_3, poseidon2_B_5_0, poseidon2_B_5_1, poseidon2_B_5_2, poseidon2_B_5_3, poseidon2_B_6_0, poseidon2_B_6_1, poseidon2_B_6_2, poseidon2_B_6_3, poseidon2_B_7_0, poseidon2_B_7_1, poseidon2_B_7_2, poseidon2_B_7_3, poseidon2_B_8_0, poseidon2_B_8_1, poseidon2_B_8_2, poseidon2_B_8_3, poseidon2_B_9_0, poseidon2_B_9_1, poseidon2_B_9_2, poseidon2_B_9_3, poseidon2_EXT_LAYER_4, poseidon2_EXT_LAYER_5, poseidon2_EXT_LAYER_6, poseidon2_EXT_LAYER_7, poseidon2_T_0_4, poseidon2_T_0_5, poseidon2_T_0_6, poseidon2_T_0_7, poseidon2_T_1_4, poseidon2_T_1_5, poseidon2_T_1_6, poseidon2_T_1_7, poseidon2_T_2_4, poseidon2_T_2_5, poseidon2_T_2_6, poseidon2_T_2_7, poseidon2_T_3_4, poseidon2_T_3_5, poseidon2_T_3_6, poseidon2_T_3_7, poseidon2_T_60_4, poseidon2_T_60_5, poseidon2_T_60_6, poseidon2_T_60_7, poseidon2_T_61_4, poseidon2_T_61_5, poseidon2_T_61_6, poseidon2_T_61_7, poseidon2_T_62_4, poseidon2_T_62_5, poseidon2_T_62_6, poseidon2_T_62_7, poseidon2_T_63_4, poseidon2_T_63_5, poseidon2_T_63_6, poseidon2_T_63_7, poseidon2_a_0, poseidon2_a_1, poseidon2_a_2, poseidon2_a_3, poseidon2_b_0, poseidon2_b_1, poseidon2_b_2, poseidon2_b_3, poseidon2_clk, poseidon2_input_addr, poseidon2_mem_addr_read_a, poseidon2_mem_addr_read_b, poseidon2_mem_addr_read_c, poseidon2_mem_addr_read_d, poseidon2_mem_addr_write_a, poseidon2_mem_addr_write_b, poseidon2_mem_addr_write_c, poseidon2_mem_addr_write_d, poseidon2_output_addr, poseidon2_sel_poseidon_perm, poseidon2_space_id, range_check_alu_rng_chk, range_check_clk, range_check_cmp_hi_bits_rng_chk, range_check_cmp_lo_bits_rng_chk, range_check_dyn_diff, range_check_dyn_rng_chk_bits, range_check_dyn_rng_chk_pow_2, range_check_gas_da_rng_chk, range_check_gas_l2_rng_chk, range_check_is_lte_u112, range_check_is_lte_u128, range_check_is_lte_u16, range_check_is_lte_u32, range_check_is_lte_u48, range_check_is_lte_u64, range_check_is_lte_u80, range_check_is_lte_u96, range_check_mem_rng_chk, range_check_rng_chk_bits, range_check_sel_lookup_0, range_check_sel_lookup_1, range_check_sel_lookup_2, range_check_sel_lookup_3, range_check_sel_lookup_4, range_check_sel_lookup_5, range_check_sel_lookup_6, range_check_sel_rng_chk, range_check_u16_r0, range_check_u16_r1, range_check_u16_r2, range_check_u16_r3, range_check_u16_r4, range_check_u16_r5, range_check_u16_r6, range_check_u16_r7, range_check_value, sha256_clk, sha256_input, sha256_output, sha256_sel_sha256_compression, sha256_state, slice_addr, slice_clk, slice_cnt, slice_col_offset, slice_one_min_inv, slice_sel_cd_cpy, slice_sel_mem_active, slice_sel_return, slice_sel_start, slice_space_id, slice_val, lookup_rng_chk_pow_2_counts, lookup_rng_chk_diff_counts, lookup_rng_chk_0_counts, lookup_rng_chk_1_counts, lookup_rng_chk_2_counts, lookup_rng_chk_3_counts, lookup_rng_chk_4_counts, lookup_rng_chk_5_counts, lookup_rng_chk_6_counts, lookup_rng_chk_7_counts, lookup_pow_2_0_counts, lookup_pow_2_1_counts, lookup_byte_lengths_counts, lookup_byte_operations_counts, lookup_opcode_gas_counts, kernel_output_lookup_counts, lookup_into_kernel_counts, lookup_cd_value_counts, lookup_ret_value_counts, incl_main_tag_err_counts, incl_mem_tag_err_counts
#define DERIVED_WITNESS_ENTITIES perm_rng_mem_inv, perm_rng_cmp_lo_inv, perm_rng_cmp_hi_inv, perm_rng_alu_inv, perm_cmp_alu_inv, perm_rng_gas_l2_inv, perm_rng_gas_da_inv, perm_l2_start_gas_inv, perm_da_start_gas_inv, perm_l2_end_gas_inv, perm_da_end_gas_inv, perm_pos_mem_read_a_inv, perm_pos_mem_read_b_inv, perm_pos_mem_read_c_inv, perm_pos_mem_read_d_inv, perm_pos_mem_write_a_inv, perm_pos_mem_write_b_inv, perm_pos_mem_write_c_inv, perm_pos_mem_write_d_inv, perm_slice_mem_inv, perm_main_alu_inv, perm_main_bin_inv, perm_main_conv_inv, perm_main_sha256_inv, perm_main_pos2_perm_inv, perm_main_slice_inv, perm_main_mem_a_inv, perm_main_mem_b_inv, perm_main_mem_c_inv, perm_main_mem_d_inv, perm_main_mem_ind_addr_a_inv, perm_main_mem_ind_addr_b_inv, perm_main_mem_ind_addr_c_inv, perm_main_mem_ind_addr_d_inv, lookup_rng_chk_pow_2_inv, lookup_rng_chk_diff_inv, lookup_rng_chk_0_inv, lookup_rng_chk_1_inv, lookup_rng_chk_2_inv, lookup_rng_chk_3_inv, lookup_rng_chk_4_inv, lookup_rng_chk_5_inv, lookup_rng_chk_6_inv, lookup_rng_chk_7_inv, lookup_pow_2_0_inv, lookup_pow_2_1_inv, lookup_byte_lengths_inv, lookup_byte_operations_inv, lookup_opcode_gas_inv, kernel_output_lookup_inv, lookup_into_kernel_inv, lookup_cd_value_inv, lookup_ret_value_inv, incl_main_tag_err_inv, incl_mem_tag_err_inv
#define SHIFTED_ENTITIES binary_acc_ia_shift, binary_acc_ib_shift, binary_acc_ic_shift, binary_mem_tag_ctr_shift, binary_op_id_shift, cmp_a_hi_shift, cmp_a_lo_shift, cmp_b_hi_shift, cmp_b_lo_shift, cmp_cmp_rng_ctr_shift, cmp_op_gt_shift, cmp_p_sub_a_hi_shift, cmp_p_sub_a_lo_shift, cmp_p_sub_b_hi_shift, cmp_p_sub_b_lo_shift, cmp_sel_rng_chk_shift, main_da_gas_remaining_shift, main_emit_l2_to_l1_msg_write_offset_shift, main_emit_note_hash_write_offset_shift, main_emit_nullifier_write_offset_shift, main_emit_unencrypted_log_write_offset_shift, main_internal_return_ptr_shift, main_l1_to_l2_msg_exists_write_offset_shift, main_l2_gas_remaining_shift, main_note_hash_exist_write_offset_shift, main_nullifier_exists_write_offset_shift, main_nullifier_non_exists_write_offset_shift, main_pc_shift, main_sel_execution_end_shift, main_sel_execution_row_shift, main_sload_write_offset_shift, main_sstore_write_offset_shift, mem_glob_addr_shift, mem_rw_shift, mem_sel_mem_shift, mem_tag_shift, mem_tsp_shift, mem_val_shift, slice_addr_shift, slice_clk_shift, slice_cnt_shift, slice_col_offset_shift, slice_sel_cd_cpy_shift, slice_sel_mem_active_shift, slice_sel_return_shift, slice_sel_start_shift, slice_space_id_shift
#define TO_BE_SHIFTED(e) e.binary_acc_ia, e.binary_acc_ib, e.binary_acc_ic, e.binary_mem_tag_ctr, e.binary_op_id, e.cmp_a_hi, e.cmp_a_lo, e.cmp_b_hi, e.cmp_b_lo, e.cmp_cmp_rng_ctr, e.cmp_op_gt, e.cmp_p_sub_a_hi, e.cmp_p_sub_a_lo, e.cmp_p_sub_b_hi, e.cmp_p_sub_b_lo, e.cmp_sel_rng_chk, e.main_da_gas_remaining, e.main_emit_l2_to_l1_msg_write_offset, e.main_emit_note_hash_write_offset, e.main_emit_nullifier_write_offset, e.main_emit_unencrypted_log_write_offset, e.main_internal_return_ptr, e.main_l1_to_l2_msg_exists_write_offset, e.main_l2_gas_remaining, e.main_note_hash_exist_write_offset, e.main_nullifier_exists_write_offset, e.main_nullifier_non_exists_write_offset, e.main_pc, e.main_sel_execution_end, e.main_sel_execution_row, e.main_sload_write_offset, e.main_sstore_write_offset, e.mem_glob_addr, e.mem_rw, e.mem_sel_mem, e.mem_tag, e.mem_tsp, e.mem_val, e.slice_addr, e.slice_clk, e.slice_cnt, e.slice_col_offset, e.slice_sel_cd_cpy, e.slice_sel_mem_active, e.slice_sel_return, e.slice_sel_start, e.slice_space_id
#define ALL_ENTITIES PRECOMPUTED_ENTITIES, WIRE_ENTITIES, DERIVED_WITNESS_ENTITIES, SHIFTED_ENTITIES
// clang-format on

namespace bb {

class AvmFlavor {
  public:
    using Curve = AvmFlavorSettings::Curve;
    using G1 = AvmFlavorSettings::G1;
    using PCS = AvmFlavorSettings::PCS;

    using FF = AvmFlavorSettings::FF;
    using Polynomial = AvmFlavorSettings::Polynomial;
    using PolynomialHandle = AvmFlavorSettings::PolynomialHandle;
    using GroupElement = AvmFlavorSettings::GroupElement;
    using Commitment = AvmFlavorSettings::Commitment;
    using CommitmentHandle = AvmFlavorSettings::CommitmentHandle;
    using CommitmentKey = AvmFlavorSettings::CommitmentKey;
    using VerifierCommitmentKey = AvmFlavorSettings::VerifierCommitmentKey;
    using RelationSeparator = AvmFlavorSettings::RelationSeparator;

    // This flavor would not be used with ZK Sumcheck
    static constexpr bool HasZK = false;

    static constexpr size_t NUM_PRECOMPUTED_ENTITIES = 21;
    static constexpr size_t NUM_WITNESS_ENTITIES = 682;
    static constexpr size_t NUM_SHIFTED_ENTITIES = 47;
    static constexpr size_t NUM_WIRES = NUM_WITNESS_ENTITIES + NUM_PRECOMPUTED_ENTITIES;
    // We have two copies of the witness entities, so we subtract the number of fixed ones (they have no shift), one for
    // the unshifted and one for the shifted
    static constexpr size_t NUM_ALL_ENTITIES = 750;
    // The total number of witnesses including shifts and derived entities.
    static constexpr size_t NUM_ALL_WITNESS_ENTITIES = NUM_WITNESS_ENTITIES + NUM_SHIFTED_ENTITIES;

    // Need to be templated for recursive verifier
    template <typename FF_>
    using MainRelations_ = std::tuple<
        // Relations
        Avm_vm::alu<FF_>,
        Avm_vm::binary<FF_>,
        Avm_vm::cmp<FF_>,
        Avm_vm::conversion<FF_>,
        Avm_vm::gas<FF_>,
        Avm_vm::keccakf1600<FF_>,
        Avm_vm::kernel<FF_>,
        Avm_vm::main<FF_>,
        Avm_vm::mem<FF_>,
        Avm_vm::mem_slice<FF_>,
        Avm_vm::poseidon2<FF_>,
        Avm_vm::range_check<FF_>,
        Avm_vm::sha256<FF_>>;

    using MainRelations = MainRelations_<FF>;

    // Need to be templated for recursive verifier
    template <typename FF_>
    using LookupRelations_ = std::tuple<
        // Lookups
        incl_main_tag_err_relation<FF_>,
        incl_mem_tag_err_relation<FF_>,
        kernel_output_lookup_relation<FF_>,
        lookup_byte_lengths_relation<FF_>,
        lookup_byte_operations_relation<FF_>,
        lookup_cd_value_relation<FF_>,
        lookup_into_kernel_relation<FF_>,
        lookup_opcode_gas_relation<FF_>,
        lookup_pow_2_0_relation<FF_>,
        lookup_pow_2_1_relation<FF_>,
        lookup_ret_value_relation<FF_>,
        lookup_rng_chk_0_relation<FF_>,
        lookup_rng_chk_1_relation<FF_>,
        lookup_rng_chk_2_relation<FF_>,
        lookup_rng_chk_3_relation<FF_>,
        lookup_rng_chk_4_relation<FF_>,
        lookup_rng_chk_5_relation<FF_>,
        lookup_rng_chk_6_relation<FF_>,
        lookup_rng_chk_7_relation<FF_>,
        lookup_rng_chk_diff_relation<FF_>,
        lookup_rng_chk_pow_2_relation<FF_>,
        perm_cmp_alu_relation<FF_>,
        perm_da_end_gas_relation<FF_>,
        perm_da_start_gas_relation<FF_>,
        perm_l2_end_gas_relation<FF_>,
        perm_l2_start_gas_relation<FF_>,
        perm_main_alu_relation<FF_>,
        perm_main_bin_relation<FF_>,
        perm_main_conv_relation<FF_>,
        perm_main_mem_a_relation<FF_>,
        perm_main_mem_b_relation<FF_>,
        perm_main_mem_c_relation<FF_>,
        perm_main_mem_d_relation<FF_>,
        perm_main_mem_ind_addr_a_relation<FF_>,
        perm_main_mem_ind_addr_b_relation<FF_>,
        perm_main_mem_ind_addr_c_relation<FF_>,
        perm_main_mem_ind_addr_d_relation<FF_>,
        perm_main_pos2_perm_relation<FF_>,
        perm_main_sha256_relation<FF_>,
        perm_main_slice_relation<FF_>,
        perm_pos_mem_read_a_relation<FF_>,
        perm_pos_mem_read_b_relation<FF_>,
        perm_pos_mem_read_c_relation<FF_>,
        perm_pos_mem_read_d_relation<FF_>,
        perm_pos_mem_write_a_relation<FF_>,
        perm_pos_mem_write_b_relation<FF_>,
        perm_pos_mem_write_c_relation<FF_>,
        perm_pos_mem_write_d_relation<FF_>,
        perm_rng_alu_relation<FF_>,
        perm_rng_cmp_hi_relation<FF_>,
        perm_rng_cmp_lo_relation<FF_>,
        perm_rng_gas_da_relation<FF_>,
        perm_rng_gas_l2_relation<FF_>,
        perm_rng_mem_relation<FF_>,
        perm_slice_mem_relation<FF_>>;

    using LookupRelations = LookupRelations_<FF>;

    // Need to be templated for recursive verifier
    template <typename FF_> using Relations_ = tuple_cat_t<MainRelations_<FF_>, LookupRelations_<FF_>>;
    using Relations = Relations_<FF>;

    static constexpr size_t MAX_PARTIAL_RELATION_LENGTH = compute_max_partial_relation_length<Relations>();

    // BATCHED_RELATION_PARTIAL_LENGTH = algebraic degree of sumcheck relation *after* multiplying by the `pow_zeta`
    // random polynomial e.g. For \sum(x) [A(x) * B(x) + C(x)] * PowZeta(X), relation length = 2 and random relation
    // length = 3
    static constexpr size_t BATCHED_RELATION_PARTIAL_LENGTH = MAX_PARTIAL_RELATION_LENGTH + 1;
    static constexpr size_t NUM_RELATIONS = std::tuple_size_v<Relations>;

    using SumcheckTupleOfTuplesOfUnivariates = decltype(create_sumcheck_tuple_of_tuples_of_univariates<Relations>());
    using TupleOfArraysOfValues = decltype(create_tuple_of_arrays_of_values<Relations>());

    static constexpr bool has_zero_row = true;

    static constexpr size_t NUM_FRS_COM = field_conversion::calc_num_bn254_frs<Commitment>();
    static constexpr size_t NUM_FRS_FR = field_conversion::calc_num_bn254_frs<FF>();

    // After any circuit changes, hover `COMPUTED_AVM_PROOF_LENGTH_IN_FIELDS` in your IDE
    // to see its value and then update `AVM_PROOF_LENGTH_IN_FIELDS` in constants.nr.
    static constexpr size_t COMPUTED_AVM_PROOF_LENGTH_IN_FIELDS =
        (NUM_WITNESS_ENTITIES + 2) * NUM_FRS_COM + (NUM_ALL_ENTITIES + 1) * NUM_FRS_FR +
        CONST_PROOF_SIZE_LOG_N * (NUM_FRS_COM + NUM_FRS_FR * BATCHED_RELATION_PARTIAL_LENGTH);

    static_assert(AVM_PROOF_LENGTH_IN_FIELDS == COMPUTED_AVM_PROOF_LENGTH_IN_FIELDS,
                  "\nUnexpected AVM proof length. This might be due to some changes in the\n"
                  "AVM circuit layout. In this case, modify AVM_PROOF_LENGTH_IN_FIELDS \n"
                  "in constants.nr accordingly.");

    // VK is composed of
    // - circuit size encoded as a fr field element
    // - num of inputs encoded as a fr field element
    // - NUM_PRECOMPUTED_ENTITIES commitments
    static_assert(AVM_VERIFICATION_KEY_LENGTH_IN_FIELDS == 2 * NUM_FRS_FR + NUM_PRECOMPUTED_ENTITIES * NUM_FRS_COM,
                  "\nUnexpected AVM VK length. This might be due to some changes in the\n"
                  "AVM circuit. In this case, modify AVM_VERIFICATION_LENGTH_IN_FIELDS \n"
                  "in constants.nr accordingly.");

    template <typename DataType_> class PrecomputedEntities : public PrecomputedEntitiesBase {
      public:
        using DataType = DataType_;

        DEFINE_FLAVOR_MEMBERS(DataType, PRECOMPUTED_ENTITIES)

        RefVector<DataType> get_selectors() { return get_all(); }
        RefVector<DataType> get_sigma_polynomials() { return {}; }
        RefVector<DataType> get_id_polynomials() { return {}; }
        RefVector<DataType> get_table_polynomials() { return {}; }
    };

  private:
    template <typename DataType> class WireEntities {
      public:
        DEFINE_FLAVOR_MEMBERS(DataType, WIRE_ENTITIES)
    };

    template <typename DataType> class DerivedWitnessEntities {
      public:
        DEFINE_FLAVOR_MEMBERS(DataType, DERIVED_WITNESS_ENTITIES)
    };

    template <typename DataType> class ShiftedEntities {
      public:
        DEFINE_FLAVOR_MEMBERS(DataType, SHIFTED_ENTITIES)
    };

    template <typename DataType, typename PrecomputedAndWitnessEntitiesSuperset>
    static auto get_to_be_shifted([[maybe_unused]] PrecomputedAndWitnessEntitiesSuperset& entities)
    {
        return RefArray<DataType, NUM_SHIFTED_ENTITIES>{ TO_BE_SHIFTED(entities) };
    }

  public:
    template <typename DataType>
    class WitnessEntities : public WireEntities<DataType>, public DerivedWitnessEntities<DataType> {
      public:
        DEFINE_COMPOUND_GET_ALL(WireEntities<DataType>, DerivedWitnessEntities<DataType>)
        auto get_wires() { return WireEntities<DataType>::get_all(); }
        auto get_derived() { return DerivedWitnessEntities<DataType>::get_all(); }
    };

    template <typename DataType>
    class AllEntities : public PrecomputedEntities<DataType>,
                        public WitnessEntities<DataType>,
                        public ShiftedEntities<DataType> {
      public:
        DEFINE_COMPOUND_GET_ALL(PrecomputedEntities<DataType>, WitnessEntities<DataType>, ShiftedEntities<DataType>)

        auto get_unshifted()
        {
            return concatenate(PrecomputedEntities<DataType>::get_all(), WitnessEntities<DataType>::get_all());
        }
        auto get_to_be_shifted() { return AvmFlavor::get_to_be_shifted<DataType>(*this); }
        auto get_shifted() { return ShiftedEntities<DataType>::get_all(); }
        auto get_precomputed() { return PrecomputedEntities<DataType>::get_all(); }
    };

    class ProvingKey : public PrecomputedEntities<Polynomial>, public WitnessEntities<Polynomial> {
      public:
        using FF = typename Polynomial::FF;

        ProvingKey() = default;
        ProvingKey(const size_t circuit_size, const size_t num_public_inputs);

        size_t circuit_size;
        bb::EvaluationDomain<FF> evaluation_domain;
        std::shared_ptr<CommitmentKey> commitment_key;

        // Offset off the public inputs from the start of the execution trace
        size_t pub_inputs_offset = 0;

        // The number of public inputs has to be the same for all instances because they are
        // folded element by element.
        std::vector<FF> public_inputs;

        std::vector<std::string> get_labels() const
        {
            return concatenate(PrecomputedEntities<Polynomial>::get_labels(),
                               WitnessEntities<Polynomial>::get_labels());
        }
        auto get_witness_polynomials() { return WitnessEntities<Polynomial>::get_all(); }
        auto get_precomputed_polynomials() { return PrecomputedEntities<Polynomial>::get_all(); }
        auto get_selectors() { return PrecomputedEntities<Polynomial>::get_all(); }
        auto get_to_be_shifted() { return AvmFlavor::get_to_be_shifted<Polynomial>(*this); }
        // This order matters! must match get_unshifted in entity classes
        auto get_all() { return concatenate(get_precomputed_polynomials(), get_witness_polynomials()); }
    };

    class VerificationKey : public VerificationKey_<PrecomputedEntities<Commitment>, VerifierCommitmentKey> {
      public:
        using FF = VerificationKey_::FF;

        VerificationKey() = default;

        VerificationKey(const std::shared_ptr<ProvingKey>& proving_key)
            : VerificationKey_(proving_key->circuit_size, static_cast<size_t>(proving_key->num_public_inputs))
        {
            for (auto [polynomial, commitment] :
                 zip_view(proving_key->get_precomputed_polynomials(), this->get_all())) {
                commitment = proving_key->commitment_key->commit(polynomial);
            }
            pcs_verification_key = std::make_shared<VerifierCommitmentKey>();
        }

        VerificationKey(const size_t circuit_size,
                        const size_t num_public_inputs,
                        std::array<Commitment, NUM_PRECOMPUTED_ENTITIES> const& precomputed_cmts)
            : VerificationKey_(circuit_size, num_public_inputs)
        {
            for (auto [vk_cmt, cmt] : zip_view(this->get_all(), precomputed_cmts)) {
                vk_cmt = cmt;
            }
            pcs_verification_key = std::make_shared<VerifierCommitmentKey>();
        }

        std::vector<FF> to_field_elements() const;
    };

    class AllValues : public AllEntities<FF> {
      public:
        using Base = AllEntities<FF>;
        using Base::Base;
    };

    class AllConstRefValues {
      public:
        using BaseDataType = const FF;
        using DataType = BaseDataType&;

        DEFINE_FLAVOR_MEMBERS(DataType, ALL_ENTITIES)

        AllConstRefValues(const RefArray<BaseDataType, NUM_ALL_ENTITIES>& il);
    };

    /**
     * @brief A container for the prover polynomials handles.
     */
    class ProverPolynomials : public AllEntities<Polynomial> {
      public:
        // Define all operations as default, except copy construction/assignment
        ProverPolynomials() = default;
        ProverPolynomials& operator=(const ProverPolynomials&) = delete;
        ProverPolynomials(const ProverPolynomials& o) = delete;
        ProverPolynomials(ProverPolynomials&& o) noexcept = default;
        ProverPolynomials& operator=(ProverPolynomials&& o) noexcept = default;
        ~ProverPolynomials() = default;

        ProverPolynomials(ProvingKey& proving_key);

        [[nodiscard]] size_t get_polynomial_size() const { return main_kernel_inputs.size(); }
        /**
         * @brief Returns the evaluations of all prover polynomials at one point on the boolean hypercube, which
         * represents one row in the execution trace.
         */
        [[nodiscard]] AllConstRefValues get_row(size_t row_idx) const;
    };

    class PartiallyEvaluatedMultivariates : public AllEntities<Polynomial> {
      public:
        PartiallyEvaluatedMultivariates() = default;
        PartiallyEvaluatedMultivariates(const size_t circuit_size);
    };

    /**
     * @brief A container for univariates used during Protogalaxy folding and sumcheck.
     * @details During folding and sumcheck, the prover evaluates the relations on these univariates.
     */
    template <size_t LENGTH> using ProverUnivariates = AllEntities<bb::Univariate<FF, LENGTH>>;

    /**
     * @brief A container for univariates used during Protogalaxy folding and sumcheck with some of the computation
     * optimistically ignored
     * @details During folding and sumcheck, the prover evaluates the relations on these univariates.
     */
    template <size_t LENGTH, size_t SKIP_COUNT>
    using ProverUnivariatesWithOptimisticSkipping = AllEntities<bb::Univariate<FF, LENGTH, 0, SKIP_COUNT>>;

    /**
     * @brief A container for univariates produced during the hot loop in sumcheck.
     */
    using ExtendedEdges = ProverUnivariates<MAX_PARTIAL_RELATION_LENGTH>;

    /**
     * @brief A container for the witness commitments.
     *
     */
    using WitnessCommitments = WitnessEntities<Commitment>;

    class CommitmentLabels : public AllEntities<std::string> {
      private:
        using Base = AllEntities<std::string>;

      public:
        CommitmentLabels();
    };

    // Templated for use in recursive verifier
    template <typename Commitment_, typename VerificationKey>
    class VerifierCommitments_ : public AllEntities<Commitment_> {
      private:
        using Base = AllEntities<Commitment_>;

      public:
        VerifierCommitments_(const std::shared_ptr<VerificationKey>& verification_key)
        {
            this->byte_lookup_sel_bin = verification_key->byte_lookup_sel_bin;
            this->byte_lookup_table_byte_lengths = verification_key->byte_lookup_table_byte_lengths;
            this->byte_lookup_table_in_tags = verification_key->byte_lookup_table_in_tags;
            this->byte_lookup_table_input_a = verification_key->byte_lookup_table_input_a;
            this->byte_lookup_table_input_b = verification_key->byte_lookup_table_input_b;
            this->byte_lookup_table_op_id = verification_key->byte_lookup_table_op_id;
            this->byte_lookup_table_output = verification_key->byte_lookup_table_output;
            this->gas_base_da_gas_fixed_table = verification_key->gas_base_da_gas_fixed_table;
            this->gas_base_l2_gas_fixed_table = verification_key->gas_base_l2_gas_fixed_table;
            this->gas_dyn_da_gas_fixed_table = verification_key->gas_dyn_da_gas_fixed_table;
            this->gas_dyn_l2_gas_fixed_table = verification_key->gas_dyn_l2_gas_fixed_table;
            this->gas_sel_gas_cost = verification_key->gas_sel_gas_cost;
            this->main_clk = verification_key->main_clk;
            this->main_sel_da_end_gas_kernel_input = verification_key->main_sel_da_end_gas_kernel_input;
            this->main_sel_da_start_gas_kernel_input = verification_key->main_sel_da_start_gas_kernel_input;
            this->main_sel_first = verification_key->main_sel_first;
            this->main_sel_l2_end_gas_kernel_input = verification_key->main_sel_l2_end_gas_kernel_input;
            this->main_sel_l2_start_gas_kernel_input = verification_key->main_sel_l2_start_gas_kernel_input;
            this->main_sel_start_exec = verification_key->main_sel_start_exec;
            this->main_zeroes = verification_key->main_zeroes;
            this->powers_power_of_2 = verification_key->powers_power_of_2;
        }
    };

    // Native version of the verifier commitments
    using VerifierCommitments = VerifierCommitments_<Commitment, VerificationKey>;

    class Transcript : public NativeTranscript {
      public:
        uint32_t circuit_size;

        std::array<Commitment, NUM_WITNESS_ENTITIES> commitments;

        std::vector<bb::Univariate<FF, BATCHED_RELATION_PARTIAL_LENGTH>> sumcheck_univariates;
        std::array<FF, NUM_ALL_ENTITIES> sumcheck_evaluations;
        std::vector<Commitment> zm_cq_comms;
        Commitment zm_cq_comm;
        Commitment zm_pi_comm;

        Transcript() = default;

        Transcript(const std::vector<FF>& proof)
            : NativeTranscript(proof)
        {}

        void deserialize_full_transcript();
        void serialize_full_transcript();
    };
};

} // namespace bb
