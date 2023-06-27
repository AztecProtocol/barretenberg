#pragma once
#include <array>
#include <concepts>
#include <span>
#include <string>
#include <type_traits>
#include <vector>
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/honk/sumcheck/polynomials/univariate.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/honk/transcript/transcript.hpp"
#include "barretenberg/plonk/proof_system/proving_key/proving_key.hpp"
#include "barretenberg/polynomials/evaluation_domain.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/proof_system/circuit_constructors/ultra_circuit_constructor.hpp"
#include "barretenberg/srs/reference_string/reference_string.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"

namespace proof_system::honk::flavor {

class ECCVM {
  public:
    using CircuitConstructor = UltraCircuitConstructor; // change to...eccvm circuit constructor 0.o
    using FF = barretenberg::fr;
    using Polynomial = barretenberg::Polynomial<FF>;
    using PolynomialHandle = std::span<FF>;
    using G1 = barretenberg::g1;
    using GroupElement = G1::element;
    using Commitment = G1::affine_element;
    using CommitmentHandle = G1::affine_element;
    using PCSParams = pcs::kzg::Params;

    static constexpr size_t NUM_WIRES = CircuitConstructor::NUM_WIRES;
    // The number of multivariate polynomials on which a sumcheck prover sumcheck operates (including shifts). We often
    // need containers of this size to hold related data, so we choose a name more agnostic than `NUM_POLYNOMIALS`.
    // Note: this number does not include the individual sorted list polynomials.
    static constexpr size_t NUM_ALL_ENTITIES = 105;
    // The number of polynomials precomputed to describe a circuit and to aid a prover in constructing a satisfying
    // assignment of witnesses. We again choose a neutral name.
    static constexpr size_t NUM_PRECOMPUTED_ENTITIES = 2;
    // The total number of witness entities not including shifts.
    static constexpr size_t NUM_WITNESS_ENTITIES = 74;

  private:
    // class Counter {
    //     constexpr size_t foo()
    //     {
    //         return Thing<>;
    //     }
    // };
    /**
     * @brief A base class labelling precomputed entities and (ordered) subsets of interest.
     * @details Used to build the proving key and verification key.
     */
    template <typename DataType, typename HandleType>
    class PrecomputedEntities : public PrecomputedEntities_<DataType, HandleType, NUM_PRECOMPUTED_ENTITIES> {
      public:
        DataType& lagrange_first = std::get<0>(this->_data);
        DataType& lagrange_last = std::get<1>(this->_data);

        std::vector<HandleType> get_selectors() override { return { lagrange_first, lagrange_last }; };
        std::vector<HandleType> get_sigma_polynomials() override { return {}; };
        std::vector<HandleType> get_id_polynomials() override { return {}; };
        std::vector<HandleType> get_table_polynomials() { return {}; };
    };

