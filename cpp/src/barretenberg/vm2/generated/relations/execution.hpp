// AUTOGENERATED FILE
#pragma once

#include "barretenberg/relations/relation_parameters.hpp"
#include "barretenberg/relations/relation_types.hpp"

namespace bb::avm2 {

template <typename FF_> class executionImpl {
  public:
    using FF = FF_;

    static constexpr std::array<size_t, 5> SUBRELATION_PARTIAL_LENGTHS = { 3, 3, 4, 4, 3 };

    template <typename ContainerOverSubrelations, typename AllEntities>
    void static accumulate(ContainerOverSubrelations& evals,
                           const AllEntities& new_term,
                           [[maybe_unused]] const RelationParameters<FF>&,
                           [[maybe_unused]] const FF& scaling_factor)
    {

        {
            using Accumulator = typename std::tuple_element_t<0, ContainerOverSubrelations>;
            auto tmp = (new_term.execution_sel * (FF(1) - new_term.execution_sel));
            tmp *= scaling_factor;
            std::get<0>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<1, ContainerOverSubrelations>;
            auto tmp = (new_term.execution_last * (FF(1) - new_term.execution_last));
            tmp *= scaling_factor;
            std::get<1>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<2, ContainerOverSubrelations>;
            auto tmp =
                (new_term.execution_sel * ((FF(1) - new_term.execution_sel_shift) * (FF(1) - new_term.execution_last)));
            tmp *= scaling_factor;
            std::get<2>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<3, ContainerOverSubrelations>;
            auto tmp = (((FF(1) - new_term.precomputed_first_row) * (FF(1) - new_term.execution_sel)) *
                        new_term.execution_sel_shift);
            tmp *= scaling_factor;
            std::get<3>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<4, ContainerOverSubrelations>;
            auto tmp = (new_term.execution_last * new_term.execution_sel_shift);
            tmp *= scaling_factor;
            std::get<4>(evals) += typename Accumulator::View(tmp);
        }
    }
};

template <typename FF> class execution : public Relation<executionImpl<FF>> {
  public:
    static constexpr const char* NAME = "execution";

    static std::string get_subrelation_label(size_t index)
    {
        switch (index) {
        case 2:
            return "TRACE_CONTINUITY_1";
        case 3:
            return "TRACE_CONTINUITY_2";
        case 4:
            return "LAST_IS_LAST";
        }
        return std::to_string(index);
    }

    // Subrelation indices constants, to be used in tests.
    static constexpr size_t SR_TRACE_CONTINUITY_1 = 2;
    static constexpr size_t SR_TRACE_CONTINUITY_2 = 3;
    static constexpr size_t SR_LAST_IS_LAST = 4;
};

} // namespace bb::avm2