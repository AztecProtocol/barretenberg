// AUTOGENERATED FILE
#pragma once

#include "../columns.hpp"
#include "barretenberg/relations/generic_lookup/generic_lookup_relation.hpp"

#include <cstddef>
#include <string_view>
#include <tuple>

namespace bb::avm2 {

/////////////////// lookup_nullifier_check_low_leaf_poseidon2 ///////////////////

class lookup_nullifier_check_low_leaf_poseidon2_settings {
  public:
    static constexpr std::string_view NAME = "LOOKUP_NULLIFIER_CHECK_LOW_LEAF_POSEIDON2";
    static constexpr std::string_view RELATION_NAME = "nullifier_check";

    static constexpr size_t READ_TERMS = 1;
    static constexpr size_t WRITE_TERMS = 1;
    static constexpr size_t READ_TERM_TYPES[READ_TERMS] = { 0 };
    static constexpr size_t WRITE_TERM_TYPES[WRITE_TERMS] = { 0 };
    static constexpr size_t LOOKUP_TUPLE_SIZE = 4;
    static constexpr size_t INVERSE_EXISTS_POLYNOMIAL_DEGREE = 4;
    static constexpr size_t READ_TERM_DEGREE = 0;
    static constexpr size_t WRITE_TERM_DEGREE = 0;

    // Columns using the Column enum.
    static constexpr Column SRC_SELECTOR = Column::nullifier_check_sel;
    static constexpr Column DST_SELECTOR = Column::poseidon2_hash_end;
    static constexpr Column COUNTS = Column::lookup_nullifier_check_low_leaf_poseidon2_counts;
    static constexpr Column INVERSES = Column::lookup_nullifier_check_low_leaf_poseidon2_inv;
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> SRC_COLUMNS = {
        ColumnAndShifts::nullifier_check_low_leaf_nullifier,
        ColumnAndShifts::nullifier_check_low_leaf_next_nullifier,
        ColumnAndShifts::nullifier_check_low_leaf_next_index,
        ColumnAndShifts::nullifier_check_low_leaf_hash
    };
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> DST_COLUMNS = {
        ColumnAndShifts::poseidon2_hash_input_0,
        ColumnAndShifts::poseidon2_hash_input_1,
        ColumnAndShifts::poseidon2_hash_input_2,
        ColumnAndShifts::poseidon2_hash_output
    };

    template <typename AllEntities> static inline auto inverse_polynomial_is_computed_at_row(const AllEntities& in)
    {
        return (in._nullifier_check_sel() == 1 || in._poseidon2_hash_end() == 1);
    }

    template <typename Accumulator, typename AllEntities>
    static inline auto compute_inverse_exists(const AllEntities& in)
    {
        using View = typename Accumulator::View;
        const auto is_operation = View(in._nullifier_check_sel());
        const auto is_table_entry = View(in._poseidon2_hash_end());
        return (is_operation + is_table_entry - is_operation * is_table_entry);
    }

