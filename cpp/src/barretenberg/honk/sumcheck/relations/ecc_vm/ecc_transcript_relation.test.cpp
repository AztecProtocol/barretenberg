#include "ecc_transcript_relation.hpp"
#include "ecc_vm_types.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/honk/sumcheck/sumcheck.hpp"
#include "barretenberg/honk/flavor/ecc_vm.hpp"
#include "barretenberg/numeric/random/engine.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"

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

template <typename Field>
using ECCVMTranscriptAlgebra = proof_system::honk::sumcheck::eccvm::ECCVMTranscriptAlgebra<Field>;

template <typename Field>
using ECCVMTranscriptProver = proof_system::honk::sumcheck::eccvm::ECCVMTranscriptProver<Field>;

template <typename Field>
using ECCVMTranscriptVerifier = proof_system::honk::sumcheck::eccvm::ECCVMTranscriptVerifier<Field>;

static constexpr size_t NUM_POLYNOMIALS = Flavor::NUM_ALL_ENTITIES;

namespace proof_system::honk_relation_tests_ecctranscript {

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
    bool reset;
    // bool set;
    [[nodiscard]] size_t value() const
    {
        auto res = static_cast<size_t>(add);
        res += res;
        res += static_cast<size_t>(mul);
        res += res;
        res += static_cast<size_t>(eq);
        res += res;
        res += static_cast<size_t>(reset);
        // res += res;
        // res += static_cast<size_t>(set);
        return res;
    }
};

struct VMOperation {
    bool add = false;
    bool mul = false;
    bool eq = false;
    bool reset = false;
    grumpkin::g1::affine_element base_point = grumpkin::g1::affine_element{ 0, 0 };
    uint256_t z1 = 0;
    uint256_t z2 = 0;
    grumpkin::fr mul_scalar_full = 0;
};
struct ExecutionTrace {
    using TranscriptRow = Flavor::RowPolynomials; // std::array<barretenberg::fr, NUM_POLYNOMIALS>;

    std::vector<VMOperation> transcript;