    /**
     * @brief Container for all witness polynomials used/constructed by the prover.
     * @details Shifts are not included here since they do not occupy their own memory.
     */
    template <typename DataType, typename HandleType>
    class WitnessEntities : public WitnessEntities_<DataType, HandleType, NUM_WITNESS_ENTITIES> {
      public:
        // clang-format off
        DataType& q_transcript_add                  = std::get<0>(this->_data);
        DataType& q_transcript_mul                  = std::get<1>(this->_data);
        DataType& q_transcript_eq                   = std::get<2>(this->_data);
        DataType& q_transcript_accumulate           = std::get<3>(this->_data);
        DataType& q_transcript_msm_transition       = std::get<4>(this->_data);
        DataType& transcript_pc                     = std::get<5>(this->_data);
        DataType& transcript_msm_count              = std::get<6>(this->_data);
        DataType& transcript_x                      = std::get<7>(this->_data);
        DataType& transcript_y                      = std::get<8>(this->_data);
        DataType& transcript_z1                     = std::get<9>(this->_data);
        DataType& transcript_z2                     = std::get<10>(this->_data);
        DataType& transcript_z1zero                 = std::get<11>(this->_data);
        DataType& transcript_z2zero                 = std::get<12>(this->_data);
        DataType& transcript_op                     = std::get<13>(this->_data);
        DataType& transcript_accumulator_x          = std::get<14>(this->_data);
        DataType& transcript_accumulator_y          = std::get<15>(this->_data);
        DataType& transcript_msm_x                  = std::get<16>(this->_data);
        DataType& transcript_msm_y                  = std::get<17>(this->_data);
        DataType& table_pc                          = std::get<18>(this->_data);
        DataType& table_point_transition            = std::get<19>(this->_data);
        DataType& table_round                       = std::get<20>(this->_data);
        DataType& table_scalar_sum                  = std::get<21>(this->_data);
        DataType& table_s1                          = std::get<22>(this->_data);
        DataType& table_s2                          = std::get<23>(this->_data);
        DataType& table_s3                          = std::get<24>(this->_data);
        DataType& table_s4                          = std::get<25>(this->_data);
        DataType& table_s5                          = std::get<26>(this->_data);
        DataType& table_s6                          = std::get<27>(this->_data);
        DataType& table_s7                          = std::get<28>(this->_data);
        DataType& table_s8                          = std::get<29>(this->_data);
        DataType& table_skew                        = std::get<30>(this->_data);
        DataType& table_dx                          = std::get<31>(this->_data);
        DataType& table_dy                          = std::get<32>(this->_data);
        DataType& table_tx                          = std::get<33>(this->_data);
        DataType& table_ty                          = std::get<34>(this->_data);
        DataType& q_msm_transition                  = std::get<35>(this->_data);
        DataType& msm_q_add                         = std::get<36>(this->_data);
        DataType& msm_q_double                      = std::get<37>(this->_data);
        DataType& msm_q_skew                        = std::get<38>(this->_data);
        DataType& msm_accumulator_x                 = std::get<39>(this->_data);
        DataType& msm_accumulator_y                 = std::get<40>(this->_data);
        DataType& msm_pc                            = std::get<41>(this->_data);
        DataType& msm_size_of_msm                   = std::get<42>(this->_data);
        DataType& msm_count                         = std::get<43>(this->_data);
        DataType& msm_round                         = std::get<44>(this->_data);
        DataType& msm_q_add1                        = std::get<45>(this->_data);
        DataType& msm_q_add2                        = std::get<46>(this->_data);
        DataType& msm_q_add3                        = std::get<47>(this->_data);
        DataType& msm_q_add4                        = std::get<48>(this->_data);
        DataType& msm_x1                            = std::get<49>(this->_data);
        DataType& msm_y1                            = std::get<50>(this->_data);
        DataType& msm_x2                            = std::get<51>(this->_data);
        DataType& msm_y2                            = std::get<52>(this->_data);
        DataType& msm_x3                            = std::get<53>(this->_data);
        DataType& msm_y3                            = std::get<54>(this->_data);
        DataType& msm_x4                            = std::get<55>(this->_data);
        DataType& msm_y4                            = std::get<56>(this->_data);
        DataType& msm_collision_x1                  = std::get<57>(this->_data);
        DataType& msm_collision_x2                  = std::get<58>(this->_data);
        DataType& msm_collision_x3                  = std::get<59>(this->_data);
        DataType& msm_collision_x4                  = std::get<60>(this->_data);
        DataType& msm_lambda1                       = std::get<61>(this->_data);
        DataType& msm_lambda2                       = std::get<62>(this->_data);
        DataType& msm_lambda3                       = std::get<63>(this->_data);
        DataType& msm_lambda4                       = std::get<64>(this->_data);
        DataType& msm_slice1                        = std::get<65>(this->_data);
        DataType& msm_slice2                        = std::get<66>(this->_data);
        DataType& msm_slice3                        = std::get<67>(this->_data);
        DataType& msm_slice4                        = std::get<68>(this->_data);
        DataType& msm_pc_shift                        = std::get<69>(this->_data);
        DataType& table_pc_shift                        = std::get<70>(this->_data);
        DataType& transcript_pc_shift                        = std::get<71>(this->_data);
        DataType& table_round_shift                        = std::get<72>(this->_data);
        DataType& q_wnaf                        = std::get<73>(this->_data);