    template <typename AllEntities> static inline auto get_const_entities(const AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_nonconst_entities(AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_entities(AllEntities&& in)
    {
        return std::forward_as_tuple(in._lookup_nullifier_check_low_leaf_poseidon2_inv(),
                                     in._lookup_nullifier_check_low_leaf_poseidon2_counts(),
                                     in._nullifier_check_sel(),
                                     in._poseidon2_hash_end(),
                                     in._nullifier_check_low_leaf_nullifier(),
                                     in._nullifier_check_low_leaf_next_nullifier(),
                                     in._nullifier_check_low_leaf_next_index(),
                                     in._nullifier_check_low_leaf_hash(),
                                     in._poseidon2_hash_input_0(),
                                     in._poseidon2_hash_input_1(),
                                     in._poseidon2_hash_input_2(),
                                     in._poseidon2_hash_output());
    }
};

template <typename FF_>
class lookup_nullifier_check_low_leaf_poseidon2_relation
    : public GenericLookupRelation<lookup_nullifier_check_low_leaf_poseidon2_settings, FF_> {
  public:
    using Settings = lookup_nullifier_check_low_leaf_poseidon2_settings;
    static constexpr std::string_view NAME = lookup_nullifier_check_low_leaf_poseidon2_settings::NAME;
    static constexpr std::string_view RELATION_NAME = lookup_nullifier_check_low_leaf_poseidon2_settings::RELATION_NAME;

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        return in.lookup_nullifier_check_low_leaf_poseidon2_inv.is_zero();
    }

    static std::string get_subrelation_label(size_t index)
    {
        if (index == 0) {
            return "INVERSES_ARE_CORRECT";
        } else if (index == 1) {
            return "ACCUMULATION_IS_CORRECT";
        }
        return std::to_string(index);
    }
};

/////////////////// lookup_nullifier_check_updated_low_leaf_poseidon2 ///////////////////

class lookup_nullifier_check_updated_low_leaf_poseidon2_settings {
  public:
    static constexpr std::string_view NAME = "LOOKUP_NULLIFIER_CHECK_UPDATED_LOW_LEAF_POSEIDON2";
    static constexpr std::string_view RELATION_NAME = "nullifier_check";

    static constexpr size_t READ_TERMS = 1;
    static constexpr size_t WRITE_TERMS = 1;
    static constexpr size_t READ_TERM_TYPES[READ_TERMS] = { 0 };
    static constexpr size_t WRITE_TERM_TYPES[WRITE_TERMS] = { 0 };
    static constexpr size_t LOOKUP_TUPLE_SIZE = 4;
    static constexpr size_t INVERSE_EXISTS_POLYNOMIAL_DEGREE = 4;
    static constexpr size_t READ_TERM_DEGREE = 0;
    static constexpr size_t WRITE_TERM_DEGREE = 0;

    // Columns using the Column enum.
    static constexpr Column SRC_SELECTOR = Column::nullifier_check_write;
    static constexpr Column DST_SELECTOR = Column::poseidon2_hash_end;
    static constexpr Column COUNTS = Column::lookup_nullifier_check_updated_low_leaf_poseidon2_counts;
    static constexpr Column INVERSES = Column::lookup_nullifier_check_updated_low_leaf_poseidon2_inv;
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> SRC_COLUMNS = {
        ColumnAndShifts::nullifier_check_low_leaf_nullifier,
        ColumnAndShifts::nullifier_check_write_low_leaf_next_nullifier,
        ColumnAndShifts::nullifier_check_write_low_leaf_next_index,
        ColumnAndShifts::nullifier_check_updated_low_leaf_hash
    };
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> DST_COLUMNS = {
        ColumnAndShifts::poseidon2_hash_input_0,
        ColumnAndShifts::poseidon2_hash_input_1,
        ColumnAndShifts::poseidon2_hash_input_2,
        ColumnAndShifts::poseidon2_hash_output
    };

    template <typename AllEntities> static inline auto inverse_polynomial_is_computed_at_row(const AllEntities& in)
    {
        return (in._nullifier_check_write() == 1 || in._poseidon2_hash_end() == 1);
    }

    template <typename Accumulator, typename AllEntities>
    static inline auto compute_inverse_exists(const AllEntities& in)
    {
        using View = typename Accumulator::View;
        const auto is_operation = View(in._nullifier_check_write());
        const auto is_table_entry = View(in._poseidon2_hash_end());
        return (is_operation + is_table_entry - is_operation * is_table_entry);
    }

