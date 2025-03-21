#include "barretenberg/vm/avm/tests/helpers.test.hpp"
#include "barretenberg/vm/avm/generated/flavor.hpp"
#include "barretenberg/vm/avm/trace/helper.hpp"
#include "barretenberg/vm/avm/trace/public_inputs.hpp"
#include "barretenberg/vm/constants.hpp"
#include "common.test.hpp"

namespace tests_avm {

using namespace bb;

std::vector<ThreeOpParamRow> gen_three_op_params(std::vector<ThreeOpParam> operands,
                                                 std::vector<bb::avm_trace::AvmMemoryTag> mem_tags)
{
    std::vector<ThreeOpParamRow> params;
    for (size_t i = 0; i < 5; i++) {
        params.emplace_back(operands[i], mem_tags[i]);
    }
    return params;
}
/**
 * @brief Helper routine checking the circuit constraints without proving
 *
 * @param trace The execution trace
 */
void validate_trace_check_circuit(std::vector<Row>&& trace)
{
    auto circuit_builder = bb::avm::AvmCircuitBuilder();
    circuit_builder.set_trace(std::move(trace));
    EXPECT_TRUE(circuit_builder.check_circuit());
};

/**
 * @brief Helper routine which checks the circuit constraints and depending on
 *        the boolean with_proof value performs a proof generation and verification.
 *
 * @param trace The execution trace
 */
void validate_trace(std::vector<Row>&& trace,
                    AvmPublicInputs const& public_inputs,
                    [[maybe_unused]] std::vector<FF> const& calldata,
                    [[maybe_unused]] std::vector<FF> const& returndata,
                    bool with_proof,
                    bool expect_proof_failure)
{
    // This is here for our nighly test runs.
    with_proof |= std::getenv("AVM_ENABLE_FULL_PROVING") != nullptr;

    const std::string avm_dump_trace_path =
        std::getenv("AVM_DUMP_TRACE_PATH") != nullptr ? std::getenv("AVM_DUMP_TRACE_PATH") : "";
    if (!avm_dump_trace_path.empty()) {
        info("Dumping trace as CSV to: " + avm_dump_trace_path);
        avm_trace::dump_trace_as_csv(trace, avm_dump_trace_path);
    }

    // Inject computed end gas values in the public inputs
    // This is ok because validate_trace is only used in cpp tests.
    // TS integration tests will provide the correct end gas values in the public inputs and
    // this will be validated.
    auto public_inputs_with_end_gas = public_inputs;
    avm_trace::inject_end_gas_values(public_inputs_with_end_gas, trace);

    auto circuit_builder = bb::avm::AvmCircuitBuilder();
    circuit_builder.set_trace(std::move(trace));
    EXPECT_TRUE(circuit_builder.check_circuit());

    if (with_proof) {
        bb::avm::AvmComposer composer = bb::avm::AvmComposer();
        bb::avm::AvmProver prover = composer.create_prover(circuit_builder);
        HonkProof proof = prover.construct_proof();

        bb::avm::AvmVerifier verifier = composer.create_verifier(circuit_builder);

        // At the current development stage (new public inputs for whole tx), we are not handling public related inputs
        // except calldata and returndata.
        std::vector<std::vector<FF>> public_inputs_as_vec{ {}, {}, {}, {}, calldata, returndata };
        // TODO: Copy all public inputs
        // bb::avm_trace::copy_public_inputs_columns(public_inputs_with_end_gas, calldata, returndata);

        bool verified = verifier.verify_proof(proof, { public_inputs_as_vec });

        if (expect_proof_failure) {
            EXPECT_FALSE(verified);
        } else {
            EXPECT_TRUE(verified);
        }
    }
};

/**
 * @brief Helper routine for the negative tests. It mutates the output value of an operation
 *        located in the Ic intermediate register. The memory trace is adapted consistently.
 *
 * @param trace Execution trace
 * @param selectRow Lambda serving to select the row in trace
 * @param newValue The value that will be written in intermediate register Ic at the selected row.
 * @param alu A boolean telling whether we mutate the ic value in alu as well.
 */
void mutate_ic_in_trace(std::vector<Row>& trace, std::function<bool(Row)>&& selectRow, FF const& newValue, bool alu)
{
    // Find the first row matching the criteria defined by selectRow
    auto row = std::ranges::find_if(trace.begin(), trace.end(), selectRow);

    // Check that we found one
    EXPECT_TRUE(row != trace.end());

    // Mutate the correct result in the main trace
    row->main_ic = newValue;

    // Optionally mutate the corresponding ic value in alu
    if (alu) {
        auto const clk = row->main_clk;
        // Find the relevant alu trace entry.
        auto alu_row = std::ranges::find_if(trace.begin(), trace.end(), [clk](Row r) { return r.alu_clk == clk; });

        EXPECT_TRUE(alu_row != trace.end());
        alu_row->alu_ic = newValue;
    }

    // Adapt the memory trace to be consistent with the wrong result
    auto const clk = row->main_clk;
    auto const addr = row->main_mem_addr_c;

    // Find the relevant memory trace entry.
    auto mem_row = std::ranges::find_if(
        trace.begin(), trace.end(), [clk, addr](Row r) { return r.mem_clk == clk && r.mem_addr == addr; });

    EXPECT_TRUE(mem_row != trace.end());
    mem_row->mem_val = newValue;
};

AvmPublicInputs generate_base_public_inputs()
{
    AvmPublicInputs public_inputs;
    public_inputs.gas_settings.gas_limits.l2_gas = DEFAULT_INITIAL_L2_GAS;
    public_inputs.gas_settings.gas_limits.da_gas = DEFAULT_INITIAL_DA_GAS;
    return public_inputs;
}

} // namespace tests_avm
