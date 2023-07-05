#include "circuit_simulator.hpp"
#include <gtest/gtest.h>

namespace {
auto& engine = numeric::random::get_debug_engine();
}

namespace proof_system::circuit_simulator_tests {

class CircuitSimulatorBN254Test : public ::testing::Test {};

TEST(CircuitSimulatorBN254Test, Base)
{
    CircuitSimulatorBN254 circuit;
    // NT::fr const& amount = 5;
    // NT::fr const& asset_id = 1;
    // NT::fr const& memo = 999;
    // std::array<NT::fr, NUM_FIELDS_PER_SHA256> const& empty_logs_hash = { NT::fr(16), NT::fr(69) };
    // NT::fr const& empty_log_preimages_length = NT::fr(100);

    // // Generate private inputs including proofs and vkeys for app circuit and previous kernel
    // auto const& private_inputs = do_private_call_get_kernel_inputs_inner(false,
    //                                                                      deposit,
    //                                                                      { amount, asset_id, memo },
    //                                                                      empty_logs_hash,
    //                                                                      empty_logs_hash,
    //                                                                      empty_log_preimages_length,
    //                                                                      empty_log_preimages_length,
    //                                                                      empty_logs_hash,
    //                                                                      empty_logs_hash,
    //                                                                      empty_log_preimages_length,
    //                                                                      empty_log_preimages_length,
    //                                                                      true);

    // // Execute and prove the first kernel iteration
    // Builder private_kernel_builder;
    // auto const& public_inputs = private_kernel_circuit(private_kernel_builder, private_inputs, true);

    // // Check the private kernel circuit
    // EXPECT_TRUE(private_kernel_builder.check_circuit());
}
} // namespace proof_system::circuit_simulator_tests