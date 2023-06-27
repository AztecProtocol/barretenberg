#include "barretenberg/honk/flavor/flavor.hpp"
#include "ecc_wnaf_relation.hpp"
#include "ecc_vm_types.hpp"
#include "ecc_point_table_relation.hpp"

#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/numeric/random/engine.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"
#include "barretenberg/honk/flavor/ecc_vm.hpp"
#include "barretenberg/honk/sumcheck/sumcheck.hpp"

#include <gtest/gtest.h>
/**
 * We want to test if all three relations (namely, ArithmeticRelation, GrandProductComputationRelation,
 * GrandProductInitializationRelation) provide correct contributions by manually computing their
 * contributions with deterministic and random inputs. The relations are supposed to work with
 * univariates (edges) of degree one (length 2) and spit out polynomials of corresponding degrees. We have
 * MAX_RELATION_LENGTH = 5, meaning the output of a relation can atmost be a degree 5 polynomial. Hence,
 * we use a method compute_mock_extended_edges() which starts with degree one input polynomial (two evaluation
 points),
 * extends them (using barycentric formula) to six evaluation points, and stores them to an array of polynomials.
 */
// static const size_t input_univariate_length = 2;
// static constexpr size_t FULL_RELATION_LENGTH = 5;

using namespace proof_system::honk::sumcheck;
using Flavor = proof_system::honk::flavor::ECCVM;
using FF = typename Flavor::FF;
using ProverPolynomials = typename Flavor::ProverPolynomials;
using RawPolynomials = typename Flavor::FoldedPolynomials;

template <typename Field> using ECCVMWnafAlgebra = proof_system::honk::sumcheck::eccvm::ECCVMWnafAlgebra<Field>;

template <typename Field> using ECCVMWnafProver = proof_system::honk::sumcheck::eccvm::ECCVMWnafProver<Field>;

template <typename Field> using ECCVMWnafVerifier = proof_system::honk::sumcheck::eccvm::ECCVMWnafVerifier<Field>;

template <typename Field>
using ECCVMPointTableAlgebra = proof_system::honk::sumcheck::eccvm::ECCVMPointTableAlgebra<Field>;

template <typename Field>
using ECCVMPointTableProver = proof_system::honk::sumcheck::eccvm::ECCVMPointTableProver<Field>;

template <typename Field>
using ECCVMPointTableVerifier = proof_system::honk::sumcheck::eccvm::ECCVMPointTableVerifier<Field>;

static constexpr size_t NUM_POLYNOMIALS = Flavor::NUM_ALL_ENTITIES;

namespace proof_system::honk_relation_tests_eccwnaf {

namespace {
auto& engine = numeric::random::get_debug_engine();
}

template <typename, size_t> struct TestWrapper {

    using UnivariateView = barretenberg::fr;

    using Univariate = barretenberg::fr;

    using RelationParameters = barretenberg::fr;
};

struct Opcode {
    bool add;
    bool mul;
    bool eq;
    bool accumulate;
    // bool set;
    [[nodiscard]] size_t value() const
    {
        auto res = static_cast<size_t>(add);
        res += res;
        res += static_cast<size_t>(mul);
        res += res;
        res += static_cast<size_t>(eq);
        res += res;
        res += static_cast<size_t>(accumulate);
        // res += res;
        // res += static_cast<size_t>(set);
        return res;
    }
};

struct ExecutionTrace {
    using EccWnafRow = Flavor::RowPolynomials;
    static constexpr size_t NUM_SCALAR_BITS = 128;
    static constexpr size_t WNAF_SLICE_BITS = 4;
    static constexpr size_t NUM_WNAF_SLICES = (NUM_SCALAR_BITS + WNAF_SLICE_BITS - 1) / WNAF_SLICE_BITS;
    static constexpr uint64_t WNAF_MASK = static_cast<uint64_t>((1ULL << WNAF_SLICE_BITS) - 1ULL);
    static constexpr size_t WNAF_SLICES_PER_ROW = 4;

