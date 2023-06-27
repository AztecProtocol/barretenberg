#include "ecc_msm_relation.hpp"
#include "ecc_vm_types.hpp"

#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/honk/sumcheck/sumcheck.hpp"
#include "barretenberg/numeric/random/engine.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"
#include "barretenberg/honk/flavor/ecc_vm.hpp"
#include "barretenberg/honk/transcript/transcript.hpp"

#include <gtest/gtest.h>
using namespace proof_system::honk::sumcheck;
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
using Flavor = proof_system::honk::flavor::ECCVM;
using FF = typename Flavor::FF;
using ProverPolynomials = typename Flavor::ProverPolynomials;
using RawPolynomials = typename Flavor::FoldedPolynomials;

template <typename Field>
using ECCMSMRelationAlgebra = proof_system::honk::sumcheck::eccvm::ECCMSMRelationAlgebra<Field>;

template <typename Field> using ECCMSMRelationProver = proof_system::honk::sumcheck::eccvm::ECCMSMRelationProver<Field>;

template <typename Field>
using ECCMSMRelationVerifier = proof_system::honk::sumcheck::eccvm::ECCMSMRelationVerifier<Field>;

static constexpr size_t NUM_POLYNOMIALS = Flavor::NUM_ALL_ENTITIES;

namespace proof_system::honk_relation_tests_eccmsm {

using POLYNOMIAL = Flavor::Polynomial;

namespace {
auto& engine = numeric::random::get_debug_engine();
}

struct ExecutionTrace {
    using EccMSMRow = Flavor::RowPolynomials; // std::array<barretenberg::fr, NUM_POLYNOMIALS>;
    using POLYNOMIAL = Flavor::Polynomial;
    static constexpr size_t NUM_SCALAR_BITS = 128;
    static constexpr size_t WNAF_SLICE_BITS = 4;
    static constexpr size_t NUM_WNAF_SLICES = (NUM_SCALAR_BITS + WNAF_SLICE_BITS - 1) / WNAF_SLICE_BITS;
    static constexpr uint64_t WNAF_MASK = static_cast<uint64_t>((1ULL << WNAF_SLICE_BITS) - 1ULL);
    static constexpr size_t WNAF_SLICES_PER_ROW = 4;
    static constexpr size_t ADDITIONS_PER_ROW = 4;

    struct VMMultiScalarMul {
        uint32_t pc;
        uint32_t msm_size;
        std::vector<uint256_t> scalars;
        std::vector<grumpkin::g1::affine_element> base_points;
    };
    std::vector<VMMultiScalarMul> ecc_muls;

    struct WNAFSlices {
        std::array<int, NUM_WNAF_SLICES> slices;
        bool skew;
    };

    struct VMMultiScalarMulState {
        uint32_t pc = 0;
        uint32_t msm_size = 0;
        uint32_t msm_count = 0;
        uint32_t msm_round = 0;
        bool q_msm_transition = false;
        bool q_add = false;
        bool q_double = false;
        bool q_skew = false;

        struct AddState {
            bool add;
            int slice;
            grumpkin::g1::affine_element point;
            barretenberg::fr lambda;
            barretenberg::fr collision_inverse;
        };
        std::array<AddState, 4> add_state;
        grumpkin::g1::affine_element accumulator{ 0, 0 };
    };

    struct VMPointState {
        grumpkin::g1::affine_element T;
        grumpkin::g1::affine_element D;
        bool point_transition;
        uint32_t pc;
        size_t round;
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

    std::vector<VMMultiScalarMulState> msm_state;
    std::vector<VMPointState> point_state;

    static constexpr size_t num_precomputed_table_points = NUM_WNAF_SLICES / WNAF_SLICES_PER_ROW;
    static constexpr size_t precomputed_table_size = num_precomputed_table_points + num_precomputed_table_points;