        // clang-format on
        std::vector<HandleType> get_wires() override
        {
            return {
                q_transcript_add,
                q_transcript_mul,
                q_transcript_eq,
                q_transcript_accumulate,
                q_transcript_msm_transition,
                transcript_pc,
                transcript_msm_count,
                transcript_x,
                transcript_y,
                transcript_z1,
                transcript_z2,
                transcript_z1zero,
                transcript_z2zero,
                transcript_op,
                transcript_accumulator_x,
                transcript_accumulator_y,
                transcript_msm_x,
                transcript_msm_y,
                table_pc,
                table_point_transition,
                table_round,
                table_scalar_sum,
                table_s1,
                table_s2,
                table_s3,
                table_s4,
                table_s5,
                table_s6,
                table_s7,
                table_s8,
                table_skew,
                table_dx,
                table_dy,
                table_tx,
                table_ty,
                q_msm_transition,
                msm_q_add,
                msm_q_double,
                msm_q_skew,
                msm_accumulator_x,
                msm_accumulator_y,
                msm_pc,
                msm_size_of_msm,
                msm_count,
                msm_round,
                msm_q_add1,
                msm_q_add2,
                msm_q_add3,
                msm_q_add4,
                msm_x1,
                msm_y1,
                msm_x2,
                msm_y2,
                msm_x3,
                msm_y3,
                msm_x4,
                msm_y4,
                msm_collision_x1,
                msm_collision_x2,
                msm_collision_x3,
                msm_collision_x4,
                msm_lambda1,
                msm_lambda2,
                msm_lambda3,
                msm_lambda4,
                msm_slice1,
                msm_slice2,
                msm_slice3,
                msm_slice4,
                msm_pc_shift,
                table_pc_shift,
                transcript_pc_shift,
                table_round_shift,
            };
        };
        // The sorted concatenations of table and witness data needed for plookup.
        std::vector<HandleType> get_sorted_polynomials() { return {}; };
    };

