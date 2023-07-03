#include "acir_format.hpp"
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include <gtest/gtest.h>
#include <vector>
#include "barretenberg/common/streams.hpp"
#include "barretenberg/serialize/test_helper.hpp"
#include "ecdsa_secp256k1.hpp"
TEST(acir_format, msgpack_logic_constraint)
{
    auto [actual, expected] = msgpack_roundtrip(acir_format::LogicConstraint{});
    EXPECT_EQ(actual, expected);
}
TEST(acir_format, test_logic_gate_from_noir_circuit)
{
    /**
     * constraints produced by Noir program:
     * fn main(x : u32, y : pub u32) {
     * let z = x ^ y;
     *
     * constrain z != 10;
     * }
     **/
    acir_format::RangeConstraint range_a{
        .witness = 1,
        .num_bits = 32,
    };
    acir_format::RangeConstraint range_b{
        .witness = 2,
        .num_bits = 32,
    };

    acir_format::LogicConstraint logic_constraint{
        .a = 1,
        .b = 2,
        .result = 3,
        .num_bits = 32,
        .is_xor_gate = 1,
    };
    poly_triple expr_a{
        .a = 3,
        .b = 4,
        .c = 0,
        .q_m = 0,
        .q_l = 1,
        .q_r = -1,
        .q_o = 0,
        .q_c = -10,
    };
    poly_triple expr_b{
        .a = 4,
        .b = 5,
        .c = 6,
        .q_m = 1,
        .q_l = 0,
        .q_r = 0,
        .q_o = -1,
        .q_c = 0,
    };
    poly_triple expr_c{
        .a = 4,
        .b = 6,
        .c = 4,
        .q_m = 1,
        .q_l = 0,
        .q_r = 0,
        .q_o = -1,
        .q_c = 0,

    };
    poly_triple expr_d{
        .a = 6,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = -1,
        .q_r = 0,
        .q_o = 0,
        .q_c = 1,
    };
    // EXPR [ (1, _4, _5) (-1, _6) 0 ]
    // EXPR [ (1, _4, _6) (-1, _4) 0 ]
    // EXPR [ (-1, _6) 1 ]
    std::cout << "made struct" << std::endl;

    acir_format::acir_format constraint_system{
        .varnum = 7,
        .public_inputs = { 2 },
        .fixed_base_scalar_mul_constraints = {},
        .logic_constraints = { logic_constraint },
        .range_constraints = { range_a, range_b },
        .schnorr_constraints = {},
        .ecdsa_constraints = {},
        .sha256_constraints = {},
        .blake2s_constraints = {},
        .keccak_constraints = {},
        .keccak_var_constraints = {},
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .block_constraints = {},
        .recursion_constraints = {},
        .constraints = { expr_a, expr_b, expr_c, expr_d },
    };

    uint256_t inverse_of_five = fr(5).invert();
    auto composer = acir_format::create_circuit_with_witness(constraint_system,
                                                             {
                                                                 5,
                                                                 10,
                                                                 15,
                                                                 5,
                                                                 inverse_of_five,
                                                                 1,
                                                             });

    std::cout << "made composer" << std::endl;

    auto prover = composer.create_ultra_with_keccak_prover();
    auto proof = prover.construct_proof();

    auto verifier = composer.create_ultra_with_keccak_verifier();

    EXPECT_EQ(verifier.verify_proof(proof), true);
}

