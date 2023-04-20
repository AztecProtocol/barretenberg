#include "ultra_circuit_constructor.hpp"
#include <gtest/gtest.h>

using namespace barretenberg;

namespace {
auto& engine = numeric::random::get_debug_engine();
}
namespace proof_system {
using plookup::ColumnIdx;
using plookup::MultiTableId;

TEST(ultra_circuit_constructor, create_gates_from_plookup_accumulators)
{
    UltraCircuitConstructor circuit_constructor = UltraCircuitConstructor();

    barretenberg::fr input_value = fr::random_element();
    const fr input_hi = uint256_t(input_value).slice(126, 256);
    const fr input_lo = uint256_t(input_value).slice(0, 126);
    const auto input_hi_index = circuit_constructor.add_variable(input_hi);
    const auto input_lo_index = circuit_constructor.add_variable(input_lo);

    const auto sequence_data_hi = plookup::get_lookup_accumulators(MultiTableId::PEDERSEN_LEFT_HI, input_hi);
    const auto sequence_data_lo = plookup::get_lookup_accumulators(MultiTableId::PEDERSEN_LEFT_LO, input_lo);

    const auto lookup_witnesses_hi = circuit_constructor.create_gates_from_plookup_accumulators(
        MultiTableId::PEDERSEN_LEFT_HI, sequence_data_hi, input_hi_index);
    const auto lookup_witnesses_lo = circuit_constructor.create_gates_from_plookup_accumulators(
        MultiTableId::PEDERSEN_LEFT_LO, sequence_data_lo, input_lo_index);

    std::vector<barretenberg::fr> expected_x;
    std::vector<barretenberg::fr> expected_y;

    const size_t num_lookups_hi =
        (128 + crypto::pedersen_hash::lookup::BITS_PER_TABLE) / crypto::pedersen_hash::lookup::BITS_PER_TABLE;
    const size_t num_lookups_lo = 126 / crypto::pedersen_hash::lookup::BITS_PER_TABLE;
    const size_t num_lookups = num_lookups_hi + num_lookups_lo;

    EXPECT_EQ(num_lookups_hi, lookup_witnesses_hi[ColumnIdx::C1].size());
    EXPECT_EQ(num_lookups_lo, lookup_witnesses_lo[ColumnIdx::C1].size());

    std::vector<barretenberg::fr> expected_scalars;
    expected_x.resize(num_lookups);
    expected_y.resize(num_lookups);
    expected_scalars.resize(num_lookups);

    {
        const size_t num_rounds = (num_lookups + 1) / 2;
        uint256_t bits(input_value);

        const auto mask = crypto::pedersen_hash::lookup::PEDERSEN_TABLE_SIZE - 1;

        for (size_t i = 0; i < num_rounds; ++i) {
            const auto& table = crypto::pedersen_hash::lookup::get_table(i);
            const size_t index = i * 2;

            uint64_t slice_a = ((bits >> (index * 9)) & mask).data[0];
            expected_x[index] = (table[(size_t)slice_a].x);
            expected_y[index] = (table[(size_t)slice_a].y);
            expected_scalars[index] = slice_a;

            if (i < 14) {
                uint64_t slice_b = ((bits >> ((index + 1) * 9)) & mask).data[0];
                expected_x[index + 1] = (table[(size_t)slice_b].x);
                expected_y[index + 1] = (table[(size_t)slice_b].y);
                expected_scalars[index + 1] = slice_b;
            }
        }
    }

    for (size_t i = num_lookups - 2; i < num_lookups; --i) {
        expected_scalars[i] += (expected_scalars[i + 1] * crypto::pedersen_hash::lookup::PEDERSEN_TABLE_SIZE);
    }

    size_t hi_shift = 126;
    const fr hi_cumulative = circuit_constructor.get_variable(lookup_witnesses_hi[ColumnIdx::C1][0]);
    for (size_t i = 0; i < num_lookups_lo; ++i) {
        const fr hi_mult = fr(uint256_t(1) << hi_shift);
        EXPECT_EQ(circuit_constructor.get_variable(lookup_witnesses_lo[ColumnIdx::C1][i]) + (hi_cumulative * hi_mult),
                  expected_scalars[i]);
        EXPECT_EQ(circuit_constructor.get_variable(lookup_witnesses_lo[ColumnIdx::C2][i]), expected_x[i]);
        EXPECT_EQ(circuit_constructor.get_variable(lookup_witnesses_lo[ColumnIdx::C3][i]), expected_y[i]);
        hi_shift -= crypto::pedersen_hash::lookup::BITS_PER_TABLE;
    }

    for (size_t i = 0; i < num_lookups_hi; ++i) {
        EXPECT_EQ(circuit_constructor.get_variable(lookup_witnesses_hi[ColumnIdx::C1][i]),
                  expected_scalars[i + num_lookups_lo]);
        EXPECT_EQ(circuit_constructor.get_variable(lookup_witnesses_hi[ColumnIdx::C2][i]),
                  expected_x[i + num_lookups_lo]);
        EXPECT_EQ(circuit_constructor.get_variable(lookup_witnesses_hi[ColumnIdx::C3][i]),
                  expected_y[i + num_lookups_lo]);
    }
    bool result = circuit_constructor.check_circuit();

    EXPECT_EQ(result, true);
}
TEST(ultra_circuit_constructor, base_case)
{
    UltraCircuitConstructor circuit_constructor = UltraCircuitConstructor();
    fr a = fr::one();
    circuit_constructor.add_public_variable(a);
    bool result = circuit_constructor.check_circuit();
    EXPECT_EQ(result, true);
}
TEST(ultra_circuit_constructor, test_no_lookup_proof)
{
    UltraCircuitConstructor circuit_constructor = UltraCircuitConstructor();

    for (size_t i = 0; i < 16; ++i) {
        for (size_t j = 0; j < 16; ++j) {
            uint64_t left = static_cast<uint64_t>(j);
            uint64_t right = static_cast<uint64_t>(i);
            uint32_t left_idx = circuit_constructor.add_variable(fr(left));
            uint32_t right_idx = circuit_constructor.add_variable(fr(right));
            uint32_t result_idx = circuit_constructor.add_variable(fr(left ^ right));

            uint32_t add_idx =
                circuit_constructor.add_variable(fr(left) + fr(right) + circuit_constructor.get_variable(result_idx));
            circuit_constructor.create_big_add_gate(
                { left_idx, right_idx, result_idx, add_idx, fr(1), fr(1), fr(1), fr(-1), fr(0) });
        }
    }

    bool result = circuit_constructor.check_circuit();
    EXPECT_EQ(result, true);
}
} // namespace proof_system