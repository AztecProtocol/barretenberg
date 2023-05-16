#include "turbo_composer.hpp"
#include "barretenberg/ecc/curves/bn254/scalar_multiplication/scalar_multiplication.hpp"
#include "barretenberg/numeric/bitop/get_msb.hpp"
#include "barretenberg/plonk/proof_system/widgets/random_widgets/permutation_widget.hpp"
#include "barretenberg/plonk/proof_system/widgets/transition_widgets/turbo_arithmetic_widget.hpp"
#include "barretenberg/plonk/proof_system/widgets/transition_widgets/fixed_base_widget.hpp"
#include "barretenberg/plonk/proof_system/widgets/transition_widgets/turbo_logic_widget.hpp"
#include "barretenberg/plonk/proof_system/widgets/transition_widgets/turbo_range_widget.hpp"
#include "barretenberg/plonk/proof_system/commitment_scheme/kate_commitment_scheme.hpp"
#include "../proof_system/widgets/transition_widgets/transition_widget.hpp"
#include "../proof_system/widgets/transition_widgets/turbo_arithmetic_widget.hpp"
#include "barretenberg/srs/reference_string/file_reference_string.hpp"

using namespace barretenberg;
using namespace proof_system;

namespace proof_system::plonk {

#define TURBO_SELECTOR_REFS                                                                                            \
    auto& q_m = selectors[TurboSelectors::QM];                                                                         \
    auto& q_c = selectors[TurboSelectors::QC];                                                                         \
    auto& q_1 = selectors[TurboSelectors::Q1];                                                                         \
    auto& q_2 = selectors[TurboSelectors::Q2];                                                                         \
    auto& q_3 = selectors[TurboSelectors::Q3];                                                                         \
    auto& q_4 = selectors[TurboSelectors::Q4];                                                                         \
    auto& q_5 = selectors[TurboSelectors::Q5];                                                                         \
    auto& q_arith = selectors[TurboSelectors::QARITH];                                                                 \
    auto& q_fixed_base = selectors[TurboSelectors::QFIXED];                                                            \
    auto& q_range = selectors[TurboSelectors::QRANGE];                                                                 \
    auto& q_logic = selectors[TurboSelectors::QLOGIC];

std::vector<ComposerBase::SelectorProperties> turbo_sel_props()
{
    const std::vector<ComposerBase::SelectorProperties> result{
        { "q_m", false },          { "q_c", false },     { "q_1", false },     { "q_2", false },
        { "q_3", false },          { "q_4", false },     { "q_5", false },     { "q_arith", false },
        { "q_fixed_base", false }, { "q_range", false }, { "q_logic", false },
    };
    return result;
}

/**
 * Turbo composer initialization with srs automatically loaded from ../srs_db
 * */
TurboComposer::TurboComposer()
    : TurboComposer("../srs_db/ignition", 0)
{}

/**
 * Turbo composer initialization, where you can specify the path
 * for loading srs and the probable number of gates in your circuit.
 *
 * @param crs_path Path to the file containing srs.
 * @param size_hint Assumed number of gates. Used to allocate space for various member
 * vectors during initialization.
 * */
TurboComposer::TurboComposer(std::string const& crs_path, const size_t size_hint)
    : TurboComposer(std::shared_ptr<ReferenceStringFactory>(new FileReferenceStringFactory(crs_path)), size_hint){};

/**
 * Turbo composer initialization, where you can specify the factory
 * for loading srs and the probable number of gates in your circuit.
 *
 * @param crs_factory The factory with srs.
 * @param size_hint Assumed number of gates. Used to allocate space for various member
 * vectors during initialization.
 * */
TurboComposer::TurboComposer(std::shared_ptr<ReferenceStringFactory> const& crs_factory, const size_t size_hint)
    : ComposerBase(crs_factory, TurboSelectors::NUM, size_hint, turbo_sel_props())
{
    w_l.reserve(size_hint);
    w_r.reserve(size_hint);
    w_o.reserve(size_hint);
    w_4.reserve(size_hint);

    zero_idx = put_constant_variable(fr::zero());
}

TurboComposer::TurboComposer(std::shared_ptr<proving_key> const& p_key,
                             std::shared_ptr<verification_key> const& v_key,
                             size_t size_hint)
    : ComposerBase(p_key, v_key, TurboSelectors::NUM, size_hint, turbo_sel_props())
{
    w_l.reserve(size_hint);
    w_r.reserve(size_hint);
    w_o.reserve(size_hint);
    w_4.reserve(size_hint);
    zero_idx = put_constant_variable(fr::zero());
};

/**
 * Create an addition gate.
 * The q_m, q_4, q_5, q_fixed_base, q_range, q_logic are zero.
 * q_artith is one. w_4 is set to 0-variable index.
 * Other parameters are received from the argument.
 *
 * @param in Specifies addition gate parameters:
 * w_l, w_r, w_o, q_1, q_2, q_3, q_c.
 * */
void TurboComposer::create_add_gate(const add_triple& in)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c });

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
    q_fixed_base.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());
    ++num_gates;
}