    std::vector<EccWnafRow> rows;

    struct VMScalarMul {
        uint32_t pc;
        uint256_t scalar;
        grumpkin::g1::affine_element base_point;
    };
    std::vector<VMScalarMul> ecc_muls;

    struct WNAFSlices {
        std::array<int, NUM_WNAF_SLICES> slices;
        bool skew;
    };

    struct VMTableState {
        int s1 = 0;
        int s2 = 0;
        int s3 = 0;
        int s4 = 0;
        int s5 = 0;
        int s6 = 0;
        int s7 = 0;
        int s8 = 0;
        bool skew = false;
        bool point_transition = false;
        uint32_t pc = 0;
        size_t round = 0;
        uint256_t scalar_sum = 0;
    };

    struct VMPointState {
        grumpkin::g1::affine_element T{ 0, 0 };
        grumpkin::g1::affine_element D{ 0, 0 };
        bool point_transition = false;
        uint32_t pc = 0;
        size_t round = 0;
    };

    /**
     * @brief Convert a 128-bit input scalar into a sequence of windowed non-adjacent-form slices.
     *        Each WNAF slice is a 4-bit value ranging from [-15, -13, ..., -1, 1, ..., 13, 15]
     *        i.e. odd values only.
     *        We do this because it's easy to negate elliptic curve points.
     *        When computing point tables, we only precompute [P, 3P, ..., 15P] and we get the negations for free
     *
     * @param scalar
     */
    static WNAFSlices convert_to_slices(uint256_t scalar)
    {

        ASSERT(scalar.get_msb() < 128);
        WNAFSlices output;

        int previous_slice = 0;
        for (size_t i = 0; i < NUM_WNAF_SLICES; ++i) {
            // slice the scalar into 4-bit chunks, starting with the least significant bits
            uint64_t raw_slice = static_cast<uint64_t>(scalar) & WNAF_MASK;

            bool is_even = ((raw_slice & 1ULL) == 0ULL);

            int wnaf_slice = static_cast<int>(raw_slice);
            if (i == 0) {
                output.skew = is_even;
            }
            if (i == 0 && is_even) {
                // if least significant slice is even, we add 1 to create an odd value && set 'skew' to true
                wnaf_slice += 1;
            } else if (is_even) {
                // for other slices, if it's even, we add 1 to the slice value
                // and subtract 16 from the previous slice to preserve the total scalar sum
                static constexpr int borrow_constant = static_cast<int>(1ULL << WNAF_SLICE_BITS);
                previous_slice -= borrow_constant;
                wnaf_slice += 1;
            }

            if (i > 0) {
                const size_t idx = i - 1;
                output.slices[NUM_WNAF_SLICES - idx - 1] = previous_slice;
            }
            previous_slice = wnaf_slice;

            // downshift raw_slice by 4 bits
            scalar = scalar >> WNAF_SLICE_BITS;
        }

        ASSERT(scalar == 0);

        output.slices[0] = previous_slice;

        return output;
    }

    std::vector<VMTableState> table_state;
    std::vector<VMPointState> point_state;

    void process_points()
    {
        static constexpr size_t num_rows_per_point = NUM_WNAF_SLICES / WNAF_SLICES_PER_ROW;
        static_assert(num_rows_per_point == 8); // current impl doesn't work if not 8!

        const size_t num_entries = ecc_muls.size();

        point_state = std::vector<VMPointState>();

        for (size_t i = 0; i < num_entries; ++i) {
            const grumpkin::g1::element point = ecc_muls[i].base_point;
            const grumpkin::g1::element d2 = point.dbl();
            std::array<grumpkin::g1::affine_element, num_rows_per_point> point_table;
            point_table[0] = point;
            for (size_t j = 1; j < num_rows_per_point; ++j) {
                point_table[j] = point_table[j - 1] + d2;
            }

            for (size_t j = 0; j < num_rows_per_point; ++j) {
                point_state.emplace_back(VMPointState{
                    .T = point_table[num_rows_per_point - j - 1],
                    .D = d2,
                    .point_transition = j == (num_rows_per_point - 1),
                    .pc = ecc_muls[i].pc,
                    .round = j,
                });
            }
        }
    }