TEST(acir_format, test_schnorr_verify_pass)
{
    std::vector<acir_format::RangeConstraint> range_constraints;
    for (uint32_t i = 0; i < 10; i++) {
        range_constraints.push_back(acir_format::RangeConstraint{
            .witness = i + 1,
            .num_bits = 15,
        });
    }

    acir_format::SchnorrConstraint schnorr_constraint{
        .message = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },
        .public_key_x = 11,
        .public_key_y = 12,
        .signature_s = 13,
        .signature_e = 14,
        .result = 15,
    };

    poly_triple result_constraint{
        .a = schnorr_constraint.result,
        .b = schnorr_constraint.result,
        .c = schnorr_constraint.result,
        .q_m = 0,
        .q_l = 0,
        .q_r = 0,
        .q_o = 1,
        .q_c = fr::neg_one(),
    }

    acir_format::acir_format constraint_system{
        .varnum = 16,
        .public_inputs = {},
        .fixed_base_scalar_mul_constraints = {},
        .logic_constraints = {},
        .range_constraints = range_constraints,
        .schnorr_constraints = { schnorr_constraint },
        .ecdsa_constraints = {},
        .sha256_constraints = {},
        .blake2s_constraints = {},
        .keccak_constraints = {},
        .keccak_var_constraints = {},
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .block_constraints = {},
        .recursion_constraints = {},
        .constraints = { result_constraint },
    };
    uint256_t pub_x = uint256_t("17cbd3ed3151ccfd170efe1d54280a6a4822640bf5c369908ad74ea21518a9c5");
    uint256_t pub_y = uint256_t("0e0456e3795c1a31f20035b741cd6158929eeccd320d299cfcac962865a6bc74");

    uint256_t sig_s = uint256_t("05ca1f9251f2f6452b6bf999c62c0e6fbf7989a6a06712b5f3e9e25f43102580");
    uint256_t sig_e = uint256_t("554c13fd1e4dc0358acd4521eca353c25489b8ddb079b31b3f463610b0fa27ef");

    auto composer = acir_format::create_circuit_with_witness(
        constraint_system, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, pub_x, pub_y, sig_s, sig_e, 1 });

    auto prover = composer.create_ultra_with_keccak_prover();
    auto proof = prover.construct_proof();

    auto verifier = composer.create_ultra_with_keccak_verifier();

    EXPECT_EQ(verifier.verify_proof(proof), true);
}

TEST(acir_format, test_schnorr_verify_small_range)
{
    std::vector<acir_format::RangeConstraint> range_constraints;
    for (uint32_t i = 0; i < 10; i++) {
        range_constraints.push_back(acir_format::RangeConstraint{
            .witness = i + 1,
            .num_bits = 8,
        });
    }

    acir_format::SchnorrConstraint schnorr_constraint{
        .message = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },
        .public_key_x = 11,
        .public_key_y = 12,
        .signature_s = 13,
        .signature_e = 14,
        .result = 15,
    };

    poly_triple result_constraint{
        .a = schnorr_constraint.result,
        .b = schnorr_constraint.result,
        .c = schnorr_constraint.result,
        .q_m = 0,
        .q_l = 0,
        .q_r = 0,
        .q_o = 1,
        .q_c = fr::neg_one(),
    }

    acir_format::acir_format constraint_system{
        .varnum = 16,
        .public_inputs = {},
        .fixed_base_scalar_mul_constraints = {},
        .logic_constraints = {},
        .range_constraints = range_constraints,
        .schnorr_constraints = { schnorr_constraint },
        .ecdsa_constraints = {},
        .sha256_constraints = {},
        .blake2s_constraints = {},
        .keccak_constraints = {},
        .keccak_var_constraints = {},
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .block_constraints = {},
        .recursion_constraints = {},
        .constraints = { result_constraint },
    };
    uint256_t pub_x = uint256_t("17cbd3ed3151ccfd170efe1d54280a6a4822640bf5c369908ad74ea21518a9c5");
    uint256_t pub_y = uint256_t("0e0456e3795c1a31f20035b741cd6158929eeccd320d299cfcac962865a6bc74");

    uint256_t sig_s = uint256_t("05ca1f9251f2f6452b6bf999c62c0e6fbf7989a6a06712b5f3e9e25f43102580");
    uint256_t sig_e = uint256_t("554c13fd1e4dc0358acd4521eca353c25489b8ddb079b31b3f463610b0fa27ef");

    auto composer = acir_format::create_circuit_with_witness(
        constraint_system, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, pub_x, pub_y, sig_s, sig_e, 1 });

    auto prover = composer.create_ultra_with_keccak_prover();
    auto proof = prover.construct_proof();

    auto verifier = composer.create_ultra_with_keccak_verifier();

    EXPECT_EQ(verifier.verify_proof(proof), true);
}