    template <typename AllEntities> static inline auto get_const_entities(const AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_nonconst_entities(AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_entities(AllEntities&& in)
    {
        return std::forward_as_tuple(in._lookup_nullifier_check_updated_low_leaf_poseidon2_inv(),
                                     in._lookup_nullifier_check_updated_low_leaf_poseidon2_counts(),
                                     in._nullifier_check_write(),
                                     in._poseidon2_hash_end(),
                                     in._nullifier_check_low_leaf_nullifier(),
                                     in._nullifier_check_write_low_leaf_next_nullifier(),
                                     in._nullifier_check_write_low_leaf_next_index(),
                                     in._nullifier_check_updated_low_leaf_hash(),
                                     in._poseidon2_hash_input_0(),
                                     in._poseidon2_hash_input_1(),
                                     in._poseidon2_hash_input_2(),
                                     in._poseidon2_hash_output());
    }
};

template <typename FF_>
class lookup_nullifier_check_updated_low_leaf_poseidon2_relation
    : public GenericLookupRelation<lookup_nullifier_check_updated_low_leaf_poseidon2_settings, FF_> {
  public:
    using Settings = lookup_nullifier_check_updated_low_leaf_poseidon2_settings;
    static constexpr std::string_view NAME = lookup_nullifier_check_updated_low_leaf_poseidon2_settings::NAME;
    static constexpr std::string_view RELATION_NAME =
        lookup_nullifier_check_updated_low_leaf_poseidon2_settings::RELATION_NAME;

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        return in.lookup_nullifier_check_updated_low_leaf_poseidon2_inv.is_zero();
    }

    static std::string get_subrelation_label(size_t index)
    {
        if (index == 0) {
            return "INVERSES_ARE_CORRECT";
        } else if (index == 1) {
            return "ACCUMULATION_IS_CORRECT";
        }
        return std::to_string(index);
    }
};

/////////////////// lookup_nullifier_check_low_leaf_merkle_check ///////////////////

class lookup_nullifier_check_low_leaf_merkle_check_settings {
  public:
    static constexpr std::string_view NAME = "LOOKUP_NULLIFIER_CHECK_LOW_LEAF_MERKLE_CHECK";
    static constexpr std::string_view RELATION_NAME = "nullifier_check";

    static constexpr size_t READ_TERMS = 1;
    static constexpr size_t WRITE_TERMS = 1;
    static constexpr size_t READ_TERM_TYPES[READ_TERMS] = { 0 };
    static constexpr size_t WRITE_TERM_TYPES[WRITE_TERMS] = { 0 };
    static constexpr size_t LOOKUP_TUPLE_SIZE = 7;
    static constexpr size_t INVERSE_EXISTS_POLYNOMIAL_DEGREE = 4;
    static constexpr size_t READ_TERM_DEGREE = 0;
    static constexpr size_t WRITE_TERM_DEGREE = 0;

    // Columns using the Column enum.
    static constexpr Column SRC_SELECTOR = Column::nullifier_check_sel;
    static constexpr Column DST_SELECTOR = Column::merkle_check_start;
    static constexpr Column COUNTS = Column::lookup_nullifier_check_low_leaf_merkle_check_counts;
    static constexpr Column INVERSES = Column::lookup_nullifier_check_low_leaf_merkle_check_inv;
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> SRC_COLUMNS = {
        ColumnAndShifts::nullifier_check_write,
        ColumnAndShifts::nullifier_check_low_leaf_hash,
        ColumnAndShifts::nullifier_check_updated_low_leaf_hash,
        ColumnAndShifts::nullifier_check_low_leaf_index,
        ColumnAndShifts::nullifier_check_tree_height,
        ColumnAndShifts::nullifier_check_root,
        ColumnAndShifts::nullifier_check_intermediate_root
    };
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> DST_COLUMNS = {
        ColumnAndShifts::merkle_check_write,      ColumnAndShifts::merkle_check_read_node,
        ColumnAndShifts::merkle_check_write_node, ColumnAndShifts::merkle_check_index,
        ColumnAndShifts::merkle_check_path_len,   ColumnAndShifts::merkle_check_read_root,
        ColumnAndShifts::merkle_check_write_root
    };

    template <typename AllEntities> static inline auto inverse_polynomial_is_computed_at_row(const AllEntities& in)
    {
        return (in._nullifier_check_sel() == 1 || in._merkle_check_start() == 1);
    }

    template <typename Accumulator, typename AllEntities>
    static inline auto compute_inverse_exists(const AllEntities& in)
    {
        using View = typename Accumulator::View;
        const auto is_operation = View(in._nullifier_check_sel());
        const auto is_table_entry = View(in._merkle_check_start());
        return (is_operation + is_table_entry - is_operation * is_table_entry);
    }