    void process_wnafs()
    {
        const size_t num_entries = ecc_muls.size();

        static constexpr size_t num_rows_per_scalar = NUM_WNAF_SLICES / WNAF_SLICES_PER_ROW;

        static_assert(WNAF_SLICES_PER_ROW == 4); // current impl doesn't work if not 4 :o

        // table_state.clear();
        for (size_t i = 0; i < num_entries; ++i) {
            const auto& entry = ecc_muls[i];
            const auto wnaf = convert_to_slices(entry.scalar);
            uint256_t scalar_sum = 0;
            for (size_t j = 0; j < num_rows_per_scalar; ++j) {
                VMTableState row;
                const int slice0 = wnaf.slices[j * WNAF_SLICES_PER_ROW];
                const int slice1 = wnaf.slices[j * WNAF_SLICES_PER_ROW + 1];
                const int slice2 = wnaf.slices[j * WNAF_SLICES_PER_ROW + 2];
                const int slice3 = wnaf.slices[j * WNAF_SLICES_PER_ROW + 3];

                const int slice0base2 = (slice0 + 15) / 2;
                const int slice1base2 = (slice1 + 15) / 2;
                const int slice2base2 = (slice2 + 15) / 2;
                const int slice3base2 = (slice3 + 15) / 2;

                // convert into 2-bit chunks
                row.s1 = slice0base2 >> 2;
                row.s2 = slice0base2 & 3;
                row.s3 = slice1base2 >> 2;
                row.s4 = slice1base2 & 3;
                row.s5 = slice2base2 >> 2;
                row.s6 = slice2base2 & 3;
                row.s7 = slice3base2 >> 2;
                row.s8 = slice3base2 & 3;

                bool last_row = (j == num_rows_per_scalar - 1);

                row.skew = last_row ? true : false;

                row.scalar_sum = scalar_sum;
                // TODO(zac). If 1st row do we apply constraint that requires slice0 to be positive?
                //            Need this if we want to rule out negative values (i.e. input has not yet been range
                //            constrained)
                const int row_chunk = slice3 + slice2 * (1 << 4) + slice1 * (1 << 8) + slice0 * (1 << 12);

                bool chunk_negative = row_chunk < 0;

                scalar_sum = scalar_sum << (WNAF_SLICE_BITS * WNAF_SLICES_PER_ROW);
                if (chunk_negative) {
                    scalar_sum -= static_cast<uint64_t>(-row_chunk);
                } else {
                    scalar_sum += static_cast<uint64_t>(row_chunk);
                }
                row.round = j;
                row.point_transition = last_row;
                row.pc = entry.pc;

                if (last_row) {
                    ASSERT(scalar_sum - wnaf.skew == entry.scalar);
                }

                table_state.emplace_back(row);
            }
        }

        // VMTableState last_row{
        //     .s1 = 0,
        //     .s2 = 0,
        //     .s3 = 0,
        //     .s4 = 0,
        //     .s5 = 0,
        //     .s6 = 0,
        //     .s7 = 0,
        //     .s8 = 0,
        //     .skew = false,
        //     .point_transition = false,
        //     .pc = state[state.size() - 1].pc + 1,
        //     .round = 0,
        //     .scalar_sum = 0,
        // };
        // state.emplace_back(last_row);
    }