    struct VMState {
        size_t pc = 0;
        size_t count = 0;
        grumpkin::g1::affine_element accumulator = grumpkin::g1::affine_point_at_infinity;
        grumpkin::g1::affine_element msm_accumulator = grumpkin::g1::affine_point_at_infinity;
        bool is_accumulator_empty = true;
    };
    RawPolynomials process_transcript()
    {
        const size_t num_entries = transcript.size();

        VMState state{
            .pc = 0,
            .count = 0,
            .accumulator = grumpkin::g1::affine_point_at_infinity,
            .msm_accumulator = grumpkin::g1::affine_point_at_infinity,
            .is_accumulator_empty = true,
        };

        // create an empty operation at the end
        // auto last = transcript[transcript.size() - 1];
        // transcript.emplace_back(VMOperation{
        //     .add = false,
        //     .base_point{0,0},
        //     .eq = false,
        //     .mul = false,
        // })
        transcript.emplace_back();
        const size_t num_rows = transcript.size();
        const size_t num_rows_log2 = numeric::get_msb64(num_rows);
        size_t num_rows_pow2 = 1UL << (num_rows_log2 + (1UL << num_rows_log2 == num_rows ? 0 : 1));

        RawPolynomials rows;

        // zero rows
        for (size_t i = 0; i < num_rows_pow2; ++i) {
            for (auto& column : rows) {
                column.push_back(0);
            }
        }
        for (size_t i = 0; i < num_entries; ++i) {
            VMOperation& entry = transcript[i];

            const VMOperation& next = transcript[i + 1];

            const bool is_mul = entry.mul;
            const bool z1_zero = (entry.mul) ? entry.z1 == 0 : true;
            const bool z2_zero = (entry.mul) ? entry.z2 == 0 : true;
            const size_t num_muls = is_mul ? (static_cast<size_t>(!z1_zero) + static_cast<size_t>(!z2_zero)) : 0;

            std::cout << "num muls = " << num_muls << std::endl;
            auto updated_state = state;

            if (entry.reset) {
                updated_state.is_accumulator_empty = true;
                updated_state.msm_accumulator = grumpkin::g1::affine_point_at_infinity;
            }
            updated_state.pc = state.pc + num_muls;

            // msm transition = current row is doing a lookup to validate output = msm output
            // i.e. next row is not part of MSM and current row is part of MSM
            //   or next row is irrelevent and current row is a straight MUL
            bool next_not_msm = !next.mul;

            bool msm_transition = entry.mul && next_not_msm;
            std::cout << "i = " << i << " , msm tx = " << msm_transition << std::endl;

            // we reset the count in updated state if we are not accumulating and not doing an msm
            bool current_msm = entry.mul;
            bool current_ongoing_msm = entry.mul && !next_not_msm;
            updated_state.count = current_ongoing_msm ? state.count + num_muls : 0;

            if (current_msm) {
                const auto P = grumpkin::g1::element(entry.base_point);
                const auto R = grumpkin::g1::element(state.msm_accumulator);
                updated_state.msm_accumulator = R + P * entry.mul_scalar_full;
            }
            // } else if (entry.mul && !entry.accumulate) {
            //     const auto P = grumpkin::g1::element(entry.base_point);
            //     updated_state.msm_accumulator = P * entry.mul_scalar_full;
            //     if (i == 0) {
            //         std::cout << "first mul. updated_state.msm acc = " << updated_state.msm_accumulator << std::endl;
            //     }
            // }

            if (entry.mul && next_not_msm) {
                if (state.is_accumulator_empty) {
                    updated_state.accumulator = updated_state.msm_accumulator;
                } else {
                    const auto R = grumpkin::g1::element(state.accumulator);
                    updated_state.accumulator = R + updated_state.msm_accumulator;
                }
                updated_state.is_accumulator_empty = false;
            }

            bool add_accumulate = entry.add;
            if (add_accumulate) {
                if (state.is_accumulator_empty) {

                    updated_state.accumulator = entry.base_point;
                } else {
                    updated_state.accumulator = grumpkin::g1::element(state.accumulator) + entry.base_point;
                }
                updated_state.is_accumulator_empty = false;
            }
            rows.lagrange_first[i] = i == 0 ? 1 : 0;
            rows.transcript_accumulator_empty[i] = state.is_accumulator_empty;
            rows.transcript_accumulator_empty_shift[i] = updated_state.is_accumulator_empty;
            rows.q_transcript_add[i] = entry.add;
            rows.q_transcript_mul[i] = entry.mul;
            rows.q_transcript_mul_shift[i] = next.mul;
            rows.q_transcript_eq[i] = entry.eq;
            rows.transcript_q_reset_accumulator[i] = entry.reset;
            rows.q_transcript_msm_transition[i] = static_cast<const int>(msm_transition);
            rows.transcript_pc[i] = state.pc;
            rows.transcript_pc_shift[i] = updated_state.pc;
            rows.transcript_msm_count[i] = state.count;
            rows.transcript_msm_count_shift[i] = updated_state.count;
            rows.transcript_x[i] = (entry.add || entry.mul || entry.eq) ? entry.base_point.x : 0;
            rows.transcript_y[i] = (entry.add || entry.mul || entry.eq) ? entry.base_point.y : 0;
            rows.transcript_z1[i] = (entry.mul) ? entry.z1 : 0;
            rows.transcript_z2[i] = (entry.mul) ? entry.z2 : 0;
            rows.transcript_z1zero[i] = z1_zero;
            rows.transcript_z2zero[i] = z2_zero;
            rows.transcript_op[i] =
                Opcode{ .add = entry.add, .mul = entry.mul, .eq = entry.eq, .reset = entry.reset }.value();
            rows.transcript_accumulator_x_shift[i] =
                updated_state.accumulator.is_point_at_infinity() ? 0 : updated_state.accumulator.x;
            rows.transcript_accumulator_x[i] = (state.accumulator.is_point_at_infinity()) ? 0 : state.accumulator.x;
            rows.transcript_accumulator_y_shift[i] =
                (updated_state.accumulator.is_point_at_infinity()) ? 0 : updated_state.accumulator.y;
            rows.transcript_accumulator_y[i] = (state.accumulator.is_point_at_infinity()) ? 0 : state.accumulator.y;
            rows.transcript_msm_x[i] =
                msm_transition
                    ? (updated_state.msm_accumulator.is_point_at_infinity() ? 0 : updated_state.msm_accumulator.x)
                    : 0;
            rows.transcript_msm_y[i] =
                msm_transition
                    ? (updated_state.msm_accumulator.is_point_at_infinity() ? 0 : updated_state.msm_accumulator.y)
                    : 0;

            state = updated_state;

            if (entry.mul && next_not_msm) {
                state.msm_accumulator = grumpkin::g1::affine_point_at_infinity;
            }
        }
        return rows;
    }

