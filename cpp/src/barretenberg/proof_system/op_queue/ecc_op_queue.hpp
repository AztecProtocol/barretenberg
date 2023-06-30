#pragma once

#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"

namespace proof_system {

struct ECCOp {
    bool add = false;
    bool mul = false;
    bool eq = false;
    bool reset = false;
    grumpkin::g1::affine_element base_point = grumpkin::g1::affine_element{ 0, 0 };
    uint256_t scalar_1 = 0;
    uint256_t scalar_2 = 0;
    grumpkin::fr mul_scalar_full = 0;
};

class ECCOpQueue {
  public:
    std::vector<ECCOp> _data;

    uint32_t get_number_of_muls()
    {
        uint32_t num_muls = 0;
        for (auto& op : _data) {
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

    void add_accumulate(const grumpkin::g1::affine_element& to_add)
    {
        _data.emplace_back(ECCOp{
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

    void mul_accumulate(const grumpkin::g1::affine_element& to_mul, const grumpkin::fr& scalar)
    {
        grumpkin::fr scalar_1 = 0;
        grumpkin::fr scalar_2 = 0;
        auto converted = scalar.from_montgomery_form();
        grumpkin::fr::split_into_endomorphism_scalars(converted, scalar_1, scalar_2);
        scalar_1 = scalar_1.to_montgomery_form();
        scalar_2 = scalar_2.to_montgomery_form();
        _data.emplace_back(ECCOp{
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
    void eq(const grumpkin::g1::affine_element& expected)
    {
        _data.emplace_back(ECCOp{
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
        _data.emplace_back(ECCOp{
            .add = false,
            .mul = false,
            .eq = false,
            .reset = false,
            .base_point = grumpkin::g1::affine_point_at_infinity,
            .scalar_1 = 0,
            .scalar_2 = 0,
            .mul_scalar_full = 0,
        });
    }
};
} // namespace proof_system