/**
 * Create an addition gate that adds 4 variables.
 * The q_m, q_5, q_fixed_base, q_range, q_logic are zero.
 * q_arith is one.
 * Other parameters are received from the argument.
 *
 * @param in Specifies addition gate parameters:
 * w_l, w_r, w_o, w_4, q_1, q_2, q_3, q_4, q_c.
 * */
void TurboComposer::create_big_add_gate(const add_quad& in)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c, in.d });

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
    q_fixed_base.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());
    ++num_gates;
}

/**
 * @brief Create an addition gate that adds 4 variables with bit extraction.
 * The q_m, q_5, q_fixed_base, q_range, q_logic are zero.
 * q_arith is 2, so an additional constraint is imposed in the nonlinear terms.
 * Other parameters are received from the argument.
 *
 * @param in Specifies addition gate parameters:
 * w_l, w_r, w_o, w_4, q_1, q_2, q_3, q_4, q_c.
 *
 * @details Impose the constraint
 * a_scaling . a + b_scaling . b + c_scaling . c + d_scaling . d
 *               + 6 * (high bit of c - 4d)                        == 0.
 * @warning This function assumes that c - 4d lies in the set {0, 1, 2, 3}. The circuit writer should take care to
 * ensure this assumption is backed by a constraint (e.g., c and d could be accumulators produced using the TurboPLONK
 * function `decompose_into_base4_accumulators`).
 * */
void TurboComposer::create_big_add_gate_with_bit_extraction(const add_quad& in)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c, in.d });

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
    q_fixed_base.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());
    ++num_gates;
}

void TurboComposer::create_big_mul_gate(const mul_quad& in)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c, in.d });

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
    q_fixed_base.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());
    ++num_gates;
}

/**
 * @brief Create an addition constraint with a range constraint on the fourth witness.
 *
 * @details The constraints imposed by this:
 *     q_1 * w_l + q_2 * w_r + q_3 * w_o  + q_4 * w_4 + q_c == 0
 *  and
 *     w_4 * (w_4 - 1) * (w_4 - 2) == 0   (i.e., w_4 is in {0, 1, 2}).
 *
 * We use this gate to evaluate additions/subtractions of bounded integers. The purpose is to ensure the prover can use
 * the output witness in constraints that require the input to be bounded. For a typical example, look at the function,
 * uint<Composer, Native>::operator+, which calculates addition modulo some number 2**w. This function must
 * calculate a long divison by 2**w and return the remainder. Without the constraint on w_4, the prover could lie about
 * the remainder.
 *
 * @warning Even with the constraint on w_3, it is typically necessary to range constrain the wire value that will be
 * returned.
 */
void TurboComposer::create_balanced_add_gate(const add_quad& in)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c, in.d });

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
    q_fixed_base.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());
    ++num_gates;
}

/**
 * Create multiplication gate.
 * w_4 is set to the index of zero variable.
 * q_1, q_2, q_4, q_4, q_fixed_base, q_range and q_logic are set to zero.
 * q_arith is set to 1.
 *
 * @param in Contains the values for w_l, w_r, w_o,
 * q_m, q_3, q_c.
 * */
