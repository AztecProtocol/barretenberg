#include "turbo_composer.hpp"
#include <ecc/curves/bn254/scalar_multiplication/scalar_multiplication.hpp>
#include <numeric/bitop/get_msb.hpp>
#include <plonk/proof_system/widgets/permutation_widget.hpp>
#include <plonk/proof_system/widgets/turbo_arithmetic_widget.hpp>
#include <plonk/proof_system/widgets/turbo_fixed_base_widget.hpp>
#include <plonk/proof_system/widgets/turbo_logic_widget.hpp>
#include <plonk/proof_system/widgets/turbo_range_widget.hpp>
#include <plonk/reference_string/file_reference_string.hpp>
#include <plonk/composer/turbo/compute_verification_key.hpp>

using namespace barretenberg;

namespace waffle {

TurboComposer::TurboComposer()
    : TurboComposer("../srs_db", 0)
{}

TurboComposer::TurboComposer(std::string const& crs_path, const size_t size_hint)
    : TurboComposer(std::unique_ptr<ReferenceStringFactory>(new FileReferenceStringFactory(crs_path)), size_hint){};

TurboComposer::TurboComposer(std::unique_ptr<ReferenceStringFactory>&& crs_factory, const size_t size_hint)
    : ComposerBase(std::move(crs_factory))
{
    w_l.reserve(size_hint);
    w_r.reserve(size_hint);
    w_o.reserve(size_hint);
    w_4.reserve(size_hint);
    q_m.reserve(size_hint);
    q_1.reserve(size_hint);
    q_2.reserve(size_hint);
    q_3.reserve(size_hint);
    q_4.reserve(size_hint);
    q_arith.reserve(size_hint);
    q_c.reserve(size_hint);
    q_5.reserve(size_hint);
    q_ecc_1.reserve(size_hint);
    q_range.reserve(size_hint);
    q_logic.reserve(size_hint);

    zero_idx = put_constant_variable(fr::zero());
    // zero_idx = add_variable(barretenberg::fr::zero());
}

TurboComposer::TurboComposer(std::shared_ptr<proving_key> const& p_key,
                             std::shared_ptr<verification_key> const& v_key,
                             size_t size_hint)
    : ComposerBase(p_key, v_key)
{
    w_l.reserve(size_hint);
    w_r.reserve(size_hint);
    w_o.reserve(size_hint);
    w_4.reserve(size_hint);
    q_m.reserve(size_hint);
    q_1.reserve(size_hint);
    q_2.reserve(size_hint);
    q_3.reserve(size_hint);
    q_4.reserve(size_hint);
    q_arith.reserve(size_hint);
    q_c.reserve(size_hint);
    q_5.reserve(size_hint);
    q_ecc_1.reserve(size_hint);
    q_range.reserve(size_hint);
    q_logic.reserve(size_hint);

    zero_idx = put_constant_variable(fr::zero());
};

void TurboComposer::create_dummy_gate()
{
    gate_flags.push_back(0);
    uint32_t idx = add_variable(fr{ 1, 1, 1, 1 }.to_montgomery_form());
    w_l.emplace_back(idx);
    w_r.emplace_back(idx);
    w_o.emplace_back(idx);
    w_4.emplace_back(idx);
    q_arith.emplace_back(fr::zero());
    q_4.emplace_back(fr::zero());
    q_5.emplace_back(fr::zero());
    q_ecc_1.emplace_back(fr::zero());
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(fr::zero());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::zero());
    q_c.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    cycle_node fourth{ static_cast<uint32_t>(n), WireType::FOURTH };

    wire_copy_cycles[static_cast<size_t>(idx)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(idx)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(idx)].emplace_back(out);
    wire_copy_cycles[static_cast<size_t>(idx)].emplace_back(fourth);

    ++n;
}

void TurboComposer::create_add_gate(const add_triple& in)
{
    gate_flags.push_back(0);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    w_4.emplace_back(zero_idx);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(in.a_scaling);
    q_2.emplace_back(in.b_scaling);
    q_3.emplace_back(in.c_scaling);
    q_c.emplace_back(in.const_scaling);
    q_arith.emplace_back(fr::one());
    q_4.emplace_back(fr::zero());
    q_5.emplace_back(fr::zero());
    q_ecc_1.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };

    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);

    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);

    ++n;
}

void TurboComposer::create_big_add_gate(const add_quad& in)
{
    gate_flags.push_back(0);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    w_4.emplace_back(in.d);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(in.a_scaling);
    q_2.emplace_back(in.b_scaling);
    q_3.emplace_back(in.c_scaling);
    q_c.emplace_back(in.const_scaling);
    q_arith.emplace_back(fr::one());
    q_4.emplace_back(in.d_scaling);
    q_5.emplace_back(fr::zero());
    q_ecc_1.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    cycle_node fourth{ static_cast<uint32_t>(n), WireType::FOURTH };

    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);
    ASSERT(wire_copy_cycles.size() > in.d);

    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);
    wire_copy_cycles[static_cast<size_t>(in.d)].emplace_back(fourth);

    ++n;
}