    void add_accumulate(const grumpkin::g1::affine_element& to_add)
    {
        transcript.emplace_back(VMOperation{
            .add = true,
            .mul = false,
            .eq = false,
            .reset = false,
            .base_point = to_add,
            .z1 = 0,
            .z2 = 0,
            .mul_scalar_full = 0,
        });
    }

    void mul_accumulate(const grumpkin::g1::affine_element& to_mul, const grumpkin::fr& scalar)
    {
        grumpkin::fr z1 = 0;
        grumpkin::fr z2 = 0;
        grumpkin::fr::split_into_endomorphism_scalars(scalar, z1, z2);
        z1 = z1.to_montgomery_form();
        z2 = z2.to_montgomery_form();
        transcript.emplace_back(VMOperation{
            .add = false,
            .mul = true,
            .eq = false,
            .reset = false,
            .base_point = to_mul,
            .z1 = z1,
            .z2 = z2,
            .mul_scalar_full = scalar,
        });
    }

    void eq(const grumpkin::g1::affine_element& expected)
    {
        transcript.emplace_back(VMOperation{
            .add = false,
            .mul = false,
            .eq = true,
            .reset = true,
            .base_point = expected,
            .z1 = 0,
            .z2 = 0,
            .mul_scalar_full = 0,
        });
    }

    void empty_row()
    {
        transcript.emplace_back(VMOperation{
            .add = false,
            .mul = false,
            .eq = false,
            .reset = false,
            .base_point = grumpkin::g1::affine_point_at_infinity,
            .z1 = 0,
            .z2 = 0,
            .mul_scalar_full = 0,
        });
    }

    RawPolynomials export_rows() { return process_transcript(); }
};