    template <typename AllEntities> static inline auto get_const_entities(const AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_nonconst_entities(AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_entities(AllEntities&& in)
    {
        return std::forward_as_tuple(in._lookup_nullifier_check_low_leaf_merkle_check_inv(),
                                     in._lookup_nullifier_check_low_leaf_merkle_check_counts(),
                                     in._nullifier_check_sel(),
                                     in._merkle_check_start(),
                                     in._nullifier_check_write(),
                                     in._nullifier_check_low_leaf_hash(),
                                     in._nullifier_check_updated_low_leaf_hash(),
                                     in._nullifier_check_low_leaf_index(),
                                     in._nullifier_check_tree_height(),
                                     in._nullifier_check_root(),
                                     in._nullifier_check_intermediate_root(),
                                     in._merkle_check_write(),
                                     in._merkle_check_read_node(),
                                     in._merkle_check_write_node(),
                                     in._merkle_check_index(),
                                     in._merkle_check_path_len(),
                                     in._merkle_check_read_root(),
                                     in._merkle_check_write_root());
    }
};

template <typename FF_>
class lookup_nullifier_check_low_leaf_merkle_check_relation
    : public GenericLookupRelation<lookup_nullifier_check_low_leaf_merkle_check_settings, FF_> {
  public:
    using Settings = lookup_nullifier_check_low_leaf_merkle_check_settings;
    static constexpr std::string_view NAME = lookup_nullifier_check_low_leaf_merkle_check_settings::NAME;
    static constexpr std::string_view RELATION_NAME =
        lookup_nullifier_check_low_leaf_merkle_check_settings::RELATION_NAME;

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        return in.lookup_nullifier_check_low_leaf_merkle_check_inv.is_zero();
    }

    static std::string get_subrelation_label(size_t index)
    {
        if (index == 0) {
            return "INVERSES_ARE_CORRECT";
        } else if (index == 1) {
            return "ACCUMULATION_IS_CORRECT";
        }
        return std::to_string(index);
    }
};

/////////////////// lookup_nullifier_check_low_leaf_nullifier_validation ///////////////////

class lookup_nullifier_check_low_leaf_nullifier_validation_settings {
  public:
    static constexpr std::string_view NAME = "LOOKUP_NULLIFIER_CHECK_LOW_LEAF_NULLIFIER_VALIDATION";
    static constexpr std::string_view RELATION_NAME = "nullifier_check";

    static constexpr size_t READ_TERMS = 1;
    static constexpr size_t WRITE_TERMS = 1;
    static constexpr size_t READ_TERM_TYPES[READ_TERMS] = { 0 };
    static constexpr size_t WRITE_TERM_TYPES[WRITE_TERMS] = { 0 };
    static constexpr size_t LOOKUP_TUPLE_SIZE = 3;
    static constexpr size_t INVERSE_EXISTS_POLYNOMIAL_DEGREE = 4;
    static constexpr size_t READ_TERM_DEGREE = 0;
    static constexpr size_t WRITE_TERM_DEGREE = 0;

    // Columns using the Column enum.
    static constexpr Column SRC_SELECTOR = Column::nullifier_check_leaf_not_exists;
    static constexpr Column DST_SELECTOR = Column::ff_gt_sel_gt;
    static constexpr Column COUNTS = Column::lookup_nullifier_check_low_leaf_nullifier_validation_counts;
    static constexpr Column INVERSES = Column::lookup_nullifier_check_low_leaf_nullifier_validation_inv;
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> SRC_COLUMNS = {
        ColumnAndShifts::nullifier_check_nullifier,
        ColumnAndShifts::nullifier_check_low_leaf_nullifier,
        ColumnAndShifts::nullifier_check_one
    };
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> DST_COLUMNS = { ColumnAndShifts::ff_gt_a,
                                                                                    ColumnAndShifts::ff_gt_b,
                                                                                    ColumnAndShifts::ff_gt_result };