void TurboComposer::create_big_add_gate_with_bit_extraction(const add_quad& in)
{
    gate_flags.push_back(0);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    w_4.emplace_back(in.d);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(in.a_scaling);
    q_2.emplace_back(in.b_scaling);
    q_3.emplace_back(in.c_scaling);
    q_c.emplace_back(in.const_scaling);
    q_arith.emplace_back(fr::one() + fr::one());
    q_4.emplace_back(in.d_scaling);
    q_5.emplace_back(fr::zero());
    q_ecc_1.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    cycle_node fourth{ static_cast<uint32_t>(n), WireType::FOURTH };

    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);
    ASSERT(wire_copy_cycles.size() > in.d);

    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);
    wire_copy_cycles[static_cast<size_t>(in.d)].emplace_back(fourth);

    ++n;
}

void TurboComposer::create_big_mul_gate(const mul_quad& in)
{
    gate_flags.push_back(0);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    w_4.emplace_back(in.d);
    q_m.emplace_back(in.mul_scaling);
    q_1.emplace_back(in.a_scaling);
    q_2.emplace_back(in.b_scaling);
    q_3.emplace_back(in.c_scaling);
    q_c.emplace_back(in.const_scaling);
    q_arith.emplace_back(fr::one());
    q_4.emplace_back(in.d_scaling);
    q_5.emplace_back(fr::zero());
    q_ecc_1.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    cycle_node fourth{ static_cast<uint32_t>(n), WireType::FOURTH };

    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);
    ASSERT(wire_copy_cycles.size() > in.d);

    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);
    wire_copy_cycles[static_cast<size_t>(in.d)].emplace_back(fourth);

    ++n;
}

// Creates a width-4 addition gate, where the fourth witness must be a boolean.
// Can be used to normalize a 32-bit addition
void TurboComposer::create_balanced_add_gate(const add_quad& in)
{
    gate_flags.push_back(0);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    w_4.emplace_back(in.d);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(in.a_scaling);
    q_2.emplace_back(in.b_scaling);
    q_3.emplace_back(in.c_scaling);
    q_c.emplace_back(in.const_scaling);
    q_arith.emplace_back(fr::one());
    q_4.emplace_back(in.d_scaling);
    q_5.emplace_back(fr::one());
    q_ecc_1.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    cycle_node fourth{ static_cast<uint32_t>(n), WireType::FOURTH };

    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);
    ASSERT(wire_copy_cycles.size() > in.d);

    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);
    wire_copy_cycles[static_cast<size_t>(in.d)].emplace_back(fourth);

    ++n;
}

void TurboComposer::create_mul_gate(const mul_triple& in)
{
    gate_flags.push_back(0);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_LEFT_WIRE);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_RIGHT_WIRE);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    w_4.emplace_back(zero_idx);
    q_m.emplace_back(in.mul_scaling);
    q_1.emplace_back(fr::zero());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(in.c_scaling);
    q_c.emplace_back(in.const_scaling);
    q_arith.emplace_back(fr::one());
    q_4.emplace_back(fr::zero());
    q_5.emplace_back(fr::zero());
    q_ecc_1.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };

    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);
    ASSERT(wire_copy_cycles.size() > zero_idx);

    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);

    ++n;
}

void TurboComposer::create_bool_gate(const uint32_t variable_index)
{
    gate_flags.push_back(0);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_LEFT_WIRE);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_RIGHT_WIRE);
    w_l.emplace_back(variable_index);
    w_r.emplace_back(variable_index);
    w_o.emplace_back(variable_index);
    w_4.emplace_back(zero_idx);
    q_arith.emplace_back(fr::one());
    q_4.emplace_back(fr::zero());
    q_5.emplace_back(fr::zero());
    q_ecc_1.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());

    q_m.emplace_back(fr::one());
    q_1.emplace_back(fr::zero());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::neg_one());
    q_c.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };

    ASSERT(wire_copy_cycles.size() > variable_index);
    wire_copy_cycles[static_cast<size_t>(variable_index)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(variable_index)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(variable_index)].emplace_back(out);

    ++n;
}

void TurboComposer::create_poly_gate(const poly_triple& in)
{
    gate_flags.push_back(0);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_LEFT_WIRE);
    add_gate_flag(gate_flags.size() - 1, GateFlags::FIXED_RIGHT_WIRE);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    w_4.emplace_back(zero_idx);
    q_m.emplace_back(in.q_m);
    q_1.emplace_back(in.q_l);
    q_2.emplace_back(in.q_r);
    q_3.emplace_back(in.q_o);
    q_c.emplace_back(in.q_c);
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    q_arith.emplace_back(fr::one());
    q_4.emplace_back(fr::zero());
    q_5.emplace_back(fr::zero());
    q_ecc_1.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };

    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);
    ASSERT(wire_copy_cycles.size() > zero_idx);

    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);

    ++n;
}

// adds a grumpkin point, from a 2-bit lookup table, into an accumulator point
void TurboComposer::create_fixed_group_add_gate(const fixed_group_add_quad& in)
{
    gate_flags.push_back(0);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    w_4.emplace_back(in.d);

    q_arith.emplace_back(fr::zero());
    q_4.emplace_back(fr::zero());
    q_5.emplace_back(fr::zero());
    q_m.emplace_back(fr::zero());
    q_c.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    q_1.emplace_back(in.q_x_1);
    q_2.emplace_back(in.q_x_2);
    q_3.emplace_back(in.q_y_1);
    q_ecc_1.emplace_back(in.q_y_2);

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    cycle_node fourth{ static_cast<uint32_t>(n), WireType::FOURTH };

    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);
    ASSERT(wire_copy_cycles.size() > in.d);

    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);
    wire_copy_cycles[static_cast<size_t>(in.d)].emplace_back(fourth);

    ++n;
}