void TurboComposer::create_mul_gate(const mul_triple& in)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c });

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
    q_fixed_base.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());
    ++num_gates;
}

/**
 * Create a gate contraining the variable to 0 or 1.
 * We set selectors in such a way that we get the
 * equation x^2-x=0.
 *
 * @param variable_index The index of the variable.
 * */
void TurboComposer::create_bool_gate(const uint32_t variable_index)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ variable_index });

    w_l.emplace_back(variable_index);
    w_r.emplace_back(variable_index);
    w_o.emplace_back(variable_index);
    w_4.emplace_back(zero_idx);
    q_arith.emplace_back(fr::one());
    q_4.emplace_back(fr::zero());
    q_5.emplace_back(fr::zero());
    q_fixed_base.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());

    q_m.emplace_back(fr::one());
    q_1.emplace_back(fr::zero());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::neg_one());
    q_c.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());
    ++num_gates;
}

/**
 * Create poly gate as in standard composer.
 * w_4 is set to zero variable.
 * q_range, q_logic, q_4, q_5, q_fixed_base are set to 0.
 * q_arith is set to 1.
 *
 * @param in Contains the values for
 * w_l, w_r, w_o, q_m, q_1, q_2, q_3, q_c.
 * */
void TurboComposer::create_poly_gate(const poly_triple& in)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c });

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
    q_fixed_base.emplace_back(fr::zero());
    ++num_gates;
}

/**
 * Add a grumpkin point, from a 2-bit lookup table, into an accumulator point.
 *
 * @param in Witnesses and values of two points.
 * */
void TurboComposer::create_fixed_group_add_gate(const fixed_group_add_quad& in)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c, in.d });

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
    q_fixed_base.emplace_back(in.q_y_2);
    ++num_gates;
}

/**
 * Add a grumpkin point into an accumulator, while also initializing the accumulator.
 *
 * @param in Addition parameters (points and coefficients).
 * @param init Initialization parameters (points).
 * */
void TurboComposer::create_fixed_group_add_gate_with_init(const fixed_group_add_quad& in,
                                                          const fixed_group_init_quad& init)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c, in.d });

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
    q_fixed_base.emplace_back(in.q_y_2);
    ++num_gates;
}

void TurboComposer::create_fixed_group_add_gate_final(const add_quad& in)
{
    create_big_add_gate(in);
}

/**
 * Add a gate that will fix the witness (make it public).
 *
 * @param witness_index Witness variable index.
 * @param witness_value Witness variable value.
 * */
void TurboComposer::fix_witness(const uint32_t witness_index, const barretenberg::fr& witness_value)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ witness_index });

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
    q_fixed_base.emplace_back(fr::zero());
    q_range.emplace_back(fr::zero());
    q_logic.emplace_back(fr::zero());
    ++num_gates;
}

/**
 * Create a constraint placing the witness in 2^{num_bits} range.
 *
 * @param witness_index The index of the witness variable to constrain.
 * @param num_bits Constraint size.
 *
 * @return Vector of variable indexes for accumulator variables used in
 * the constraint.
 * */
