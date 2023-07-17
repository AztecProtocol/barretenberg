#pragma once

#include "barretenberg/ecc/curves/bn254/bn254.hpp"

namespace proof_system {

/**
 * @brief Raw description of an ECC operation used to produce equivalent descriptions over different curves.
 */
struct ECCOp {
    const bool add = false;
    const bool mul = false;
    const bool eq = false;
    const bool reset = false;
    const barretenberg::g1::affine_element base_point = barretenberg::g1::affine_element{ 0, 0 };
    const uint256_t scalar_1 = 0;
    const uint256_t scalar_2 = 0;
    const barretenberg::fr mul_scalar_full = 0;
};

/**
 * @brief Used to construct execution trace representations of elliptic curve operations.
 *
 * @details Currently the targets in execution traces are: four advice wires in UltraCircuitBuilder and 5 wires in the
 * ECCVM. In each case, the variable values are stored in this class, since the same values will need to be used later
 * by the TranslationVMCircuitBuilder. The circuit builders will store witness indices which are indices in the
 * ultra (resp. eccvm) ops members of this class (rather than in the builder's variables array).
 */
class ECCOpQueue {
    using Point = curve::BN254::AffineElement;
    Point point_at_infinity = curve::BN254::Group::affine_point_at_infinity;
    using Fr = curve::BN254::ScalarField;
    using Fq = curve::BN254::BaseField; // Grumpkin's scalar field

    Point accumulator = point_at_infinity; // current accumulator point

  public:
    std::vector<ECCOp> raw_ops;
    std::vector<std::array<Fr, 4>> ultra_ops;
    std::vector<std::array<Fq, 5>> eccvm_ops;

    uint32_t get_number_of_muls()
    {
        uint32_t num_muls = 0;
        for (auto& op : raw_ops) {
            if (op.mul) {
                if (op.scalar_1 != 0) {
                    num_muls++;
                }
                if (op.scalar_2 != 0) {
                    num_muls++;
                }
            }
        }
        return num_muls;
    }

    Point get_accumulator() { return accumulator; }

    void add_accumulate(const Point& to_add)
    {
        accumulator.self_set_infinity(); // WORKTODO: this shouldnt be here should it?
        accumulator = accumulator + to_add;

        raw_ops.emplace_back(ECCOp{
            .add = true,
            .mul = false,
            .eq = false,
            .reset = false,
            .base_point = to_add,
            .scalar_1 = 0,
            .scalar_2 = 0,
            .mul_scalar_full = 0,
        });
    }

    void mul_accumulate(const Point& to_mul, const Fr& scalar)
    {
        accumulator = accumulator + to_mul * scalar;

        Fr scalar_1 = 0;
        Fr scalar_2 = 0;
        auto converted = scalar.from_montgomery_form();
        Fr::split_into_endomorphism_scalars(converted, scalar_1, scalar_2);
        scalar_1 = scalar_1.to_montgomery_form();
        scalar_2 = scalar_2.to_montgomery_form();
        raw_ops.emplace_back(ECCOp{
            .add = false,
            .mul = true,
            .eq = false,
            .reset = false,
            .base_point = to_mul,
            .scalar_1 = scalar_1,
            .scalar_2 = scalar_2,
            .mul_scalar_full = scalar,
        });
    }
    void eq(const Point& expected)
    {
        accumulator.self_set_infinity(); // WORKTODO: is this always desired?

        raw_ops.emplace_back(ECCOp{
            .add = false,
            .mul = false,
            .eq = true,
            .reset = true,
            .base_point = expected,
            .scalar_1 = 0,
            .scalar_2 = 0,
            .mul_scalar_full = 0,
        });
    }

    void empty_row()
    {
        raw_ops.emplace_back(ECCOp{
            .add = false,
            .mul = false,
            .eq = false,
            .reset = false,
            .base_point = point_at_infinity,
            .scalar_1 = 0,
            .scalar_2 = 0,
            .mul_scalar_full = 0,
        });
    }
};

} // namespace proof_system
