#include "acir_format.hpp"
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include <gtest/gtest.h>
#include <vector>
#include "barretenberg/common/streams.hpp"
#include "ecdsa_secp256k1.hpp"

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
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .compute_merkle_root_constraints = {},
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

    std::vector<uint32_t> signature(64);
    for (uint32_t i = 0, value = 13; i < 64; i++, value++) {
        signature[i] = value;
        range_constraints.push_back(acir_format::RangeConstraint{
            .witness = value,
            .num_bits = 15,
        });
    }

    acir_format::SchnorrConstraint schnorr_constraint{
        .message = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },
        .public_key_x = 11,
        .public_key_y = 12,
        .result = 77,
        .signature = signature,
    };

    acir_format::acir_format constraint_system{
        .varnum = 82,
        .public_inputs = {},
        .fixed_base_scalar_mul_constraints = {},
        .logic_constraints = {},
        .range_constraints = range_constraints,
        .schnorr_constraints = { schnorr_constraint },
        .ecdsa_constraints = {},
        .sha256_constraints = {},
        .blake2s_constraints = {},
        .keccak_constraints = {},
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .compute_merkle_root_constraints = {},
        .constraints = { poly_triple{
            .a = schnorr_constraint.result,
            .b = schnorr_constraint.result,
            .c = schnorr_constraint.result,
            .q_m = 0,
            .q_l = 0,
            .q_r = 0,
            .q_o = 1,
            .q_c = fr::neg_one(),
        } },
    };
    uint256_t pub_x = uint256_t("17cbd3ed3151ccfd170efe1d54280a6a4822640bf5c369908ad74ea21518a9c5");
    uint256_t pub_y = uint256_t("0e0456e3795c1a31f20035b741cd6158929eeccd320d299cfcac962865a6bc74");

    auto composer = acir_format::create_circuit_with_witness(
        constraint_system,
        { 0,  1,   2,   3,   4,   5,   6,   7,   8,   9,   pub_x, pub_y, 5,   202, 31, 146, 81,  242, 246, 69,
          43, 107, 249, 153, 198, 44,  14,  111, 191, 121, 137,   166,   160, 103, 18, 181, 243, 233, 226, 95,
          67, 16,  37,  128, 85,  76,  19,  253, 30,  77,  192,   53,    138, 205, 69, 33,  236, 163, 83,  194,
          84, 137, 184, 221, 176, 121, 179, 27,  63,  70,  54,    16,    176, 250, 39, 239, 1,   0,   0,   0 });

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

    std::vector<uint32_t> signature(64);
    for (uint32_t i = 0, value = 13; i < 64; i++, value++) {
        signature[i] = value;
        range_constraints.push_back(acir_format::RangeConstraint{
            .witness = value,
            .num_bits = 8,
        });
    }

    acir_format::SchnorrConstraint schnorr_constraint{
        .message = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },
        .public_key_x = 11,
        .public_key_y = 12,
        .result = 77,
        .signature = signature,
    };

    acir_format::acir_format constraint_system{
        .varnum = 82,
        .public_inputs = {},
        .fixed_base_scalar_mul_constraints = {},
        .logic_constraints = {},
        .range_constraints = range_constraints,
        .schnorr_constraints = { schnorr_constraint },
        .ecdsa_constraints = {},
        .sha256_constraints = {},
        .blake2s_constraints = {},
        .keccak_constraints = {},
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .compute_merkle_root_constraints = {},
        .constraints = { poly_triple{
            .a = schnorr_constraint.result,
            .b = schnorr_constraint.result,
            .c = schnorr_constraint.result,
            .q_m = 0,
            .q_l = 0,
            .q_r = 0,
            .q_o = 1,
            .q_c = fr::neg_one(),
        } },
    };
    uint256_t pub_x = uint256_t("17cbd3ed3151ccfd170efe1d54280a6a4822640bf5c369908ad74ea21518a9c5");
    uint256_t pub_y = uint256_t("0e0456e3795c1a31f20035b741cd6158929eeccd320d299cfcac962865a6bc74");

    auto composer = acir_format::create_circuit_with_witness(
        constraint_system,
        { 0,  1,   2,   3,   4,   5,   6,   7,   8,   9,   pub_x, pub_y, 5,   202, 31, 146, 81,  242, 246, 69,
          43, 107, 249, 153, 198, 44,  14,  111, 191, 121, 137,   166,   160, 103, 18, 181, 243, 233, 226, 95,
          67, 16,  37,  128, 85,  76,  19,  253, 30,  77,  192,   53,    138, 205, 69, 33,  236, 163, 83,  194,
          84, 137, 184, 221, 176, 121, 179, 27,  63,  70,  54,    16,    176, 250, 39, 239, 1,   0,   0,   0 });

    auto prover = composer.create_ultra_with_keccak_prover();
    auto proof = prover.construct_proof();

    auto verifier = composer.create_ultra_with_keccak_verifier();

    EXPECT_EQ(verifier.verify_proof(proof), true);
}