std::vector<uint32_t> TurboComposer::decompose_into_base4_accumulators(const uint32_t witness_index,
                                                                       const size_t num_bits,
                                                                       std::string const& msg)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ witness_index });

    ASSERT(num_bits > 0);

    /*
     * The range constraint accumulates base 4 values into a sum.
     * We do this by evaluating a kind of 'raster scan', where we compare adjacent elements
     * and validate that their weighted differences lie in a base for value expansion in powers of 4.
     * Let's say that we want to perform a 32-bit range constraint on a field element x.
     * We can expand x to the desired length via 16 constituent base-4 'quads' {q_0, ..., q_15}:
     *
     *          15
     *          ===
     *          \          i
     *     x =  /    q  . 4
     *          ===   i
     *         i = 0
     *
     * In program memory, we place an accumulating base-4 sum of x {a_0, ..., a_15}, where
     *
     *
     *            i                         |
     *           ===                        | a_0  =                         q_15
     *           \                  i - j   | a_1  =              q_15 . 4 + q_14
     *    a   =  /    q         .  4        | a_2  = q_15 . 4^2 + q_14 . 4 + q_13
     *     i     ===   (15 - j)             |   ...
     *          j = 0                       | a_15 = x
     *
     *
     * From this, we can use our range transition constraint to validate that
     *
     *
     *  a      - 4 . a  ϵ {0, 1, 2, 3}  (for the a_i above, we have
     *   i + 1        i                  a_{i+1} - 4.a_i = q_{14-i}, for i = 0, ..., 15),
     *
     * setting a_{-1} = 0.
     *
     * We place our accumulating sums in program memory in the following sequence:
     *
     * +-----+-----+-----+-----+
     * |  A  |  B  |  C  |  D  |
     * +-----+-----+-----+-----+
     * | a2  | a1  | a0  | 0   |
     * | a6  | a5  | a4  | a3  |
     * | a10 | a9  | a8  | a7  |
     * | a14 | a13 | a12 | a11 |
     * | --- | --- | --- | a15 |
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
     * We will prepend 0 quads to our sequence of accumulator values so that the final accumulator value (equal to the
     * witness value if the range constraint holds) will be in the 4th column of an otherwise unused row. More
     * explicitly, in general (num_bits > 0), a_0 will be placed in column:
     *    - C if num_bits = 7, 8 mod 8
     *    - D if num_bits = 1, 2 mod 8
     *    - A if num_bits = 3, 4 mod 8
     *    - B if num_bits = 5, 6 mod 8
     *
     *  Examples:
     *          7,8-bit                   9,10-bit                11,12-bit                 13,14-bit
     * +-----+-----+-----+-----+ +-----+-----+-----+-----+ +-----+-----+-----+-----+ +-----+-----+-----+-----+
     * |  A  |  B  |  C  |  D  | |  A  |  B  |  C  |  D  | |  A  |  B  |  C  |  D  | |  A  |  B  |  C  |  D  |
     * +-----+-----+-----+-----+ +-----+-----+-----+-----+ +-----+-----+-----+-----+ +-----+-----+-----+-----+
     * | a2  | a1  | a0  | 0   | |  0  |  0  |  0  | 0   | | a0  |  0  |  0  | 0   | | a1  | a0  |  0  | 0   |
     * |  0  |  0  |  0  | a3  | | a3  | a2  | a1  | a0  | | a4  | a3  | a2  | a1  | | a5  | a4  | a3  | a2  |
     * | --- | --- | --- | --- | |  0  |  0  |  0  | a4  | |  0  |  0  |  0  | a5  | |  0  |  0  |  0  | a6  |
     * +-----+-----+-----+-----+ +-----+-----+-----+-----+ +-----+-----+-----+-----+ +-----+-----+-----+-----+
     *
     *
     **/

    const uint256_t witness_value(get_variable(witness_index));

    if (witness_value.get_msb() >= num_bits && !failed()) {
        failure(msg);
    }
    /* num_quad_gates is the minimum number of gates needed to record num_bits-many bits in a table, putting two-bits (a
     * quad) at each position. Since our table has width 4, we can fit 8 bits on a row, hence num_quad_gates is
     * num_bits/8 when num_bits is a multiple of 8, and otherwise it is 1 + (num_bits/8). Because we will always pre-pad
     * with 0 quads to ensure that the final accumulator is in the fourth column, num_quad_gates is also equal to one
     * less than the total number of rows that will be used to record the accumulator values. */
    size_t num_quad_gates = (num_bits >> 3);
    num_quad_gates = (num_quad_gates << 3 == num_bits) ? num_quad_gates : num_quad_gates + 1;

    std::vector<uint32_t>* wires[4]{ &w_4, &w_o, &w_r, &w_l };

    // num_quads = the number of accumulators used in the table, not including the output row.
    const size_t num_quads = (num_quad_gates << 2);
    // (num_quads << 1) is the number of bits in the non-output row, including 0 quads.
    // ((num_quads << 1) - num_bits) >> 1 is the number of padding 0 quads.
    const size_t forced_zero_threshold = 1 + (((num_quads << 1) - num_bits) >> 1);

    std::vector<uint32_t> accumulators;
    fr accumulator(0);
    uint32_t most_significant_segment = 0;
    // iterate through entries of all but final row
    for (size_t i = 0; i < num_quads + 1; ++i) {
        uint32_t accumulator_index;
        // prepend padding 0 quads
        if (i < forced_zero_threshold) {
            accumulator_index = zero_idx;
        } else {
            // accumulate quad
            const size_t bit_index = (num_quads - i) << 1;
            const uint64_t quad = static_cast<uint64_t>(witness_value.get_bit(bit_index)) +
                                  2ULL * static_cast<uint64_t>(witness_value.get_bit(bit_index + 1));
            const fr quad_element = fr{ quad, 0, 0, 0 }.to_montgomery_form();
            accumulator += accumulator;
            accumulator += accumulator;
            accumulator += quad_element;

            accumulator_index = add_variable(accumulator);
            accumulators.emplace_back(accumulator_index);

            if (i == forced_zero_threshold) {
                // mark this to constrain top bit to 0 in case num_bits is odd
                most_significant_segment = accumulator_index;
            }
        }

        (*(wires + (i & 3)))->emplace_back(accumulator_index);
    }

    // we use one additional gate to record the final accumulator value.
    size_t used_gates = 1 + num_quad_gates;
    for (size_t i = 0; i < used_gates; ++i) {
        q_m.emplace_back(0);
        q_1.emplace_back(0);
        q_2.emplace_back(0);
        q_3.emplace_back(0);
        q_c.emplace_back(0);
        q_arith.emplace_back(0);
        q_4.emplace_back(0);
        q_5.emplace_back(0);
        q_fixed_base.emplace_back(0);
        q_logic.emplace_back(0);
        q_range.emplace_back(1);
    }

    // switch off range widget for final row; fill wire values not in use with zeros
    q_range[q_range.size() - 1] = 0;
    w_l.emplace_back(zero_idx);
    w_r.emplace_back(zero_idx);
    w_o.emplace_back(zero_idx);

    assert_equal(witness_index, accumulators[accumulators.size() - 1], msg);

    accumulators[accumulators.size() - 1] = witness_index;

    num_gates += used_gates;

    // constrain top bit of top quad to zero in case num_bits is odd
    if ((num_bits & 1ULL) == 1ULL) {
        create_bool_gate(most_significant_segment);
    }
    return accumulators;
}

