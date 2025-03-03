// AUTOGENERATED FILE
#pragma once

#include <string_view>

#include "barretenberg/relations/relation_parameters.hpp"
#include "barretenberg/relations/relation_types.hpp"

namespace bb::avm2 {

template <typename FF_> class eccImpl {
  public:
    using FF = FF_;

    static constexpr std::array<size_t, 18> SUBRELATION_PARTIAL_LENGTHS = { 3, 3, 3, 3, 3, 3, 3, 3, 5,
                                                                            3, 5, 3, 6, 5, 6, 8, 8, 3 };

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        const auto& new_term = in;
        return (new_term.ecc_sel).is_zero();
    }

    template <typename ContainerOverSubrelations, typename AllEntities>
    void static accumulate(ContainerOverSubrelations& evals,
                           const AllEntities& new_term,
                           [[maybe_unused]] const RelationParameters<FF>&,
                           [[maybe_unused]] const FF& scaling_factor)
    {
        const auto ecc_INFINITY_X = FF(0);
        const auto ecc_INFINITY_Y = FF(0);
        const auto ecc_X_DIFF = (new_term.ecc_q_x - new_term.ecc_p_x);
        const auto ecc_Y_DIFF = (new_term.ecc_q_y - new_term.ecc_p_y);
        const auto ecc_INFINITY_PRED = new_term.ecc_x_match * (FF(1) - new_term.ecc_y_match);
        const auto ecc_BOTH_INF = new_term.ecc_p_is_inf * new_term.ecc_q_is_inf;
        const auto ecc_BOTH_NON_INF = (FF(1) - new_term.ecc_p_is_inf) * (FF(1) - new_term.ecc_q_is_inf);
        const auto ecc_COMPUTED_R_X =
            ((new_term.ecc_lambda * new_term.ecc_lambda - new_term.ecc_p_x) - new_term.ecc_q_x);
        const auto ecc_COMPUTED_R_Y = (new_term.ecc_lambda * (new_term.ecc_p_x - new_term.ecc_r_x) - new_term.ecc_p_y);
        const auto ecc_EITHER_INF = ((new_term.ecc_p_is_inf + new_term.ecc_q_is_inf) - FF(2) * ecc_BOTH_INF);
        const auto ecc_USE_COMPUTED_RESULT =
            (FF(1) - new_term.ecc_p_is_inf) * (FF(1) - new_term.ecc_q_is_inf) * (FF(1) - ecc_INFINITY_PRED);

        {
            using Accumulator = typename std::tuple_element_t<0, ContainerOverSubrelations>;
            auto tmp = new_term.ecc_sel * (FF(1) - new_term.ecc_sel);
            tmp *= scaling_factor;
            std::get<0>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<1, ContainerOverSubrelations>;
            auto tmp = new_term.ecc_double_op * (FF(1) - new_term.ecc_double_op);
            tmp *= scaling_factor;
            std::get<1>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<2, ContainerOverSubrelations>;
            auto tmp = new_term.ecc_add_op * (FF(1) - new_term.ecc_add_op);
            tmp *= scaling_factor;
            std::get<2>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<3, ContainerOverSubrelations>;
            auto tmp = (new_term.ecc_sel - (new_term.ecc_double_op + new_term.ecc_add_op + ecc_INFINITY_PRED));
            tmp *= scaling_factor;
            std::get<3>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<4, ContainerOverSubrelations>;
            auto tmp = new_term.ecc_p_is_inf * (FF(1) - new_term.ecc_p_is_inf);
            tmp *= scaling_factor;
            std::get<4>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<5, ContainerOverSubrelations>;
            auto tmp = new_term.ecc_q_is_inf * (FF(1) - new_term.ecc_q_is_inf);
            tmp *= scaling_factor;
            std::get<5>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<6, ContainerOverSubrelations>;
            auto tmp = new_term.ecc_r_is_inf * (FF(1) - new_term.ecc_r_is_inf);
            tmp *= scaling_factor;
            std::get<6>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<7, ContainerOverSubrelations>;
            auto tmp = new_term.ecc_x_match * (FF(1) - new_term.ecc_x_match);
            tmp *= scaling_factor;
            std::get<7>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<8, ContainerOverSubrelations>;
            auto tmp =
                new_term.ecc_sel *
                ((ecc_X_DIFF * (new_term.ecc_x_match * (FF(1) - new_term.ecc_inv_x_diff) + new_term.ecc_inv_x_diff) -
                  FF(1)) +
                 new_term.ecc_x_match);
            tmp *= scaling_factor;
            std::get<8>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<9, ContainerOverSubrelations>;
            auto tmp = new_term.ecc_y_match * (FF(1) - new_term.ecc_y_match);
            tmp *= scaling_factor;
            std::get<9>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<10, ContainerOverSubrelations>;
            auto tmp =
                new_term.ecc_sel *
                ((ecc_Y_DIFF * (new_term.ecc_y_match * (FF(1) - new_term.ecc_inv_y_diff) + new_term.ecc_inv_y_diff) -
                  FF(1)) +
                 new_term.ecc_y_match);
            tmp *= scaling_factor;
            std::get<10>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<11, ContainerOverSubrelations>;
            auto tmp = (new_term.ecc_double_op - new_term.ecc_x_match * new_term.ecc_y_match);
            tmp *= scaling_factor;
            std::get<11>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<12, ContainerOverSubrelations>;
            auto tmp = new_term.ecc_sel *
                       (new_term.ecc_result_infinity - (ecc_INFINITY_PRED * ecc_BOTH_NON_INF + ecc_BOTH_INF));
            tmp *= scaling_factor;
            std::get<12>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<13, ContainerOverSubrelations>;
            auto tmp = (FF(1) - new_term.ecc_result_infinity) * new_term.ecc_double_op *
                       (FF(2) * new_term.ecc_p_y * new_term.ecc_inv_2_p_y - FF(1));
            tmp *= scaling_factor;
            std::get<13>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<14, ContainerOverSubrelations>;
            auto tmp = new_term.ecc_sel *
                       (new_term.ecc_lambda -
                        (new_term.ecc_double_op * FF(3) * new_term.ecc_p_x * new_term.ecc_p_x * new_term.ecc_inv_2_p_y +
                         new_term.ecc_add_op * ecc_Y_DIFF * new_term.ecc_inv_x_diff));
            tmp *= scaling_factor;
            std::get<14>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<15, ContainerOverSubrelations>;
            auto tmp =
                new_term.ecc_sel * (((new_term.ecc_r_x - ecc_EITHER_INF * (new_term.ecc_p_is_inf * new_term.ecc_q_x +
                                                                           new_term.ecc_q_is_inf * new_term.ecc_p_x)) -
                                     new_term.ecc_result_infinity * ecc_INFINITY_X) -
                                    ecc_USE_COMPUTED_RESULT * ecc_COMPUTED_R_X);
            tmp *= scaling_factor;
            std::get<15>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<16, ContainerOverSubrelations>;
            auto tmp =
                new_term.ecc_sel * (((new_term.ecc_r_y - ecc_EITHER_INF * (new_term.ecc_p_is_inf * new_term.ecc_q_y +
                                                                           new_term.ecc_q_is_inf * new_term.ecc_p_y)) -
                                     new_term.ecc_result_infinity * ecc_INFINITY_Y) -
                                    ecc_USE_COMPUTED_RESULT * ecc_COMPUTED_R_Y);
            tmp *= scaling_factor;
            std::get<16>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<17, ContainerOverSubrelations>;
            auto tmp = new_term.ecc_sel * (new_term.ecc_r_is_inf - new_term.ecc_result_infinity);
            tmp *= scaling_factor;
            std::get<17>(evals) += typename Accumulator::View(tmp);
        }
    }
};

template <typename FF> class ecc : public Relation<eccImpl<FF>> {
  public:
    static constexpr const std::string_view NAME = "ecc";

    static std::string get_subrelation_label(size_t index)
    {
        switch (index) {
        case 11:
            return "DOUBLE_PRED";
        case 12:
            return "INFINITY_RESULT";
        case 14:
            return "COMPUTED_LAMBDA";
        case 15:
            return "OUTPUT_X_COORD";
        case 16:
            return "OUTPUT_Y_COORD";
        case 17:
            return "OUTPUT_INF_FLAG";
        }
        return std::to_string(index);
    }

    // Subrelation indices constants, to be used in tests.
    static constexpr size_t SR_DOUBLE_PRED = 11;
    static constexpr size_t SR_INFINITY_RESULT = 12;
    static constexpr size_t SR_COMPUTED_LAMBDA = 14;
    static constexpr size_t SR_OUTPUT_X_COORD = 15;
    static constexpr size_t SR_OUTPUT_Y_COORD = 16;
    static constexpr size_t SR_OUTPUT_INF_FLAG = 17;
};

} // namespace bb::avm2