    /**
     * @brief A base class labelling all entities (for instance, all of the polynomials used by the prover during
     * sumcheck) in this Honk variant along with particular subsets of interest
     * @details Used to build containers for: the prover's polynomial during sumcheck; the sumcheck's folded
     * polynomials; the univariates consturcted during during sumcheck; the evaluations produced by sumcheck.
     *
     * Symbolically we have: AllEntities = PrecomputedEntities + WitnessEntities + "ShiftedEntities". It could be
     * implemented as such, but we have this now.
     */
    template <typename DataType, typename HandleType>
    class AllEntities : public AllEntities_<DataType, HandleType, NUM_ALL_ENTITIES> {
      public:
        // clang-format off
        DataType& lagrange_first                    = std::get<0>(this->_data);
        DataType& lagrange_last                     = std::get<1>(this->_data);
        DataType& q_transcript_add                  = std::get<2 + 0>(this->_data);
        DataType& q_transcript_mul                  = std::get<2 + 1>(this->_data);
        DataType& q_transcript_eq                   = std::get<2 + 2>(this->_data);
        DataType& q_transcript_accumulate           = std::get<2 + 3>(this->_data);
        DataType& q_transcript_msm_transition       = std::get<2 + 4>(this->_data);
        DataType& transcript_pc                     = std::get<2 + 5>(this->_data);
        DataType& transcript_msm_count              = std::get<2 + 6>(this->_data);
        DataType& transcript_x                      = std::get<2 + 7>(this->_data);
        DataType& transcript_y                      = std::get<2 + 8>(this->_data);
        DataType& transcript_z1                     = std::get<2 + 9>(this->_data);
        DataType& transcript_z2                     = std::get<2 + 10>(this->_data);
        DataType& transcript_z1zero                 = std::get<2 + 11>(this->_data);
        DataType& transcript_z2zero                 = std::get<2 + 12>(this->_data);
        DataType& transcript_op                     = std::get<2 + 13>(this->_data);
        DataType& transcript_accumulator_x          = std::get<2 + 14>(this->_data);
        DataType& transcript_accumulator_y          = std::get<2 + 15>(this->_data);
        DataType& transcript_msm_x                  = std::get<2 + 16>(this->_data);
        DataType& transcript_msm_y                  = std::get<2 + 17>(this->_data);
        DataType& table_pc                          = std::get<2 + 18>(this->_data);
        DataType& table_point_transition            = std::get<2 + 19>(this->_data);
        DataType& table_round                       = std::get<2 + 20>(this->_data);
        DataType& table_scalar_sum                  = std::get<2 + 21>(this->_data);
        DataType& table_s1                          = std::get<2 + 22>(this->_data);
        DataType& table_s2                          = std::get<2 + 23>(this->_data);
        DataType& table_s3                          = std::get<2 + 24>(this->_data);
        DataType& table_s4                          = std::get<2 + 25>(this->_data);
        DataType& table_s5                          = std::get<2 + 26>(this->_data);
        DataType& table_s6                          = std::get<2 + 27>(this->_data);
        DataType& table_s7                          = std::get<2 + 28>(this->_data);
        DataType& table_s8                          = std::get<2 + 29>(this->_data);
        DataType& table_skew                        = std::get<2 + 30>(this->_data);
        DataType& table_dx                          = std::get<2 + 31>(this->_data);
        DataType& table_dy                          = std::get<2 + 32>(this->_data);
        DataType& table_tx                          = std::get<2 + 33>(this->_data);
        DataType& table_ty                          = std::get<2 + 34>(this->_data);
        DataType& q_msm_transition                  = std::get<2 + 35>(this->_data);
        DataType& msm_q_add                         = std::get<2 + 36>(this->_data);
        DataType& msm_q_double                      = std::get<2 + 37>(this->_data);
        DataType& msm_q_skew                        = std::get<2 + 38>(this->_data);
        DataType& msm_accumulator_x                 = std::get<2 + 39>(this->_data);
        DataType& msm_accumulator_y                 = std::get<2 + 40>(this->_data);
        DataType& msm_pc                            = std::get<2 + 41>(this->_data);
        DataType& msm_size_of_msm                   = std::get<2 + 42>(this->_data);
        DataType& msm_count                         = std::get<2 + 43>(this->_data);
        DataType& msm_round                         = std::get<2 + 44>(this->_data);
        DataType& msm_q_add1                        = std::get<2 + 45>(this->_data);
        DataType& msm_q_add2                        = std::get<2 + 46>(this->_data);
        DataType& msm_q_add3                        = std::get<2 + 47>(this->_data);
        DataType& msm_q_add4                        = std::get<2 + 48>(this->_data);
        DataType& msm_x1                            = std::get<2 + 49>(this->_data);
        DataType& msm_y1                            = std::get<2 + 50>(this->_data);
        DataType& msm_x2                            = std::get<2 + 51>(this->_data);
        DataType& msm_y2                            = std::get<2 + 52>(this->_data);
        DataType& msm_x3                            = std::get<2 + 53>(this->_data);
        DataType& msm_y3                            = std::get<2 + 54>(this->_data);
        DataType& msm_x4                            = std::get<2 + 55>(this->_data);
        DataType& msm_y4                            = std::get<2 + 56>(this->_data);
        DataType& msm_collision_x1                  = std::get<2 + 57>(this->_data);
        DataType& msm_collision_x2                  = std::get<2 + 58>(this->_data);
        DataType& msm_collision_x3                  = std::get<2 + 59>(this->_data);
        DataType& msm_collision_x4                  = std::get<2 + 60>(this->_data);
        DataType& msm_lambda1                       = std::get<2 + 61>(this->_data);
        DataType& msm_lambda2                       = std::get<2 + 62>(this->_data);
        DataType& msm_lambda3                       = std::get<2 + 63>(this->_data);
        DataType& msm_lambda4                       = std::get<2 + 64>(this->_data);
        DataType& msm_slice1                        = std::get<2 + 65>(this->_data);
        DataType& msm_slice2                        = std::get<2 + 66>(this->_data);
        DataType& msm_slice3                        = std::get<2 + 67>(this->_data);
        DataType& msm_slice4                        = std::get<2 + 68>(this->_data);
        DataType& q_transcript_mul_shift            = std::get<2 + 69>(this->_data);
        DataType& q_transcript_accumulate_shift     = std::get<2 + 70>(this->_data);
        DataType& transcript_msm_count_shift        = std::get<2 + 71>(this->_data);
        DataType& transcript_accumulator_x_shift    = std::get<2 + 72>(this->_data);
        DataType& transcript_accumulator_y_shift    = std::get<2 + 73>(this->_data);
        DataType& table_scalar_sum_shift            = std::get<2 + 74>(this->_data);
        DataType& table_dx_shift                    = std::get<2 + 75>(this->_data);
        DataType& table_dy_shift                    = std::get<2 + 76>(this->_data);
        DataType& table_tx_shift                    = std::get<2 + 77>(this->_data);
        DataType& table_ty_shift                    = std::get<2 + 78>(this->_data);
        DataType& q_msm_transition_shift            = std::get<2 + 79>(this->_data);
        DataType& msm_q_add_shift                   = std::get<2 + 80>(this->_data);
        DataType& msm_q_double_shift                = std::get<2 + 81>(this->_data);
        DataType& msm_q_skew_shift                  = std::get<2 + 82>(this->_data);
        DataType& msm_accumulator_x_shift           = std::get<2 + 83>(this->_data);
        DataType& msm_accumulator_y_shift           = std::get<2 + 84>(this->_data);
        DataType& msm_size_of_msm_shift             = std::get<2 + 85>(this->_data);
        DataType& msm_count_shift                   = std::get<2 + 86>(this->_data);
        DataType& msm_round_shift                   = std::get<2 + 87>(this->_data);
        DataType& msm_q_add1_shift                  = std::get<2 + 88>(this->_data);
        DataType& msm_pc_shift                      = std::get<2 + 89>(this->_data);
        DataType& table_pc_shift                    = std::get<2 + 90>(this->_data);
        DataType& transcript_pc_shift               = std::get<2 + 91>(this->_data);
        DataType& table_round_shift                 = std::get<2 + 92>(this->_data);
        DataType& transcript_accumulator_empty      = std::get<2 + 93>(this->_data);
        DataType& transcript_accumulator_empty_shift = std::get<2 + 94>(this->_data);
        DataType& transcript_q_reset_accumulator = std::get<2 + 95>(this->_data);
        DataType& q_wnaf = std::get<2 + 96>(this->_data);
        DataType& q_wnaf_shift = std::get<2 + 97>(this->_data);
        DataType& z_perm = std::get<2 + 98>(this->_data);
        DataType& z_perm_shift = std::get<2 + 99>(this->_data);
        DataType& lookup_read_counts_0 = std::get<2 + 100>(this->_data);
        DataType& lookup_read_counts_1 = std::get<2 + 101>(this->_data);
        DataType& lookup_inverses = std::get<2 + 102>(this->_data);

