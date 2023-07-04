#include "goblin_ultra_circuit_builder.hpp"
#include "barretenberg/crypto/generators/generator_data.hpp"
#include <gtest/gtest.h>

using namespace barretenberg;

namespace {
auto& engine = numeric::random::get_debug_engine();
}
namespace proof_system {

TEST(goblin_ultra_circuit_constructor, base_case)
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

TEST(goblin_ultra_circuit_constructor, simple)
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

} // namespace proof_system