    RawPolynomials export_rows()
    {
        // TODO(@zac-williamson): make this const!
        // add_row(next_row, accumulator_);
        process_wnafs();
        process_points();

        const size_t num_rows = table_state.size();
        const size_t num_rows_log2 = numeric::get_msb64(num_rows);
        size_t num_rows_pow2 = 1UL << (num_rows_log2 + (1UL << num_rows_log2 == num_rows ? 0 : 1));

        RawPolynomials rows;

        for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
            for (size_t j = 0; j < num_rows_pow2; ++j) {
                rows[i].push_back(0);
            }
        }
        ASSERT(table_state.size() == point_state.size());
        for (size_t i = 0; i < table_state.size(); ++i) {
            bool last = i == table_state.size() - 1;

            rows.q_wnaf[i] = 1; // todo document, derive etc etc
            rows.table_pc[i] = table_state[i].pc;
            rows.table_pc_shift[i] = last ? 0 : table_state[i + 1].pc;
            rows.table_point_transition[i] = static_cast<uint64_t>(table_state[i].point_transition);
            // rows.table_point_transition_shift = static_cast<uint64_t>(table_state[i].point_transition);
            rows.table_round[i] = table_state[i].round;
            rows.table_round_shift[i] = last ? 0 : table_state[i + 1].round;
            rows.table_scalar_sum[i] = table_state[i].scalar_sum;
            rows.table_scalar_sum_shift[i] = last ? 0 : table_state[i + 1].scalar_sum;

            rows.table_s1[i] = table_state[i].s1;
            rows.table_s2[i] = table_state[i].s2;
            rows.table_s3[i] = table_state[i].s3;
            rows.table_s4[i] = table_state[i].s4;
            rows.table_s5[i] = table_state[i].s5;
            rows.table_s6[i] = table_state[i].s6;
            rows.table_s7[i] = table_state[i].s7;
            rows.table_s8[i] = table_state[i].s8;
            rows.table_skew[i] = table_state[i].skew ? 7 : 0; // static_cast<uint64_t>(table_state[i].skew);

            rows.table_dx[i] = point_state[i].D.x;
            rows.table_dy[i] = point_state[i].D.y;
            rows.table_dx_shift[i] = last ? 0 : point_state[i + 1].D.x;
            rows.table_dy_shift[i] = last ? 0 : point_state[i + 1].D.y;
            rows.table_tx[i] = point_state[i].T.x;
            rows.table_ty[i] = point_state[i].T.y;
            rows.table_tx_shift[i] = last ? 0 : point_state[i + 1].T.x;
            rows.table_ty_shift[i] = last ? 0 : point_state[i + 1].T.y;

            ASSERT(table_state[i].pc == point_state[i].pc);
            ASSERT(table_state[i].round == point_state[i].round);
            ASSERT(table_state[i].point_transition == point_state[i].point_transition);
        }
        return rows;
    }
};

ExecutionTrace generate_wnaf_rows_native()
{
    /**
     * Create following transcript:
     *
     * 1. MUL x1 * [A]
     * 2. MADD x2 * [B]
     * 3. EQ [C]
     */
    ExecutionTrace result;

    // N.B. DOES NOT WORK IF NUM ROWS NOT A POWER OF 2!
    // RELATION FAILS FOR EMPTY ROW
    const size_t num_scalar_muls = 5;

    for (size_t i = 0; i < num_scalar_muls; ++i) {
        grumpkin::fr x = grumpkin::fr::random_element(&engine);
        grumpkin::g1::affine_element base_point = grumpkin::g1::one * x;
        uint256_t scalar = 0;
        scalar.data[0] = engine.get_random_uint64();
        scalar.data[1] = engine.get_random_uint64();

        // pc decrements and stops at 1.
        // N.B. this only works if for the transcript, we make pc starts at 1!
        // (For the wnaf relation, we check pc is valid by making sure pc - pc_shift = 1 on transitions).
        // pc_shift = 0 at the final row due to there being no data to shift!
        // We could also solve this by multiplying this check by a Lagrange polynomial to turn it off at the last row
        result.ecc_muls.emplace_back(ExecutionTrace::VMScalarMul{
            .pc = static_cast<uint32_t>(num_scalar_muls - i),
            .scalar = scalar,
            .base_point = base_point,
        });
    }
    return result;
}

ProverPolynomials construct_full_polynomials(RawPolynomials& container)
{
    // convert vector to span :/
    ProverPolynomials result;

    size_t poly_count = 0;
    for (auto& row : container) {
        result[poly_count++] = row;
    }
    return result;
}