// adds a grumpkin point into an accumulator, while also initializing the accumulator
void TurboComposer::create_fixed_group_add_gate_with_init(const fixed_group_add_quad& in,
                                                          const fixed_group_init_quad& init)
{
    gate_flags.push_back(0);
    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    w_4.emplace_back(in.d);

    q_arith.emplace_back(fr::zero());
    q_4.emplace_back(init.q_x_1);
    q_5.emplace_back(init.q_x_2);
    q_m.emplace_back(init.q_y_1);
    q_c.emplace_back(init.q_y_2);
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    q_1.emplace_back(in.q_x_1);
    q_2.emplace_back(in.q_x_2);
    q_3.emplace_back(in.q_y_1);
    q_ecc_1.emplace_back(in.q_y_2);

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };
    cycle_node right{ static_cast<uint32_t>(n), WireType::RIGHT };
    cycle_node out{ static_cast<uint32_t>(n), WireType::OUTPUT };
    cycle_node fourth{ static_cast<uint32_t>(n), WireType::FOURTH };

    ASSERT(wire_copy_cycles.size() > in.a);
    ASSERT(wire_copy_cycles.size() > in.b);
    ASSERT(wire_copy_cycles.size() > in.c);
    ASSERT(wire_copy_cycles.size() > in.d);

    wire_copy_cycles[static_cast<size_t>(in.a)].emplace_back(left);
    wire_copy_cycles[static_cast<size_t>(in.b)].emplace_back(right);
    wire_copy_cycles[static_cast<size_t>(in.c)].emplace_back(out);
    wire_copy_cycles[static_cast<size_t>(in.d)].emplace_back(fourth);

    ++n;
}

void TurboComposer::fix_witness(const uint32_t witness_index, const barretenberg::fr& witness_value)
{
    gate_flags.push_back(0);

    w_l.emplace_back(witness_index);
    w_r.emplace_back(zero_idx);
    w_o.emplace_back(zero_idx);
    w_4.emplace_back(zero_idx);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(fr::one());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::zero());
    q_c.emplace_back(-witness_value);
    q_arith.emplace_back(fr::one());
    q_4.emplace_back(fr::zero());
    q_5.emplace_back(fr::zero());
    q_ecc_1.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());

    cycle_node left{ static_cast<uint32_t>(n), WireType::LEFT };

    ASSERT(wire_copy_cycles.size() > witness_index);
    ASSERT(wire_copy_cycles.size() > zero_idx);
    ASSERT(wire_copy_cycles.size() > zero_idx);
    wire_copy_cycles[static_cast<size_t>(witness_index)].emplace_back(left);

    ++n;
}