ExecutionTrace generate_transcript_native(numeric::random::Engine* engine = nullptr)
{
    if (engine == nullptr) {
        engine = &numeric::random::get_debug_engine();
    }
    //   auto& engine_inner = numeric::random::get_debug_engine(/*reset*/ true);

    /**
     * Create following transcript:
     *
     * 1. MUL x1 * [A]
     * 2. MADD x2 * [B]
     * 3. EQ [C]
     */
    ExecutionTrace result;

    grumpkin::g1::element a = grumpkin::get_generator(0);
    grumpkin::g1::element b = grumpkin::get_generator(1);
    grumpkin::g1::element c = grumpkin::get_generator(2);
    grumpkin::fr x = grumpkin::fr::random_element(engine);
    grumpkin::g1::element expected_1 = (a * x) + a + (b * x) + (b * x) + (b * x);
    grumpkin::g1::element expected_2 = (a * x) + c + (b * x);

    // result.mul(a, x);
    // NB ALGEBRA FAILS HERE???
    // result.mul_accumulate(a, x);
    result.mul_accumulate(a, x);
    result.mul_accumulate(b, x);
    result.mul_accumulate(b, x);
    result.add_accumulate(a);
    result.mul_accumulate(b, x);
    result.eq(expected_1);
    result.add_accumulate(c);
    result.mul_accumulate(a, x);
    result.mul_accumulate(b, x);
    result.eq(expected_2);

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

TEST(SumcheckRelation, ECCVMTranscriptRelationAlgebra)
{
    const auto run_test = []() {
        auto relation = ECCVMTranscriptAlgebra<barretenberg::fr>();

        barretenberg::fr scaling_factor = barretenberg::fr::random_element(&engine);

        ExecutionTrace trace = generate_transcript_native();

        auto rows = trace.export_rows();

        const size_t num_rows = rows[0].size();
        for (size_t i = 0; i < num_rows; ++i) {
            ExecutionTrace::TranscriptRow row;
            for (size_t j = 0; j < NUM_POLYNOMIALS; ++j) {
                row[j] = rows[j][i];
            }
            barretenberg::fr result = 0;
            relation.add_edge_contribution(result, row, {}, scaling_factor);
            std::cout << " row " << i << std::endl;
            EXPECT_EQ(result, 0);
        }
    };
    run_test();
};

TEST(SumcheckRelation, ECCVMTranscriptConsistencyTest)
{
    std::vector<RawPolynomials> rows_lhs;
    std::vector<RawPolynomials> rows_rhs;

    const size_t num_repetitions = 999;

    auto& engine_lhs = numeric::random::get_debug_engine(/*reset*/ true);
    for (size_t i = 0; i < num_repetitions; ++i) {
        ExecutionTrace trace = generate_transcript_native(&engine_lhs);
        rows_lhs.push_back(trace.export_rows());
    }
    auto& engine_rhs = numeric::random::get_debug_engine(/*reset*/ true);

    for (size_t i = 0; i < num_repetitions; ++i) {
        ExecutionTrace trace = generate_transcript_native(&engine_rhs);
        rows_rhs.push_back(trace.export_rows());
    }
    const size_t multivariate_n = rows_lhs[0][0].size();
    const size_t multivariate_d = numeric::get_msb64(multivariate_n);
    EXPECT_EQ(1ULL << multivariate_d, multivariate_n);
    EXPECT_EQ(multivariate_n, rows_rhs[0][0].size());

    for (size_t k = 0; k < num_repetitions; ++k) {

        for (size_t i = 0; i < NUM_POLYNOMIALS; ++i) {
            for (size_t j = 0; j < multivariate_n; ++j) {
                EXPECT_EQ(rows_lhs[k][i][j], rows_rhs[k][i][j]);
            }
        }
    }
}

TEST(SumcheckRelation, ECCVMTranscriptRelationProver)
{

    // const size_t multivariate_d(9);
    // const size_t multivariate_n(1 << multivariate_d);

    const auto run_test = []() {
        ExecutionTrace trace = generate_transcript_native();
        auto rows = trace.export_rows();

        const size_t multivariate_n = rows[0].size();
        const size_t multivariate_d = numeric::get_msb64(multivariate_n);

        EXPECT_EQ(1ULL << multivariate_d, multivariate_n);
        auto full_polynomials = construct_full_polynomials(rows);
        honk::sumcheck::RelationParameters<FF> relation_parameters{
            .beta = FF::random_element(&engine),
            .gamma = FF::random_element(&engine),
            .public_input_delta = FF::one(),
        };
        auto prover_transcript = honk::ProverTranscript<FF>::init_empty();
        // auto relation = ECCMSMRelation<barretenberg::fr, TestWrapper>();

        auto sumcheck_prover =
            Sumcheck<Flavor, honk::ProverTranscript<FF>, ECCVMTranscriptProver>(multivariate_n, prover_transcript);

        auto prover_output = sumcheck_prover.execute_prover(full_polynomials, relation_parameters);

        auto verifier_transcript = honk::VerifierTranscript<FF>::init_empty(prover_transcript);

        auto sumcheck_verifier = Sumcheck<Flavor, honk::VerifierTranscript<FF>, ECCVMTranscriptVerifier>(
            multivariate_n, verifier_transcript);

        std::optional verifier_output = sumcheck_verifier.execute_verifier(relation_parameters);

        ASSERT_TRUE(verifier_output.has_value());
        ASSERT_EQ(prover_output, *verifier_output);
        std::cout << "verified? " << (prover_output == *verifier_output) << std::endl;
    };

    for (size_t i = 0; i < 5; ++i) {
        run_test();
        run_test();
        run_test();
        run_test();
        run_test();
    }
}

} // namespace proof_system::honk_relation_tests_ecctranscript
