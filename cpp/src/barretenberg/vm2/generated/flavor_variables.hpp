// AUTOGENERATED FILE
#pragma once

// Relations
#include "relations/address_derivation.hpp"
#include "relations/alu.hpp"
#include "relations/bc_decomposition.hpp"
#include "relations/bc_hashing.hpp"
#include "relations/bc_retrieval.hpp"
#include "relations/bitwise.hpp"
#include "relations/class_id_derivation.hpp"
#include "relations/context.hpp"
#include "relations/context_stack.hpp"
#include "relations/ecc.hpp"
#include "relations/execution.hpp"
#include "relations/ff_gt.hpp"
#include "relations/instr_fetching.hpp"
#include "relations/merkle_check.hpp"
#include "relations/nullifier_read.hpp"
#include "relations/poseidon2_hash.hpp"
#include "relations/poseidon2_perm.hpp"
#include "relations/public_data_read.hpp"
#include "relations/range_check.hpp"
#include "relations/scalar_mul.hpp"
#include "relations/sha256.hpp"
#include "relations/to_radix.hpp"
#include "relations/update_check.hpp"

// Lookup and permutation relations
#include "relations/lookups_address_derivation.hpp"
#include "relations/lookups_bc_decomposition.hpp"
#include "relations/lookups_bc_hashing.hpp"
#include "relations/lookups_bc_retrieval.hpp"
#include "relations/lookups_bitwise.hpp"
#include "relations/lookups_class_id_derivation.hpp"
#include "relations/lookups_ff_gt.hpp"
#include "relations/lookups_instr_fetching.hpp"
#include "relations/lookups_merkle_check.hpp"
#include "relations/lookups_nullifier_read.hpp"
#include "relations/lookups_poseidon2_hash.hpp"
#include "relations/lookups_public_data_read.hpp"
#include "relations/lookups_range_check.hpp"
#include "relations/lookups_scalar_mul.hpp"
#include "relations/lookups_sha256.hpp"
#include "relations/lookups_to_radix.hpp"
#include "relations/lookups_update_check.hpp"

namespace bb::avm2 {

struct AvmFlavorVariables {
    static constexpr size_t NUM_PRECOMPUTED_ENTITIES = 45;
    static constexpr size_t NUM_WITNESS_ENTITIES = 998;
    static constexpr size_t NUM_SHIFTED_ENTITIES = 135;
    static constexpr size_t NUM_WIRES = NUM_WITNESS_ENTITIES + NUM_PRECOMPUTED_ENTITIES;
    static constexpr size_t NUM_ALL_ENTITIES = 1178;

    // Need to be templated for recursive verifier
    template <typename FF_>
    using MainRelations_ = std::tuple<
        // Relations
        avm2::address_derivation<FF_>,
        avm2::alu<FF_>,
        avm2::bc_decomposition<FF_>,
        avm2::bc_hashing<FF_>,
        avm2::bc_retrieval<FF_>,
        avm2::bitwise<FF_>,
        avm2::class_id_derivation<FF_>,
        avm2::context<FF_>,
        avm2::context_stack<FF_>,
        avm2::ecc<FF_>,
        avm2::execution<FF_>,
        avm2::ff_gt<FF_>,
        avm2::instr_fetching<FF_>,
        avm2::merkle_check<FF_>,
        avm2::nullifier_read<FF_>,
        avm2::poseidon2_hash<FF_>,
        avm2::poseidon2_perm<FF_>,
        avm2::public_data_read<FF_>,
        avm2::range_check<FF_>,
        avm2::scalar_mul<FF_>,
        avm2::sha256<FF_>,
        avm2::to_radix<FF_>,
        avm2::update_check<FF_>>;

