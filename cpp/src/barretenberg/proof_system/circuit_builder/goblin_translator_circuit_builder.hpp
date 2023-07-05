#pragma once
/**
 * @file goblin_translator_builder.hpp
 * @author @Rumata888
 * @brief Circuit Logic generation for Goblin Plonk translator (checks equivalence of Queues/Transcripts for ECCVM and
 * Recursive Circuits)
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "barretenberg/proof_system/arithmetization/arithmetization.hpp"
#include "barretenberg/ecc/curves/bn254/fq.hpp"
#include "circuit_builder_base.hpp"
namespace proof_system {
class GoblinTranslatorCircuitBuilder : CircuitBuilderBase<arithmetization::GoblinTranslator> {
    using Fr = barretenberg::fr;
    using Fp = barretenberg::fq;

  public:
    void create_add_gate(const add_triple&) override{};
    void create_mul_gate(const mul_triple&) override{};
    void create_bool_gate(const uint32_t) override{};
    void create_poly_gate(const poly_triple&) override{};
    size_t get_num_constant_gates() const override { return 0; };

    enum WireIds {
        OP,
        X_LO_Y_HI,
        X_HI_Z_1,
        Y_LO_Z_2,
        P_X_LOWER_LIMBS,
        P_X_LOW_LIMBS_RANGE_CONSTRAINT_0,
        P_X_LOW_LIMBS_RANGE_CONSTRAINT_1,
        P_X_LOW_LIMBS_RANGE_CONSTRAINT_2,
        P_X_LOW_LIMBS_RANGE_CONSTRAINT_3,
        P_X_LOW_LIMBS_RANGE_CONSTRAINT_4,
        P_X_LOW_LIMBS_RANGE_CONSTRAINT_TAIL,
        P_X_HIGH_LIMBS,
        P_X_HIGH_LIMBS_RANGE_CONSTRAINT_0,
        P_X_HIGH_LIMBS_RANGE_CONSTRAINT_1,
        P_X_HIGH_LIMBS_RANGE_CONSTRAINT_2,
        P_X_HIGH_LIMBS_RANGE_CONSTRAINT_3,
        P_X_HIGH_LIMBS_RANGE_CONSTRAINT_4,
        P_X_HIGH_LIMBS_RANGE_CONSTRAINT_TAIL,
        P_Y_LOWER_LIMBS,
        P_Y_LOW_LIMBS_RANGE_CONSTRAINT_0,
        P_Y_LOW_LIMBS_RANGE_CONSTRAINT_1,
        P_Y_LOW_LIMBS_RANGE_CONSTRAINT_2,
        P_Y_LOW_LIMBS_RANGE_CONSTRAINT_3,
        P_Y_LOW_LIMBS_RANGE_CONSTRAINT_4,
        P_Y_LOW_LIMBS_RANGE_CONSTRAINT_TAIL,
        P_Y_HIGH_LIMBS,
        P_Y_HIGH_LIMBS_RANGE_CONSTRAINT_0,
        P_Y_HIGH_LIMBS_RANGE_CONSTRAINT_1,
        P_Y_HIGH_LIMBS_RANGE_CONSTRAINT_2,
        P_Y_HIGH_LIMBS_RANGE_CONSTRAINT_3,
        P_Y_HIGH_LIMBS_RANGE_CONSTRAINT_4,
        P_Y_HIGH_LIMBS_RANGE_CONSTRAINT_TAIL,
    };
    static constexpr size_t NUM_LIMB_BITS = 68;
    static constexpr size_t MICRO_LIMB_BITS = 12;
    static constexpr size_t LEFTOVER_CHUNK_BITS = 8;
    static constexpr size_t NUM_MICRO_LIMBS = 6;
    static constexpr size_t NUM_BINARY_LIMBS = 4;
    static constexpr auto MICRO_SHIFT = uint256_t(1) << MICRO_LIMB_BITS;
    static constexpr auto MAXIMUM_LEFTOVER_LIMB_SIZE = (uint256_t(1) << LEFTOVER_CHUNK_BITS) - 1;
    static constexpr size_t NUM_LAST_LIMB_BITS = Fp::modulus.get_msb() + 1 - 3 * NUM_LIMB_BITS;
    static constexpr auto MAX_LOW_WIDE_LIMB_SIZE = (uint256_t(1) << (NUM_LIMB_BITS * 2)) - 1;
    static constexpr auto MAX_HIGH_WIDE_LIMB_SIZE = (uint256_t(1) << (NUM_LIMB_BITS + NUM_LAST_LIMB_BITS)) - 1;
    static constexpr auto SHIFT_1 = uint256_t(1) << NUM_LIMB_BITS;
    static constexpr std::string_view NAME_STRING = "GoblinTranslatorArithmetization";
    // TODO(kesha): fix size hints
    GoblinTranslatorCircuitBuilder()
        : CircuitBuilderBase({}, 0){};
    GoblinTranslatorCircuitBuilder(const GoblinTranslatorCircuitBuilder& other) = delete;
    GoblinTranslatorCircuitBuilder(GoblinTranslatorCircuitBuilder&& other) noexcept
        : CircuitBuilderBase(std::move(other)){};
    GoblinTranslatorCircuitBuilder& operator=(const GoblinTranslatorCircuitBuilder& other) = delete;
    GoblinTranslatorCircuitBuilder& operator=(GoblinTranslatorCircuitBuilder&& other) noexcept
    {
        CircuitBuilderBase::operator=(std::move(other));
        return *this;
    };
    ~GoblinTranslatorCircuitBuilder() override = default;

    struct accumulation_input {
        Fr op; // Operator
        Fr P_x_lo;
        Fr P_x_hi;
        std::array<Fr, NUM_BINARY_LIMBS + 1> P_x_limbs;
        std::array<std::array<Fr, NUM_MICRO_LIMBS>, NUM_BINARY_LIMBS> P_x_microlimbs;
        Fr P_y_lo;
        Fr P_y_hi;
        std::array<Fr, NUM_BINARY_LIMBS + 1> P_y_limbs;
        std::array<std::array<Fr, NUM_MICRO_LIMBS>, NUM_BINARY_LIMBS> P_y_microlimbs;

        Fr z_1;
        std::array<Fr, 2> z_1_limbs;
        std::array<Fr, 5> z_1_0_decomposition;
        std::array<Fr, 5> z_1_1_decomposition;
        Fr z_2;
        std::array<Fr, 2> z_2_limbs;
        std::array<Fr, 5> z_2_0_decomposition;
        std::array<Fr, 5> z_2_1_decomposition;

        std::array<Fr, 5> A_prev;
        std::array<Fr, 5> A_prev_0_decomposition;
        std::array<Fr, 5> A_prev_1_decomposition;
        std::array<Fr, 5> A_prev_2_decomposition;
        std::array<Fr, 5> A_prev_3_decomposition;
        std::array<Fr, 5> A_current;
        std::array<Fr, 5> A_current_0_decomposition;
        std::array<Fr, 5> A_current_1_decomposition;
        std::array<Fr, 5> A_current_2_decomposition;
        std::array<Fr, 5> A_current_3_decomposition;
        std::array<Fr, 5> Q; // Quotient
        std::array<Fr, 5> Q_0_decomposition;
        std::array<Fr, 5> Q_1_decomposition;
        std::array<Fr, 5> Q_2_decomposition;
        std::array<Fr, 5> Q_3_decomposition;
    };
    void create_accumulation_gate(const accumulation_input acc_step)
    {
        // The first wires OpQueue/Transcript wires
        ASSERT(uint256_t(acc_step.op) < 4);
        auto& op_wire = std::get<WireIds::OP>(wires);
        op_wire.push_back(add_variable(acc_step.op));
        op_wire.push_back(zero_idx);

        auto insert_pair_into_wire = [this](WireIds wire_index, Fr first, Fr second) {
            auto& current_wire = wires[wire_index];
            current_wire.push_back(add_variable(first));
            current_wire.push_back(add_variable(second));
        };

        ASSERT(uint256_t(acc_step.P_x_lo) <= MAX_LOW_WIDE_LIMB_SIZE);
        ASSERT(uint256_t(acc_step.P_y_hi) <= MAX_HIGH_WIDE_LIMB_SIZE);
        insert_pair_into_wire(WireIds::X_LO_Y_HI, acc_step.P_x_lo, acc_step.P_y_hi);

        ASSERT(uint256_t(acc_step.P_x_hi) <= MAX_HIGH_WIDE_LIMB_SIZE);
        ASSERT(uint256_t(acc_step.z_1) <= MAX_LOW_WIDE_LIMB_SIZE);
        insert_pair_into_wire(WireIds::X_HI_Z_1, acc_step.P_x_hi, acc_step.z_1);

        ASSERT(uint256_t(acc_step.P_y_lo) <= MAX_LOW_WIDE_LIMB_SIZE);
        ASSERT(uint256_t(acc_step.z_2) <= MAX_LOW_WIDE_LIMB_SIZE);
        insert_pair_into_wire(WireIds::Y_LO_Z_2, acc_step.P_y_lo, acc_step.z_2);

        // The P_x limb wires

        ASSERT(acc_step.P_x_lo == (acc_step.P_x_limbs[0] + acc_step.P_x_limbs[1] * SHIFT_1));
        ASSERT(acc_step.P_x_hi == (acc_step.P_x_limbs[2] + acc_step.P_x_limbs[3] * SHIFT_1));
        ASSERT(acc_step.P_y_lo == (acc_step.P_y_limbs[0] + acc_step.P_y_limbs[1] * SHIFT_1));
        ASSERT(acc_step.P_y_hi == (acc_step.P_y_limbs[2] + acc_step.P_y_limbs[3] * SHIFT_1));
        ASSERT(uint256_t(acc_step.P_x_limbs[0]) < SHIFT_1);
        ASSERT(uint256_t(acc_step.P_x_limbs[1]) < SHIFT_1);
        ASSERT(uint256_t(acc_step.P_x_limbs[2]) < SHIFT_1);
        ASSERT(uint256_t(acc_step.P_x_limbs[3]) < ((uint256_t(1) << NUM_LAST_LIMB_BITS)));
        ASSERT(uint256_t(acc_step.P_y_limbs[0]) < SHIFT_1);
        ASSERT(uint256_t(acc_step.P_y_limbs[1]) < SHIFT_1);
        ASSERT(uint256_t(acc_step.P_y_limbs[2]) < SHIFT_1);
        ASSERT(uint256_t(acc_step.P_y_limbs[3]) < ((uint256_t(1) << NUM_LAST_LIMB_BITS)));

        // TODO(kesha): Continue mini-refactor
        auto& p_x_0_p_x_1_wire = std::get<WireIds::P_X_LOWER_LIMBS>(wires);
        p_x_0_p_x_1_wire.push_back(add_variable(acc_step.P_x_limbs[0]));
        p_x_0_p_x_1_wire.push_back(add_variable(acc_step.P_x_limbs[1]));
        auto& p_x_2_p_x_3_wire = std::get<WireIds::P_X_HIGH_LIMBS>(wires);
        p_x_2_p_x_3_wire.push_back(add_variable(acc_step.P_x_limbs[2]));
        p_x_2_p_x_3_wire.push_back(add_variable(acc_step.P_x_limbs[3]));
        auto& p_y_0_p_y_1_wire = std::get<WireIds::P_X_LOWER_LIMBS>(wires);
        p_y_0_p_y_1_wire.push_back(add_variable(acc_step.P_x_limbs[0]));
        p_y_0_p_y_1_wire.push_back(add_variable(acc_step.P_x_limbs[1]));
        auto& p_y_2_p_y_3_wire = std::get<WireIds::P_X_HIGH_LIMBS>(wires);
        p_y_2_p_y_3_wire.push_back(add_variable(acc_step.P_x_limbs[2]));
        p_y_2_p_y_3_wire.push_back(add_variable(acc_step.P_x_limbs[3]));

        for (size_t limb_index = 0; limb_index < (NUM_BINARY_LIMBS); limb_index++) {
            for (size_t micro_limb_index = 0; micro_limb_index < (micro_limb_index - 1); micro_limb_index++) {
                ASSERT(uint256_t(acc_step.P_x_microlimbs[limb_index][micro_limb_index]) < MICRO_SHIFT);
            }
        }
        auto lay_limbs_in_row = [this]<size_t array_size>(std::array<Fr, array_size> input, WireIds starting_wire) {
            for (size_t i = 0; i < array_size; i++) {
                wires[starting_wire + i].push_back(add_variable(input[i]));
            }
        };
        lay_limbs_in_row(acc_step.P_x_microlimbs[0], WireIds::P_X_LOW_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.P_x_microlimbs[1], WireIds::P_X_LOW_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.P_x_microlimbs[2], WireIds::P_X_HIGH_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.P_x_microlimbs[3], WireIds::P_X_HIGH_LIMBS_RANGE_CONSTRAINT_0);
        num_gates += 2;
    }
    bool check_circuit()
    {
        auto& x_lo_y_hi_wire = std::get<WireIds::X_LO_Y_HI>(wires);
        auto& x_hi_z_1_wire = std::get<WireIds::X_HI_Z_1>(wires);
        auto& p_x_0_p_x_1_wire = std::get<WireIds::P_X_LOWER_LIMBS>(wires);
        auto& p_x_2_p_x_3_wire = std::get<WireIds::P_X_HIGH_LIMBS>(wires);

        auto get_sequential_micro_chunks = [this](size_t gate_index, WireIds starting_wire_index, size_t chunk_count) {
            std::vector<Fr> chunks;
            for (size_t i = starting_wire_index; i < starting_wire_index + chunk_count; i++) {
                chunks.push_back(get_variable(wires[i][gate_index]));
            }
            return chunks;
        };
        auto accumulate_limb_from_micro_chunks = [](const std::vector<Fr>& chunks) {
            Fr mini_accumulator(0);
            for (auto it = chunks.end(); it != chunks.begin();) {
                --it;
                mini_accumulator = mini_accumulator * MICRO_SHIFT + *it;
            }
            return mini_accumulator;
        };
        for (size_t i = 0; i < num_gates; i++) {
            if (!(i & 1)) {
                Fr p_x_0 = get_variable(p_x_0_p_x_1_wire[i]);
                Fr p_x_1 = get_variable(p_x_0_p_x_1_wire[i + 1]);
                Fr p_x_lo = get_variable(x_lo_y_hi_wire[i]);
                Fr p_x_2 = get_variable(p_x_2_p_x_3_wire[i]);
                Fr p_x_3 = get_variable(p_x_2_p_x_3_wire[i + 1]);
                Fr p_x_hi = get_variable(x_hi_z_1_wire[i]);

                // These need to be range constrained, but that logic is not present yet
                auto p_x_0_chunks =
                    get_sequential_micro_chunks(i, WireIds::P_X_LOW_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS);

                auto p_x_1_chunks =
                    get_sequential_micro_chunks(i + 1, WireIds::P_X_LOW_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS);
                auto p_x_2_chunks =
                    get_sequential_micro_chunks(i, WireIds::P_X_HIGH_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS);
                auto p_x_3_chunks =
                    get_sequential_micro_chunks(i + 1, WireIds::P_X_HIGH_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS);

                if ((p_x_0 + Fr(SHIFT_1) * p_x_1) != p_x_lo) {
                    return false;
                }
                if ((p_x_2 + Fr(SHIFT_1) * p_x_3) != p_x_hi) {
                    return false;
                }

                if (p_x_0 != accumulate_limb_from_micro_chunks(p_x_0_chunks)) {
                    return false;
                }
                if (p_x_1 != accumulate_limb_from_micro_chunks(p_x_1_chunks)) {
                    return false;
                }
                if (p_x_2 != accumulate_limb_from_micro_chunks(p_x_2_chunks)) {
                    return false;
                }
                if (p_x_3 != accumulate_limb_from_micro_chunks(p_x_3_chunks)) {
                    return false;
                }
            }
        }
        return true;
    }
};
} // namespace proof_system