        template <size_t index>
        const DataType& lookup_read_counts() const
        {
            static_assert(index == 0 || index == 1);
            return std::get<2 + 100 + index>(this->_data);
        }
        // clang-format on

        std::vector<HandleType> get_wires() override
        {
            return {
                q_transcript_add,
                q_transcript_mul,
                q_transcript_eq,
                q_transcript_accumulate,
                q_transcript_msm_transition,
                transcript_pc,
                transcript_msm_count,
                transcript_x,
                transcript_y,
                transcript_z1,
                transcript_z2,
                transcript_z1zero,
                transcript_z2zero,
                transcript_op,
                transcript_accumulator_x,
                transcript_accumulator_y,
                transcript_msm_x,
                transcript_msm_y,
                table_pc,
                table_point_transition,
                table_round,
                table_scalar_sum,
                table_s1,
                table_s2,
                table_s3,
                table_s4,
                table_s5,
                table_s6,
                table_s7,
                table_s8,
                table_skew,
                table_dx,
                table_dy,
                table_tx,
                table_ty,
                q_msm_transition,
                msm_q_add,
                msm_q_double,
                msm_q_skew,
                msm_accumulator_x,
                msm_accumulator_y,
                msm_pc,
                msm_pc_shift,
                msm_size_of_msm,
                msm_count,
                msm_round,
                msm_q_add1,
                msm_q_add2,
                msm_q_add3,
                msm_q_add4,
                msm_x1,
                msm_y1,
                msm_x2,
                msm_y2,
                msm_x3,
                msm_y3,
                msm_x4,
                msm_y4,
                msm_collision_x1,
                msm_collision_x2,
                msm_collision_x3,
                msm_collision_x4,
                msm_lambda1,
                msm_lambda2,
                msm_lambda3,
                msm_lambda4,
                msm_slice1,
                msm_slice2,
                msm_slice3,
                msm_slice4,
                q_transcript_mul_shift,
                q_transcript_accumulate_shift,
                transcript_msm_count_shift,
                transcript_accumulator_x_shift,
                transcript_accumulator_y_shift,
                table_scalar_sum_shift,
                table_dx_shift,
                table_dy_shift,
                table_tx_shift,
                table_ty_shift,
                q_msm_transition_shift,
                msm_q_add_shift,
                msm_q_double_shift,
                msm_q_skew_shift,
                msm_accumulator_x_shift,
                msm_accumulator_y_shift,
                msm_size_of_msm_shift,
                msm_count_shift,
                msm_round_shift,
                msm_q_add1_shift,
                msm_pc_shift,
                table_pc_shift,
                transcript_pc_shift,
                table_round_shift,
                z_perm,
                z_perm_shift,
            };
        };
        // Gemini-specific getters.
        std::vector<HandleType> get_unshifted() override
        {
            return {
                lagrange_first,
                lagrange_last,
                q_transcript_add,
                q_transcript_mul,
                q_transcript_eq,
                q_transcript_accumulate,
                q_transcript_msm_transition,
                transcript_pc,
                transcript_msm_count,
                transcript_x,
                transcript_y,
                transcript_z1,
                transcript_z2,
                transcript_z1zero,
                transcript_z2zero,
                transcript_op,
                transcript_accumulator_x,
                transcript_accumulator_y,
                transcript_msm_x,
                transcript_msm_y,
                table_pc,
                table_point_transition,
                table_round,
                table_scalar_sum,
                table_s1,
                table_s2,
                table_s3,
                table_s4,
                table_s5,
                table_s6,
                table_s7,
                table_s8,
                table_skew,
                table_dx,
                table_dy,
                table_tx,
                table_ty,
                q_msm_transition,
                msm_q_add,
                msm_q_double,
                msm_q_skew,
                msm_accumulator_x,
                msm_accumulator_y,
                msm_pc,
                msm_size_of_msm,
                msm_count,
                msm_round,
                msm_q_add1,
                msm_q_add2,
                msm_q_add3,
                msm_q_add4,
                msm_x1,
                msm_y1,
                msm_x2,
                msm_y2,
                msm_x3,
                msm_y3,
                msm_x4,
                msm_y4,
                msm_collision_x1,
                msm_collision_x2,
                msm_collision_x3,
                msm_collision_x4,
                msm_lambda1,
                msm_lambda2,
                msm_lambda3,
                msm_lambda4,
                msm_slice1,
                msm_slice2,
                msm_slice3,
                msm_slice4,
                z_perm,
            };
        };

