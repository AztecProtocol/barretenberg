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
    [[nodiscard]] size_t get_num_constant_gates() const override { return 0; };

    enum WireIds {
        OP,
        X_LO_Y_HI,
        X_HI_Z_1,
        Y_LO_Z_2,
        P_X_LOW_LIMBS,
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
        P_Y_LOW_LIMBS,
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
        Z_LO_LIMBS, // Low limbs of z_1 and z_2
        Z_LO_LIMBS_RANGE_CONSTRAINT_0,
        Z_LO_LIMBS_RANGE_CONSTRAINT_1,
        Z_LO_LIMBS_RANGE_CONSTRAINT_2,
        Z_LO_LIMBS_RANGE_CONSTRAINT_3,
        Z_LO_LIMBS_RANGE_CONSTRAINT_4,
        Z_LO_LIMBS_RANGE_CONSTRAINT_TAIL,
        Z_HI_LIMBS, // Hi Limbs of z_1 and z_2
        Z_HI_LIMBS_RANGE_CONSTRAINT_0,
        Z_HI_LIMBS_RANGE_CONSTRAINT_1,
        Z_HI_LIMBS_RANGE_CONSTRAINT_2,
        Z_HI_LIMBS_RANGE_CONSTRAINT_3,
        Z_HI_LIMBS_RANGE_CONSTRAINT_4,
        Z_HI_LIMBS_RANGE_CONSTRAINT_TAIL,
        ACCUMULATORS_BINARY_LIMBS_0,
        ACCUMULATORS_BINARY_LIMBS_1,
        ACCUMULATORS_BINARY_LIMBS_2,
        ACCUMULATORS_BINARY_LIMBS_3,
        ACCUMULATOR_LO_LIMBS_RANGE_CONSTRAINT_0,
        ACCUMULATOR_LO_LIMBS_RANGE_CONSTRAINT_1,
        ACCUMULATOR_LO_LIMBS_RANGE_CONSTRAINT_2,
        ACCUMULATOR_LO_LIMBS_RANGE_CONSTRAINT_3,
        ACCUMULATOR_LO_LIMBS_RANGE_CONSTRAINT_4,
        ACCUMULATOR_LO_LIMBS_RANGE_CONSTRAINT_TAIL,
        ACCUMULATOR_HI_LIMBS_RANGE_CONSTRAINT_0,
        ACCUMULATOR_HI_LIMBS_RANGE_CONSTRAINT_1,
        ACCUMULATOR_HI_LIMBS_RANGE_CONSTRAINT_2,
        ACCUMULATOR_HI_LIMBS_RANGE_CONSTRAINT_3,
        ACCUMULATOR_HI_LIMBS_RANGE_CONSTRAINT_4,
        ACCUMULATOR_HI_LIMBS_RANGE_CONSTRAINT_TAIL,
    };
    static constexpr size_t NUM_LIMB_BITS = 68;
    static constexpr size_t NUM_Z_LIMBS = 2;
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
        std::array<Fr, NUM_Z_LIMBS> z_1_limbs;
        std::array<std::array<Fr, NUM_MICRO_LIMBS>, NUM_Z_LIMBS> z_1_microlimbs;
        Fr z_2;
        std::array<Fr, NUM_Z_LIMBS> z_2_limbs;
        std::array<std::array<Fr, NUM_MICRO_LIMBS>, NUM_Z_LIMBS> z_2_microlimbs;

        std::array<Fr, NUM_BINARY_LIMBS> previous_accumulator;
        std::array<Fr, NUM_BINARY_LIMBS> current_accumulator;
        std::array<std::array<Fr, NUM_MICRO_LIMBS>, NUM_BINARY_LIMBS> current_accumulator_microlimbs;
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
        ASSERT(acc_step.z_1 == (acc_step.z_1_limbs[0] + acc_step.z_1_limbs[1] * SHIFT_1));
        ASSERT(acc_step.z_2 == (acc_step.z_2_limbs[0] + acc_step.z_2_limbs[1] * SHIFT_1));

        auto check_binary_limbs_maximum_values = []<size_t total_limbs>(const std::array<Fr, total_limbs>& limbs) {
            if constexpr (total_limbs == (NUM_BINARY_LIMBS + 1)) {
                for (size_t i = 0; i < NUM_BINARY_LIMBS - 1; i++) {
                    ASSERT(uint256_t(limbs[i]) < SHIFT_1);
                }
                ASSERT(uint256_t(limbs[NUM_BINARY_LIMBS - 1]) < (uint256_t(1) << NUM_LAST_LIMB_BITS));
            } else {
                for (size_t i = 0; i < total_limbs; i++) {
                    ASSERT(uint256_t(limbs[i]) < SHIFT_1);
                }
            }
        };
        auto check_micro_limbs_maximum_values =
            []<size_t binary_limb_count, size_t micro_limb_count>(
                const std::array<std::array<Fr, micro_limb_count>, binary_limb_count>& limbs) {
                for (size_t i = 0; i < binary_limb_count; i++) {
                    for (size_t j = 0; j < micro_limb_count; j++) {
                        ASSERT(uint256_t(limbs[i][j]) < MICRO_SHIFT);
                    }
                }
            };
        check_binary_limbs_maximum_values(acc_step.P_x_limbs);
        check_binary_limbs_maximum_values(acc_step.P_y_limbs);
        check_binary_limbs_maximum_values(acc_step.z_1_limbs);
        check_binary_limbs_maximum_values(acc_step.z_2_limbs);
        check_binary_limbs_maximum_values(acc_step.previous_accumulator);
        check_binary_limbs_maximum_values(acc_step.current_accumulator);

        insert_pair_into_wire(WireIds::P_X_LOW_LIMBS, acc_step.P_x_limbs[0], acc_step.P_x_limbs[1]);
        insert_pair_into_wire(WireIds::P_X_HIGH_LIMBS, acc_step.P_x_limbs[2], acc_step.P_x_limbs[3]);
        insert_pair_into_wire(WireIds::P_Y_LOW_LIMBS, acc_step.P_y_limbs[0], acc_step.P_y_limbs[1]);
        insert_pair_into_wire(WireIds::P_Y_HIGH_LIMBS, acc_step.P_y_limbs[2], acc_step.P_y_limbs[3]);
        insert_pair_into_wire(WireIds::Z_LO_LIMBS, acc_step.z_1_limbs[0], acc_step.z_2_limbs[0]);
        insert_pair_into_wire(WireIds::Z_HI_LIMBS, acc_step.z_1_limbs[1], acc_step.z_2_limbs[1]);

        check_micro_limbs_maximum_values(acc_step.P_x_microlimbs);
        check_micro_limbs_maximum_values(acc_step.P_y_microlimbs);
        check_micro_limbs_maximum_values(acc_step.z_1_microlimbs);
        check_micro_limbs_maximum_values(acc_step.z_2_microlimbs);
        check_micro_limbs_maximum_values(acc_step.current_accumulator_microlimbs);

        auto lay_limbs_in_row = [this]<size_t array_size>(std::array<Fr, array_size> input, WireIds starting_wire) {
            for (size_t i = 0; i < array_size; i++) {
                wires[starting_wire + i].push_back(add_variable(input[i]));

                info("Pushed ",
                     input[i],
                     " to ",
                     starting_wire + i,
                     ":",
                     i,
                     " at ",
                     wires[starting_wire + i].size() - 1);
            }
        };
        lay_limbs_in_row(acc_step.P_x_microlimbs[0], P_X_LOW_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.P_x_microlimbs[1], P_X_LOW_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.P_x_microlimbs[2], P_X_HIGH_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.P_x_microlimbs[3], P_X_HIGH_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.P_y_microlimbs[0], P_Y_LOW_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.P_y_microlimbs[1], P_Y_LOW_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.P_y_microlimbs[2], P_Y_HIGH_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.P_y_microlimbs[3], P_Y_HIGH_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.z_1_microlimbs[0], Z_LO_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.z_2_microlimbs[0], Z_LO_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.z_1_microlimbs[1], Z_HI_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.z_2_microlimbs[1], Z_HI_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.previous_accumulator, ACCUMULATORS_BINARY_LIMBS_0);
        lay_limbs_in_row(acc_step.current_accumulator, ACCUMULATORS_BINARY_LIMBS_0);
        lay_limbs_in_row(acc_step.current_accumulator_microlimbs[0], ACCUMULATOR_LO_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.current_accumulator_microlimbs[1], ACCUMULATOR_LO_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.current_accumulator_microlimbs[2], ACCUMULATOR_HI_LIMBS_RANGE_CONSTRAINT_0);
        lay_limbs_in_row(acc_step.current_accumulator_microlimbs[3], ACCUMULATOR_HI_LIMBS_RANGE_CONSTRAINT_0);

        num_gates += 2;
    }
    bool check_circuit()
    {
        auto& x_lo_y_hi_wire = std::get<X_LO_Y_HI>(wires);
        auto& x_hi_z_1_wire = std::get<X_HI_Z_1>(wires);
        auto& y_lo_z_2_wire = std::get<Y_LO_Z_2>(wires);
        auto& p_x_0_p_x_1_wire = std::get<P_X_LOW_LIMBS>(wires);
        auto& p_x_2_p_x_3_wire = std::get<P_X_HIGH_LIMBS>(wires);
        auto& p_y_0_p_y_1_wire = std::get<P_Y_LOW_LIMBS>(wires);
        auto& p_y_2_p_y_3_wire = std::get<P_Y_HIGH_LIMBS>(wires);
        auto& z_lo_wire = std::get<Z_LO_LIMBS>(wires);
        auto& z_hi_wire = std::get<Z_HI_LIMBS>(wires);
        auto& accumulators_binary_limbs_0_wire = std::get<ACCUMULATORS_BINARY_LIMBS_0>(wires);
        auto& accumulators_binary_limbs_1_wire = std::get<ACCUMULATORS_BINARY_LIMBS_1>(wires);
        auto& accumulators_binary_limbs_2_wire = std::get<ACCUMULATORS_BINARY_LIMBS_2>(wires);
        auto& accumulators_binary_limbs_3_wire = std::get<ACCUMULATORS_BINARY_LIMBS_3>(wires);

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
                info("Micro = ", *it);
                mini_accumulator = mini_accumulator * MICRO_SHIFT + *it;
            }
            return mini_accumulator;
        };
        for (size_t i = 0; i < num_gates; i++) {
            if (!(i & 1)) {
                Fr p_x_lo = get_variable(x_lo_y_hi_wire[i]);
                Fr p_x_hi = get_variable(x_hi_z_1_wire[i]);
                Fr p_x_0 = get_variable(p_x_0_p_x_1_wire[i]);
                Fr p_x_1 = get_variable(p_x_0_p_x_1_wire[i + 1]);
                Fr p_x_2 = get_variable(p_x_2_p_x_3_wire[i]);
                Fr p_x_3 = get_variable(p_x_2_p_x_3_wire[i + 1]);
                auto p_x_binary_limbs = { p_x_0, p_x_1, p_x_2, p_x_3 };
                Fr p_y_lo = get_variable(y_lo_z_2_wire[i]);
                Fr p_y_hi = get_variable(x_lo_y_hi_wire[i + 1]);
                Fr p_y_0 = get_variable(p_y_0_p_y_1_wire[i]);
                Fr p_y_1 = get_variable(p_y_0_p_y_1_wire[i + 1]);
                Fr p_y_2 = get_variable(p_y_2_p_y_3_wire[i]);
                Fr p_y_3 = get_variable(p_y_2_p_y_3_wire[i + 1]);
                auto p_y_binary_limbs = { p_y_0, p_y_1, p_y_2, p_y_3 };
                Fr z_1 = get_variable(x_hi_z_1_wire[i + 1]);
                Fr z_2 = get_variable(y_lo_z_2_wire[i + 1]);
                Fr z_1_lo = get_variable(z_lo_wire[i]);
                Fr z_2_lo = get_variable(z_lo_wire[i + 1]);
                Fr z_1_hi = get_variable(z_hi_wire[i]);
                Fr z_2_hi = get_variable(z_hi_wire[i + 1]);
                auto z_1_binary_limbs = { z_1_lo, z_1_hi };
                auto z_2_binary_limbs = { z_2_lo, z_2_hi };
                auto current_accumulator_binary_limbs = {
                    get_variable(accumulators_binary_limbs_0_wire[i + 1]),
                    get_variable(accumulators_binary_limbs_1_wire[i + 1]),
                    get_variable(accumulators_binary_limbs_2_wire[i + 1]),
                    get_variable(accumulators_binary_limbs_3_wire[i + 1]),
                };

                // These need to be range constrained, but that logic is not present yet
                auto p_x_micro_chunks = {
                    get_sequential_micro_chunks(i, P_X_LOW_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                    get_sequential_micro_chunks(i + 1, P_X_LOW_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                    get_sequential_micro_chunks(i, P_X_HIGH_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                    get_sequential_micro_chunks(i + 1, P_X_HIGH_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS)
                };
                auto p_y_micro_chunks = {
                    get_sequential_micro_chunks(i, P_Y_LOW_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                    get_sequential_micro_chunks(i + 1, P_Y_LOW_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                    get_sequential_micro_chunks(i, P_Y_HIGH_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                    get_sequential_micro_chunks(i + 1, P_Y_HIGH_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS)
                };
                auto z_1_micro_chunks = {
                    get_sequential_micro_chunks(i, Z_LO_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),

                    get_sequential_micro_chunks(i, Z_HI_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                };

                auto z_2_micro_chunks = {

                    get_sequential_micro_chunks(i + 1, Z_LO_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                    get_sequential_micro_chunks(i + 1, Z_HI_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS)
                };

                auto current_accumulator_micro_chunks = {
                    get_sequential_micro_chunks(i, ACCUMULATOR_LO_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                    get_sequential_micro_chunks(i + 1, ACCUMULATOR_LO_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                    get_sequential_micro_chunks(i, ACCUMULATOR_HI_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                    get_sequential_micro_chunks(i + 1, ACCUMULATOR_HI_LIMBS_RANGE_CONSTRAINT_0, NUM_MICRO_LIMBS),
                };

                auto check_wide_limb_into_binary_limb_relation = [](const std::vector<Fr>& wide_limbs,
                                                                    const std::vector<Fr> binary_limbs) {
                    ASSERT(wide_limbs.size() * 2 == binary_limbs.size());
                    for (size_t i = 0; i < wide_limbs.size(); i++) {
                        if ((binary_limbs[i * 2] + Fr(SHIFT_1) * binary_limbs[i * 2 + 1]) != wide_limbs[i]) {
                            return false;
                        }
                    }
                    return true;
                };
                if (!(check_wide_limb_into_binary_limb_relation({ p_x_lo, p_x_hi }, p_x_binary_limbs) &&
                      check_wide_limb_into_binary_limb_relation({ p_y_lo, p_y_hi }, p_y_binary_limbs) &&
                      check_wide_limb_into_binary_limb_relation({ z_1 }, z_1_binary_limbs) &&
                      check_wide_limb_into_binary_limb_relation({ z_2 }, z_2_binary_limbs))) {
                    return false;
                }

                auto check_micro_limb_decomposition_correctness =
                    [&accumulate_limb_from_micro_chunks](const std::vector<Fr>& binary_limbs,
                                                         const std::vector<std::vector<Fr>>& micro_limbs) {
                        ASSERT(binary_limbs.size() == micro_limbs.size());
                        for (size_t i = 0; i < binary_limbs.size(); i++) {
                            info("Binary = ", binary_limbs[i]);
                            if (binary_limbs[i] != accumulate_limb_from_micro_chunks(micro_limbs[i])) {
                                return false;
                            }
                        }
                        return true;
                    };

                if (!check_micro_limb_decomposition_correctness(p_x_binary_limbs, p_x_micro_chunks)) {
                    return false;
                }
                if (!check_micro_limb_decomposition_correctness(p_y_binary_limbs, p_y_micro_chunks)) {
                    return false;
                }
                if (!check_micro_limb_decomposition_correctness(z_1_binary_limbs, z_1_micro_chunks)) {
                    return false;
                }
                if (!check_micro_limb_decomposition_correctness(z_2_binary_limbs, z_2_micro_chunks)) {
                    return false;
                }
                if (!check_micro_limb_decomposition_correctness(current_accumulator_binary_limbs,
                                                                current_accumulator_micro_chunks)) {
                    return false;
                }
            }
        }
        return true;
    }
};
} // namespace proof_system