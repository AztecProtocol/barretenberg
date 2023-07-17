#include "barretenberg/crypto/generators/generator_data.hpp"
#include "goblin_ultra_circuit_builder.hpp"
#include <gtest/gtest.h>

using namespace barretenberg;

namespace {
auto& engine = numeric::random::get_debug_engine();
}
namespace proof_system {

/**
 * @brief Simple add gate to check basic Ultra functionality
 *
 */
TEST(GoblinCircuitBuilder, BaseCase)
{
    auto circuit_constructor = GoblinUltraCircuitBuilder();
    fr a = fr::random_element();
    fr b = fr::random_element();

    auto a_idx = circuit_constructor.add_variable(a);
    auto b_idx = circuit_constructor.add_variable(b);
    auto c_idx = circuit_constructor.add_variable(a + b);

    circuit_constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), -fr::one(), fr::zero() });

    circuit_constructor.get_num_gates();

    EXPECT_EQ(circuit_constructor.check_circuit(), true);
}

/**
 * @brief Test correctness of native ecc computations performed behind the scenes when adding simple ecc op gates
 *
 */
TEST(GoblinCircuitBuilder, Simple)
{
    auto builder = GoblinUltraCircuitBuilder();

    // Compute a simple point accumulation natively
    auto P1 = g1::affine_element::random_element();
    auto P2 = g1::affine_element::random_element();
    auto z = fr::random_element();
    auto P_expected = P1 + P2 * z;

    // Add gates corresponding to the above operations
    builder.queue_ecc_add_accum(P1);
    builder.queue_ecc_mul_accum(P2, z);

    // Extract current accumulator point from the op queue and check the result
    auto P_result = builder.op_queue.get_accumulator();
    EXPECT_EQ(P_result, P_expected);

    // Use the result to add equality op gates
    builder.queue_ecc_eq(P_result);

    // Check that the accumulator in the op queue has been reset
    auto accumulator = builder.op_queue.get_accumulator();
    EXPECT_EQ(accumulator, g1::affine_point_at_infinity);
}

/**
 * @brief Test correctness of native ecc batch mul performed behind the scenes when adding ecc op gates for a batch mul
 *
 */
TEST(GoblinCircuitBuilder, BatchMul)
{
    using Point = g1::affine_element;
    using Scalar = fr;

    auto builder = GoblinUltraCircuitBuilder();
    const size_t num_muls = 3;

    // Compute some random points and scalars to batch multiply
    std::vector<Point> points;
    std::vector<Scalar> scalars;
    auto batched_expected = Point::infinity();
    for (size_t i = 0; i < num_muls; ++i) {
        points.emplace_back(Point::random_element());
        scalars.emplace_back(Scalar::random_element());
        batched_expected = batched_expected + points[i] * scalars[i];
    }

    // Populate the batch mul operands in the op wires and natively compute the result
    auto batched_result = builder.batch_mul(points, scalars);

    // Extract current accumulator point from the op queue and check the result
    EXPECT_EQ(batched_result, batched_expected);
}

} // namespace proof_system