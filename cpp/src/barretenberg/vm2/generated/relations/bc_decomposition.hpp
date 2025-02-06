// AUTOGENERATED FILE
#pragma once

#include <string_view>

#include "barretenberg/relations/relation_parameters.hpp"
#include "barretenberg/relations/relation_types.hpp"

namespace bb::avm2 {

template <typename FF_> class bc_decompositionImpl {
  public:
    using FF = FF_;

    static constexpr std::array<size_t, 39> SUBRELATION_PARTIAL_LENGTHS = { 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                                                            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                                                            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        const auto& new_term = in;
        return (new_term.bc_decomposition_sel).is_zero();
    }

    template <typename ContainerOverSubrelations, typename AllEntities>
    void static accumulate(ContainerOverSubrelations& evals,
                           const AllEntities& new_term,
                           [[maybe_unused]] const RelationParameters<FF>&,
                           [[maybe_unused]] const FF& scaling_factor)
    {
        const auto bc_decomposition_WINDOW_SIZE = FF(36);

        {
            using Accumulator = typename std::tuple_element_t<0, ContainerOverSubrelations>;
            auto tmp =
                new_term.bc_decomposition_last_of_contract * (FF(1) - new_term.bc_decomposition_last_of_contract);
            tmp *= scaling_factor;
            std::get<0>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<1, ContainerOverSubrelations>;
            auto tmp = new_term.bc_decomposition_sel_overflow_correction_needed *
                       (FF(1) - new_term.bc_decomposition_sel_overflow_correction_needed);
            tmp *= scaling_factor;
            std::get<1>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<2, ContainerOverSubrelations>;
            auto tmp = new_term.bc_decomposition_sel *
                       ((FF(1) - new_term.bc_decomposition_sel_overflow_correction_needed) *
                            (new_term.bc_decomposition_bytes_to_read - bc_decomposition_WINDOW_SIZE) +
                        new_term.bc_decomposition_sel_overflow_correction_needed *
                            (new_term.bc_decomposition_bytes_to_read - new_term.bc_decomposition_bytes_remaining));
            tmp *= scaling_factor;
            std::get<2>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<3, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_to_read_unary -
                        new_term.bc_decomposition_sel * (FF(1) + new_term.bc_decomposition_sel_pc_plus_1 * FF(2) +
                                                         new_term.bc_decomposition_sel_pc_plus_2 * FF(4) +
                                                         new_term.bc_decomposition_sel_pc_plus_3 * FF(8) +
                                                         new_term.bc_decomposition_sel_pc_plus_4 * FF(16) +
                                                         new_term.bc_decomposition_sel_pc_plus_5 * FF(32) +
                                                         new_term.bc_decomposition_sel_pc_plus_6 * FF(64) +
                                                         new_term.bc_decomposition_sel_pc_plus_7 * FF(128) +
                                                         new_term.bc_decomposition_sel_pc_plus_8 * FF(256) +
                                                         new_term.bc_decomposition_sel_pc_plus_9 * FF(512) +
                                                         new_term.bc_decomposition_sel_pc_plus_10 * FF(1024) +
                                                         new_term.bc_decomposition_sel_pc_plus_11 * FF(2048) +
                                                         new_term.bc_decomposition_sel_pc_plus_12 * FF(4096) +
                                                         new_term.bc_decomposition_sel_pc_plus_13 * FF(8192) +
                                                         new_term.bc_decomposition_sel_pc_plus_14 * FF(16384) +
                                                         new_term.bc_decomposition_sel_pc_plus_15 * FF(32768) +
                                                         new_term.bc_decomposition_sel_pc_plus_16 * FF(65536) +
                                                         new_term.bc_decomposition_sel_pc_plus_17 * FF(131072) +
                                                         new_term.bc_decomposition_sel_pc_plus_18 * FF(262144) +
                                                         new_term.bc_decomposition_sel_pc_plus_19 * FF(524288) +
                                                         new_term.bc_decomposition_sel_pc_plus_20 * FF(1048576) +
                                                         new_term.bc_decomposition_sel_pc_plus_21 * FF(2097152) +
                                                         new_term.bc_decomposition_sel_pc_plus_22 * FF(4194304) +
                                                         new_term.bc_decomposition_sel_pc_plus_23 * FF(8388608) +
                                                         new_term.bc_decomposition_sel_pc_plus_24 * FF(16777216) +
                                                         new_term.bc_decomposition_sel_pc_plus_25 * FF(33554432) +
                                                         new_term.bc_decomposition_sel_pc_plus_26 * FF(67108864) +
                                                         new_term.bc_decomposition_sel_pc_plus_27 * FF(134217728) +
                                                         new_term.bc_decomposition_sel_pc_plus_28 * FF(268435456) +
                                                         new_term.bc_decomposition_sel_pc_plus_29 * FF(536870912) +
                                                         new_term.bc_decomposition_sel_pc_plus_30 * FF(1073741824) +
                                                         new_term.bc_decomposition_sel_pc_plus_31 * FF(2147483648UL) +
                                                         new_term.bc_decomposition_sel_pc_plus_32 * FF(4294967296UL) +
                                                         new_term.bc_decomposition_sel_pc_plus_33 * FF(8589934592UL) +
                                                         new_term.bc_decomposition_sel_pc_plus_34 * FF(17179869184UL) +
                                                         new_term.bc_decomposition_sel_pc_plus_35 * FF(34359738368UL)));
            tmp *= scaling_factor;
            std::get<3>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<4, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_1 -
                        new_term.bc_decomposition_sel_pc_plus_1 * new_term.bc_decomposition_bytes_shift);
            tmp *= scaling_factor;
            std::get<4>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<5, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_2 -
                        new_term.bc_decomposition_sel_pc_plus_2 * new_term.bc_decomposition_bytes_pc_plus_1_shift);
            tmp *= scaling_factor;
            std::get<5>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<6, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_3 -
                        new_term.bc_decomposition_sel_pc_plus_3 * new_term.bc_decomposition_bytes_pc_plus_2_shift);
            tmp *= scaling_factor;
            std::get<6>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<7, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_4 -
                        new_term.bc_decomposition_sel_pc_plus_4 * new_term.bc_decomposition_bytes_pc_plus_3_shift);
            tmp *= scaling_factor;
            std::get<7>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<8, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_5 -
                        new_term.bc_decomposition_sel_pc_plus_5 * new_term.bc_decomposition_bytes_pc_plus_4_shift);
            tmp *= scaling_factor;
            std::get<8>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<9, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_6 -
                        new_term.bc_decomposition_sel_pc_plus_6 * new_term.bc_decomposition_bytes_pc_plus_5_shift);
            tmp *= scaling_factor;
            std::get<9>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<10, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_7 -
                        new_term.bc_decomposition_sel_pc_plus_7 * new_term.bc_decomposition_bytes_pc_plus_6_shift);
            tmp *= scaling_factor;
            std::get<10>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<11, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_8 -
                        new_term.bc_decomposition_sel_pc_plus_8 * new_term.bc_decomposition_bytes_pc_plus_7_shift);
            tmp *= scaling_factor;
            std::get<11>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<12, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_9 -
                        new_term.bc_decomposition_sel_pc_plus_9 * new_term.bc_decomposition_bytes_pc_plus_8_shift);
            tmp *= scaling_factor;
            std::get<12>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<13, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_10 -
                        new_term.bc_decomposition_sel_pc_plus_10 * new_term.bc_decomposition_bytes_pc_plus_9_shift);
            tmp *= scaling_factor;
            std::get<13>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<14, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_11 -
                        new_term.bc_decomposition_sel_pc_plus_11 * new_term.bc_decomposition_bytes_pc_plus_10_shift);
            tmp *= scaling_factor;
            std::get<14>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<15, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_12 -
                        new_term.bc_decomposition_sel_pc_plus_12 * new_term.bc_decomposition_bytes_pc_plus_11_shift);
            tmp *= scaling_factor;
            std::get<15>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<16, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_13 -
                        new_term.bc_decomposition_sel_pc_plus_13 * new_term.bc_decomposition_bytes_pc_plus_12_shift);
            tmp *= scaling_factor;
            std::get<16>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<17, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_14 -
                        new_term.bc_decomposition_sel_pc_plus_14 * new_term.bc_decomposition_bytes_pc_plus_13_shift);
            tmp *= scaling_factor;
            std::get<17>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<18, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_15 -
                        new_term.bc_decomposition_sel_pc_plus_15 * new_term.bc_decomposition_bytes_pc_plus_14_shift);
            tmp *= scaling_factor;
            std::get<18>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<19, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_16 -
                        new_term.bc_decomposition_sel_pc_plus_16 * new_term.bc_decomposition_bytes_pc_plus_15_shift);
            tmp *= scaling_factor;
            std::get<19>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<20, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_17 -
                        new_term.bc_decomposition_sel_pc_plus_17 * new_term.bc_decomposition_bytes_pc_plus_16_shift);
            tmp *= scaling_factor;
            std::get<20>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<21, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_18 -
                        new_term.bc_decomposition_sel_pc_plus_18 * new_term.bc_decomposition_bytes_pc_plus_17_shift);
            tmp *= scaling_factor;
            std::get<21>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<22, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_19 -
                        new_term.bc_decomposition_sel_pc_plus_19 * new_term.bc_decomposition_bytes_pc_plus_18_shift);
            tmp *= scaling_factor;
            std::get<22>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<23, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_20 -
                        new_term.bc_decomposition_sel_pc_plus_20 * new_term.bc_decomposition_bytes_pc_plus_19_shift);
            tmp *= scaling_factor;
            std::get<23>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<24, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_21 -
                        new_term.bc_decomposition_sel_pc_plus_21 * new_term.bc_decomposition_bytes_pc_plus_20_shift);
            tmp *= scaling_factor;
            std::get<24>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<25, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_22 -
                        new_term.bc_decomposition_sel_pc_plus_22 * new_term.bc_decomposition_bytes_pc_plus_21_shift);
            tmp *= scaling_factor;
            std::get<25>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<26, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_23 -
                        new_term.bc_decomposition_sel_pc_plus_23 * new_term.bc_decomposition_bytes_pc_plus_22_shift);
            tmp *= scaling_factor;
            std::get<26>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<27, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_24 -
                        new_term.bc_decomposition_sel_pc_plus_24 * new_term.bc_decomposition_bytes_pc_plus_23_shift);
            tmp *= scaling_factor;
            std::get<27>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<28, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_25 -
                        new_term.bc_decomposition_sel_pc_plus_25 * new_term.bc_decomposition_bytes_pc_plus_24_shift);
            tmp *= scaling_factor;
            std::get<28>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<29, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_26 -
                        new_term.bc_decomposition_sel_pc_plus_26 * new_term.bc_decomposition_bytes_pc_plus_25_shift);
            tmp *= scaling_factor;
            std::get<29>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<30, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_27 -
                        new_term.bc_decomposition_sel_pc_plus_27 * new_term.bc_decomposition_bytes_pc_plus_26_shift);
            tmp *= scaling_factor;
            std::get<30>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<31, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_28 -
                        new_term.bc_decomposition_sel_pc_plus_28 * new_term.bc_decomposition_bytes_pc_plus_27_shift);
            tmp *= scaling_factor;
            std::get<31>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<32, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_29 -
                        new_term.bc_decomposition_sel_pc_plus_29 * new_term.bc_decomposition_bytes_pc_plus_28_shift);
            tmp *= scaling_factor;
            std::get<32>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<33, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_30 -
                        new_term.bc_decomposition_sel_pc_plus_30 * new_term.bc_decomposition_bytes_pc_plus_29_shift);
            tmp *= scaling_factor;
            std::get<33>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<34, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_31 -
                        new_term.bc_decomposition_sel_pc_plus_31 * new_term.bc_decomposition_bytes_pc_plus_30_shift);
            tmp *= scaling_factor;
            std::get<34>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<35, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_32 -
                        new_term.bc_decomposition_sel_pc_plus_32 * new_term.bc_decomposition_bytes_pc_plus_31_shift);
            tmp *= scaling_factor;
            std::get<35>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<36, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_33 -
                        new_term.bc_decomposition_sel_pc_plus_33 * new_term.bc_decomposition_bytes_pc_plus_32_shift);
            tmp *= scaling_factor;
            std::get<36>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<37, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_34 -
                        new_term.bc_decomposition_sel_pc_plus_34 * new_term.bc_decomposition_bytes_pc_plus_33_shift);
            tmp *= scaling_factor;
            std::get<37>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<38, ContainerOverSubrelations>;
            auto tmp = (new_term.bc_decomposition_bytes_pc_plus_35 -
                        new_term.bc_decomposition_sel_pc_plus_35 * new_term.bc_decomposition_bytes_pc_plus_34_shift);
            tmp *= scaling_factor;
            std::get<38>(evals) += typename Accumulator::View(tmp);
        }
    }
};

template <typename FF> class bc_decomposition : public Relation<bc_decompositionImpl<FF>> {
  public:
    static constexpr const std::string_view NAME = "bc_decomposition";

    static std::string get_subrelation_label(size_t index)
    {
        switch (index) {
        case 2:
            return "BYTECODE_OVERFLOW_CORRECTION_VALUE";
        case 3:
            return "BYTECODE_UNARY_RECONSTRUCTION";
        }
        return std::to_string(index);
    }

    // Subrelation indices constants, to be used in tests.
    static constexpr size_t SR_BYTECODE_OVERFLOW_CORRECTION_VALUE = 2;
    static constexpr size_t SR_BYTECODE_UNARY_RECONSTRUCTION = 3;
};

} // namespace bb::avm2