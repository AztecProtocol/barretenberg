// AUTOGENERATED FILE
#pragma once

#include <string_view>

#include "barretenberg/relations/relation_parameters.hpp"
#include "barretenberg/relations/relation_types.hpp"

namespace bb::avm2 {

template <typename FF_> class instr_fetchingImpl {
  public:
    using FF = FF_;

    static constexpr std::array<size_t, 9> SUBRELATION_PARTIAL_LENGTHS = { 3, 3, 3, 3, 3, 3, 3, 3, 3 };

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        const auto& new_term = in;
        return (new_term.instr_fetching_sel).is_zero();
    }

    template <typename ContainerOverSubrelations, typename AllEntities>
    void static accumulate(ContainerOverSubrelations& evals,
                           const AllEntities& new_term,
                           [[maybe_unused]] const RelationParameters<FF>&,
                           [[maybe_unused]] const FF& scaling_factor)
    {
        const auto instr_fetching_SEL_OP_DC_18 =
            new_term.instr_fetching_sel_op_dc_2 + new_term.instr_fetching_sel_op_dc_6;

        {
            using Accumulator = typename std::tuple_element_t<0, ContainerOverSubrelations>;
            auto tmp = new_term.instr_fetching_sel * (FF(1) - new_term.instr_fetching_sel);
            tmp *= scaling_factor;
            std::get<0>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<1, ContainerOverSubrelations>;
            auto tmp = (new_term.instr_fetching_indirect -
                        (new_term.instr_fetching_sel_op_dc_0 *
                             (new_term.instr_fetching_bd1 * FF(256) + new_term.instr_fetching_bd2 * FF(1)) +
                         instr_fetching_SEL_OP_DC_18 * new_term.instr_fetching_bd1 * FF(1)));
            tmp *= scaling_factor;
            std::get<1>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<2, ContainerOverSubrelations>;
            auto tmp = (new_term.instr_fetching_op1 -
                        (new_term.instr_fetching_sel_op_dc_0 *
                             (new_term.instr_fetching_bd3 * FF(256) + new_term.instr_fetching_bd4 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_2 *
                             (new_term.instr_fetching_bd2 * FF(256) + new_term.instr_fetching_bd3 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_6 * new_term.instr_fetching_bd2 * FF(1) +
                         new_term.instr_fetching_sel_op_dc_15 *
                             (new_term.instr_fetching_bd1 * FF(16777216) + new_term.instr_fetching_bd2 * FF(65536) +
                              new_term.instr_fetching_bd3 * FF(256) + new_term.instr_fetching_bd4 * FF(1))));
            tmp *= scaling_factor;
            std::get<2>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<3, ContainerOverSubrelations>;
            auto tmp = (new_term.instr_fetching_op2 -
                        (new_term.instr_fetching_sel_op_dc_0 *
                             (new_term.instr_fetching_bd5 * FF(256) + new_term.instr_fetching_bd6 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_3 *
                             (new_term.instr_fetching_bd4 * FF(256) + new_term.instr_fetching_bd5 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_6 * new_term.instr_fetching_bd3 * FF(1) +
                         new_term.instr_fetching_sel_op_dc_8 * new_term.instr_fetching_bd4 * FF(1) +
                         new_term.instr_fetching_sel_op_dc_16 *
                             (new_term.instr_fetching_bd4 * FF(16777216) + new_term.instr_fetching_bd5 * FF(65536) +
                              new_term.instr_fetching_bd6 * FF(256) + new_term.instr_fetching_bd7 * FF(1))));
            tmp *= scaling_factor;
            std::get<3>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<4, ContainerOverSubrelations>;
            auto tmp = (new_term.instr_fetching_op3 -
                        (new_term.instr_fetching_sel_op_dc_0 *
                             (new_term.instr_fetching_bd7 * FF(256) + new_term.instr_fetching_bd8 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_4 *
                             (new_term.instr_fetching_bd6 * FF(256) + new_term.instr_fetching_bd7 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_9 *
                             (new_term.instr_fetching_bd5 * FF(uint256_t{ 0UL, 0UL, 0UL, 72057594037927936UL }) +
                              new_term.instr_fetching_bd6 * FF(uint256_t{ 0UL, 0UL, 0UL, 281474976710656UL }) +
                              new_term.instr_fetching_bd7 * FF(uint256_t{ 0UL, 0UL, 0UL, 1099511627776UL }) +
                              new_term.instr_fetching_bd8 * FF(uint256_t{ 0UL, 0UL, 0UL, 4294967296UL }) +
                              new_term.instr_fetching_bd9 * FF(uint256_t{ 0UL, 0UL, 0UL, 16777216UL }) +
                              new_term.instr_fetching_bd10 * FF(uint256_t{ 0UL, 0UL, 0UL, 65536UL }) +
                              new_term.instr_fetching_bd11 * FF(uint256_t{ 0UL, 0UL, 0UL, 256UL }) +
                              new_term.instr_fetching_bd12 * FF(uint256_t{ 0UL, 0UL, 0UL, 1UL }) +
                              new_term.instr_fetching_bd13 * FF(uint256_t{ 0UL, 0UL, 72057594037927936UL, 0UL }) +
                              new_term.instr_fetching_bd14 * FF(uint256_t{ 0UL, 0UL, 281474976710656UL, 0UL }) +
                              new_term.instr_fetching_bd15 * FF(uint256_t{ 0UL, 0UL, 1099511627776UL, 0UL }) +
                              new_term.instr_fetching_bd16 * FF(uint256_t{ 0UL, 0UL, 4294967296UL, 0UL }) +
                              new_term.instr_fetching_bd17 * FF(uint256_t{ 0UL, 0UL, 16777216UL, 0UL }) +
                              new_term.instr_fetching_bd18 * FF(uint256_t{ 0UL, 0UL, 65536UL, 0UL }) +
                              new_term.instr_fetching_bd19 * FF(uint256_t{ 0UL, 0UL, 256UL, 0UL }) +
                              new_term.instr_fetching_bd20 * FF(uint256_t{ 0UL, 0UL, 1UL, 0UL }) +
                              new_term.instr_fetching_bd21 * FF(uint256_t{ 0UL, 72057594037927936UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd22 * FF(uint256_t{ 0UL, 281474976710656UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd23 * FF(uint256_t{ 0UL, 1099511627776UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd24 * FF(uint256_t{ 0UL, 4294967296UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd25 * FF(uint256_t{ 0UL, 16777216UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd26 * FF(uint256_t{ 0UL, 65536UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd27 * FF(uint256_t{ 0UL, 256UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd28 * FF(uint256_t{ 0UL, 1UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd29 * FF(72057594037927936UL) +
                              new_term.instr_fetching_bd30 * FF(281474976710656UL) +
                              new_term.instr_fetching_bd31 * FF(1099511627776UL) +
                              new_term.instr_fetching_bd32 * FF(4294967296UL) +
                              new_term.instr_fetching_bd33 * FF(16777216) + new_term.instr_fetching_bd34 * FF(65536) +
                              new_term.instr_fetching_bd35 * FF(256) + new_term.instr_fetching_bd36 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_10 *
                             (new_term.instr_fetching_bd5 * FF(uint256_t{ 0UL, 72057594037927936UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd6 * FF(uint256_t{ 0UL, 281474976710656UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd7 * FF(uint256_t{ 0UL, 1099511627776UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd8 * FF(uint256_t{ 0UL, 4294967296UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd9 * FF(uint256_t{ 0UL, 16777216UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd10 * FF(uint256_t{ 0UL, 65536UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd11 * FF(uint256_t{ 0UL, 256UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd12 * FF(uint256_t{ 0UL, 1UL, 0UL, 0UL }) +
                              new_term.instr_fetching_bd13 * FF(72057594037927936UL) +
                              new_term.instr_fetching_bd14 * FF(281474976710656UL) +
                              new_term.instr_fetching_bd15 * FF(1099511627776UL) +
                              new_term.instr_fetching_bd16 * FF(4294967296UL) +
                              new_term.instr_fetching_bd17 * FF(16777216) + new_term.instr_fetching_bd18 * FF(65536) +
                              new_term.instr_fetching_bd19 * FF(256) + new_term.instr_fetching_bd20 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_11 *
                             (new_term.instr_fetching_bd5 * FF(72057594037927936UL) +
                              new_term.instr_fetching_bd6 * FF(281474976710656UL) +
                              new_term.instr_fetching_bd7 * FF(1099511627776UL) +
                              new_term.instr_fetching_bd8 * FF(4294967296UL) +
                              new_term.instr_fetching_bd9 * FF(16777216) + new_term.instr_fetching_bd10 * FF(65536) +
                              new_term.instr_fetching_bd11 * FF(256) + new_term.instr_fetching_bd12 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_12 *
                             (new_term.instr_fetching_bd5 * FF(16777216) + new_term.instr_fetching_bd6 * FF(65536) +
                              new_term.instr_fetching_bd7 * FF(256) + new_term.instr_fetching_bd8 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_13 *
                             (new_term.instr_fetching_bd5 * FF(256) + new_term.instr_fetching_bd6 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_14 * new_term.instr_fetching_bd4 * FF(1) +
                         new_term.instr_fetching_sel_op_dc_17 * new_term.instr_fetching_bd6 * FF(1)));
            tmp *= scaling_factor;
            std::get<4>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<5, ContainerOverSubrelations>;
            auto tmp = (new_term.instr_fetching_op4 -
                        (new_term.instr_fetching_sel_op_dc_0 *
                             (new_term.instr_fetching_bd9 * FF(256) + new_term.instr_fetching_bd10 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_5 *
                             (new_term.instr_fetching_bd8 * FF(256) + new_term.instr_fetching_bd9 * FF(1)) +
                         new_term.instr_fetching_sel_op_dc_7 * new_term.instr_fetching_bd8 * FF(1)));
            tmp *= scaling_factor;
            std::get<5>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<6, ContainerOverSubrelations>;
            auto tmp = (new_term.instr_fetching_op5 -
                        new_term.instr_fetching_sel_op_dc_0 *
                            (new_term.instr_fetching_bd11 * FF(256) + new_term.instr_fetching_bd12 * FF(1)));
            tmp *= scaling_factor;
            std::get<6>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<7, ContainerOverSubrelations>;
            auto tmp = (new_term.instr_fetching_op6 -
                        new_term.instr_fetching_sel_op_dc_1 *
                            (new_term.instr_fetching_bd13 * FF(256) + new_term.instr_fetching_bd14 * FF(1)));
            tmp *= scaling_factor;
            std::get<7>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<8, ContainerOverSubrelations>;
            auto tmp = (new_term.instr_fetching_op7 -
                        new_term.instr_fetching_sel_op_dc_1 *
                            (new_term.instr_fetching_bd15 * FF(256) + new_term.instr_fetching_bd16 * FF(1)));
            tmp *= scaling_factor;
            std::get<8>(evals) += typename Accumulator::View(tmp);
        }
    }
};

template <typename FF> class instr_fetching : public Relation<instr_fetchingImpl<FF>> {
  public:
    static constexpr const std::string_view NAME = "instr_fetching";

    static std::string get_subrelation_label(size_t index)
    {
        switch (index) {
        case 1:
            return "INDIRECT_BYTES_DECOMPOSITION";
        case 2:
            return "OP1_BYTES_DECOMPOSITION";
        case 3:
            return "OP2_BYTES_DECOMPOSITION";
        case 4:
            return "OP3_BYTES_DECOMPOSITION";
        case 5:
            return "OP4_BYTES_DECOMPOSITION";
        case 6:
            return "OP5_BYTES_DECOMPOSITION";
        case 7:
            return "OP6_BYTES_DECOMPOSITION";
        case 8:
            return "OP7_BYTES_DECOMPOSITION";
        }
        return std::to_string(index);
    }

    // Subrelation indices constants, to be used in tests.
    static constexpr size_t SR_INDIRECT_BYTES_DECOMPOSITION = 1;
    static constexpr size_t SR_OP1_BYTES_DECOMPOSITION = 2;
    static constexpr size_t SR_OP2_BYTES_DECOMPOSITION = 3;
    static constexpr size_t SR_OP3_BYTES_DECOMPOSITION = 4;
    static constexpr size_t SR_OP4_BYTES_DECOMPOSITION = 5;
    static constexpr size_t SR_OP5_BYTES_DECOMPOSITION = 6;
    static constexpr size_t SR_OP6_BYTES_DECOMPOSITION = 7;
    static constexpr size_t SR_OP7_BYTES_DECOMPOSITION = 8;
};

} // namespace bb::avm2