    template <typename AllEntities> static inline auto inverse_polynomial_is_computed_at_row(const AllEntities& in)
    {
        return (in._nullifier_check_leaf_not_exists() == 1 || in._ff_gt_sel_gt() == 1);
    }

    template <typename Accumulator, typename AllEntities>
    static inline auto compute_inverse_exists(const AllEntities& in)
    {
        using View = typename Accumulator::View;
        const auto is_operation = View(in._nullifier_check_leaf_not_exists());
        const auto is_table_entry = View(in._ff_gt_sel_gt());
        return (is_operation + is_table_entry - is_operation * is_table_entry);
    }

    template <typename AllEntities> static inline auto get_const_entities(const AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_nonconst_entities(AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_entities(AllEntities&& in)
    {
        return std::forward_as_tuple(in._lookup_nullifier_check_low_leaf_nullifier_validation_inv(),
                                     in._lookup_nullifier_check_low_leaf_nullifier_validation_counts(),
                                     in._nullifier_check_leaf_not_exists(),
                                     in._ff_gt_sel_gt(),
                                     in._nullifier_check_nullifier(),
                                     in._nullifier_check_low_leaf_nullifier(),
                                     in._nullifier_check_one(),
                                     in._ff_gt_a(),
                                     in._ff_gt_b(),
                                     in._ff_gt_result());
    }
};

template <typename FF_>
class lookup_nullifier_check_low_leaf_nullifier_validation_relation
    : public GenericLookupRelation<lookup_nullifier_check_low_leaf_nullifier_validation_settings, FF_> {
  public:
    using Settings = lookup_nullifier_check_low_leaf_nullifier_validation_settings;
    static constexpr std::string_view NAME = lookup_nullifier_check_low_leaf_nullifier_validation_settings::NAME;
    static constexpr std::string_view RELATION_NAME =
        lookup_nullifier_check_low_leaf_nullifier_validation_settings::RELATION_NAME;

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        return in.lookup_nullifier_check_low_leaf_nullifier_validation_inv.is_zero();
    }

    static std::string get_subrelation_label(size_t index)
    {
        if (index == 0) {
            return "INVERSES_ARE_CORRECT";
        } else if (index == 1) {
            return "ACCUMULATION_IS_CORRECT";
        }
        return std::to_string(index);
    }
};

/////////////////// lookup_nullifier_check_low_leaf_next_nullifier_validation ///////////////////

class lookup_nullifier_check_low_leaf_next_nullifier_validation_settings {
  public:
    static constexpr std::string_view NAME = "LOOKUP_NULLIFIER_CHECK_LOW_LEAF_NEXT_NULLIFIER_VALIDATION";
    static constexpr std::string_view RELATION_NAME = "nullifier_check";

    static constexpr size_t READ_TERMS = 1;
    static constexpr size_t WRITE_TERMS = 1;
    static constexpr size_t READ_TERM_TYPES[READ_TERMS] = { 0 };
    static constexpr size_t WRITE_TERM_TYPES[WRITE_TERMS] = { 0 };
    static constexpr size_t LOOKUP_TUPLE_SIZE = 3;
    static constexpr size_t INVERSE_EXISTS_POLYNOMIAL_DEGREE = 4;
    static constexpr size_t READ_TERM_DEGREE = 0;
    static constexpr size_t WRITE_TERM_DEGREE = 0;

    // Columns using the Column enum.
    static constexpr Column SRC_SELECTOR = Column::nullifier_check_next_nullifier_is_nonzero;
    static constexpr Column DST_SELECTOR = Column::ff_gt_sel_gt;
    static constexpr Column COUNTS = Column::lookup_nullifier_check_low_leaf_next_nullifier_validation_counts;
    static constexpr Column INVERSES = Column::lookup_nullifier_check_low_leaf_next_nullifier_validation_inv;
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> SRC_COLUMNS = {
        ColumnAndShifts::nullifier_check_low_leaf_next_nullifier,
        ColumnAndShifts::nullifier_check_nullifier,
        ColumnAndShifts::nullifier_check_one
    };
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> DST_COLUMNS = { ColumnAndShifts::ff_gt_a,
                                                                                    ColumnAndShifts::ff_gt_b,
                                                                                    ColumnAndShifts::ff_gt_result };