    static std::array<grumpkin::g1::affine_element, precomputed_table_size> compute_point_table(
        const grumpkin::g1::affine_element& input)
    {
        static_assert(num_precomputed_table_points == 8); // current impl doesn't work if not 8!

        const grumpkin::g1::element point(input);
        const grumpkin::g1::element d2 = point.dbl();
        std::array<grumpkin::g1::affine_element, precomputed_table_size> point_table;
        point_table[num_precomputed_table_points] = point;
        for (size_t j = 1; j < num_precomputed_table_points; ++j) {
            point_table[num_precomputed_table_points + j] = point_table[num_precomputed_table_points + j - 1] + d2;
        }
        for (size_t j = 0; j < num_precomputed_table_points; ++j) {
            point_table[j] = -point_table[precomputed_table_size - 1 - j];
        }
        return point_table;
    }

    void process_msms()
    {
        const size_t num_entries = ecc_muls.size();

        uint32_t pc = 0;
        for (size_t i = 0; i < num_entries; ++i) {
            const auto& entry = ecc_muls[i];

            const size_t msm_size = entry.msm_size;

            const size_t rows_per_round = (msm_size / ADDITIONS_PER_ROW) + (msm_size % ADDITIONS_PER_ROW != 0 ? 1 : 0);

            static constexpr size_t num_rounds = NUM_SCALAR_BITS / WNAF_SLICE_BITS;

            const auto& base_points = entry.base_points;
            const auto& scalars = entry.scalars;

            ASSERT(base_points.size() == scalars.size());
            ASSERT(base_points.size() == msm_size);
            std::vector<std::array<grumpkin::g1::affine_element, 16>> point_tables(msm_size);
            std::vector<WNAFSlices> scalar_slices(msm_size);
            for (size_t i = 0; i < msm_size; ++i) {
                point_tables[i] = compute_point_table(base_points[i]);
                scalar_slices[i] = convert_to_slices(scalars[i]);
            }

            grumpkin::g1::element accumulator = grumpkin::g1::point_at_infinity;

            const auto add_points = [](auto& P1, auto& P2, auto& lambda, auto& collision_inverse, bool predicate) {
                lambda = predicate ? (P2.y - P1.y) / (P2.x - P1.x) : 0;
                collision_inverse = predicate ? (P2.x - P1.x).invert() : 0;
                auto x3 = predicate ? lambda * lambda - (P2.x + P1.x) : P1.x;
                auto y3 = predicate ? lambda * (P1.x - x3) - P1.y : P1.y;
                return grumpkin::g1::affine_element(x3, y3);
            };

            for (size_t j = 0; j < num_rounds; ++j) {
                for (size_t k = 0; k < rows_per_round; ++k) {
                    VMMultiScalarMulState row;

                    const size_t points_per_row =
                        (k + 1) * ADDITIONS_PER_ROW > msm_size ? msm_size % ADDITIONS_PER_ROW : ADDITIONS_PER_ROW;
                    const size_t idx = k * ADDITIONS_PER_ROW;
                    row.q_msm_transition = (j == 0) && (k == 0);

                    grumpkin::g1::affine_element acc(accumulator);
                    grumpkin::g1::element acc_expected = accumulator;
                    for (size_t m = 0; m < 4; ++m) {
                        auto& add_state = row.add_state[m];
                        add_state.add = points_per_row > m;
                        int slice = add_state.add ? scalar_slices[idx + m].slices[j] : 0;
                        add_state.slice = add_state.add ? (slice + 15) / 2 : 0;
                        add_state.point = add_state.add ? point_tables[idx + m][static_cast<size_t>(add_state.slice)]
                                                        : grumpkin::g1::affine_element{ 0, 0 };
                        bool add_predicate = (m == 0 ? (j != 0 || k != 0) : add_state.add);

                        auto& p1 = (m == 0) ? add_state.point : acc;
                        auto& p2 = (m == 0) ? acc : add_state.point;

                        acc_expected = add_predicate ? (acc_expected + add_state.point) : grumpkin::g1::element(p1);
                        acc = add_points(p1, p2, add_state.lambda, add_state.collision_inverse, add_predicate);
                        ASSERT(acc == grumpkin::g1::affine_element(acc_expected));
                    }
                    row.q_add = true;
                    row.q_double = false;
                    row.q_skew = false;
                    row.msm_round = static_cast<uint32_t>(j);
                    row.msm_size = static_cast<uint32_t>(msm_size);
                    row.msm_count = static_cast<uint32_t>(idx);

                    row.accumulator = acc;
                    row.pc = pc;
                    accumulator = acc;
                    msm_state.push_back(row);
                }

                if (j < num_rounds - 1) {
                    VMMultiScalarMulState row;
                    row.q_msm_transition = false;
                    row.msm_round = static_cast<uint32_t>(j + 1);
                    row.msm_size = static_cast<uint32_t>(msm_size);
                    row.msm_count = static_cast<uint32_t>(0);
                    row.q_add = false;
                    row.q_double = true;
                    row.q_skew = false;

                    auto dx = accumulator.normalize().x;
                    auto dy = accumulator.normalize().y;
                    for (size_t m = 0; m < 4; ++m) {
                        auto& add_state = row.add_state[m];
                        add_state.add = false;
                        add_state.slice = 0;
                        add_state.point = { 0, 0 };
                        add_state.collision_inverse = 0;
                        add_state.lambda = ((dx + dx + dx) * dx) / (dy + dy);
                        auto x3 = add_state.lambda.sqr() - dx - dx;
                        dy = add_state.lambda * (dx - x3) - dy;
                        dx = x3;
                    }

                    accumulator = accumulator.dbl().dbl().dbl().dbl();

                    row.accumulator = accumulator;
                    ASSERT(row.accumulator.x == dx);
                    ASSERT(row.accumulator.y == dy);
                    row.pc = pc;
                    msm_state.push_back(row);
                } else {
                    for (size_t k = 0; k < rows_per_round; ++k) {
                        VMMultiScalarMulState row;

                        const size_t points_per_row =
                            (k + 1) * ADDITIONS_PER_ROW > msm_size ? msm_size % ADDITIONS_PER_ROW : ADDITIONS_PER_ROW;
                        const size_t idx = k * ADDITIONS_PER_ROW;
                        row.q_msm_transition = false;

                        grumpkin::g1::affine_element acc(accumulator);
                        grumpkin::g1::element acc_expected = accumulator;

                        for (size_t m = 0; m < 4; ++m) {
                            auto& add_state = row.add_state[m];
                            add_state.add = points_per_row > m;
                            add_state.slice = add_state.add ? scalar_slices[idx + m].skew ? 7 : 0 : 0;

                            add_state.point = point_tables[idx + m][static_cast<size_t>(add_state.slice)];

                            bool add_predicate = add_state.add ? scalar_slices[idx + m].skew : false;
                            acc = add_points(
                                acc, add_state.point, add_state.lambda, add_state.collision_inverse, add_predicate);
                            acc_expected = add_predicate ? (acc_expected + add_state.point) : acc_expected;
                            ASSERT(acc == grumpkin::g1::affine_element(acc_expected));
                        }
                        row.q_add = false;
                        row.q_double = false;
                        row.q_skew = true;
                        row.msm_round = static_cast<uint32_t>(j + 1);
                        row.msm_size = static_cast<uint32_t>(msm_size);
                        row.msm_count = static_cast<uint32_t>(idx);

                        row.accumulator = acc;
                        row.pc = pc;
                        accumulator = acc;
                        msm_state.push_back(row);
                    }
                }
            }
            pc += static_cast<uint32_t>(msm_size);
            // Validate our computed accumulator matches the real MSM result!
            grumpkin::g1::element expected = grumpkin::g1::point_at_infinity;
            for (size_t i = 0; i < base_points.size(); ++i) {
                expected += (grumpkin::g1::element(base_points[i]) * scalars[i]);
            }
            ASSERT(accumulator == expected);
        }
    }

