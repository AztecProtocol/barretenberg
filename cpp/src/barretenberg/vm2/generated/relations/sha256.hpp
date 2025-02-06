// AUTOGENERATED FILE
#pragma once

#include <string_view>

#include "barretenberg/relations/relation_parameters.hpp"
#include "barretenberg/relations/relation_types.hpp"

namespace bb::avm2 {

template <typename FF_> class sha256Impl {
  public:
    using FF = FF_;

    static constexpr std::array<size_t, 72> SUBRELATION_PARTIAL_LENGTHS = {
        3, 3, 2, 3, 3, 3, 3, 2, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4
    };

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        const auto& new_term = in;
        return (new_term.sha256_sel).is_zero();
    }

    template <typename ContainerOverSubrelations, typename AllEntities>
    void static accumulate(ContainerOverSubrelations& evals,
                           const AllEntities& new_term,
                           [[maybe_unused]] const RelationParameters<FF>&,
                           [[maybe_unused]] const FF& scaling_factor)
    {
        const auto sha256_LAST = new_term.sha256_sel * new_term.sha256_latch;
        const auto sha256_NUM_ROUNDS = FF(64);
        const auto sha256_COMPUTED_W =
            new_term.sha256_helper_w0 + new_term.sha256_w_s_0 + new_term.sha256_helper_w9 + new_term.sha256_w_s_1;
        const auto sha256_TMP_1 = new_term.sha256_h + new_term.sha256_s_1 + new_term.sha256_ch +
                                  new_term.sha256_round_constant + new_term.sha256_w;
        const auto sha256_NEXT_A = new_term.sha256_s_0 + new_term.sha256_maj + sha256_TMP_1;
        const auto sha256_NEXT_E = new_term.sha256_d + sha256_TMP_1;
        const auto sha256_OUT_A = new_term.sha256_a + new_term.sha256_init_a;
        const auto sha256_OUT_B = new_term.sha256_b + new_term.sha256_init_b;
        const auto sha256_OUT_C = new_term.sha256_c + new_term.sha256_init_c;
        const auto sha256_OUT_D = new_term.sha256_d + new_term.sha256_init_d;
        const auto sha256_OUT_E = new_term.sha256_e + new_term.sha256_init_e;
        const auto sha256_OUT_F = new_term.sha256_f + new_term.sha256_init_f;
        const auto sha256_OUT_G = new_term.sha256_g + new_term.sha256_init_g;
        const auto sha256_OUT_H = new_term.sha256_h + new_term.sha256_init_h;

        {
            using Accumulator = typename std::tuple_element_t<0, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_sel * (FF(1) - new_term.sha256_sel);
            tmp *= scaling_factor;
            std::get<0>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<1, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_xor_sel - FF(2));
            tmp *= scaling_factor;
            std::get<1>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<2, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_and_sel;
            tmp *= scaling_factor;
            std::get<2>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<3, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_start * (FF(1) - new_term.sha256_start);
            tmp *= scaling_factor;
            std::get<3>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<4, ContainerOverSubrelations>;
            auto tmp = (new_term.sha256_start_shift - new_term.sha256_latch * new_term.sha256_sel_shift);
            tmp *= scaling_factor;
            std::get<4>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<5, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_latch * (FF(1) - new_term.sha256_latch);
            tmp *= scaling_factor;
            std::get<5>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<6, ContainerOverSubrelations>;
            auto tmp = (new_term.sha256_perform_round - new_term.sha256_sel * (FF(1) - new_term.sha256_latch));
            tmp *= scaling_factor;
            std::get<6>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<7, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_dummy_zero;
            tmp *= scaling_factor;
            std::get<7>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<8, ContainerOverSubrelations>;
            auto tmp = (new_term.sha256_start * (new_term.sha256_rounds_remaining - sha256_NUM_ROUNDS) +
                        new_term.sha256_perform_round *
                            ((new_term.sha256_rounds_remaining - new_term.sha256_rounds_remaining_shift) - FF(1)));
            tmp *= scaling_factor;
            std::get<8>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<9, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_sel *
                       (new_term.sha256_round_count - (sha256_NUM_ROUNDS - new_term.sha256_rounds_remaining));
            tmp *= scaling_factor;
            std::get<9>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<10, ContainerOverSubrelations>;
            auto tmp =
                new_term.sha256_sel * ((new_term.sha256_rounds_remaining *
                                            (new_term.sha256_latch * (FF(1) - new_term.sha256_rounds_remaining_inv) +
                                             new_term.sha256_rounds_remaining_inv) -
                                        FF(1)) +
                                       new_term.sha256_latch);
            tmp *= scaling_factor;
            std::get<10>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<11, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w0_shift - new_term.sha256_helper_w1);
            tmp *= scaling_factor;
            std::get<11>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<12, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w1_shift - new_term.sha256_helper_w2);
            tmp *= scaling_factor;
            std::get<12>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<13, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w2_shift - new_term.sha256_helper_w3);
            tmp *= scaling_factor;
            std::get<13>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<14, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w3_shift - new_term.sha256_helper_w4);
            tmp *= scaling_factor;
            std::get<14>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<15, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w4_shift - new_term.sha256_helper_w5);
            tmp *= scaling_factor;
            std::get<15>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<16, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w5_shift - new_term.sha256_helper_w6);
            tmp *= scaling_factor;
            std::get<16>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<17, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w6_shift - new_term.sha256_helper_w7);
            tmp *= scaling_factor;
            std::get<17>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<18, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w7_shift - new_term.sha256_helper_w8);
            tmp *= scaling_factor;
            std::get<18>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<19, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w8_shift - new_term.sha256_helper_w9);
            tmp *= scaling_factor;
            std::get<19>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<20, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w9_shift - new_term.sha256_helper_w10);
            tmp *= scaling_factor;
            std::get<20>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<21, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w10_shift - new_term.sha256_helper_w11);
            tmp *= scaling_factor;
            std::get<21>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<22, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w11_shift - new_term.sha256_helper_w12);
            tmp *= scaling_factor;
            std::get<22>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<23, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w12_shift - new_term.sha256_helper_w13);
            tmp *= scaling_factor;
            std::get<23>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<24, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w13_shift - new_term.sha256_helper_w14);
            tmp *= scaling_factor;
            std::get<24>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<25, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w14_shift - new_term.sha256_helper_w15);
            tmp *= scaling_factor;
            std::get<25>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<26, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_helper_w15_shift - new_term.sha256_w);
            tmp *= scaling_factor;
            std::get<26>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<27, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       ((new_term.sha256_computed_w_lhs * FF(4294967296UL) + new_term.sha256_computed_w_rhs) -
                        sha256_COMPUTED_W);
            tmp *= scaling_factor;
            std::get<27>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<28, ContainerOverSubrelations>;
            auto tmp =
                new_term.sha256_perform_round *
                (new_term.sha256_w - (new_term.sha256_is_input_round * new_term.sha256_helper_w0 +
                                      (FF(1) - new_term.sha256_is_input_round) * new_term.sha256_computed_w_rhs));
            tmp *= scaling_factor;
            std::get<28>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<29, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_helper_w1 - (new_term.sha256_lhs_w_7 * FF(128) + new_term.sha256_rhs_w_7));
            tmp *= scaling_factor;
            std::get<29>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<30, ContainerOverSubrelations>;
            auto tmp =
                new_term.sha256_perform_round *
                (new_term.sha256_w_15_rotr_7 - (new_term.sha256_rhs_w_7 * FF(33554432) + new_term.sha256_lhs_w_7));
            tmp *= scaling_factor;
            std::get<30>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<31, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_helper_w1 - (new_term.sha256_lhs_w_18 * FF(262144) + new_term.sha256_rhs_w_18));
            tmp *= scaling_factor;
            std::get<31>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<32, ContainerOverSubrelations>;
            auto tmp =
                new_term.sha256_perform_round *
                (new_term.sha256_w_15_rotr_18 - (new_term.sha256_rhs_w_18 * FF(16384) + new_term.sha256_lhs_w_18));
            tmp *= scaling_factor;
            std::get<32>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<33, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_helper_w1 - (new_term.sha256_lhs_w_3 * FF(8) + new_term.sha256_rhs_w_3));
            tmp *= scaling_factor;
            std::get<33>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<34, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_w_15_rshift_3 - new_term.sha256_lhs_w_3);
            tmp *= scaling_factor;
            std::get<34>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<35, ContainerOverSubrelations>;
            auto tmp =
                new_term.sha256_perform_round *
                (new_term.sha256_helper_w14 - (new_term.sha256_lhs_w_17 * FF(131072) + new_term.sha256_rhs_w_17));
            tmp *= scaling_factor;
            std::get<35>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<36, ContainerOverSubrelations>;
            auto tmp =
                new_term.sha256_perform_round *
                (new_term.sha256_w_2_rotr_17 - (new_term.sha256_rhs_w_17 * FF(32768) + new_term.sha256_lhs_w_17));
            tmp *= scaling_factor;
            std::get<36>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<37, ContainerOverSubrelations>;
            auto tmp =
                new_term.sha256_perform_round *
                (new_term.sha256_helper_w14 - (new_term.sha256_lhs_w_19 * FF(524288) + new_term.sha256_rhs_w_19));
            tmp *= scaling_factor;
            std::get<37>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<38, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_w_2_rotr_19 - (new_term.sha256_rhs_w_19 * FF(8192) + new_term.sha256_lhs_w_19));
            tmp *= scaling_factor;
            std::get<38>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<39, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_helper_w14 - (new_term.sha256_lhs_w_10 * FF(1024) + new_term.sha256_rhs_w_10));
            tmp *= scaling_factor;
            std::get<39>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<40, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_w_2_rshift_10 - new_term.sha256_lhs_w_10);
            tmp *= scaling_factor;
            std::get<40>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<41, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_e - (new_term.sha256_lhs_e_6 * FF(64) + new_term.sha256_rhs_e_6));
            tmp *= scaling_factor;
            std::get<41>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<42, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_e_rotr_6 - (new_term.sha256_rhs_e_6 * FF(67108864) + new_term.sha256_lhs_e_6));
            tmp *= scaling_factor;
            std::get<42>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<43, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_e - (new_term.sha256_lhs_e_11 * FF(2048) + new_term.sha256_rhs_e_11));
            tmp *= scaling_factor;
            std::get<43>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<44, ContainerOverSubrelations>;
            auto tmp =
                new_term.sha256_perform_round *
                (new_term.sha256_e_rotr_11 - (new_term.sha256_rhs_e_11 * FF(2097152) + new_term.sha256_lhs_e_11));
            tmp *= scaling_factor;
            std::get<44>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<45, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_e - (new_term.sha256_lhs_e_25 * FF(33554432) + new_term.sha256_rhs_e_25));
            tmp *= scaling_factor;
            std::get<45>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<46, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_e_rotr_25 - (new_term.sha256_rhs_e_25 * FF(128) + new_term.sha256_lhs_e_25));
            tmp *= scaling_factor;
            std::get<46>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<47, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * ((new_term.sha256_e + new_term.sha256_not_e) - FF(4294967295UL));
            tmp *= scaling_factor;
            std::get<47>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<48, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_a - (new_term.sha256_lhs_a_2 * FF(4) + new_term.sha256_rhs_a_2));
            tmp *= scaling_factor;
            std::get<48>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<49, ContainerOverSubrelations>;
            auto tmp =
                new_term.sha256_perform_round *
                (new_term.sha256_a_rotr_2 - (new_term.sha256_rhs_a_2 * FF(1073741824) + new_term.sha256_lhs_a_2));
            tmp *= scaling_factor;
            std::get<49>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<50, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_a - (new_term.sha256_lhs_a_13 * FF(8192) + new_term.sha256_rhs_a_13));
            tmp *= scaling_factor;
            std::get<50>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<51, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_a_rotr_13 - (new_term.sha256_rhs_a_13 * FF(524288) + new_term.sha256_lhs_a_13));
            tmp *= scaling_factor;
            std::get<51>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<52, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_a - (new_term.sha256_lhs_a_22 * FF(4194304) + new_term.sha256_rhs_a_22));
            tmp *= scaling_factor;
            std::get<52>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<53, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       (new_term.sha256_a_rotr_22 - (new_term.sha256_rhs_a_22 * FF(1024) + new_term.sha256_lhs_a_22));
            tmp *= scaling_factor;
            std::get<53>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<54, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       ((new_term.sha256_next_a_lhs * FF(4294967296UL) + new_term.sha256_next_a_rhs) - sha256_NEXT_A);
            tmp *= scaling_factor;
            std::get<54>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<55, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round *
                       ((new_term.sha256_next_e_lhs * FF(4294967296UL) + new_term.sha256_next_e_rhs) - sha256_NEXT_E);
            tmp *= scaling_factor;
            std::get<55>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<56, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_a_shift - new_term.sha256_next_a_rhs);
            tmp *= scaling_factor;
            std::get<56>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<57, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_b_shift - new_term.sha256_a);
            tmp *= scaling_factor;
            std::get<57>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<58, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_c_shift - new_term.sha256_b);
            tmp *= scaling_factor;
            std::get<58>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<59, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_d_shift - new_term.sha256_c);
            tmp *= scaling_factor;
            std::get<59>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<60, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_e_shift - new_term.sha256_next_e_rhs);
            tmp *= scaling_factor;
            std::get<60>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<61, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_f_shift - new_term.sha256_e);
            tmp *= scaling_factor;
            std::get<61>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<62, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_g_shift - new_term.sha256_f);
            tmp *= scaling_factor;
            std::get<62>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<63, ContainerOverSubrelations>;
            auto tmp = new_term.sha256_perform_round * (new_term.sha256_h_shift - new_term.sha256_g);
            tmp *= scaling_factor;
            std::get<63>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<64, ContainerOverSubrelations>;
            auto tmp = sha256_LAST * (sha256_OUT_A -
                                      (new_term.sha256_output_a_lhs * FF(4294967296UL) + new_term.sha256_output_a_rhs));
            tmp *= scaling_factor;
            std::get<64>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<65, ContainerOverSubrelations>;
            auto tmp = sha256_LAST * (sha256_OUT_B -
                                      (new_term.sha256_output_b_lhs * FF(4294967296UL) + new_term.sha256_output_b_rhs));
            tmp *= scaling_factor;
            std::get<65>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<66, ContainerOverSubrelations>;
            auto tmp = sha256_LAST * (sha256_OUT_C -
                                      (new_term.sha256_output_c_lhs * FF(4294967296UL) + new_term.sha256_output_c_rhs));
            tmp *= scaling_factor;
            std::get<66>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<67, ContainerOverSubrelations>;
            auto tmp = sha256_LAST * (sha256_OUT_D -
                                      (new_term.sha256_output_d_lhs * FF(4294967296UL) + new_term.sha256_output_d_rhs));
            tmp *= scaling_factor;
            std::get<67>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<68, ContainerOverSubrelations>;
            auto tmp = sha256_LAST * (sha256_OUT_E -
                                      (new_term.sha256_output_e_lhs * FF(4294967296UL) + new_term.sha256_output_e_rhs));
            tmp *= scaling_factor;
            std::get<68>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<69, ContainerOverSubrelations>;
            auto tmp = sha256_LAST * (sha256_OUT_F -
                                      (new_term.sha256_output_f_lhs * FF(4294967296UL) + new_term.sha256_output_f_rhs));
            tmp *= scaling_factor;
            std::get<69>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<70, ContainerOverSubrelations>;
            auto tmp = sha256_LAST * (sha256_OUT_G -
                                      (new_term.sha256_output_g_lhs * FF(4294967296UL) + new_term.sha256_output_g_rhs));
            tmp *= scaling_factor;
            std::get<70>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<71, ContainerOverSubrelations>;
            auto tmp = sha256_LAST * (sha256_OUT_H -
                                      (new_term.sha256_output_h_lhs * FF(4294967296UL) + new_term.sha256_output_h_rhs));
            tmp *= scaling_factor;
            std::get<71>(evals) += typename Accumulator::View(tmp);
        }
    }
};

template <typename FF> class sha256 : public Relation<sha256Impl<FF>> {
  public:
    static constexpr const std::string_view NAME = "sha256";

    static std::string get_subrelation_label(size_t index)
    {
        switch (index) {}
        return std::to_string(index);
    }
};

} // namespace bb::avm2