/**
 * @file ultra_circuit_builder.cpp
 * @author Luke (ledwards2225) and Kesha (Rumata888)
 * @brief This file contains the implementation of UltraCircuitBuilder class that defines the logic of ultra-style
 * circuits and is intended for the use in UltraHonk and UltraPlonk systems
 *
 * @todo 1) Replace barretenberg::fr with templated FF or Field
 *
 */
#include "goblin_ultra_circuit_builder.hpp"
#include <barretenberg/plonk/proof_system/constants.hpp>
#include <unordered_map>
#include <unordered_set>

using namespace barretenberg;

namespace proof_system {

/**
 * @brief Add gates corresponding to a batched mul
 *
 * @param points
 * @param scalars
 * @return g1::affine_element Result of batched mul
 */
g1::affine_element GoblinUltraCircuitBuilder::batch_mul(const std::vector<g1::affine_element>& points,
                                                        const std::vector<fr>& scalars)
{
    // WORKTODO: This may need some checks, e.g.
    ASSERT(op_queue.get_accumulator().is_point_at_infinity());

    size_t num_muls = points.size();
    for (size_t idx = 0; idx < num_muls; ++idx) {
        queue_ecc_mul_accum(points[idx], scalars[idx]);
    }
    return op_queue.get_accumulator();
}

/**
 * @brief Add gates for simple point addition without scalar and compute corresponding op natively
 *
 * @param point
 */
void GoblinUltraCircuitBuilder::queue_ecc_add_accum(const barretenberg::g1::affine_element& point)
{
    // Add raw op to queue
    op_queue.add_accumulate(point);

    // Add ecc op gates
    uint32_t op = 0; // WORKTODO: how to specify these values. ENUM?
    add_ecc_op_gates(op, point);
}

/**
 * @brief Add gates for point mul and add and compute corresponding op natively
 *
 * @param point
 * @param scalar
 */
void GoblinUltraCircuitBuilder::queue_ecc_mul_accum(const barretenberg::g1::affine_element& point,
                                                    const barretenberg::fr& scalar)
{
    // Add raw op to op queue
    op_queue.mul_accumulate(point, scalar);

    // Add ecc op gates
    uint32_t op = 1; // WORKTODO: Enum?
    add_ecc_op_gates(op, point, scalar);
}

/**
 * @brief Add point equality gates
 *
 * @param point
 */
void GoblinUltraCircuitBuilder::queue_ecc_eq(const barretenberg::g1::affine_element& point)
{
    // Add raw op to op queue
    op_queue.eq(point);

    // Add ecc op gates
    uint32_t op = 2;
    add_ecc_op_gates(op, point);
}

/**
 * @brief Add ecc op gates for specified operation
 *
 * @param op Op code
 * @param point
 * @param scalar
 */
void GoblinUltraCircuitBuilder::add_ecc_op_gates(uint32_t op, const g1::affine_element& point, const fr& scalar)
{
    auto op_tuple = make_ecc_op_tuple(op, point, scalar);

    queue_ecc_op(op_tuple);
}

/**
 * @brief Decompose ecc operands into components, add corresponding variables, return ecc op index tuple
 *
 * @param op
 * @param point
 * @param scalar
 * @return ecc_op_tuple Tuple of indices into variables array used to construct pair of ecc op gates
 */
ecc_op_tuple GoblinUltraCircuitBuilder::make_ecc_op_tuple(uint32_t op,
                                                          const g1::affine_element& point,
                                                          const fr& scalar)
{
    auto x_256 = uint256_t(point.x);
    auto y_256 = uint256_t(point.y);
    auto x_lo_idx = add_variable(x_256.slice(0, NUM_LIMB_BITS * 2));
    auto x_hi_idx = add_variable(x_256.slice(NUM_LIMB_BITS * 2, NUM_LIMB_BITS * 4));
    auto y_lo_idx = add_variable(y_256.slice(0, NUM_LIMB_BITS * 2));
    auto y_hi_idx = add_variable(y_256.slice(NUM_LIMB_BITS * 2, NUM_LIMB_BITS * 4));

    // Split scalar into 128 bit endomorphism scalars
    fr z_1 = 0;
    fr z_2 = 0;
    // WORKTODO: do I need to do this montgomery conversion here?
    // auto converted = scalar.from_montgomery_form();
    // fr::split_into_endomorphism_scalars(converted, z_1, z_2);
    // z_1 = z_1.to_montgomery_form();
    // z_2 = z_2.to_montgomery_form();
    fr::split_into_endomorphism_scalars(scalar, z_1, z_2);
    auto z_1_idx = add_variable(z_1);
    auto z_2_idx = add_variable(z_2);

    return { op, x_lo_idx, x_hi_idx, y_lo_idx, y_hi_idx, z_1_idx, z_2_idx };
}

/**
 * @brief Add ecc operation to queue
 *
 * @param in Variables array indices corresponding to operation inputs
 * @note We dont increment the gate count here since these will simply be placed at the zeroth gate index inside a block
 * of predetermined size
 */
void GoblinUltraCircuitBuilder::queue_ecc_op(const ecc_op_tuple& in)
{
    op_wire_1.emplace_back(in.op);
    op_wire_2.emplace_back(in.x_lo);
    op_wire_3.emplace_back(in.x_hi);
    op_wire_4.emplace_back(in.y_lo);

    op_wire_1.emplace_back(in.op); // WORKTODO: sort of a dummy. is "op" ok?
    op_wire_2.emplace_back(in.y_hi);
    op_wire_3.emplace_back(in.z_lo);
    op_wire_4.emplace_back(in.z_hi);

    num_ecc_op_gates += 2;
};

} // namespace proof_system