    template <typename AllEntities> static inline auto inverse_polynomial_is_computed_at_row(const AllEntities& in)
    {
        return (in._nullifier_check_next_nullifier_is_nonzero() == 1 || in._ff_gt_sel_gt() == 1);
    }

    template <typename Accumulator, typename AllEntities>
    static inline auto compute_inverse_exists(const AllEntities& in)
    {
        using View = typename Accumulator::View;
        const auto is_operation = View(in._nullifier_check_next_nullifier_is_nonzero());
        const auto is_table_entry = View(in._ff_gt_sel_gt());
        return (is_operation + is_table_entry - is_operation * is_table_entry);
    }

    template <typename AllEntities> static inline auto get_const_entities(const AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_nonconst_entities(AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_entities(AllEntities&& in)
    {
        return std::forward_as_tuple(in._lookup_nullifier_check_low_leaf_next_nullifier_validation_inv(),
                                     in._lookup_nullifier_check_low_leaf_next_nullifier_validation_counts(),
                                     in._nullifier_check_next_nullifier_is_nonzero(),
                                     in._ff_gt_sel_gt(),
                                     in._nullifier_check_low_leaf_next_nullifier(),
                                     in._nullifier_check_nullifier(),
                                     in._nullifier_check_one(),
                                     in._ff_gt_a(),
                                     in._ff_gt_b(),
                                     in._ff_gt_result());
    }
};

template <typename FF_>
class lookup_nullifier_check_low_leaf_next_nullifier_validation_relation
    : public GenericLookupRelation<lookup_nullifier_check_low_leaf_next_nullifier_validation_settings, FF_> {
  public:
    using Settings = lookup_nullifier_check_low_leaf_next_nullifier_validation_settings;
    static constexpr std::string_view NAME = lookup_nullifier_check_low_leaf_next_nullifier_validation_settings::NAME;
    static constexpr std::string_view RELATION_NAME =
        lookup_nullifier_check_low_leaf_next_nullifier_validation_settings::RELATION_NAME;

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        return in.lookup_nullifier_check_low_leaf_next_nullifier_validation_inv.is_zero();
    }

    static std::string get_subrelation_label(size_t index)
    {
        if (index == 0) {
            return "INVERSES_ARE_CORRECT";
        } else if (index == 1) {
            return "ACCUMULATION_IS_CORRECT";
        }
        return std::to_string(index);
    }
};

/////////////////// lookup_nullifier_check_new_leaf_poseidon2 ///////////////////

class lookup_nullifier_check_new_leaf_poseidon2_settings {
  public:
    static constexpr std::string_view NAME = "LOOKUP_NULLIFIER_CHECK_NEW_LEAF_POSEIDON2";
    static constexpr std::string_view RELATION_NAME = "nullifier_check";

    static constexpr size_t READ_TERMS = 1;
    static constexpr size_t WRITE_TERMS = 1;
    static constexpr size_t READ_TERM_TYPES[READ_TERMS] = { 0 };
    static constexpr size_t WRITE_TERM_TYPES[WRITE_TERMS] = { 0 };
    static constexpr size_t LOOKUP_TUPLE_SIZE = 4;
    static constexpr size_t INVERSE_EXISTS_POLYNOMIAL_DEGREE = 4;
    static constexpr size_t READ_TERM_DEGREE = 0;
    static constexpr size_t WRITE_TERM_DEGREE = 0;