TEST(acir_format, test_ecdsa_verify)
{

    // Offset by 1 since we cannot use the zero index
    uint32_t offset = 1;

    std::vector<uint32_t> hashed_message(32);
    std::vector<acir_format::RangeConstraint> range_constraints;
    for (uint32_t i = 0; i < 32; i++) {
        hashed_message[i] = i + offset;
        range_constraints.push_back(acir_format::RangeConstraint{
            .witness = i + offset,
            .num_bits = 8,
        });
    }

    // Offset by 32 since we just used 32 witness indices to create the hashed message
    offset += 32;

    std::vector<uint32_t> pub_key_x(32);
    for (uint32_t i = 0; i < 32; i++) {
        pub_key_x[i] = i + offset;
    }

    // Offset by 32 since we just used 32 witness indices to create the x coordinate for public key
    offset += 32;

    std::vector<uint32_t> pub_key_y(32);
    for (uint32_t i = 0; i < 32; i++) {
        pub_key_y[i] = i + offset;
    }

    // Offset by 32 since we just used 32 witness indices to create the y coordinate for public key
    offset += 32;

    std::vector<uint32_t> signature(64);
    for (uint32_t i = 0; i < 64; i++) {
        signature[i] = i + offset;
        range_constraints.push_back(acir_format::RangeConstraint{
            .witness = i + offset,
            .num_bits = 8,
        });
    }

    // Offset by 64 since we just used 64 witness indices to create the signature
    offset += 64;

    acir_format::EcdsaSecp256k1Constraint ecdsa_constraint{
        .message = hashed_message,
        .pub_x_indices = pub_key_x,
        .pub_y_indices = pub_key_y,
        .result = offset + 1,
        .signature = signature,
    };

    acir_format::acir_format constraint_system{
        .varnum = offset + 10,
        .public_inputs = {},
        .fixed_base_scalar_mul_constraints = {},
        .logic_constraints = {},
        .range_constraints = range_constraints,
        .schnorr_constraints = {},
        .ecdsa_constraints = { ecdsa_constraint },
        .sha256_constraints = {},
        .blake2s_constraints = {},
        .keccak_constraints = {},
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .compute_merkle_root_constraints = {},
        .constraints = { poly_triple{
            .a = ecdsa_constraint.result,
            .b = ecdsa_constraint.result,
            .c = ecdsa_constraint.result,
            .q_m = 0,
            .q_l = 0,
            .q_r = 0,
            .q_o = 1,
            .q_c = fr::neg_one(),
        } },
    };

    auto composer = acir_format::create_circuit_with_witness(constraint_system,
                                                             {
                                                                 // hashed message
                                                                 84,
                                                                 112,
                                                                 91,
                                                                 163,
                                                                 186,
                                                                 175,
                                                                 219,
                                                                 223,
                                                                 186,
                                                                 140,
                                                                 95,
                                                                 154,
                                                                 112,
                                                                 247,
                                                                 168,
                                                                 155,
                                                                 238,
                                                                 152,
                                                                 217,
                                                                 6,
                                                                 181,
                                                                 62,
                                                                 49,
                                                                 7,
                                                                 77,
                                                                 167,
                                                                 186,
                                                                 236,
                                                                 220,
                                                                 13,
                                                                 169,
                                                                 173,
                                                                 // public key for x
                                                                 77,
                                                                 75,
                                                                 108,
                                                                 209,
                                                                 54,
                                                                 16,
                                                                 50,
                                                                 202,
                                                                 155,
                                                                 210,
                                                                 174,
                                                                 185,
                                                                 217,
                                                                 0,
                                                                 170,
                                                                 77,
                                                                 69,
                                                                 217,
                                                                 234,
                                                                 216,
                                                                 10,
                                                                 201,
                                                                 66,
                                                                 51,
                                                                 116,
                                                                 196,
                                                                 81,
                                                                 167,
                                                                 37,
                                                                 77,
                                                                 7,
                                                                 102,
                                                                 // public key y
                                                                 42,
                                                                 62,
                                                                 173,
                                                                 162,
                                                                 208,
                                                                 254,
                                                                 32,
                                                                 139,
                                                                 109,
                                                                 37,
                                                                 124,
                                                                 235,
                                                                 15,
                                                                 6,
                                                                 66,
                                                                 132,
                                                                 102,
                                                                 46,
                                                                 133,
                                                                 127,
                                                                 87,
                                                                 182,
                                                                 107,
                                                                 84,
                                                                 193,
                                                                 152,
                                                                 189,
                                                                 49,
                                                                 13,
                                                                 237,
                                                                 54,
                                                                 208,
                                                                 // Verification result
                                                                 1,
                                                                 // signature
                                                                 100,
                                                                 235,
                                                                 162,
                                                                 94,
                                                                 185,
                                                                 111,
                                                                 28,
                                                                 107,
                                                                 66,
                                                                 168,
                                                                 87,
                                                                 57,
                                                                 125,
                                                                 222,
                                                                 85,
                                                                 114,
                                                                 140,
                                                                 61,
                                                                 161,
                                                                 105,
                                                                 140,
                                                                 229,
                                                                 209,
                                                                 158,
                                                                 178,
                                                                 38,
                                                                 160,
                                                                 164,
                                                                 142,
                                                                 171,
                                                                 167,
                                                                 202,
                                                                 46,
                                                                 231,
                                                                 34,
                                                                 171,
                                                                 10,
                                                                 190,
                                                                 86,
                                                                 37,
                                                                 238,
                                                                 245,
                                                                 49,
                                                                 189,
                                                                 207,
                                                                 208,
                                                                 54,
                                                                 107,
                                                                 61,
                                                                 247,
                                                                 167,
                                                                 163,
                                                                 157,
                                                                 86,
                                                                 108,
                                                                 223,
                                                                 78,
                                                                 8,
                                                                 252,
                                                                 169,
                                                                 61,
                                                                 158,
                                                                 6,
                                                                 42,

                                                             });
    auto prover = composer.create_ultra_with_keccak_prover();
    auto proof = prover.construct_proof();

    auto verifier = composer.create_ultra_with_keccak_verifier();

    EXPECT_EQ(verifier.verify_proof(proof), true);
}