    // Need to be templated for recursive verifier
    template <typename FF_>
    using LookupRelations_ = std::tuple<
        // Lookups
        lookup_address_derivation_address_ecadd_relation<FF_>,
        lookup_address_derivation_partial_address_poseidon2_relation<FF_>,
        lookup_address_derivation_preaddress_poseidon2_relation<FF_>,
        lookup_address_derivation_preaddress_scalar_mul_relation<FF_>,
        lookup_address_derivation_public_keys_hash_poseidon2_0_relation<FF_>,
        lookup_address_derivation_public_keys_hash_poseidon2_1_relation<FF_>,
        lookup_address_derivation_public_keys_hash_poseidon2_2_relation<FF_>,
        lookup_address_derivation_public_keys_hash_poseidon2_3_relation<FF_>,
        lookup_address_derivation_public_keys_hash_poseidon2_4_relation<FF_>,
        lookup_address_derivation_salted_initialization_hash_poseidon2_0_relation<FF_>,
        lookup_address_derivation_salted_initialization_hash_poseidon2_1_relation<FF_>,
        lookup_bc_decomposition_abs_diff_is_u16_relation<FF_>,
        lookup_bc_decomposition_bytes_are_bytes_relation<FF_>,
        lookup_bc_hashing_get_packed_field_relation<FF_>,
        lookup_bc_hashing_iv_is_len_relation<FF_>,
        lookup_bc_hashing_poseidon2_hash_relation<FF_>,
        lookup_bc_retrieval_address_derivation_relation<FF_>,
        lookup_bc_retrieval_bytecode_hash_is_correct_relation<FF_>,
        lookup_bc_retrieval_class_id_derivation_relation<FF_>,
        lookup_bc_retrieval_update_check_relation<FF_>,
        lookup_bitwise_byte_operations_relation<FF_>,
        lookup_bitwise_integral_tag_length_relation<FF_>,
        lookup_class_id_derivation_class_id_poseidon2_0_relation<FF_>,
        lookup_class_id_derivation_class_id_poseidon2_1_relation<FF_>,
        lookup_ff_gt_a_hi_range_relation<FF_>,
        lookup_ff_gt_a_lo_range_relation<FF_>,
        lookup_instr_fetching_bytecode_size_from_bc_dec_relation<FF_>,
        lookup_instr_fetching_bytes_from_bc_dec_relation<FF_>,
        lookup_instr_fetching_instr_abs_diff_positive_relation<FF_>,
        lookup_instr_fetching_pc_abs_diff_positive_relation<FF_>,
        lookup_instr_fetching_tag_value_validation_relation<FF_>,
        lookup_instr_fetching_wire_instruction_info_relation<FF_>,
        lookup_merkle_check_merkle_poseidon2_read_relation<FF_>,
        lookup_merkle_check_merkle_poseidon2_write_relation<FF_>,
        lookup_nullifier_read_low_leaf_membership_relation<FF_>,
        lookup_nullifier_read_low_leaf_next_nullifier_validation_relation<FF_>,
        lookup_nullifier_read_low_leaf_nullifier_validation_relation<FF_>,
        lookup_nullifier_read_low_leaf_poseidon2_relation<FF_>,
        lookup_poseidon2_hash_poseidon2_perm_relation<FF_>,
        lookup_public_data_read_low_leaf_membership_relation<FF_>,
        lookup_public_data_read_low_leaf_next_slot_validation_relation<FF_>,
        lookup_public_data_read_low_leaf_poseidon2_0_relation<FF_>,
        lookup_public_data_read_low_leaf_poseidon2_1_relation<FF_>,
        lookup_public_data_read_low_leaf_slot_validation_relation<FF_>,
        lookup_range_check_dyn_diff_is_u16_relation<FF_>,
        lookup_range_check_dyn_rng_chk_pow_2_relation<FF_>,
        lookup_range_check_r0_is_u16_relation<FF_>,
        lookup_range_check_r1_is_u16_relation<FF_>,
        lookup_range_check_r2_is_u16_relation<FF_>,
        lookup_range_check_r3_is_u16_relation<FF_>,
        lookup_range_check_r4_is_u16_relation<FF_>,
        lookup_range_check_r5_is_u16_relation<FF_>,
        lookup_range_check_r6_is_u16_relation<FF_>,
        lookup_range_check_r7_is_u16_relation<FF_>,
        lookup_scalar_mul_add_relation<FF_>,
        lookup_scalar_mul_double_relation<FF_>,
        lookup_scalar_mul_to_radix_relation<FF_>,
        lookup_sha256_round_constant_relation<FF_>,
        lookup_to_radix_fetch_p_limb_relation<FF_>,
        lookup_to_radix_fetch_safe_limbs_relation<FF_>,
        lookup_to_radix_limb_less_than_radix_range_relation<FF_>,
        lookup_to_radix_limb_p_diff_range_relation<FF_>,
        lookup_to_radix_limb_range_relation<FF_>,
        lookup_update_check_block_of_change_cmp_range_relation<FF_>,
        lookup_update_check_shared_mutable_leaf_slot_poseidon2_relation<FF_>,
        lookup_update_check_shared_mutable_slot_poseidon2_relation<FF_>,
        lookup_update_check_update_hash_poseidon2_relation<FF_>,
        lookup_update_check_update_hash_public_data_read_relation<FF_>,
        lookup_update_check_update_hi_metadata_range_relation<FF_>,
        lookup_update_check_update_lo_metadata_range_relation<FF_>>;
};

} // namespace bb::avm2