    // Columns using the Column enum.
    static constexpr Column SRC_SELECTOR = Column::nullifier_check_write;
    static constexpr Column DST_SELECTOR = Column::poseidon2_hash_end;
    static constexpr Column COUNTS = Column::lookup_nullifier_check_new_leaf_poseidon2_counts;
    static constexpr Column INVERSES = Column::lookup_nullifier_check_new_leaf_poseidon2_inv;
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> SRC_COLUMNS = {
        ColumnAndShifts::nullifier_check_nullifier,
        ColumnAndShifts::nullifier_check_low_leaf_next_nullifier,
        ColumnAndShifts::nullifier_check_low_leaf_next_index,
        ColumnAndShifts::nullifier_check_new_leaf_hash
    };
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> DST_COLUMNS = {
        ColumnAndShifts::poseidon2_hash_input_0,
        ColumnAndShifts::poseidon2_hash_input_1,
        ColumnAndShifts::poseidon2_hash_input_2,
        ColumnAndShifts::poseidon2_hash_output
    };

    template <typename AllEntities> static inline auto inverse_polynomial_is_computed_at_row(const AllEntities& in)
    {
        return (in._nullifier_check_write() == 1 || in._poseidon2_hash_end() == 1);
    }

    template <typename Accumulator, typename AllEntities>
    static inline auto compute_inverse_exists(const AllEntities& in)
    {
        using View = typename Accumulator::View;
        const auto is_operation = View(in._nullifier_check_write());
        const auto is_table_entry = View(in._poseidon2_hash_end());
        return (is_operation + is_table_entry - is_operation * is_table_entry);
    }

    template <typename AllEntities> static inline auto get_const_entities(const AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_nonconst_entities(AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_entities(AllEntities&& in)
    {
        return std::forward_as_tuple(in._lookup_nullifier_check_new_leaf_poseidon2_inv(),
                                     in._lookup_nullifier_check_new_leaf_poseidon2_counts(),
                                     in._nullifier_check_write(),
                                     in._poseidon2_hash_end(),
                                     in._nullifier_check_nullifier(),
                                     in._nullifier_check_low_leaf_next_nullifier(),
                                     in._nullifier_check_low_leaf_next_index(),
                                     in._nullifier_check_new_leaf_hash(),
                                     in._poseidon2_hash_input_0(),
                                     in._poseidon2_hash_input_1(),
                                     in._poseidon2_hash_input_2(),
                                     in._poseidon2_hash_output());
    }
};

template <typename FF_>
class lookup_nullifier_check_new_leaf_poseidon2_relation
    : public GenericLookupRelation<lookup_nullifier_check_new_leaf_poseidon2_settings, FF_> {
  public:
    using Settings = lookup_nullifier_check_new_leaf_poseidon2_settings;
    static constexpr std::string_view NAME = lookup_nullifier_check_new_leaf_poseidon2_settings::NAME;
    static constexpr std::string_view RELATION_NAME = lookup_nullifier_check_new_leaf_poseidon2_settings::RELATION_NAME;

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        return in.lookup_nullifier_check_new_leaf_poseidon2_inv.is_zero();
    }

    static std::string get_subrelation_label(size_t index)
    {
        if (index == 0) {
            return "INVERSES_ARE_CORRECT";
        } else if (index == 1) {
            return "ACCUMULATION_IS_CORRECT";
        }
        return std::to_string(index);
    }
};

/////////////////// lookup_nullifier_check_new_leaf_merkle_check ///////////////////

class lookup_nullifier_check_new_leaf_merkle_check_settings {
  public:
    static constexpr std::string_view NAME = "LOOKUP_NULLIFIER_CHECK_NEW_LEAF_MERKLE_CHECK";
    static constexpr std::string_view RELATION_NAME = "nullifier_check";

    static constexpr size_t READ_TERMS = 1;
    static constexpr size_t WRITE_TERMS = 1;
    static constexpr size_t READ_TERM_TYPES[READ_TERMS] = { 0 };
    static constexpr size_t WRITE_TERM_TYPES[WRITE_TERMS] = { 0 };
    static constexpr size_t LOOKUP_TUPLE_SIZE = 7;
    static constexpr size_t INVERSE_EXISTS_POLYNOMIAL_DEGREE = 4;
    static constexpr size_t READ_TERM_DEGREE = 0;
    static constexpr size_t WRITE_TERM_DEGREE = 0;

