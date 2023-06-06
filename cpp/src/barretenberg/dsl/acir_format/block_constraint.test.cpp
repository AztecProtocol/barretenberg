#include "acir_format.hpp"
#include "block_constraint.hpp"
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/plonk/proof_system/verification_key/verification_key.hpp"

#include <gtest/gtest.h>
#include <vector>

size_t generate_block_constraint(acir_format::BlockConstraint& constraint, std::vector<fr>& witness_values)
{
    size_t witness_len = 1;
    witness_values.emplace_back(1);
    witness_len++;

    fr two = fr::one() + fr::one();
    poly_triple a0 = poly_triple{
        .a = 1,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = two,
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };
    fr three = fr::one() + two;
    poly_triple a1 = poly_triple{
        .a = 0,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = 0,
        .q_r = 0,
        .q_o = 0,
        .q_c = three,
    };
    poly_triple r1 = poly_triple{
        .a = 1,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = fr::neg_one(),
    };
    poly_triple r2 = poly_triple{
        .a = 1,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = two,
        .q_r = 0,
        .q_o = 0,
        .q_c = fr::neg_one(),
    };
    poly_triple y = poly_triple{
        .a = 2,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };
    witness_values.emplace_back(2);
    witness_len++;
    poly_triple z = poly_triple{
        .a = 3,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };
    witness_values.emplace_back(3);
    witness_len++;
    acir_format::MemOp op1 = acir_format::MemOp{
        .access_type = 0,
        .index = r1,
        .value = y,
    };
    acir_format::MemOp op2 = acir_format::MemOp{
        .access_type = 0,
        .index = r2,
        .value = z,
    };
    constraint = acir_format::BlockConstraint{
        .init = { a0, a1 },
        .trace = { op1, op2 },
        .type = acir_format::BlockType::ROM,
    };

    return witness_len;
}

TEST(up_ram, TestBlockConstraint)
{

    acir_format::BlockConstraint block;
    std::vector<fr> witness_values;
    size_t num_variables = generate_block_constraint(block, witness_values);
    acir_format::acir_format constraint_system{
        .varnum = static_cast<uint32_t>(num_variables),
        .public_inputs = {},
        .fixed_base_scalar_mul_constraints = {},
        .logic_constraints = {},
        .range_constraints = {},
        .schnorr_constraints = {},
        .ecdsa_constraints = {},
        .sha256_constraints = {},
        .blake2s_constraints = {},
        .keccak_constraints = {},
        .keccak_var_constraints = {},
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .block_constraints = { block },
        .constraints = {},
    };

    auto composer = acir_format::create_circuit_with_witness(constraint_system, witness_values);

    auto prover = composer.create_prover();

    auto proof = prover.construct_proof();
    auto composer2 = acir_format::Composer();
    acir_format::create_circuit(composer2, constraint_system);
    auto verifier = composer2.create_verifier();
    EXPECT_EQ(verifier.verify_proof(proof), true);
}

TEST(up_ram, TestUPRAM)
{
    barretenberg::fr m_3 = fr::neg_one() + fr::neg_one() + fr::neg_one();
    info(m_3);
    acir_format::BlockConstraint block;
    // a =[x1;2x1;3x1;x1+3;x1+4] => x1
    // z witness => 3
    // a[z]           =>y
    // a[z-3];         x3;

    size_t witness_len = 1;
    std::vector<fr> witness_values;
    // x1 = 1
    // x2 =a[z]
    // x3 = a[z-3]
    // x4 = z

    witness_values.push_back(fr::one());
    witness_len++;

    fr two = fr::one() + fr::one();
    // ao = x1 = 1
    poly_triple a0 = poly_triple{
        .a = 1,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };
    fr three = fr::one() + two;
    // a1 = 2x1 = 2
    poly_triple a1 = poly_triple{
        .a = 1,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = two,
        .q_r = 0,
        .q_o = 0,
        .q_c = three,
    };
    // a2 = 3x1
    poly_triple a2 = poly_triple{
        .a = 1,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = three,
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };
    // a3 = x1+3
    poly_triple a3 = poly_triple{
        .a = 1,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = 1,
        .q_r = 0,
        .q_o = 0,
        .q_c = three,
    };
    // a4 = x1+4
    poly_triple a4 = poly_triple{
        .a = 1,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = 1,
        .q_r = 0,
        .q_o = 0,
        .q_c = three + fr::one(),
    };
    // y= x2
    poly_triple y = poly_triple{
        .a = 2,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };
    // y = 4
    witness_values.push_back(three + fr::one());
    witness_len++;
    // z = 3 = x3
    // y = 4
    witness_values.push_back(three);
    witness_len++;
    poly_triple z = poly_triple{
        .a = 3,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };
    poly_triple z2 = poly_triple{
        .a = 3,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = m_3,
    };
    // y2 = x4 = a[z-3]=a[0]=x1=1
    poly_triple y2 = poly_triple{
        .a = 4,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };
    witness_values.push_back(fr::one());
    witness_len++;

    acir_format::MemOp op1 = acir_format::MemOp{
        .access_type = 1,
        .index = z,
        .value =
            poly_triple{
                .a = 0,
                .b = 0,
                .c = 0,
                .q_m = 0,
                .q_l = 0,
                .q_r = 0,
                .q_o = 0,
                .q_c = 0,
            },
    };

    acir_format::MemOp op2 = acir_format::MemOp{
        .access_type = 0,
        .index = z2,
        .value = y2,
    };
    if (op1.access_type == 5) {
        op2.access_type = 0;
        a2.q_o = 0;
    }
    block = acir_format::BlockConstraint{
        .init = { a0, a1, a2, a3, a4 },
        .trace = { op1, op2 },
        .type = acir_format::BlockType::RAM,
    };

    // return witness_len;

    size_t num_variables = witness_len; // generate_block_constraint(block, witness_values);
    acir_format::acir_format constraint_system{
        .varnum = static_cast<uint32_t>(num_variables),
        .public_inputs = {},
        .fixed_base_scalar_mul_constraints = {},
        .logic_constraints = {},
        .range_constraints = {},
        .schnorr_constraints = {},
        .ecdsa_constraints = {},
        .sha256_constraints = {},
        .blake2s_constraints = {},
        .keccak_constraints = {},
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .block_constraints = { block },
        .constraints = {},
    };

    auto composer = acir_format::create_circuit_with_witness(constraint_system, witness_values);

    auto prover = composer.create_prover();

    auto proof = prover.construct_proof();

    auto vkey = composer.compute_verification_key();
    auto composer2 = acir_format::Composer(nullptr, vkey);
    // auto composer2 = acir_format::Composer();
    acir_format::create_circuit(composer2, constraint_system);
    auto verifier = composer2.create_verifier();
    // auto verifier = composer.create_verifier();
    EXPECT_EQ(verifier.verify_proof(proof), true);
}
