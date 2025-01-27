// AUTOGENERATED FILE
#pragma once

#include <string_view>

#include "barretenberg/relations/relation_parameters.hpp"
#include "barretenberg/relations/relation_types.hpp"

namespace bb::avm {

template <typename FF_> class poseidon2_fullImpl {
  public:
    using FF = FF_;

    static constexpr std::array<size_t, 20> SUBRELATION_PARTIAL_LENGTHS = { 3, 2, 3, 4, 4, 3, 3, 3, 5, 3,
                                                                            3, 3, 3, 6, 3, 6, 3, 6, 3, 6 };

    template <typename ContainerOverSubrelations, typename AllEntities>
    void static accumulate(ContainerOverSubrelations& evals,
                           const AllEntities& new_term,
                           [[maybe_unused]] const RelationParameters<FF>&,
                           [[maybe_unused]] const FF& scaling_factor)
    {
        const auto poseidon2_full_TWOPOW64 = FF(uint256_t{ 0UL, 1UL, 0UL, 0UL });
        const auto poseidon2_full_IV = (poseidon2_full_TWOPOW64 * new_term.poseidon2_full_input_len);
        const auto poseidon2_full_PADDED_LEN = (new_term.poseidon2_full_input_len + new_term.poseidon2_full_padding);
        const auto poseidon2_full_NEXT_INPUT_IS_PREV_OUTPUT_SEL =
            ((new_term.poseidon2_full_execute_poseidon_perm_shift * (FF(1) - new_term.poseidon2_full_start_poseidon)) *
             new_term.poseidon2_full_execute_poseidon_perm);

        {
            using Accumulator = typename std::tuple_element_t<0, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_sel_poseidon * (FF(1) - new_term.poseidon2_full_sel_poseidon));
            tmp *= scaling_factor;
            std::get<0>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<1, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_sel_poseidon -
                        (new_term.poseidon2_full_execute_poseidon_perm + new_term.poseidon2_full_end_poseidon));
            tmp *= scaling_factor;
            std::get<1>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<2, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_start_poseidon * (FF(1) - new_term.poseidon2_full_start_poseidon));
            tmp *= scaling_factor;
            std::get<2>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<3, ContainerOverSubrelations>;
            auto tmp = ((new_term.poseidon2_full_sel_poseidon_shift * (FF(1) - new_term.main_sel_first)) *
                        (new_term.poseidon2_full_start_poseidon_shift - new_term.poseidon2_full_end_poseidon));
            tmp *= scaling_factor;
            std::get<3>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<4, ContainerOverSubrelations>;
            auto tmp = ((new_term.poseidon2_full_padding * (new_term.poseidon2_full_padding - FF(1))) *
                        (new_term.poseidon2_full_padding - FF(2)));
            tmp *= scaling_factor;
            std::get<4>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<5, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_start_poseidon *
                        (((new_term.poseidon2_full_num_perm_rounds_rem + FF(1)) * FF(3)) - poseidon2_full_PADDED_LEN));
            tmp *= scaling_factor;
            std::get<5>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<6, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_end_poseidon * (FF(1) - new_term.poseidon2_full_end_poseidon));
            tmp *= scaling_factor;
            std::get<6>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<7, ContainerOverSubrelations>;
            auto tmp =
                (new_term.poseidon2_full_end_poseidon * (new_term.poseidon2_full_output - new_term.poseidon2_full_b_0));
            tmp *= scaling_factor;
            std::get<7>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<8, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_sel_poseidon *
                        (((new_term.poseidon2_full_num_perm_rounds_rem *
                           ((new_term.poseidon2_full_end_poseidon *
                             (FF(1) - new_term.poseidon2_full_num_perm_rounds_rem_inv)) +
                            new_term.poseidon2_full_num_perm_rounds_rem_inv)) -
                          FF(1)) +
                         new_term.poseidon2_full_end_poseidon));
            tmp *= scaling_factor;
            std::get<8>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<9, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_execute_poseidon_perm *
                        (FF(1) - new_term.poseidon2_full_execute_poseidon_perm));
            tmp *= scaling_factor;
            std::get<9>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<10, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_sel_poseidon * ((FF(1) - new_term.poseidon2_full_end_poseidon) -
                                                                new_term.poseidon2_full_execute_poseidon_perm));
            tmp *= scaling_factor;
            std::get<10>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<11, ContainerOverSubrelations>;
            auto tmp =
                (new_term.poseidon2_full_execute_poseidon_perm *
                 ((new_term.poseidon2_full_num_perm_rounds_rem_shift - new_term.poseidon2_full_num_perm_rounds_rem) +
                  FF(1)));
            tmp *= scaling_factor;
            std::get<11>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<12, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_start_poseidon *
                        (new_term.poseidon2_full_a_0 - new_term.poseidon2_full_input_0));
            tmp *= scaling_factor;
            std::get<12>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<13, ContainerOverSubrelations>;
            auto tmp = ((new_term.poseidon2_full_sel_poseidon * poseidon2_full_NEXT_INPUT_IS_PREV_OUTPUT_SEL) *
                        ((new_term.poseidon2_full_a_0_shift - new_term.poseidon2_full_b_0) -
                         new_term.poseidon2_full_input_0_shift));
            tmp *= scaling_factor;
            std::get<13>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<14, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_start_poseidon *
                        (new_term.poseidon2_full_a_1 - new_term.poseidon2_full_input_1));
            tmp *= scaling_factor;
            std::get<14>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<15, ContainerOverSubrelations>;
            auto tmp = ((new_term.poseidon2_full_sel_poseidon * poseidon2_full_NEXT_INPUT_IS_PREV_OUTPUT_SEL) *
                        ((new_term.poseidon2_full_a_1_shift - new_term.poseidon2_full_b_1) -
                         new_term.poseidon2_full_input_1_shift));
            tmp *= scaling_factor;
            std::get<15>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<16, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_start_poseidon *
                        (new_term.poseidon2_full_a_2 - new_term.poseidon2_full_input_2));
            tmp *= scaling_factor;
            std::get<16>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<17, ContainerOverSubrelations>;
            auto tmp = ((new_term.poseidon2_full_sel_poseidon * poseidon2_full_NEXT_INPUT_IS_PREV_OUTPUT_SEL) *
                        ((new_term.poseidon2_full_a_2_shift - new_term.poseidon2_full_b_2) -
                         new_term.poseidon2_full_input_2_shift));
            tmp *= scaling_factor;
            std::get<17>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<18, ContainerOverSubrelations>;
            auto tmp = (new_term.poseidon2_full_start_poseidon * (new_term.poseidon2_full_a_3 - poseidon2_full_IV));
            tmp *= scaling_factor;
            std::get<18>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<19, ContainerOverSubrelations>;
            auto tmp = ((new_term.poseidon2_full_sel_poseidon * poseidon2_full_NEXT_INPUT_IS_PREV_OUTPUT_SEL) *
                        (new_term.poseidon2_full_a_3_shift - new_term.poseidon2_full_b_3));
            tmp *= scaling_factor;
            std::get<19>(evals) += typename Accumulator::View(tmp);
        }
    }
};

template <typename FF> class poseidon2_full : public Relation<poseidon2_fullImpl<FF>> {
  public:
    static constexpr const std::string_view NAME = "poseidon2_full";

    static std::string get_subrelation_label(size_t index)
    {
        switch (index) {}
        return std::to_string(index);
    }
};

} // namespace bb::avm