    // Columns using the Column enum.
    static constexpr Column SRC_SELECTOR = Column::nullifier_check_write;
    static constexpr Column DST_SELECTOR = Column::merkle_check_start;
    static constexpr Column COUNTS = Column::lookup_nullifier_check_new_leaf_merkle_check_counts;
    static constexpr Column INVERSES = Column::lookup_nullifier_check_new_leaf_merkle_check_inv;
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> SRC_COLUMNS = {
        ColumnAndShifts::nullifier_check_one,           ColumnAndShifts::precomputed_zero,
        ColumnAndShifts::nullifier_check_new_leaf_hash, ColumnAndShifts::nullifier_check_tree_size_before_write,
        ColumnAndShifts::nullifier_check_tree_height,   ColumnAndShifts::nullifier_check_intermediate_root,
        ColumnAndShifts::nullifier_check_write_root
    };
    static constexpr std::array<ColumnAndShifts, LOOKUP_TUPLE_SIZE> DST_COLUMNS = {
        ColumnAndShifts::merkle_check_write,      ColumnAndShifts::merkle_check_read_node,
        ColumnAndShifts::merkle_check_write_node, ColumnAndShifts::merkle_check_index,
        ColumnAndShifts::merkle_check_path_len,   ColumnAndShifts::merkle_check_read_root,
        ColumnAndShifts::merkle_check_write_root
    };

    template <typename AllEntities> static inline auto inverse_polynomial_is_computed_at_row(const AllEntities& in)
    {
        return (in._nullifier_check_write() == 1 || in._merkle_check_start() == 1);
    }

    template <typename Accumulator, typename AllEntities>
    static inline auto compute_inverse_exists(const AllEntities& in)
    {
        using View = typename Accumulator::View;
        const auto is_operation = View(in._nullifier_check_write());
        const auto is_table_entry = View(in._merkle_check_start());
        return (is_operation + is_table_entry - is_operation * is_table_entry);
    }

    template <typename AllEntities> static inline auto get_const_entities(const AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_nonconst_entities(AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_entities(AllEntities&& in)
    {
        return std::forward_as_tuple(in._lookup_nullifier_check_new_leaf_merkle_check_inv(),
                                     in._lookup_nullifier_check_new_leaf_merkle_check_counts(),
                                     in._nullifier_check_write(),
                                     in._merkle_check_start(),
                                     in._nullifier_check_one(),
                                     in._precomputed_zero(),
                                     in._nullifier_check_new_leaf_hash(),
                                     in._nullifier_check_tree_size_before_write(),
                                     in._nullifier_check_tree_height(),
                                     in._nullifier_check_intermediate_root(),
                                     in._nullifier_check_write_root(),
                                     in._merkle_check_write(),
                                     in._merkle_check_read_node(),
                                     in._merkle_check_write_node(),
                                     in._merkle_check_index(),
                                     in._merkle_check_path_len(),
                                     in._merkle_check_read_root(),
                                     in._merkle_check_write_root());
    }
};

template <typename FF_>
class lookup_nullifier_check_new_leaf_merkle_check_relation
    : public GenericLookupRelation<lookup_nullifier_check_new_leaf_merkle_check_settings, FF_> {
  public:
    using Settings = lookup_nullifier_check_new_leaf_merkle_check_settings;
    static constexpr std::string_view NAME = lookup_nullifier_check_new_leaf_merkle_check_settings::NAME;
    static constexpr std::string_view RELATION_NAME =
        lookup_nullifier_check_new_leaf_merkle_check_settings::RELATION_NAME;

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        return in.lookup_nullifier_check_new_leaf_merkle_check_inv.is_zero();
    }

    static std::string get_subrelation_label(size_t index)
    {
        if (index == 0) {
            return "INVERSES_ARE_CORRECT";
        } else if (index == 1) {
            return "ACCUMULATION_IS_CORRECT";
        }
        return std::to_string(index);
    }
};

} // namespace bb::avm2