std::vector<uint32_t> TurboComposer::create_range_constraint(const uint32_t witness_index, const size_t num_bits)
{
    ASSERT(static_cast<uint32_t>(variables.size()) > witness_index);
    ASSERT(((num_bits >> 1U) << 1U) == num_bits);

    /*
     * The range constraint accumulates base 4 values into a sum.
     * We do this by evaluating a kind of 'raster scan', where we compare adjacent elements
     * and validate that their differences map to a base for value  *
     * Let's say that we want to perform a 32-bit range constraint in 'x'.
     * We can represent x via 16 constituent base-4 'quads' {q_0, ..., q_15}:
     *
     *      15
     *      ===
     *      \          i
     * x =  /    q  . 4
     *      ===   i
     *     i = 0
     *
     * In program memory, we place an accumulating base-4 sum of x {a_0, ..., a_15}, where
     *
     *         i
     *        ===
     *        \                  j
     * a   =  /    q         .  4
     *  i     ===   (15 - j)
     *       j = 0
     *
     *
     * From this, we can use our range transition constraint to validate that
     *
     *
     *  a      - 4 . a  ?? [0, 1, 2, 3]
     *   i + 1        i
     *
     *
     * We place our accumulating sums in program memory in the following sequence:
     *
     * +-----+-----+-----+-----+
     * |  A  |  B  |  C  |  D  |
     * +-----+-----+-----+-----+
     * | a3  | a2  | a1  | 0   |
     * | a7  | a6  | a5  | a4  |
     * | a11 | a10 | a9  | a8  |
     * | a15 | a14 | a13 | a12 |
     * | --- | --- | --- | a16 |
     * +-----+-----+-----+-----+
     *
     * Our range transition constraint on row 'i'
     * performs our base-4 range check on the follwing pairs:
     *
     * (D_{i}, C_{i}), (C_{i}, B_{i}), (B_{i}, A_{i}), (A_{i}, D_{i+1})
     *
     * We need to start our raster scan at zero, so we simplify matters and just force the first value
     * to be zero.
     *
     * The output will be in the 4th column of an otherwise unused row. Assuming this row can
     * be used for a width-3 standard gate, the total number of gates for an n-bit range constraint
     * is (n / 8) gates
     *
     **/

    const fr witness_value = variables[witness_index].from_montgomery_form();

    // one gate accmulates 4 quads, or 8 bits.
    // # gates = (bits / 8)
    size_t num_quad_gates = (num_bits >> 3);

    num_quad_gates = (num_quad_gates << 3 == num_bits) ? num_quad_gates : num_quad_gates + 1;

    // hmm
    std::vector<uint32_t>* wires[4]{ &w_4, &w_o, &w_r, &w_l };

    // hmmm
    WireType wire_types[4]{ WireType::FOURTH, WireType::OUTPUT, WireType::RIGHT, WireType::LEFT };

    const size_t num_quads = (num_quad_gates << 2);
    const size_t forced_zero_threshold = 1 + (((num_quads << 1) - num_bits) >> 1);
    std::vector<uint32_t> accumulators;
    fr accumulator = fr::zero();

    for (size_t i = 0; i < num_quads + 1; ++i) {
        const size_t gate_index = n + (i / 4);
        uint32_t accumulator_index;
        if (i < forced_zero_threshold) {
            accumulator_index = zero_idx;
        } else {
            const size_t bit_index = (num_quads - i) << 1;
            const uint64_t quad = static_cast<uint64_t>(witness_value.get_bit(bit_index)) +
                                  2ULL * static_cast<uint64_t>(witness_value.get_bit(bit_index + 1));
            const fr quad_element = fr{ quad, 0, 0, 0 }.to_montgomery_form();
            accumulator += accumulator;
            accumulator += accumulator;
            accumulator += quad_element;

            accumulator_index = add_variable(accumulator);
            accumulators.emplace_back(accumulator_index);
        }

        // hmmmm
        (*(wires + (i & 3)))->emplace_back(accumulator_index);
        const size_t wire_index = i & 3;

        wire_copy_cycles[accumulator_index].emplace_back(
            cycle_node(static_cast<uint32_t>(gate_index), wire_types[wire_index]));
    }
    size_t used_gates = (num_quads + 1) / 4;

    // TODO: handle partially used gates. For now just set them to be zero
    if (used_gates * 4 != (num_quads + 1)) {
        ++used_gates;
    }

    for (size_t i = 0; i < used_gates; ++i) {
        q_m.emplace_back(fr::zero());
        q_1.emplace_back(fr::zero());
        q_2.emplace_back(fr::zero());
        q_3.emplace_back(fr::zero());
        q_c.emplace_back(fr::zero());
        q_arith.emplace_back(fr::zero());
        q_4.emplace_back(fr::zero());
        q_5.emplace_back(fr::zero());
        q_ecc_1.emplace_back(fr::zero());
        q_logic.emplace_back(fr::zero());
        q_range.emplace_back(fr::one());
    }

    q_range[q_range.size() - 1] = fr::zero();

    w_l.emplace_back(zero_idx);
    w_r.emplace_back(zero_idx);
    w_o.emplace_back(zero_idx);

    assert_equal(accumulators[accumulators.size() - 1], witness_index);
    accumulators[accumulators.size() - 1] = witness_index;

    n += used_gates;
    return accumulators;
}