        std::vector<HandleType> get_to_be_shifted() override
        {
            return {
                q_transcript_mul,
                q_transcript_accumulate,
                transcript_msm_count,
                transcript_accumulator_x,
                transcript_accumulator_y,
                table_scalar_sum,
                table_dx,
                table_dy,
                table_tx,
                table_ty,
                q_msm_transition,
                msm_q_add,
                msm_q_double,
                msm_q_skew,
                msm_accumulator_x,
                msm_accumulator_y,
                msm_size_of_msm,
                msm_count,
                msm_round,
                msm_q_add1,
                msm_pc,
                table_pc,
                transcript_pc,
                table_round,
            };
        };
        std::vector<HandleType> get_shifted() override
        {
            return {
                q_transcript_mul_shift,
                q_transcript_accumulate_shift,
                transcript_msm_count_shift,
                transcript_accumulator_x_shift,
                transcript_accumulator_y_shift,
                table_scalar_sum_shift,
                table_dx_shift,
                table_dy_shift,
                table_tx_shift,
                table_ty_shift,
                q_msm_transition_shift,
                msm_q_add_shift,
                msm_q_double_shift,
                msm_q_skew_shift,
                msm_accumulator_x_shift,
                msm_accumulator_y_shift,
                msm_size_of_msm_shift,
                msm_count_shift,
                msm_round_shift,
                msm_q_add1_shift,
                msm_pc_shift,
                table_pc_shift,
                transcript_pc_shift,
                table_round_shift,
            };
        };