TEST(SumcheckRelation, ECCVMWnafRelationAlgebra)
{
    auto relation = ECCVMWnafAlgebra<barretenberg::fr>();
    // T::typename Univariate<FF, RELATION_LENGTH>& evals,
    //                                const auto& extended_edges,
    //                                const T::typename RelationParameters<FF>&,
    //                                const FF& scaling_factor

    barretenberg::fr result = 0;
    barretenberg::fr scaling_factor = 1;
    honk::sumcheck::RelationParameters<barretenberg::fr> relation_parameters{
        .beta = 1,
        .gamma = 1,
        .public_input_delta = 1,
    };
    ExecutionTrace trace = generate_wnaf_rows_native();

    auto raw_rows = trace.export_rows();

    auto rows = construct_full_polynomials(raw_rows);

    const size_t num_rows = raw_rows[0].size();

    for (size_t i = 0; i < num_rows; ++i) {
        ExecutionTrace::EccWnafRow row;
        for (size_t j = 0; j < NUM_POLYNOMIALS; ++j) {
            row[j] = rows[j][i];
        }
        relation.add_edge_contribution(result, row, relation_parameters, scaling_factor);
        EXPECT_EQ(result, 0);
    }
};

TEST(SumcheckRelation, ECCVMPointRelationAlgebra)
{
    auto relation = ECCVMPointTableAlgebra<barretenberg::fr>();

    barretenberg::fr result = 0;
    barretenberg::fr scaling_factor = 1;
    honk::sumcheck::RelationParameters<barretenberg::fr> relation_parameters{
        .beta = 1,
        .gamma = 1,
        .public_input_delta = 1,
    };
    ExecutionTrace trace = generate_wnaf_rows_native();
    auto raw_rows = trace.export_rows();

    auto rows = construct_full_polynomials(raw_rows);

    const size_t num_rows = raw_rows[0].size();

    for (size_t i = 0; i < num_rows; ++i) {
        ExecutionTrace::EccWnafRow row;
        for (size_t j = 0; j < NUM_POLYNOMIALS; ++j) {
            row[j] = rows[j][i];
        }
        relation.add_edge_contribution(result, row, relation_parameters, scaling_factor);
        EXPECT_EQ(result, 0);
    }
};

TEST(SumcheckRelation, ECCVMPointRelationProver)
{
    const auto run_test = []() {
        honk::sumcheck::RelationParameters<barretenberg::fr> relation_parameters{
            .beta = 1,
            .gamma = 1,
            .public_input_delta = 1,
        };
        ExecutionTrace trace = generate_wnaf_rows_native();
        auto raw_rows = trace.export_rows();
        const size_t multivariate_n = raw_rows[0].size();
        const size_t multivariate_d = numeric::get_msb64(multivariate_n);

        EXPECT_EQ(1ULL << multivariate_d, multivariate_n);

        auto full_polynomials = construct_full_polynomials(raw_rows);

        auto prover_transcript = honk::ProverTranscript<FF>::init_empty();

        auto sumcheck_prover = Sumcheck<Flavor, honk::ProverTranscript<FF>, ECCVMWnafProver, ECCVMPointTableProver>(
            multivariate_n, prover_transcript);

        auto prover_output = sumcheck_prover.execute_prover(full_polynomials, relation_parameters);

        auto verifier_transcript = honk::VerifierTranscript<FF>::init_empty(prover_transcript);

        auto sumcheck_verifier =
            Sumcheck<Flavor, honk::VerifierTranscript<FF>, ECCVMWnafVerifier, ECCVMPointTableVerifier>(
                multivariate_n, verifier_transcript);

        std::optional verifier_output = sumcheck_verifier.execute_verifier(relation_parameters);

        ASSERT_TRUE(verifier_output.has_value());
        ASSERT_EQ(prover_output, *verifier_output);
    };
    for (size_t i = 0; i < 50; ++i) {
        run_test();
    }
};

} // namespace proof_system::honk_relation_tests_eccwnaf