waffle::accumulator_triple TurboComposer::create_logic_constraint(const uint32_t a,
                                                                  const uint32_t b,
                                                                  const size_t num_bits,
                                                                  const bool is_xor_gate)
{
    ASSERT(static_cast<uint32_t>(variables.size()) > a);
    ASSERT(static_cast<uint32_t>(variables.size()) > b);
    ASSERT(((num_bits >> 1U) << 1U) == num_bits); // no odd number of bits! bad! only quads!

    /*
     * The LOGIC constraint accumulates 3 base-4 values (a, b, c) into a sum, where c = a & b OR c = a ^ b
     *
     * In program memory, we place an accumulating base-4 sum of a, b, c {a_0, ..., a_15}, where
     *
     *         i
     *        ===
     *        \                  j
     * a   =  /    q         .  4
     *  i     ===   (15 - j)
     *       j = 0
     *
     *
     * From this, we can use our logic transition constraint to validate that
     *
     *
     *  a      - 4 . a  ?? [0, 1, 2, 3]
     *   i + 1        i
     *
     *
     *
     *
     *  b      - 4 . b  ?? [0, 1, 2, 3]
     *   i + 1        i
     *
     *
     *
     *
     *                    /                 \          /                 \
     *  c      - 4 . c  = | a      - 4 . a  | (& OR ^) | b      - b . a  |
     *   i + 1        i   \  i + 1        i /          \  i + 1        i /
     *
     *
     * We also need the following temporary, w, stored in program memory:
     *
     *      /                 \   /                 \
     * w  = | a      - 4 . a  | * | b      - b . a  |
     *  i   \  i + 1        i /   \  i + 1        i /
     *
     *
     * w is needed to prevent the degree of our quotient polynomial from blowing up
     *
     * We place our accumulating sums in program memory in the following sequence:
     *
     * +-----+-----+-----+-----+
     * |  A  |  B  |  C  |  D  |
     * +-----+-----+-----+-----+
     * | 0   | 0   | w1  | 0   |
     * | a1  | b1  | w2  | c1  |
     * | a2  | b2  | w3  | c2  |
     * |  :  |  :  |  :  |  :  |
     * | an  | bn  | --- | cn  |
     * +-----+-----+-----+-----+
     *
     * Our transition constraint extracts quads by taking the difference between two accumulating sums,
     * so we need to start the chain with a row of zeroes
     *
     * The total number of gates required to evaluate an AND operation is (n / 2) + 1,
     * where n = max(num_bits(a), num_bits(b))
     *
     * One additional benefit of this constraint, is that both our inputs and output are in 'native' uint32 form.
     * This means we *never* have to decompose a uint32 into bits and back in order to chain together
     * addition and logic operations.
     *
     **/

    const fr left_witness_value = variables[a].from_montgomery_form();
    const fr right_witness_value = variables[b].from_montgomery_form();

    // one gate accmulates 1 quads, or 2 bits.
    // # gates = (bits / 2)
    const size_t num_quads = (num_bits >> 1);

    waffle::accumulator_triple accumulators;
    fr left_accumulator = fr::zero();
    fr right_accumulator = fr::zero();
    fr out_accumulator = fr::zero();

    // Step 1: populate 1st row accumulators with zero
    w_l.emplace_back(zero_idx);
    w_r.emplace_back(zero_idx);
    w_4.emplace_back(zero_idx);

    wire_copy_cycles[zero_idx].emplace_back(cycle_node(static_cast<uint32_t>(n), WireType::LEFT));
    wire_copy_cycles[zero_idx].emplace_back(cycle_node(static_cast<uint32_t>(n), WireType::RIGHT));
    wire_copy_cycles[zero_idx].emplace_back(cycle_node(static_cast<uint32_t>(n), WireType::FOURTH));

    // w_l, w_r, w_4 should now point to 1 gate ahead of w_o
    for (size_t i = 0; i < num_quads; ++i) {
        const size_t gate_index = n + i + 1;
        uint32_t left_accumulator_index;
        uint32_t right_accumulator_index;
        uint32_t out_accumulator_index;
        uint32_t product_index;

        const size_t bit_index = (num_quads - 1 - i) << 1;
        const uint64_t left_quad = static_cast<uint64_t>(left_witness_value.get_bit(bit_index)) +
                                   2ULL * static_cast<uint64_t>(left_witness_value.get_bit(bit_index + 1));

        const uint64_t right_quad = static_cast<uint64_t>(right_witness_value.get_bit(bit_index)) +
                                    2ULL * static_cast<uint64_t>(right_witness_value.get_bit(bit_index + 1));
        const fr left_quad_element = fr{ left_quad, 0, 0, 0 }.to_montgomery_form();
        const fr right_quad_element = fr{ right_quad, 0, 0, 0 }.to_montgomery_form();
        fr out_quad_element;
        if (is_xor_gate) {
            out_quad_element = fr{ left_quad ^ right_quad, 0, 0, 0 }.to_montgomery_form();
        } else {
            out_quad_element = fr{ left_quad & right_quad, 0, 0, 0 }.to_montgomery_form();
        }

        const fr product_quad_element = fr{ left_quad * right_quad, 0, 0, 0 }.to_montgomery_form();

        left_accumulator += left_accumulator;
        left_accumulator += left_accumulator;
        left_accumulator += left_quad_element;

        right_accumulator += right_accumulator;
        right_accumulator += right_accumulator;
        right_accumulator += right_quad_element;

        out_accumulator += out_accumulator;
        out_accumulator += out_accumulator;
        out_accumulator += out_quad_element;

        left_accumulator_index = add_variable(left_accumulator);
        accumulators.left.emplace_back(left_accumulator_index);

        right_accumulator_index = add_variable(right_accumulator);
        accumulators.right.emplace_back(right_accumulator_index);

        out_accumulator_index = add_variable(out_accumulator);
        accumulators.out.emplace_back(out_accumulator_index);

        product_index = add_variable(product_quad_element);

        w_l.emplace_back(left_accumulator_index);
        w_r.emplace_back(right_accumulator_index);
        w_4.emplace_back(out_accumulator_index);
        w_o.emplace_back(product_index);

        wire_copy_cycles[left_accumulator_index].emplace_back(
            cycle_node(static_cast<uint32_t>(gate_index), WireType::LEFT));
        wire_copy_cycles[right_accumulator_index].emplace_back(
            cycle_node(static_cast<uint32_t>(gate_index), WireType::RIGHT));
        wire_copy_cycles[out_accumulator_index].emplace_back(
            cycle_node(static_cast<uint32_t>(gate_index), WireType::FOURTH));
        wire_copy_cycles[product_index].emplace_back(
            cycle_node(static_cast<uint32_t>(gate_index - 1), WireType::OUTPUT));
    }

    w_o.emplace_back(zero_idx);

    for (size_t i = 0; i < num_quads + 1; ++i) {
        q_m.emplace_back(fr::zero());
        q_1.emplace_back(fr::zero());
        q_2.emplace_back(fr::zero());
        q_3.emplace_back(fr::zero());
        q_arith.emplace_back(fr::zero());
        q_4.emplace_back(fr::zero());
        q_5.emplace_back(fr::zero());
        q_ecc_1.emplace_back(fr::zero());
        q_range.emplace_back(fr::zero());
        if (is_xor_gate) {
            q_c.emplace_back(fr::neg_one());
            q_logic.emplace_back(fr::neg_one());
        } else {
            q_c.emplace_back(fr::one());
            q_logic.emplace_back(fr::one());
        }
    }
    q_c[q_c.size() - 1] = fr::zero();         // last gate is a noop
    q_logic[q_logic.size() - 1] = fr::zero(); // last gate is a noop

    assert_equal(accumulators.left[accumulators.left.size() - 1], a);
    accumulators.left[accumulators.left.size() - 1] = a;

    assert_equal(accumulators.right[accumulators.right.size() - 1], b);
    accumulators.right[accumulators.right.size() - 1] = b;

    n += (num_quads + 1);
    return accumulators;
}

