#include "ecc_vm_types.hpp"
#include "barretenberg/honk/sumcheck/sumcheck.hpp"
#include "barretenberg/honk/flavor/ecc_vm.hpp"
#include "barretenberg/numeric/random/engine.hpp"

#include "./pseudo_builder/pseudo_builder.hpp"

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

template <typename Field> using ECCVMWnafAlgebra = proof_system::honk::sumcheck::eccvm::ECCVMWnafAlgebra<Field>;

template <typename Field> using ECCVMWnafProver = proof_system::honk::sumcheck::eccvm::ECCVMWnafProver<Field>;

template <typename Field> using ECCVMWnafVerifier = proof_system::honk::sumcheck::eccvm::ECCVMWnafVerifier<Field>;

template <typename Field>
using ECCVMPointTableAlgebra = proof_system::honk::sumcheck::eccvm::ECCVMPointTableAlgebra<Field>;

template <typename Field>
using ECCVMPointTableProver = proof_system::honk::sumcheck::eccvm::ECCVMPointTableProver<Field>;

template <typename Field>
using ECCVMPointTableVerifier = proof_system::honk::sumcheck::eccvm::ECCVMPointTableVerifier<Field>;

template <typename Field>
using ECCMSMRelationAlgebra = proof_system::honk::sumcheck::eccvm::ECCMSMRelationAlgebra<Field>;

template <typename Field> using ECCMSMRelationProver = proof_system::honk::sumcheck::eccvm::ECCMSMRelationProver<Field>;

template <typename Field>
using ECCMSMRelationVerifier = proof_system::honk::sumcheck::eccvm::ECCMSMRelationVerifier<Field>;

template <typename Field>
using ECCVMSetRelationAlgebra = proof_system::honk::sumcheck::eccvm::ECCVMSetRelationAlgebra<Field>;

template <typename Field>
using ECCVMSetRelationProver = proof_system::honk::sumcheck::eccvm::ECCVMSetRelationProver<Field>;

template <typename Field>
using ECCVMSetRelationVerifier = proof_system::honk::sumcheck::eccvm::ECCVMSetRelationVerifier<Field>;

template <typename Field>
using ECCVMLookupRelationAlgebra = proof_system::honk::sumcheck::eccvm::ECCVMLookupRelationAlgebra<Field>;

template <typename Field>
using ECCVMLookupRelationProver = proof_system::honk::sumcheck::eccvm::ECCVMLookupRelationProver<Field>;

template <typename Field>
using ECCVMLookupRelationVerifier = proof_system::honk::sumcheck::eccvm::ECCVMLookupRelationVerifier<Field>;

static constexpr size_t NUM_POLYNOMIALS = Flavor::NUM_ALL_ENTITIES;