    RawPolynomials export_rows()
    {
        process_msms();
        RawPolynomials rows;
        const size_t num_rows = msm_state.size();
        const size_t num_rows_log2 = numeric::get_msb64(num_rows);
        size_t num_rows_pow2 = 1UL << (num_rows_log2 + (1UL << num_rows_log2 == num_rows ? 0 : 1));

        // zero rows
        for (size_t i = 0; i < num_rows_pow2; ++i) {
            for (auto& column : rows) {
                column.push_back(0);
            }
        }
        for (size_t i = 0; i < msm_state.size(); ++i) {
            bool last = i == msm_state.size() - 1;
            VMMultiScalarMulState prev = (i == 0) ? VMMultiScalarMulState() : msm_state[i - 1];
            VMMultiScalarMulState next = last ? VMMultiScalarMulState() : msm_state[i + 1];

            rows.q_msm_transition[i] = static_cast<int>(msm_state[i].q_msm_transition);

            // TODO(zac): handle edge case of last row!
            rows.q_msm_transition_shift[i] = (i == msm_state.size() - 1) ? 1 : static_cast<int>(next.q_msm_transition);
            rows.msm_q_add[i] = static_cast<int>(msm_state[i].q_add);
            rows.msm_q_add_shift[i] = static_cast<int>(next.q_add);
            rows.msm_q_double[i] = static_cast<int>(msm_state[i].q_double);
            rows.msm_q_double_shift[i] = static_cast<int>(next.q_double);
            rows.msm_q_skew[i] = static_cast<int>(msm_state[i].q_skew);
            rows.msm_q_skew_shift[i] = static_cast<int>(next.q_skew);
            rows.msm_accumulator_x[i] = prev.accumulator.x;
            rows.msm_accumulator_y[i] = prev.accumulator.y;
            // TODO(zac): handle edge case of last row!
            rows.msm_accumulator_x_shift[i] = msm_state[i].accumulator.x;
            rows.msm_accumulator_y_shift[i] = msm_state[i].accumulator.y;
            rows.msm_pc[i] = msm_state[i].pc;
            // TODO(zac): handle edge case of last row!
            rows.msm_pc_shift[i] = (i == msm_state.size() - 1) ? msm_state[i].pc + msm_state[i].msm_size : next.pc;
            rows.msm_size_of_msm[i] = msm_state[i].msm_size;
            rows.msm_size_of_msm_shift[i] = next.msm_size;
            rows.msm_count[i] = msm_state[i].msm_count;
            rows.msm_count_shift[i] = next.msm_count;
            rows.msm_round[i] = msm_state[i].msm_round;
            rows.msm_round_shift[i] = next.msm_round;
            rows.msm_q_add1[i] = static_cast<int>(msm_state[i].add_state[0].add);
            rows.msm_q_add1_shift[i] = static_cast<int>(next.add_state[0].add);
            rows.msm_q_add2[i] = static_cast<int>(msm_state[i].add_state[1].add);
            rows.msm_q_add3[i] = static_cast<int>(msm_state[i].add_state[2].add);
            rows.msm_q_add4[i] = static_cast<int>(msm_state[i].add_state[3].add);
            rows.msm_x1[i] = msm_state[i].add_state[0].point.x;
            rows.msm_y1[i] = msm_state[i].add_state[0].point.y;
            rows.msm_x2[i] = msm_state[i].add_state[1].point.x;
            rows.msm_y2[i] = msm_state[i].add_state[1].point.y;
            rows.msm_x3[i] = msm_state[i].add_state[2].point.x;
            rows.msm_y3[i] = msm_state[i].add_state[2].point.y;
            rows.msm_x4[i] = msm_state[i].add_state[3].point.x;
            rows.msm_y4[i] = msm_state[i].add_state[3].point.y;
            rows.msm_collision_x1[i] = msm_state[i].add_state[0].collision_inverse;
            rows.msm_collision_x2[i] = msm_state[i].add_state[1].collision_inverse;
            rows.msm_collision_x3[i] = msm_state[i].add_state[2].collision_inverse;
            rows.msm_collision_x4[i] = msm_state[i].add_state[3].collision_inverse;
            rows.msm_lambda1[i] = msm_state[i].add_state[0].lambda;
            rows.msm_lambda2[i] = msm_state[i].add_state[1].lambda;
            rows.msm_lambda3[i] = msm_state[i].add_state[2].lambda;
            rows.msm_lambda4[i] = msm_state[i].add_state[3].lambda;
            rows.msm_slice1[i] = msm_state[i].add_state[0].slice;
            rows.msm_slice2[i] = msm_state[i].add_state[1].slice;
            rows.msm_slice3[i] = msm_state[i].add_state[2].slice;
            rows.msm_slice4[i] = msm_state[i].add_state[3].slice;
        }

        return rows;
    }
};

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

ExecutionTrace generate_msms_native()
{
    /**
     * Create following transcript:
     *
     * 1. MUL x1 * [A]
     * 2. MADD x2 * [B]
     * 3. EQ [C]
     */
    ExecutionTrace result;

    const size_t num_msms = 5;
    uint32_t msm_counter = 0;
    const std::vector<size_t> msm_sizes{ 3, 5, 1, 8, 13 };

    for (size_t i = 0; i < num_msms; ++i) {
        ExecutionTrace::VMMultiScalarMul msm;
        msm.pc = msm_counter;
        msm.msm_size = static_cast<uint32_t>(msm_sizes[i]);

        for (size_t j = 0; j < msm_sizes[i]; ++j) {
            grumpkin::fr x = grumpkin::fr::random_element();

            grumpkin::g1::affine_element base_point = grumpkin::g1::one * x;
            uint256_t scalar = 0;
            scalar.data[0] = engine.get_random_uint64();
            scalar.data[1] = engine.get_random_uint64();
            msm.scalars.push_back(scalar);
            msm.base_points.push_back(base_point);
        }
        result.ecc_muls.emplace_back(msm);
    }
    return result;
}

TEST(SumcheckRelation, ECCVMMSMRelationAlgebra)
{
    ECCMSMRelationAlgebra<FF> relation = ECCMSMRelationAlgebra<FF>();
    barretenberg::fr result = 0;
    barretenberg::fr scaling_factor = 1;

    ExecutionTrace trace = generate_msms_native();

    auto rows = trace.export_rows();
    const size_t num_rows = rows[0].size();
    for (size_t i = 0; i < num_rows; ++i) {
        ExecutionTrace::EccMSMRow row;
        for (size_t j = 0; j < NUM_POLYNOMIALS; ++j) {
            row[j] = rows[j][i];
        }
        relation.add_edge_contribution(result, row, {}, scaling_factor);
        ASSERT(result == 0);
        EXPECT_EQ(result, 0);
    }
};

TEST(Sumcheck, ECCVMMSMSumcheck)
{

    auto run_test = [](bool) {
        const size_t multivariate_d(9);
        const size_t multivariate_n(1 << multivariate_d);

        ExecutionTrace trace = generate_msms_native();
        auto rows = trace.export_rows();

        auto full_polynomials = construct_full_polynomials(rows);

        honk::sumcheck::RelationParameters<FF> relation_parameters{
            .beta = 1,
            .gamma = 1,
            .public_input_delta = 1,
        };
        auto prover_transcript = honk::ProverTranscript<FF>::init_empty();

        auto sumcheck_prover =
            Sumcheck<Flavor, honk::ProverTranscript<FF>, ECCMSMRelationProver>(multivariate_n, prover_transcript);

        auto [multivariate_challenge, evaluations] =
            sumcheck_prover.execute_prover(full_polynomials, relation_parameters);

        auto verifier_transcript = honk::VerifierTranscript<FF>::init_empty(prover_transcript);
        auto [alpha, zeta] = verifier_transcript.get_challenges("Sumcheck:alpha", "Sumcheck:zeta");
        PowUnivariate<FF> pow_univariate(zeta);
        std::string round_univariate_label = "Sumcheck:univariate_" + std::to_string(0);
        FF target_total_sum = 0;
        auto univariate_0 =
            verifier_transcript.template receive_from_prover<Univariate<FF, ECCMSMRelationProver<FF>::RELATION_LENGTH>>(
                round_univariate_label);

        {
            FF total_sum = univariate_0.value_at(0) + (pow_univariate.zeta_pow * univariate_0.value_at(1));
            // target_total_sum = sigma_{l} =
            std::cout << "check_sum = " << total_sum << std::endl;
            bool sumcheck_round_failed = (target_total_sum != total_sum);
            EXPECT_FALSE(sumcheck_round_failed);
        }
        FF round_challenge = verifier_transcript.get_challenge("Sumcheck:u_" + std::to_string(0));
        {
            auto barycentric = BarycentricData<FF,
                                               ECCMSMRelationProver<FF>::RELATION_LENGTH,
                                               ECCMSMRelationProver<FF>::RELATION_LENGTH>();
            // Evaluate T^{l}(u_{l})
            target_total_sum = barycentric.evaluate(univariate_0, round_challenge);
            // Evaluate (1−u_l) + u_l ⋅ ζ^{2^l} )
            FF pow_monomial_eval = pow_univariate.univariate_eval(round_challenge);
            // sigma_{l+1} = S^l(u_l) = (1−u_l) + u_l⋅ζ^{2^l} ) ⋅ T^{l}(u_l)
            target_total_sum *= pow_monomial_eval;
        }
        pow_univariate.partially_evaluate(round_challenge);

        round_univariate_label = "Sumcheck:univariate_" + std::to_string(1);
        auto univariate_1 =
            verifier_transcript.template receive_from_prover<Univariate<FF, ECCMSMRelationProver<FF>::RELATION_LENGTH>>(
                round_univariate_label);
        {
            FF total_sum = univariate_1.value_at(0) + (pow_univariate.zeta_pow * univariate_1.value_at(1));
            // target_total_sum = sigma_{l} =
            std::cout << "check_sum = " << total_sum << std::endl;
            bool sumcheck_round_failed = (target_total_sum != total_sum);
            EXPECT_FALSE(sumcheck_round_failed);
        }

        round_challenge = verifier_transcript.get_challenge("Sumcheck:u_" + std::to_string(1));
        {
            auto barycentric = BarycentricData<FF,
                                               ECCMSMRelationProver<FF>::RELATION_LENGTH,
                                               ECCMSMRelationProver<FF>::RELATION_LENGTH>();
            // Evaluate T^{l}(u_{l})
            target_total_sum = barycentric.evaluate(univariate_1, round_challenge);
            // Evaluate (1−u_l) + u_l ⋅ ζ^{2^l} )
            FF pow_monomial_eval = pow_univariate.univariate_eval(round_challenge);
            // sigma_{l+1} = S^l(u_l) = (1−u_l) + u_l⋅ζ^{2^l} ) ⋅ T^{l}(u_l)
            target_total_sum *= pow_monomial_eval;
        }
        pow_univariate.partially_evaluate(round_challenge);
        round_univariate_label = "Sumcheck:univariate_" + std::to_string(2);
        auto univariate_2 =
            verifier_transcript.template receive_from_prover<Univariate<FF, ECCMSMRelationProver<FF>::RELATION_LENGTH>>(
                round_univariate_label);
        {
            FF total_sum = univariate_2.value_at(0) + (pow_univariate.zeta_pow * univariate_2.value_at(1));
            // target_total_sum = sigma_{l} =
            std::cout << "check_sum = " << total_sum << std::endl;
            bool sumcheck_round_failed = (target_total_sum != total_sum);
            EXPECT_FALSE(sumcheck_round_failed);
        }
    };
    run_test(/* is_random_input=*/false);
}

TEST(Sumcheck, ECCVMMSMProver)
{

    const size_t multivariate_d(9);
    const size_t multivariate_n(1 << multivariate_d);

    ExecutionTrace trace = generate_msms_native();
    auto rows = trace.export_rows();

    auto full_polynomials = construct_full_polynomials(rows);
    honk::sumcheck::RelationParameters<FF> relation_parameters{
        .beta = FF::random_element(),
        .gamma = FF::random_element(),
        .public_input_delta = FF::one(),
    };
    auto prover_transcript = honk::ProverTranscript<FF>::init_empty();
    // auto relation = ECCMSMRelation<barretenberg::fr, TestWrapper>();

    auto sumcheck_prover =
        Sumcheck<Flavor, honk::ProverTranscript<FF>, ECCMSMRelationProver>(multivariate_n, prover_transcript);

    auto prover_output = sumcheck_prover.execute_prover(full_polynomials, relation_parameters);

    auto verifier_transcript = honk::VerifierTranscript<FF>::init_empty(prover_transcript);

    auto sumcheck_verifier =
        Sumcheck<Flavor, honk::VerifierTranscript<FF>, ECCMSMRelationVerifier>(multivariate_n, verifier_transcript);

    std::optional verifier_output = sumcheck_verifier.execute_verifier(relation_parameters);

    ASSERT_TRUE(verifier_output.has_value());
    ASSERT_EQ(prover_output, *verifier_output);
}

} // namespace proof_system::honk_relation_tests_eccmsm