/**
 * @brief Implements AND and XOR.
 *
 * @returns A triple of vectors of accumulator values.
 *
 * @details If T is the returned triple, then the last element of T.left is guaranteed to be
 * our input a, regardless of a's relation with the rest of T.left. For instance, if num_bits is
 * smaller than the bit length of a, then the constraint that a is reproduced by T.left will fail:
 * for u = T.left[T.left.size()-2], u will be too small to express a in the form a = 4u + quad.
 * The same holds, mutatis mutandis, for T.right.
 */
accumulator_triple TurboComposer::create_logic_constraint(const uint32_t a,
                                                          const uint32_t b,
                                                          const size_t num_bits,
                                                          const bool is_xor_gate)
{
    TURBO_SELECTOR_REFS
    assert_valid_variables({ a, b });

    ASSERT(((num_bits >> 1U) << 1U) == num_bits); // Do not allow constraint for an odd number of bits.

    // one gate accmulates 1 quads, or 2 bits.
    // # gates = (bits / 2)
    const size_t num_quads = (num_bits >> 1);

    /*
     * The LOGIC constraint accumulates 3 base-4 values (a, b, c) into a sum, where c = a & b OR c = a ^ b
     *
     * In program memory, we place an accumulating base-4 sum of a, b, c {a_0, ..., a_{(num_bits/2)-1}}, where
     *         i
     *        ===
     *        \                              i - j
     * a   =  /    q                     .  4
     *  i     ===   (num_bits/2 - 1 - j)
     *       j = 0
     *
     * Note that a_0 = q_15, a_1 = 4.q_15 + q_14 = 4.a_0 + q_14, and, in general, we have the
     * accumulator relation
     *
     * a       =  4 a  + q                     (for i > 0).
     *  i + 1        i    num_bits/2 - 1 -i
     *
     * We can use our logic transition constraint to validate that
     *
     *
     *  a      - 4 . a  ϵ [0, 1, 2, 3]
     *   i + 1        i
     *
     *
     *
     *
     *  b      - 4 . b  ϵ [0, 1, 2, 3]
     *   i + 1        i
     *
     *
     *
     *                    /                 \          /                 \
     *  c      - 4 . c  = | a      - 4 . a  | (& OR ^) | b      - 4 . b  |
     *   i + 1        i   \  i + 1        i /          \  i + 1        i /
     *
     *
     * We also need the following temporary, w, stored in program memory:
     *
     *      /                 \   /                 \
     * w  = | a      - 4 . a  | * | b      - 4 . b  |
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
     * | 0   | 0   | w0  | 0   |
     * | a0  | b0  | w1  | c0  |
     * | a1  | b1  | w2  | c1  |
     * |  :  |  :  |  :  |  :  |
     * | aN  | bN  |  0  | cN  |     where N = num_bits/2 - 1  (num_bits is assumed even)
     * +-----+-----+-----+-----+
     *
     * Typically we will set num_bits = max(num_bits(a), num_bits(b)), so that c computes the AND or XOR
     * of a and b, depending on the value of is_xor_gate.
     *
     * Our transition constraint extracts quads by taking the difference between two accumulating sums,
     * so we need to start the chain with a row of zeroes
     *
     * One additional benefit of this constraint, is that both our inputs and output are in 'native' uint32 form.
     * This means we *never* have to decompose a uint32 into bits and back in order to chain together
     * addition and logic operations.
     **/

    const uint256_t left_witness_value(get_variable(a));
    const uint256_t right_witness_value(get_variable(b));

    accumulator_triple accumulators;
    fr left_accumulator = fr::zero();
    fr right_accumulator = fr::zero();
    fr out_accumulator = fr::zero();

    // Step 1: populate 1st row accumulators with zero
    w_l.emplace_back(zero_idx);
    w_r.emplace_back(zero_idx);
    w_4.emplace_back(zero_idx);

    // w_l, w_r, w_4 now point to 1 gate ahead of w_o
    for (size_t j = 0; j < num_quads; ++j) {
        uint32_t left_accumulator_index;
        uint32_t right_accumulator_index;
        uint32_t out_accumulator_index;
        uint32_t product_index;

        const size_t bit_index = (num_quads - 1 - j) << 1; // subscript of q as defined above
        // get quad coeffs of 4^{num_quads - 1 - j} in a and b, respectively
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

        // replace accumulator by 4.accumulator + quad for a, b and c
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
        q_fixed_base.emplace_back(fr::zero());
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

    assert_equal(a, accumulators.left[accumulators.left.size() - 1], "cannot reproduce `a` value using accumulator.");

    accumulators.left[accumulators.left.size() - 1] = a;

    assert_equal(b, accumulators.right[accumulators.right.size() - 1], "cannot reproduce `b` value using accumulator.");

    accumulators.right[accumulators.right.size() - 1] = b;

    num_gates += (num_quads + 1);

    return accumulators;
}

accumulator_triple TurboComposer::create_and_constraint(const uint32_t a, const uint32_t b, const size_t num_bits)
{
    return create_logic_constraint(a, b, num_bits, false);
}

accumulator_triple TurboComposer::create_xor_constraint(const uint32_t a, const uint32_t b, const size_t num_bits)
{
    return create_logic_constraint(a, b, num_bits, true);
}

uint32_t TurboComposer::put_constant_variable(const barretenberg::fr& variable)
{
    if (constant_variable_indices.contains(variable)) {
        return constant_variable_indices.at(variable);
    } else {
        uint32_t variable_index = add_variable(variable);
        fix_witness(variable_index, variable);
        constant_variable_indices.insert({ variable, variable_index });
        return variable_index;
    }
}

/**
 * Check if the circuit constraints hold.
 *
 * @return true if circuit is correct, false if not.
 * */
bool TurboComposer::check_circuit()
{
    // Initialize each of the kernels
    TurboArithmeticChecker arithmetic_checker;
    TurboRangeChecker range_checker;
    TurboLogicChecker logic_checker;
    TurboFixedBaseChecker fixed_base_checker;

    // Create various challenge_arrays
    plonk::widget::containers::challenge_array<barretenberg::fr, TurboArithmeticChecker::num_independent_relations>
        arithmetic_challenges;
    plonk::widget::containers::challenge_array<barretenberg::fr, TurboRangeChecker::num_independent_relations>
        range_challenges;
    plonk::widget::containers::challenge_array<barretenberg::fr, TurboLogicChecker::num_independent_relations>
        logic_challenges;
    plonk::widget::containers::challenge_array<barretenberg::fr, TurboFixedBaseChecker::num_independent_relations>
        fixed_base_challenges;

    plonk::widget::containers::coefficient_array<barretenberg::fr> linear_parts;
    barretenberg::fr result_nonlinear_part;
    barretenberg::fr result_linear_part;

    // Create different challenges for each type of widget (maximizes probability of failure)
    for (size_t i = 0; i < arithmetic_challenges.elements.size(); i++) {
        arithmetic_challenges.elements[i] = fr::random_element();
        range_challenges.elements[i] = fr::random_element();
        logic_challenges.elements[i] = fr::random_element();

        fixed_base_challenges.elements[i] = fr::random_element();
    }

    // Instead of powers of alpha, fill all "alpha_powers" with random elements
    for (size_t i = 0; i < arithmetic_challenges.alpha_powers.size(); i++) {
        arithmetic_challenges.alpha_powers[i] = fr::random_element();
    }
    for (size_t i = 0; i < range_challenges.alpha_powers.size(); i++) {
        range_challenges.alpha_powers[i] = fr::random_element();
    }
    for (size_t i = 0; i < logic_challenges.alpha_powers.size(); i++) {
        logic_challenges.alpha_powers[i] = fr::random_element();
    }
    for (size_t i = 0; i < fixed_base_challenges.alpha_powers.size(); i++) {
        fixed_base_challenges.alpha_powers[i] = fr::random_element();
    }
    // Check for each gate
    for (size_t i = 0; i < get_num_gates(); i++) {
        result_nonlinear_part = barretenberg::fr::zero();
        // Do arithmetic
        if (arithmetic_checker.gate_enabled(*this, i)) {
            arithmetic_checker.compute_linear_terms(*this, arithmetic_challenges, linear_parts, i);
            result_linear_part = arithmetic_checker.sum_linear_terms(*this, arithmetic_challenges, linear_parts, i);
            arithmetic_checker.compute_non_linear_terms(*this, arithmetic_challenges, result_nonlinear_part, i);
            if (!(result_linear_part + result_nonlinear_part).is_zero()) {
#ifndef FUZZING
                std::cerr << "Arithmetic error at gate " << i << std::endl;
#endif
                return false;
            }
        }

        // Do range
        if (range_checker.gate_enabled(*this, i)) {
            range_checker.compute_linear_terms(*this, range_challenges, linear_parts, i);
            result_linear_part = range_checker.sum_linear_terms(*this, range_challenges, linear_parts, i);
            range_checker.compute_non_linear_terms(*this, range_challenges, result_nonlinear_part, i);
            if (!(result_linear_part + result_nonlinear_part).is_zero()) {
#ifndef FUZZING
                std::cerr << "Range error at gate " << i << std::endl;
#endif
                return false;
            }
        }
        // Do logic
        if (logic_checker.gate_enabled(*this, i)) {
            logic_checker.compute_linear_terms(*this, logic_challenges, linear_parts, i);
            result_linear_part = logic_checker.sum_linear_terms(*this, logic_challenges, linear_parts, i);
            logic_checker.compute_non_linear_terms(*this, logic_challenges, result_nonlinear_part, i);
            if (!(result_linear_part + result_nonlinear_part).is_zero()) {
#ifndef FUZZING
                std::cerr << "Logic error at gate " << i << std::endl;
#endif
                return false;
            }
        }
        // Do fixed base
        if (fixed_base_checker.gate_enabled(*this, i)) {
            fixed_base_checker.compute_linear_terms(*this, fixed_base_challenges, linear_parts, i);
            result_linear_part = fixed_base_checker.sum_linear_terms(*this, fixed_base_challenges, linear_parts, i);
            fixed_base_checker.compute_non_linear_terms(*this, fixed_base_challenges, result_nonlinear_part, i);
            // Fixed base is the only one where independent parts are not zero
            if (!(result_linear_part + result_nonlinear_part).is_zero()) {
#ifndef FUZZING
                std::cerr << "Fixed base error at gate " << i << std::endl;
#endif
                return false;
            }
        }
    }
    return true;
}

std::shared_ptr<proving_key> TurboComposer::compute_proving_key()
{
    if (circuit_proving_key) {
        return circuit_proving_key;
    }

    ComposerBase::compute_proving_key_base(type);

    compute_sigma_permutations<4, false>(circuit_proving_key.get());

    circuit_proving_key->recursive_proof_public_input_indices =
        std::vector<uint32_t>(recursive_proof_public_input_indices.begin(), recursive_proof_public_input_indices.end());
    circuit_proving_key->contains_recursive_proof = contains_recursive_proof;
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
        ComposerBase::compute_verification_key_base(circuit_proving_key, crs_factory_->get_verifier_crs());
    circuit_verification_key->composer_type = type;
    circuit_verification_key->recursive_proof_public_input_indices =
        std::vector<uint32_t>(recursive_proof_public_input_indices.begin(), recursive_proof_public_input_indices.end());
    circuit_verification_key->contains_recursive_proof = contains_recursive_proof;

    return circuit_verification_key;
}

void TurboComposer::compute_witness()
{

    ComposerBase::compute_witness_base<turbo_settings::program_width>();
}

TurboProver TurboComposer::create_prover()
{
    compute_proving_key();
    compute_witness();

    TurboProver output_state(circuit_proving_key, create_manifest(public_inputs.size()));

    std::unique_ptr<ProverPermutationWidget<4, false>> permutation_widget =
        std::make_unique<ProverPermutationWidget<4, false>>(circuit_proving_key.get());

    std::unique_ptr<ProverTurboRangeWidget<turbo_settings>> range_widget =
        std::make_unique<ProverTurboRangeWidget<turbo_settings>>(circuit_proving_key.get());
    std::unique_ptr<ProverTurboLogicWidget<turbo_settings>> logic_widget =
        std::make_unique<ProverTurboLogicWidget<turbo_settings>>(circuit_proving_key.get());

    std::unique_ptr<ProverTurboArithmeticWidget<turbo_settings>> arithmetic_widget =
        std::make_unique<ProverTurboArithmeticWidget<turbo_settings>>(circuit_proving_key.get());
    std::unique_ptr<ProverTurboFixedBaseWidget<turbo_settings>> fixed_base_widget =
        std::make_unique<ProverTurboFixedBaseWidget<turbo_settings>>(circuit_proving_key.get());

    output_state.random_widgets.emplace_back(std::move(permutation_widget));

    output_state.transition_widgets.emplace_back(std::move(arithmetic_widget));
    output_state.transition_widgets.emplace_back(std::move(fixed_base_widget));
    output_state.transition_widgets.emplace_back(std::move(range_widget));
    output_state.transition_widgets.emplace_back(std::move(logic_widget));

    std::unique_ptr<KateCommitmentScheme<turbo_settings>> kate_commitment_scheme =
        std::make_unique<KateCommitmentScheme<turbo_settings>>();

    output_state.commitment_scheme = std::move(kate_commitment_scheme);

    return output_state;
}

TurboVerifier TurboComposer::create_verifier()
{
    compute_verification_key();

    std::unique_ptr<KateCommitmentScheme<turbo_settings>> kate_commitment_scheme =
        std::make_unique<KateCommitmentScheme<turbo_settings>>();

    TurboVerifier output_state(circuit_verification_key, create_manifest(public_inputs.size()));

    output_state.commitment_scheme = std::move(kate_commitment_scheme);

    return output_state;
}
} // namespace proof_system::plonk