namespace proof_system::honk_relation_tests_ecc_vm_full {

namespace {
auto& engine = numeric::random::get_debug_engine();
}

ECCVMBuilder generate_trace(numeric::random::Engine* engine = nullptr)
{
    ECCVMBuilder result;

    grumpkin::g1::element a = grumpkin::get_generator(0);
    grumpkin::g1::element b = grumpkin::get_generator(1);
    grumpkin::g1::element c = grumpkin::get_generator(2);
    grumpkin::fr x = grumpkin::fr::random_element(engine);
    grumpkin::fr y = grumpkin::fr::random_element(engine);

    grumpkin::g1::element expected_1 = (a * x) + a + (b * x) + (b * x) + (b * x);
    grumpkin::g1::element expected_2 = (a * x) + c + (b * x);

    result.mul_accumulate(a, x);
    // result.mul_accumulate(b, y);

    // result.mul_accumulate(a, x);
    // result.mul_accumulate(b, x);
    // result.mul_accumulate(b, x);
    // result.add_accumulate(a);
    // result.mul_accumulate(b, x);
    // result.eq(expected_1);
    // result.add_accumulate(c);
    // result.mul_accumulate(a, x);
    // result.mul_accumulate(b, x);
    // result.eq(expected_2);
    // result.mul_accumulate(a, x);
    // result.mul_accumulate(b, x);
    // result.mul_accumulate(c, x);

    return result;
}

void compute_lookup_inverse_polynomial(RawPolynomials& polynomials,
                                       const honk::sumcheck::RelationParameters<FF>& relation_parameters)
{
    auto lookup_relation = ECCVMLookupRelationAlgebra<barretenberg::fr>();

    const size_t num_rows = polynomials[0].size();
    constexpr size_t READ_TERMS = ECCVMLookupRelationAlgebra<barretenberg::fr>::READ_TERMS;
    constexpr size_t WRITE_TERMS = ECCVMLookupRelationAlgebra<barretenberg::fr>::WRITE_TERMS;

    std::vector<size_t> slice_read_counts;
    std::vector<size_t> slice_write_counts;
    std::vector<size_t> positive_read_counts;
    std::vector<size_t> negative_read_counts;
    std::vector<size_t> pc_write_counts;

    for (size_t i = 0; i < 16; ++i) {
        slice_read_counts.push_back(0);
        slice_write_counts.push_back(0);
    }
    for (size_t i = 0; i < num_rows; ++i) {
        positive_read_counts.push_back(0);
        negative_read_counts.push_back(0);
        pc_write_counts.push_back(0);
    }
    // how to test inclusion in lookup relation?
    // we want to map pc, slice, x, y

    for (size_t i = 0; i < num_rows; ++i) {
        if (polynomials.q_wnaf[i] == 1) {
            const uint256_t round = polynomials.table_round[i];
            const size_t slice_negative = static_cast<size_t>(round);
            const size_t slice_positive = 15ULL - static_cast<size_t>(round);
            const uint256_t pc = polynomials.table_pc[i] * 16;

            slice_write_counts[slice_positive] += 1;
            slice_write_counts[slice_negative] += 1;
            pc_write_counts[static_cast<size_t>(pc) + slice_positive] += 1;
            pc_write_counts[static_cast<size_t>(pc) + slice_negative] += 1;
        }
        if (polynomials.msm_q_add1[i] == 1) {
            const uint256_t slice = polynomials.msm_slice1[i];
            size_t converted_slice = static_cast<size_t>(slice);
            slice_read_counts[converted_slice] += 1;

            const uint256_t pc = (polynomials.msm_pc[i] - polynomials.msm_count[i]);
            if (slice < 8) {
                negative_read_counts[static_cast<size_t>(pc * 8) + converted_slice] += 1;
            } else {
                positive_read_counts[static_cast<size_t>(pc * 8) + converted_slice - 8] += 1;
            }
        }
        if (polynomials.msm_q_add2[i] == 1) {
            const uint256_t slice = polynomials.msm_slice2[i];
            size_t converted_slice = static_cast<size_t>(slice);
            slice_read_counts[converted_slice] += 1;

            const uint256_t pc = polynomials.msm_pc[i] - polynomials.msm_count[i] - 1;
            if (slice < 8) {
                negative_read_counts[static_cast<size_t>(pc * 8) + converted_slice] += 1;
            } else {
                positive_read_counts[static_cast<size_t>(pc * 8) + converted_slice - 8] += 1;
            }
        }
        if (polynomials.msm_q_add3[i] == 1) {
            const uint256_t slice = polynomials.msm_slice3[i];
            size_t converted_slice = static_cast<size_t>(slice);
            slice_read_counts[converted_slice] += 1;

            const uint256_t pc = polynomials.msm_pc[i] - polynomials.msm_count[i] - 2;
            if (slice < 8) {
                negative_read_counts[static_cast<size_t>(pc * 8) + converted_slice] += 1;
            } else {
                positive_read_counts[static_cast<size_t>(pc * 8) + converted_slice - 8] += 1;
            }
        }
        if (polynomials.msm_q_add4[i] == 1) {
            const uint256_t slice = polynomials.msm_slice4[i];
            size_t converted_slice = static_cast<size_t>(slice);
            slice_read_counts[converted_slice] += 1;

            const uint256_t pc = polynomials.msm_pc[i] - polynomials.msm_count[i] - 3;
            if (slice < 8) {
                negative_read_counts[static_cast<size_t>(pc * 8) + converted_slice] += 1;
            } else {
                positive_read_counts[static_cast<size_t>(pc * 8) + converted_slice - 8] += 1;
            }
        }
    }

    // for (size_t i = 0; i < 170; ++i) {
    //     std::cout << "positive read count (expected vs res)[" << i << "] = " << positive_read_counts[i] << " : "
    //               << polynomials.lookup_read_counts_0[i] << std::endl;
    //     std::cout << "negative read count (expected vs res)[" << i << "] = " << negative_read_counts[i] << " : "
    //               << polynomials.lookup_read_counts_1[i] << std::endl;

    //     // ASSERT(positive_read_counts[i] == (size_t)uint256_t(polynomials.lookup_read_counts_0[i]));
    //     // ASSERT(negative_read_counts[i] == (size_t)uint256_t(polynomials.lookup_read_counts_1[i]));
    // }
    FF lhs = 0;
    FF rhs = 0;

    std::vector<size_t> lhs_counts;
    std::vector<size_t> rhs_counts;

    for (size_t i = 0; i < num_rows; ++i) {
        lhs_counts.push_back(0);
        rhs_counts.push_back(0);
    }
    for (size_t i = 0; i < num_rows; ++i) {

        barretenberg::constexpr_for<0, READ_TERMS, 1>([&]<size_t read_index> {
            if (lookup_relation.compute_read_term_predicate<read_index>(polynomials, relation_parameters, i) == 1) {
                auto read_term = lookup_relation.compute_read_term<read_index>(polynomials, relation_parameters, i);
                lhs += (FF(1) / read_term);
                size_t count_idx = 0;
                uint256_t msm_pc = polynomials.msm_pc[i];
                uint256_t msm_count = polynomials.msm_count[i];
                size_t pc = static_cast<size_t>(msm_pc - msm_count);
                if (read_index == 0) {
                    count_idx = static_cast<size_t>(uint256_t(polynomials.msm_slice1[i]));
                }
                if (read_index == 1) {
                    pc -= 1;
                    count_idx = static_cast<size_t>(uint256_t(polynomials.msm_slice2[i]));
                }
                if (read_index == 2) {
                    pc -= 2;
                    count_idx = static_cast<size_t>(uint256_t(polynomials.msm_slice3[i]));
                }
                if (read_index == 3) {
                    pc -= 3;
                    count_idx = static_cast<size_t>(uint256_t(polynomials.msm_slice4[i]));
                }
                ASSERT(count_idx < 16);
                lhs_counts[((pc - 1) * 16) + count_idx]++;
            }
        });

        size_t round = i % 8;

        uint256_t round_pc = polynomials.table_pc[i];
        size_t positive_round = 15 - round;

        if (polynomials.q_wnaf[i] == 1) {
            rhs_counts[(static_cast<size_t>(round_pc - 1) * 16) + positive_round] =
                static_cast<size_t>(uint256_t(polynomials.lookup_read_counts_0[i]));
            rhs_counts[(static_cast<size_t>(round_pc - 1) * 16) + round] =
                static_cast<size_t>(uint256_t(polynomials.lookup_read_counts_1[i]));
        }
        barretenberg::constexpr_for<0, WRITE_TERMS, 1>([&]<size_t write_index> {
            if (lookup_relation.compute_write_term_predicate<write_index>(polynomials, relation_parameters, i) == 1) {
                auto write_term = lookup_relation.compute_write_term<write_index>(polynomials, relation_parameters, i);
                if (write_index == 0) {
                    rhs += polynomials.lookup_read_counts_0[i] * (FF(1) / write_term);
                } else {
                    rhs += polynomials.lookup_read_counts_1[i] * (FF(1) / write_term);
                }
            }
        });
    }

    // for (size_t i = 0; i < num_rows; ++i) {
    //     // std::cout << "lhs_counts/rhs_counts[" << i
    //     //           << "] = " << static_cast<size_t>(uint256_t(polynomials.lookup_read_counts_0[i])) << " : "
    //     //           << static_cast<size_t>(uint256_t(polynomials.lookup_read_counts_1[i])) << std::endl;

    //     std::cout << "lhs_counts/rhs_counts[" << i << "] = " << lhs_counts[i] << " : " << rhs_counts[i] << std::endl;
    //     //       ASSERT(lhs_counts[i] == rhs_counts[i]);
    // }
    ASSERT(lhs == rhs);
    // for (size_t i = 0; i < 32; ++i) {
    //     std::cout << "read_counts[positive][" << i << "] = " << polynomials.lookup_read_counts_0[i] << std::endl;
    //     std::cout << "read_counts[negative][" << i << "] = " << polynomials.lookup_read_counts_1[i] << std::endl;
    // }

    //  bool has_inverse = false;
    auto& inverse_polynomial = polynomials.lookup_inverses;
    for (size_t i = 0; i < num_rows; ++i) {
        FF denominator = 1;
        //  bool has_inverse = false;
        barretenberg::constexpr_for<0, READ_TERMS, 1>([&]<size_t read_index> {
            auto denominator_term = lookup_relation.compute_read_term<read_index>(polynomials, relation_parameters, i);
            denominator *= denominator_term;
            // if (lookup_relation.compute_read_term_predicate<read_index>(polynomials, relation_parameters, i) == 1) {

            //     has_inverse = true;
            // }
        });
        barretenberg::constexpr_for<0, WRITE_TERMS, 1>([&]<size_t write_index> {
            auto denominator_term =
                lookup_relation.compute_write_term<write_index>(polynomials, relation_parameters, i);
            denominator *= denominator_term;
            // if (lookup_relation.compute_write_term_predicate<write_index>(polynomials, relation_parameters, i) == 1)
            // {
            //     has_inverse = true;
            // }
        });
        // leave as zero if no inverse
        bool has_inverse =
            polynomials.msm_q_add[i] == 1 || polynomials.q_wnaf[i] == 1 || polynomials.msm_q_skew[i] == 1;
        if (has_inverse) {
            inverse_polynomial[i] = denominator;
        }
    };

    // todo might be inverting zero in field bleh bleh
    auto numerator = inverse_polynomial;
    FF::batch_invert(inverse_polynomial);

    for (size_t i = 0; i < num_rows; ++i) {
        if (numerator[i] != 0) {
            ASSERT(numerator[i] * inverse_polynomial[i] == 1);
        }
    }
}

void compute_permutation_polynomials(RawPolynomials& polynomials,
                                     const honk::sumcheck::RelationParameters<FF>& relation_parameters)
{

    auto set_relation = ECCVMSetRelationAlgebra<barretenberg::fr>();

    constexpr size_t NUMERATOR_TERMS = ECCVMSetRelationAlgebra<barretenberg::fr>::NUMERATOR_TERMS;
    constexpr size_t DENOMINATOR_TERMS = ECCVMSetRelationAlgebra<barretenberg::fr>::DENOMINATOR_TERMS;

    const size_t num_rows = polynomials[0].size();

    std::array<std::vector<barretenberg::fr>, NUMERATOR_TERMS> numerator_accumulator;
    std::array<std::vector<barretenberg::fr>, std::max(DENOMINATOR_TERMS, size_t(2))> denominator_accumulator;

    if (DENOMINATOR_TERMS < 2) {
        denominator_accumulator[1].resize(num_rows);
    }
    // compute accumulators
    // TODO: spawn 1 thread per constexpr_for iteration :o
    barretenberg::constexpr_for<0, NUMERATOR_TERMS, 1>([&]<size_t numerator_index> {
        numerator_accumulator[numerator_index].reserve(num_rows);
        for (size_t i = 0; i < num_rows; ++i) {

            numerator_accumulator[numerator_index].emplace_back(
                set_relation.compute_numerator_term<numerator_index>(polynomials, relation_parameters, i));
        }
    });

    barretenberg::constexpr_for<0, DENOMINATOR_TERMS, 1>([&]<size_t denominator_index> {
        denominator_accumulator[denominator_index].reserve(num_rows);
        for (size_t i = 0; i < num_rows; ++i) {
            denominator_accumulator[denominator_index].emplace_back(
                set_relation.compute_denominator_term<denominator_index>(polynomials, relation_parameters, i));
        }
    });

    // std::vector<uint256_t> numerator;
    // std::vector<uint256_t> denominator;
    // for (size_t j = 0; j < NUMERATOR_TERMS; ++j) {
    //     for (size_t i = 0; i < num_rows; ++i) {
    //         if (numerator_accumulator[j][i] != 1) {
    //             numerator.push_back(numerator_accumulator[j][i]);
    //         }
    //     }
    // }
    // for (size_t j = 0; j < DENOMINATOR_TERMS; ++j) {
    //     for (size_t i = 0; i < num_rows; ++i) {
    //         if (denominator_accumulator[j][i] != 1) {
    //             denominator.push_back(denominator_accumulator[j][i]);
    //         }
    //     }
    // // }

    // std::sort(numerator.begin(), numerator.end());
    // std::sort(denominator.begin(), denominator.end());

    // for (size_t i = 0; i < std::min(numerator.size(), denominator.size()); ++i) {
    //     std::cout << "[" << i << "] = " << numerator[i] << " : " << denominator[i] << std::endl;
    // }
    // std::cout << "size diff = " << numerator.size() << " : " << denominator.size() << std::endl;

    // combine accumulators
    // TODO: multithread
    for (size_t i = 0; i < num_rows; ++i) {
        for (size_t j = 1; j < NUMERATOR_TERMS; ++j) {
            numerator_accumulator[0][i] *= numerator_accumulator[j][i];
        }

        for (size_t j = 1; j < DENOMINATOR_TERMS; ++j) {
            denominator_accumulator[0][i] *= denominator_accumulator[j][i];
        }
    }

    // compute product
    for (size_t i = 0; i < num_rows - 1; ++i) {
        numerator_accumulator[0][i + 1] *= numerator_accumulator[0][i];
        denominator_accumulator[0][i + 1] *= denominator_accumulator[0][i];
    }

    uint256_t a = uint256_t(numerator_accumulator[0][num_rows - 1]);
    uint256_t b = uint256_t(denominator_accumulator[0][num_rows - 1]);
    ASSERT(a == b);
    // std::cout << "assert test? " << (a == b) << std::endl;
    // ASSERT(uint256_t(numerator_accumulator[0][num_rows - 1]) == uint256_t(denominator_accumulator[0][num_rows -
    // 1])); Step (4) Use Montgomery batch inversion to compute z_perm[i+1] = numerator_accumulator[0][i] /
    // denominator_accumulator[0][i]. At the end of this computation, the quotient numerator_accumulator[0] /
    // denominator_accumulator[0] is stored in numerator_accumulator[0].
    // Note: Since numerator_accumulator[0][i] corresponds to z_lookup[i+1], we only iterate up to i = (n - 2).
    FF* inversion_coefficients = &denominator_accumulator[1][0]; // arbitrary scratch space

    FF inversion_accumulator = FF::one();
    for (size_t i = 0; i < num_rows - 1; ++i) {
        inversion_coefficients[i] = numerator_accumulator[0][i] * inversion_accumulator;
        inversion_accumulator *= denominator_accumulator[0][i];
    }
    inversion_accumulator = inversion_accumulator.invert(); // perform single inversion per thread
    for (size_t i = num_rows - 2; i != std::numeric_limits<size_t>::max(); --i) {
        numerator_accumulator[0][i] = inversion_accumulator * inversion_coefficients[i];
        inversion_accumulator *= denominator_accumulator[0][i];
    }

    // construct permutation polynomial
    // Initialize 0th coefficient to 0 to ensure z_perm is left-shiftable via division by X in gemini
    polynomials.z_perm[0] = 0;
    for (size_t i = 0; i < num_rows - 1; ++i) {
        polynomials.z_perm[i + 1] = numerator_accumulator[0][i];
        polynomials.z_perm_shift[i] = numerator_accumulator[0][i];
    }
}

TEST(SumcheckRelation, ECCVMLookupRelationAlgebra)
{
    const auto run_test = []() {
        auto lookup_relation = ECCVMLookupRelationAlgebra<barretenberg::fr>();

        barretenberg::fr scaling_factor = barretenberg::fr::random_element();
        const FF gamma = FF::random_element(&engine);
        const FF eta = FF::random_element(&engine);
        const FF eta_sqr = eta.sqr();
        const FF eta_cube = eta_sqr * eta;
        auto permutation_offset =
            gamma * (gamma + eta_sqr) * (gamma + eta_sqr + eta_sqr) * (gamma + eta_sqr + eta_sqr + eta_sqr);
        permutation_offset = permutation_offset.invert();
        honk::sumcheck::RelationParameters<barretenberg::fr> relation_params{
            .eta = eta,
            .beta = 1,
            .gamma = gamma,
            .public_input_delta = 1,
            .lookup_grand_product_delta = 1,
            .eta_sqr = eta_sqr,
            .eta_cube = eta_cube,
            .permutation_offset = permutation_offset,
        };

        ECCVMBuilder trace2 = generate_trace(&engine);

        RawPolynomials rows = trace2.compute_full_polynomials();
        compute_lookup_inverse_polynomial(rows, relation_params);

        compute_permutation_polynomials(rows, relation_params);

        // auto transcript_trace = transcript_trace.export_rows();

        const size_t num_rows = rows[0].size();
        barretenberg::fr result = 0;
        for (size_t i = 0; i < num_rows; ++i) {
            Flavor::RowPolynomials row;
            for (size_t j = 0; j < NUM_POLYNOMIALS; ++j) {
                row[j] = rows[j][i];
            }
            lookup_relation.add_edge_contribution(result, row, relation_params, scaling_factor);
        }
        EXPECT_EQ(result, 0);
    };
    run_test();
}

TEST(SumcheckRelation, ECCVMFullRelationAlgebra)
{
    const auto run_test = []() {
        auto transcript_relation = ECCVMTranscriptAlgebra<barretenberg::fr>();
        auto point_relation = ECCVMPointTableAlgebra<barretenberg::fr>();
        auto wnaf_relation = ECCVMWnafAlgebra<barretenberg::fr>();
        auto msm_relation = ECCMSMRelationAlgebra<barretenberg::fr>();
        auto set_relation = ECCVMSetRelationAlgebra<barretenberg::fr>();
        auto lookup_relation = ECCVMLookupRelationAlgebra<barretenberg::fr>();

        barretenberg::fr scaling_factor = barretenberg::fr::random_element();
        const FF gamma = FF::random_element(&engine);
        const FF eta = FF::random_element(&engine);
        const FF eta_sqr = eta.sqr();
        const FF eta_cube = eta_sqr * eta;
        auto permutation_offset =
            gamma * (gamma + eta_sqr) * (gamma + eta_sqr + eta_sqr) * (gamma + eta_sqr + eta_sqr + eta_sqr);
        permutation_offset = permutation_offset.invert();
        honk::sumcheck::RelationParameters<barretenberg::fr> relation_params{
            .eta = eta,
            .beta = 1,
            .gamma = gamma,
            .public_input_delta = 1,
            .lookup_grand_product_delta = 1,
            .eta_sqr = eta_sqr,
            .eta_cube = eta_cube,
            .permutation_offset = permutation_offset,
        };

        ECCVMBuilder trace2 = generate_trace(&engine);

        RawPolynomials rows = trace2.compute_full_polynomials();
        compute_permutation_polynomials(rows, relation_params);
        compute_lookup_inverse_polynomial(rows, relation_params);

        // auto transcript_trace = transcript_trace.export_rows();

        FF lookup_result = 0;
        const size_t num_rows = rows[0].size();
        for (size_t i = 0; i < num_rows; ++i) {
            Flavor::RowPolynomials row;
            for (size_t j = 0; j < NUM_POLYNOMIALS; ++j) {
                row[j] = rows[j][i];
            }
            barretenberg::fr result = 0;
            transcript_relation.add_edge_contribution(result, row, relation_params, scaling_factor);
            EXPECT_EQ(result, 0);
            ASSERT(result == 0);
            result = 0;
            point_relation.add_edge_contribution(result, row, relation_params, scaling_factor);
            EXPECT_EQ(result, 0);
            ASSERT(result == 0);
            result = 0;
            wnaf_relation.add_edge_contribution(result, row, relation_params, scaling_factor);
            EXPECT_EQ(result, 0);
            ASSERT(result == 0);
            result = 0;
            msm_relation.add_edge_contribution(result, row, relation_params, scaling_factor);
            EXPECT_EQ(result, 0);
            ASSERT(result == 0);
            set_relation.add_edge_contribution(result, row, relation_params, scaling_factor);
            EXPECT_EQ(result, 0);
            ASSERT(result == 0);
            lookup_relation.add_edge_contribution(lookup_result, row, relation_params, scaling_factor);
        }
        EXPECT_EQ(lookup_result, 0);
    };
    run_test();
}

TEST(SumcheckRelation, ECCVMFullRelationProver)
{
    const auto run_test = []() {
        const FF gamma = FF::random_element(&engine);
        const FF eta = FF::random_element(&engine);
        const FF eta_sqr = eta.sqr();
        const FF eta_cube = eta_sqr * eta;
        auto permutation_offset =
            gamma * (gamma + eta_sqr) * (gamma + eta_sqr + eta_sqr) * (gamma + eta_sqr + eta_sqr + eta_sqr);
        permutation_offset = permutation_offset.invert();
        honk::sumcheck::RelationParameters<barretenberg::fr> relation_params{
            .eta = eta,
            .beta = 1,
            .gamma = gamma,
            .public_input_delta = 1,
            .lookup_grand_product_delta = 1,
            .eta_sqr = eta_sqr,
            .eta_cube = eta_cube,
            .permutation_offset = permutation_offset,
        };

        barretenberg::fr::random_element(&engine);

        ECCVMBuilder trace2 = generate_trace(&engine);

        RawPolynomials full_polynomials = trace2.compute_full_polynomials();
        compute_permutation_polynomials(full_polynomials, relation_params);
        compute_lookup_inverse_polynomial(full_polynomials, relation_params);

        const size_t multivariate_n = full_polynomials[0].size();
        const size_t multivariate_d = numeric::get_msb64(multivariate_n);

        EXPECT_EQ(1ULL << multivariate_d, multivariate_n);

        auto prover_transcript = honk::ProverTranscript<FF>::init_empty();

        auto sumcheck_prover = Sumcheck<Flavor,
                                        honk::ProverTranscript<FF>,
                                        ECCVMTranscriptProver,
                                        ECCVMWnafProver,
                                        ECCVMPointTableProver,
                                        ECCMSMRelationProver,
                                        ECCVMSetRelationProver,
                                        ECCVMLookupRelationProver>(multivariate_n, prover_transcript);

        auto prover_output = sumcheck_prover.execute_prover(full_polynomials, relation_params);

        auto verifier_transcript = honk::VerifierTranscript<FF>::init_empty(prover_transcript);

        auto sumcheck_verifier = Sumcheck<Flavor,
                                          honk::VerifierTranscript<FF>,
                                          ECCVMTranscriptVerifier,
                                          ECCVMWnafVerifier,
                                          ECCVMPointTableVerifier,
                                          ECCMSMRelationVerifier,
                                          ECCVMSetRelationVerifier,
                                          ECCVMLookupRelationVerifier>(multivariate_n, verifier_transcript);

        std::optional verifier_output = sumcheck_verifier.execute_verifier(relation_params);

        ASSERT_TRUE(verifier_output.has_value());
        ASSERT_EQ(prover_output, *verifier_output);
    };
    run_test();
}
} // namespace proof_system::honk_relation_tests_ecc_vm_full