        AllEntities() = default;

        AllEntities(const AllEntities& other)
            : AllEntities_<DataType, HandleType, NUM_ALL_ENTITIES>(other){};

        AllEntities(AllEntities&& other)
            : AllEntities_<DataType, HandleType, NUM_ALL_ENTITIES>(other){};

        AllEntities& operator=(const AllEntities& other)
        {
            if (this == &other) {
                return *this;
            }
            AllEntities_<DataType, HandleType, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        AllEntities& operator=(AllEntities&& other)
        {
            AllEntities_<DataType, HandleType, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        ~AllEntities() = default;
    };

  public:
    /**
     * @brief The proving key is responsible for storing the polynomials used by the prover.
     * @note TODO(Cody): Maybe multiple inheritance is the right thing here. In that case, nothing should eve inherit
     * from ProvingKey.
     */
    class ProvingKey : public ProvingKey_<PrecomputedEntities<Polynomial, PolynomialHandle>,
                                          WitnessEntities<Polynomial, PolynomialHandle>> {
      public:
        // Expose constructors on the base class
        using Base = ProvingKey_<PrecomputedEntities<Polynomial, PolynomialHandle>,
                                 WitnessEntities<Polynomial, PolynomialHandle>>;
        using Base::Base;

        std::vector<uint32_t> memory_read_records;
        std::vector<uint32_t> memory_write_records;

        // The plookup wires that store plookup read data.
        std::array<PolynomialHandle, 3> get_table_column_wires() { return {}; };
    };

    /**
     * @brief The verification key is responsible for storing the the commitments to the precomputed (non-witnessk)
     * polynomials used by the verifier.
     *
     * @note Note the discrepancy with what sort of data is stored here vs in the proving key. We may want to resolve
     * that, and split out separate PrecomputedPolynomials/Commitments data for clarity but also for portability of our
     * circuits.
     */
    using VerificationKey = VerificationKey_<PrecomputedEntities<Commitment, CommitmentHandle>>;

    /**
     * @brief A container for polynomials handles; only stores spans.
     */
    using ProverPolynomials = AllEntities<PolynomialHandle, PolynomialHandle>;

    /**
     * @brief A container for polynomials produced after the first round of sumcheck.
     * @todo TODO(#394) Use polynomial classes for guaranteed memory alignment.
     */
    using FoldedPolynomials = AllEntities<std::vector<FF>, PolynomialHandle>;

    /**
     * @brief A container for polynomials produced after the first round of sumcheck.
     * @todo TODO(#394) Use polynomial classes for guaranteed memory alignment.
     */
    using RowPolynomials = AllEntities<FF, FF>;

    /**
     * @brief A container for univariates produced during the hot loop in sumcheck.
     * @todo TODO(#390): Simplify this by moving MAX_RELATION_LENGTH?
     */
    template <size_t MAX_RELATION_LENGTH>
    using ExtendedEdges =
        AllEntities<sumcheck::Univariate<FF, MAX_RELATION_LENGTH>, sumcheck::Univariate<FF, MAX_RELATION_LENGTH>>;

    /**
     * @brief A container for the polynomials evaluations produced during sumcheck, which are purported to be the
     * evaluations of polynomials committed in earlier rounds.
     */
    class PurportedEvaluations : public AllEntities<FF, FF> {
      public:
        using Base = AllEntities<FF, FF>;
        using Base::Base;
        PurportedEvaluations(std::array<FF, NUM_ALL_ENTITIES> _data_in) { this->_data = _data_in; }
    };

    /**
     * @brief A container for commitment labels.
     * @note It's debatable whether this should inherit from AllEntities. since most entries are not strictly needed. It
     * has, however, been useful during debugging to have these labels available.
     *
     */
    class CommitmentLabels : public AllEntities<std::string, std::string> {
      public:
        CommitmentLabels()
        {
            q_transcript_add = "_Q_TRANSCRIPT_ADD";
            q_transcript_mul = "_Q_TRANSCRIPT_MUL";
            q_transcript_eq = "_Q_TRANSCRIPT_EQ";
            q_transcript_accumulate = "_Q_TRANSCRIPT_ACCUMULATE";
            q_transcript_msm_transition = "_Q_TRANSCRIPT_MSM_TRANSITION";
            transcript_pc = "_TRANSCRIPT_PC";
            transcript_msm_count = "_TRANSCRIPT_MSM_COUNT";
            transcript_x = "_TRANSCRIPT_X";
            transcript_y = "_TRANSCRIPT_Y";
            transcript_z1 = "_TRANSCRIPT_Z1";
            transcript_z2 = "_TRANSCRIPT_Z2";
            transcript_z1zero = "_TRANSCRIPT_Z1ZERO";
            transcript_z2zero = "_TRANSCRIPT_Z2ZERO";
            transcript_op = "_TRANSCRIPT_OP";
            transcript_accumulator_x = "_TRANSCRIPT_ACCUMULATOR_X";
            transcript_accumulator_y = "_TRANSCRIPT_ACCUMULATOR_Y";
            transcript_msm_x = "_TRANSCRIPT_MSM_X";
            transcript_msm_y = "_TRANSCRIPT_MSM_Y";
            table_pc = "_TABLE_PC";
            table_point_transition = "_TABLE_POINT_TRANSITION";
            table_round = "_TABLE_ROUND";
            table_scalar_sum = "_TABLE_SCALAR_SUM";
            table_s1 = "_TABLE_S1";
            table_s2 = "_TABLE_S2";
            table_s3 = "_TABLE_S3";
            table_s4 = "_TABLE_S4";
            table_s5 = "_TABLE_S5";
            table_s6 = "_TABLE_S6";
            table_s7 = "_TABLE_S7";
            table_s8 = "_TABLE_S8";
            table_skew = "_TABLE_SKEW";
            table_dx = "_TABLE_DX";
            table_dy = "_TABLE_DY";
            table_tx = "_TABLE_TX";
            table_ty = "_TABLE_TY";
            q_msm_transition = "_Q_MSM_TRANSITION";
            msm_q_add = "_MSM_Q_ADD";
            msm_q_double = "_MSM_Q_DOUBLE";
            msm_q_skew = "_MSM_Q_SKEW";
            msm_accumulator_x = "_MSM_ACCUMULATOR_X";
            msm_accumulator_y = "_MSM_ACCUMULATOR_Y";
            msm_pc = "_MSM_PC";
            msm_size_of_msm = "_MSM_SIZE_OF_MSM";
            msm_count = "_MSM_COUNT";
            msm_round = "_MSM_ROUND";
            msm_q_add1 = "_MSM_Q_ADD1";
            msm_q_add2 = "_MSM_Q_ADD2";
            msm_q_add3 = "_MSM_Q_ADD3";
            msm_q_add4 = "_MSM_Q_ADD4";
            msm_x1 = "_MSM_X1";
            msm_y1 = "_MSM_Y1";
            msm_x2 = "_MSM_X2";
            msm_y2 = "_MSM_Y2";
            msm_x3 = "_MSM_X3";
            msm_y3 = "_MSM_Y3";
            msm_x4 = "_MSM_X4";
            msm_y4 = "_MSM_Y4";
            msm_collision_x1 = "_MSM_COLLISION_X1";
            msm_collision_x2 = "_MSM_COLLISION_X2";
            msm_collision_x3 = "_MSM_COLLISION_X3";
            msm_collision_x4 = "_MSM_COLLISION_X4";
            msm_lambda1 = "_MSM_LAMBDA1";
            msm_lambda2 = "_MSM_LAMBDA2";
            msm_lambda3 = "_MSM_LAMBDA3";
            msm_lambda4 = "_MSM_LAMBDA4";
            msm_slice1 = "_MSM_SLICE1";
            msm_slice2 = "_MSM_SLICE2";
            msm_slice3 = "_MSM_SLICE3";
            msm_slice4 = "_MSM_SLICE4";
        };
    };

    class VerifierCommitments : public AllEntities<Commitment, CommitmentHandle> {
      public:
        VerifierCommitments(std::shared_ptr<VerificationKey> verification_key, VerifierTranscript<FF> transcript)
        {
            static_cast<void>(transcript);
            lagrange_first = verification_key->lagrange_first;
            lagrange_last = verification_key->lagrange_last;
        }
    };
};

} // namespace proof_system::honk::flavor