waffle::accumulator_triple TurboComposer::create_and_constraint(const uint32_t a,
                                                                const uint32_t b,
                                                                const size_t num_bits)
{
    return create_logic_constraint(a, b, num_bits, false);
}

waffle::accumulator_triple TurboComposer::create_xor_constraint(const uint32_t a,
                                                                const uint32_t b,
                                                                const size_t num_bits)
{
    return create_logic_constraint(a, b, num_bits, true);
}

uint32_t TurboComposer::put_constant_variable(const barretenberg::fr& variable)
{
    if (constant_variables.count(variable) == 1) {
        return constant_variables.at(variable);
    } else {
        uint32_t variable_index = add_variable(variable);
        fix_witness(variable_index, variable);
        constant_variables.insert({ variable, variable_index });
        return variable_index;
    }
}

std::shared_ptr<proving_key> TurboComposer::compute_proving_key()
{
    if (circuit_proving_key) {
        return circuit_proving_key;
    }
    create_dummy_gate();
    ASSERT(wire_copy_cycles.size() == variables.size());
    ASSERT(n == q_m.size());
    ASSERT(n == q_1.size());
    ASSERT(n == q_2.size());
    ASSERT(n == q_3.size());
    ASSERT(n == q_3.size());
    ASSERT(n == q_4.size());
    ASSERT(n == q_5.size());
    ASSERT(n == q_arith.size());
    ASSERT(n == q_ecc_1.size());
    ASSERT(n == q_range.size());
    ASSERT(n == q_logic.size());

    const size_t total_num_gates = n + public_inputs.size();

    size_t log2_n = static_cast<size_t>(numeric::get_msb(total_num_gates + 1));
    if ((1UL << log2_n) != (total_num_gates + 1)) {
        ++log2_n;
    }
    size_t new_n = 1UL << log2_n;

    for (size_t i = total_num_gates; i < new_n; ++i) {
        q_m.emplace_back(fr::zero());
        q_1.emplace_back(fr::zero());
        q_2.emplace_back(fr::zero());
        q_3.emplace_back(fr::zero());
        q_c.emplace_back(fr::zero());
        q_4.emplace_back(fr::zero());
        q_5.emplace_back(fr::zero());
        q_arith.emplace_back(fr::zero());
        q_ecc_1.emplace_back(fr::zero());
        q_range.emplace_back(fr::zero());
        q_logic.emplace_back(fr::zero());
    }

    auto crs = crs_factory_->get_prover_crs(new_n);
    circuit_proving_key = std::make_shared<proving_key>(new_n, public_inputs.size(), crs);

    for (size_t i = 0; i < public_inputs.size(); ++i) {
        cycle_node left{ static_cast<uint32_t>(i - public_inputs.size()), WireType::LEFT };
        cycle_node right{ static_cast<uint32_t>(i - public_inputs.size()), WireType::RIGHT };

        std::vector<cycle_node>& old_cycle = wire_copy_cycles[static_cast<size_t>(public_inputs[i])];

        std::vector<cycle_node> new_cycle;

        new_cycle.emplace_back(left);
        new_cycle.emplace_back(right);
        for (size_t i = 0; i < old_cycle.size(); ++i) {
            new_cycle.emplace_back(old_cycle[i]);
        }
        old_cycle = new_cycle;
    }

    polynomial poly_q_m(new_n);
    polynomial poly_q_c(new_n);
    polynomial poly_q_1(new_n);
    polynomial poly_q_2(new_n);
    polynomial poly_q_3(new_n);
    polynomial poly_q_4(new_n);
    polynomial poly_q_5(new_n);
    polynomial poly_q_arith(new_n);
    polynomial poly_q_ecc_1(new_n);
    polynomial poly_q_range(new_n);
    polynomial poly_q_logic(new_n);

    for (size_t i = 0; i < public_inputs.size(); ++i) {
        poly_q_m[i] = fr::zero();
        poly_q_1[i] = fr::zero();
        poly_q_2[i] = fr::zero();
        poly_q_3[i] = fr::zero();
        poly_q_4[i] = fr::zero();
        poly_q_5[i] = fr::zero();
        poly_q_arith[i] = fr::zero();
        poly_q_ecc_1[i] = fr::zero();
        poly_q_c[i] = fr::zero();
        poly_q_range[i] = fr::zero();
        poly_q_logic[i] = fr::zero();
    }

    for (size_t i = public_inputs.size(); i < new_n; ++i) {
        poly_q_m[i] = q_m[i - public_inputs.size()];
        poly_q_1[i] = q_1[i - public_inputs.size()];
        poly_q_2[i] = q_2[i - public_inputs.size()];
        poly_q_3[i] = q_3[i - public_inputs.size()];
        poly_q_c[i] = q_c[i - public_inputs.size()];
        poly_q_4[i] = q_4[i - public_inputs.size()];
        poly_q_5[i] = q_5[i - public_inputs.size()];
        poly_q_arith[i] = q_arith[i - public_inputs.size()];
        poly_q_ecc_1[i] = q_ecc_1[i - public_inputs.size()];
        poly_q_range[i] = q_range[i - public_inputs.size()];
        poly_q_logic[i] = q_logic[i - public_inputs.size()];
    }

    poly_q_1.ifft(circuit_proving_key->small_domain);
    poly_q_2.ifft(circuit_proving_key->small_domain);
    poly_q_3.ifft(circuit_proving_key->small_domain);
    poly_q_4.ifft(circuit_proving_key->small_domain);
    poly_q_5.ifft(circuit_proving_key->small_domain);
    poly_q_m.ifft(circuit_proving_key->small_domain);
    poly_q_c.ifft(circuit_proving_key->small_domain);
    poly_q_arith.ifft(circuit_proving_key->small_domain);
    poly_q_ecc_1.ifft(circuit_proving_key->small_domain);
    poly_q_range.ifft(circuit_proving_key->small_domain);
    poly_q_logic.ifft(circuit_proving_key->small_domain);

    polynomial poly_q_1_fft(poly_q_1, new_n * 4);
    polynomial poly_q_2_fft(poly_q_2, new_n * 4);
    polynomial poly_q_3_fft(poly_q_3, new_n * 4);
    polynomial poly_q_4_fft(poly_q_4, new_n * 4);
    polynomial poly_q_5_fft(poly_q_5, new_n * 4);
    polynomial poly_q_m_fft(poly_q_m, new_n * 4);
    polynomial poly_q_c_fft(poly_q_c, new_n * 4);
    polynomial poly_q_arith_fft(poly_q_arith, new_n * 4);
    polynomial poly_q_ecc_1_fft(poly_q_ecc_1, new_n * 4);
    polynomial poly_q_range_fft(poly_q_range, new_n * 4);
    polynomial poly_q_logic_fft(poly_q_logic, new_n * 4);

    poly_q_1_fft.coset_fft(circuit_proving_key->large_domain);
    poly_q_2_fft.coset_fft(circuit_proving_key->large_domain);
    poly_q_3_fft.coset_fft(circuit_proving_key->large_domain);
    poly_q_4_fft.coset_fft(circuit_proving_key->large_domain);
    poly_q_5_fft.coset_fft(circuit_proving_key->large_domain);
    poly_q_m_fft.coset_fft(circuit_proving_key->large_domain);
    poly_q_c_fft.coset_fft(circuit_proving_key->large_domain);
    poly_q_arith_fft.coset_fft(circuit_proving_key->large_domain);
    poly_q_ecc_1_fft.coset_fft(circuit_proving_key->large_domain);
    poly_q_range_fft.coset_fft(circuit_proving_key->large_domain);
    poly_q_logic_fft.coset_fft(circuit_proving_key->large_domain);

    circuit_proving_key->constraint_selectors.insert({ "q_m", std::move(poly_q_m) });
    circuit_proving_key->constraint_selectors.insert({ "q_c", std::move(poly_q_c) });
    circuit_proving_key->constraint_selectors.insert({ "q_arith", std::move(poly_q_arith) });
    circuit_proving_key->constraint_selectors.insert({ "q_ecc_1", std::move(poly_q_ecc_1) });
    circuit_proving_key->constraint_selectors.insert({ "q_1", std::move(poly_q_1) });
    circuit_proving_key->constraint_selectors.insert({ "q_2", std::move(poly_q_2) });
    circuit_proving_key->constraint_selectors.insert({ "q_3", std::move(poly_q_3) });
    circuit_proving_key->constraint_selectors.insert({ "q_4", std::move(poly_q_4) });
    circuit_proving_key->constraint_selectors.insert({ "q_5", std::move(poly_q_5) });
    circuit_proving_key->constraint_selectors.insert({ "q_range", std::move(poly_q_range) });
    circuit_proving_key->constraint_selectors.insert({ "q_logic", std::move(poly_q_logic) });

    circuit_proving_key->constraint_selector_ffts.insert({ "q_m_fft", std::move(poly_q_m_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_c_fft", std::move(poly_q_c_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_arith_fft", std::move(poly_q_arith_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_ecc_1_fft", std::move(poly_q_ecc_1_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_1_fft", std::move(poly_q_1_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_2_fft", std::move(poly_q_2_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_3_fft", std::move(poly_q_3_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_4_fft", std::move(poly_q_4_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_5_fft", std::move(poly_q_5_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_range_fft", std::move(poly_q_range_fft) });
    circuit_proving_key->constraint_selector_ffts.insert({ "q_logic_fft", std::move(poly_q_logic_fft) });

    compute_sigma_permutations<4>(circuit_proving_key.get());
    return circuit_proving_key;
}

std::shared_ptr<verification_key> TurboComposer::compute_verification_key()
{
    if (circuit_verification_key) {
        return circuit_verification_key;
    }
    if (!circuit_proving_key) {
        compute_proving_key();
    }

    circuit_verification_key =
        turbo_composer::compute_verification_key(circuit_proving_key, crs_factory_->get_verifier_crs());

    return circuit_verification_key;
}

std::shared_ptr<program_witness> TurboComposer::compute_witness()
{
    if (computed_witness) {
        return witness;
    }
    const size_t total_num_gates = n + public_inputs.size();
    size_t log2_n = static_cast<size_t>(numeric::get_msb(total_num_gates + 1));
    if ((1UL << log2_n) != (total_num_gates + 1)) {
        ++log2_n;
    }
    size_t new_n = 1UL << log2_n;

    for (size_t i = total_num_gates; i < new_n; ++i) {
        w_l.emplace_back(zero_idx);
        w_r.emplace_back(zero_idx);
        w_o.emplace_back(zero_idx);
        w_4.emplace_back(zero_idx);
    }

    polynomial poly_w_1(new_n);
    polynomial poly_w_2(new_n);
    polynomial poly_w_3(new_n);
    polynomial poly_w_4(new_n);

    for (size_t i = 0; i < public_inputs.size(); ++i) {
        fr::__copy(variables[public_inputs[i]], poly_w_1[i]);
        fr::__copy(variables[public_inputs[i]], poly_w_2[i]);
        fr::__copy(fr::zero(), poly_w_3[i]);
        fr::__copy(fr::zero(), poly_w_4[i]);
    }
    for (size_t i = public_inputs.size(); i < new_n; ++i) {
        fr::__copy(variables[w_l[i - public_inputs.size()]], poly_w_1.at(i));
        fr::__copy(variables[w_r[i - public_inputs.size()]], poly_w_2.at(i));
        fr::__copy(variables[w_o[i - public_inputs.size()]], poly_w_3.at(i));
        fr::__copy(variables[w_4[i - public_inputs.size()]], poly_w_4.at(i));
    }

    witness = std::make_shared<program_witness>();
    witness->wires.insert({ "w_1", std::move(poly_w_1) });
    witness->wires.insert({ "w_2", std::move(poly_w_2) });
    witness->wires.insert({ "w_3", std::move(poly_w_3) });
    witness->wires.insert({ "w_4", std::move(poly_w_4) });

    computed_witness = true;
    return witness;
}

TurboProver TurboComposer::create_prover()
{
    compute_proving_key();
    compute_witness();

    TurboProver output_state(circuit_proving_key, witness, create_manifest(public_inputs.size()));

    std::unique_ptr<ProverPermutationWidget<4>> permutation_widget =
        std::make_unique<ProverPermutationWidget<4>>(circuit_proving_key.get(), witness.get());
    std::unique_ptr<ProverTurboFixedBaseWidget> fixed_base_widget =
        std::make_unique<ProverTurboFixedBaseWidget>(circuit_proving_key.get(), witness.get());
    std::unique_ptr<ProverTurboRangeWidget> range_widget =
        std::make_unique<ProverTurboRangeWidget>(circuit_proving_key.get(), witness.get());
    std::unique_ptr<ProverTurboLogicWidget> logic_widget =
        std::make_unique<ProverTurboLogicWidget>(circuit_proving_key.get(), witness.get());

    output_state.widgets.emplace_back(std::move(permutation_widget));
    output_state.widgets.emplace_back(std::move(fixed_base_widget));
    output_state.widgets.emplace_back(std::move(range_widget));
    output_state.widgets.emplace_back(std::move(logic_widget));

    return output_state;
}

UnrolledTurboProver TurboComposer::create_unrolled_prover()
{
    compute_proving_key();
    compute_witness();

    UnrolledTurboProver output_state(circuit_proving_key, witness, create_unrolled_manifest(public_inputs.size()));

    std::unique_ptr<ProverPermutationWidget<4>> permutation_widget =
        std::make_unique<ProverPermutationWidget<4>>(circuit_proving_key.get(), witness.get());
    std::unique_ptr<ProverTurboFixedBaseWidget> fixed_base_widget =
        std::make_unique<ProverTurboFixedBaseWidget>(circuit_proving_key.get(), witness.get());
    std::unique_ptr<ProverTurboRangeWidget> range_widget =
        std::make_unique<ProverTurboRangeWidget>(circuit_proving_key.get(), witness.get());
    std::unique_ptr<ProverTurboLogicWidget> logic_widget =
        std::make_unique<ProverTurboLogicWidget>(circuit_proving_key.get(), witness.get());

    output_state.widgets.emplace_back(std::move(permutation_widget));
    output_state.widgets.emplace_back(std::move(fixed_base_widget));
    output_state.widgets.emplace_back(std::move(range_widget));
    output_state.widgets.emplace_back(std::move(logic_widget));

    return output_state;
}

TurboVerifier TurboComposer::create_verifier()
{
    compute_verification_key();

    TurboVerifier output_state(circuit_verification_key, create_manifest(public_inputs.size()));

    return output_state;
}

UnrolledTurboVerifier TurboComposer::create_unrolled_verifier()
{
    compute_verification_key();

    UnrolledTurboVerifier output_state(circuit_verification_key, create_unrolled_manifest(public_inputs.size()));

    return output_state;
}
} // namespace waffle