#include "acir_format.hpp"
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include <gtest/gtest.h>
#include <vector>
#include "barretenberg/common/streams.hpp"

TEST(acir_format, test_logic_gates)
{
    std::cout << "in the logic gate test" << std::endl;

    acir_format::LogicConstraint logic_constraint{
        .a = 8,
        .b = 9,
        .result = 7,
        .num_bits = 32,
        .is_xor_gate = 1,
    };
    std::cout << "made struct" << std::endl;

    acir_format::acir_format constraint_system{
        .varnum = 12,
        .public_inputs = {},
        .fixed_base_scalar_mul_constraints = {},
        .logic_constraints = { logic_constraint },
        .range_constraints = {},
        .schnorr_constraints = {},
        .ecdsa_constraints = {},
        .sha256_constraints = {},
        .blake2s_constraints = {},
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .merkle_membership_constraints = {},
        .constraints = {},
    };
    std::cout << "about to make some vals" << std::endl;

    uint256_t x = 5000000;
    std::cout << "got 5000000 " << std::endl;

    // uint256_t y = uint256_t("a0000");
    uint256_t y = 0xa0000;

    uint256_t q = uint256_t("8AA49BCCC58DE750C8171A8595F2F4E71DADC04E079BC498EF70C5D0E21B36C");

    uint256_t chunk_one = 0xFAF5FFFF;
    uint256_t chunk_two = 0xFAFFFFFF;
    uint256_t chunk_three = 0xFFF5FFFF;
    uint256_t chunk_four = 0x509FFF6;
    std::cout << "got some vals" << std::endl;

    auto composer = acir_format::create_circuit_with_witness(
        constraint_system, { 5, 10, x, 0, y, 0, chunk_one, chunk_two, chunk_three, chunk_four, q, 1 });
    std::cout << "made composer" << std::endl;

    auto prover = composer.create_prover();
    auto proof = prover.construct_proof();

    auto verifier = composer.create_verifier();

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
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .merkle_membership_constraints = {},
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

    auto prover = composer.create_prover();
    auto proof = prover.construct_proof();

    auto verifier = composer.create_verifier();

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
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .merkle_membership_constraints = {},
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

    auto prover = composer.create_prover();
    auto proof = prover.construct_proof();

    auto verifier = composer.create_verifier();

    EXPECT_EQ(verifier.verify_proof(proof), true);
}