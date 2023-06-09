#include "acir_proofs.hpp"
#include "../acir_format/acir_format.hpp"
#include "barretenberg/srs/io.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace proof_system::plonk;

void create_logic_circuit(acir_format::acir_format& constraint_system, std::vector<fr>& witness)
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

    constraint_system = acir_format::acir_format{
        .varnum = 7,
        .public_inputs = { 2, 3 },
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
        .block_constraints = {},
        .constraints = { expr_a, expr_b, expr_c, expr_d },
    };

    uint256_t inverse_of_five = fr(5).invert();

    witness.emplace_back(5);
    witness.emplace_back(10);
    witness.emplace_back(15);
    witness.emplace_back(5);
    witness.emplace_back(inverse_of_five);
    witness.emplace_back(1);
}

// TEST(AcirProofs, TestSerialization)
// {
//     acir_format::acir_format constraint_system;
//     std::vector<fr> witness;
//     create_logic_circuit(constraint_system, witness);

//     std::vector<uint8_t> witness_buf;           // = to_buffer(witness);
//     std::vector<uint8_t> constraint_system_buf; //
//     write(constraint_system_buf, constraint_system);
//     write(witness_buf, witness);

//     uint32_t total_circuit_size = acir_proofs::get_total_circuit_size(&constraint_system_buf[0]);
//     info("total_circuit_size: ", total_circuit_size);
//     uint32_t pow2_size = 1 << (numeric::get_msb(total_circuit_size) + 1);
//     auto env_crs = std::make_unique<proof_system::EnvReferenceStringFactory>();
//     auto verifier_crs = env_crs->get_verifier_crs();

//     uint8_t const* pk_buf = nullptr;
//     acir_proofs::init_proving_key(&constraint_system_buf[0], &pk_buf);
//     barretenberg::g2::affine_element g2x = verifier_crs->get_g2x();

//     auto* pippenger_ptr_base = new scalar_multiplication::Pippenger(env_load_prover_crs(pow2_size + 1), pow2_size +
//     1); void* pippenger_ptr = reinterpret_cast<void*>(pippenger_ptr_base);

//     std::vector<uint8_t> g2x_buffer(128);
//     io::write_g2_elements_to_buffer(&g2x, (char*)(&g2x_buffer[0]), 1);

//     uint8_t const* vk_buf = nullptr;
//     acir_proofs::init_verification_key(pippenger_ptr, &g2x_buffer[0], pk_buf, &vk_buf);

//     uint8_t* proof_data_buf = nullptr;
//     size_t proof_length = acir_proofs::new_proof(
//         pippenger_ptr, &g2x_buffer[0], pk_buf, &constraint_system_buf[0], &witness_buf[0], &proof_data_buf);

//     bool verified = acir_proofs::verify_proof(
//         &g2x_buffer[0], vk_buf, &constraint_system_buf[0], proof_data_buf, static_cast<uint32_t>(proof_length));
//     EXPECT_EQ(verified, true);
// }

void generate_ram_constraint_system(acir_format::acir_format& constraint_system, std::vector<fr>& witness_values)
{
    /**
     * constraints produced by Noir program:
     * fn main(mut x: [u32; 5], idx: Field) {
     *     x[idx - 3] = 0;
     * }
     **/
    barretenberg::fr m_3 = fr::neg_one() + fr::neg_one() + fr::neg_one();
    info(m_3);

    acir_format::BlockConstraint block;

    fr two = fr::one() + fr::one();

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

    poly_triple a1 = poly_triple{
        .a = 2,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };

    poly_triple a2 = poly_triple{
        .a = 3,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };
    fr four = fr::one() + three;

    poly_triple a3 = poly_triple{
        .a = 4,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };

    poly_triple a4 = poly_triple{
        .a = 5,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };

    poly_triple z = poly_triple{
        .a = 6,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0, // m_3,
    };

    poly_triple y = poly_triple{
        .a = 7,
        .b = 0,
        .c = 0,
        .q_m = 0,
        .q_l = fr::one(),
        .q_r = 0,
        .q_o = 0,
        .q_c = 0,
    };

    acir_format::MemOp op1 = acir_format::MemOp{
        .access_type = 0,
        .index = z,
        .value = y,
    };

    block = acir_format::BlockConstraint{
        .init = { a0, a1, a2, a3, a4 },
        .trace = { op1 },
        // Change to ROM to see test pass
        .type = acir_format::BlockType::RAM,
    };

    witness_values.emplace_back(fr(104));
    witness_values.emplace_back(fr(101));
    witness_values.emplace_back(fr(108));
    witness_values.emplace_back(fr(108));
    witness_values.emplace_back(fr(111));
    witness_values.emplace_back(fr(4));
    witness_values.emplace_back(fr(111));

    constraint_system = acir_format::acir_format{
        .varnum = static_cast<uint32_t>(8),
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
}

TEST(AcirProofs, TestRAMSerialization)
{
    acir_format::acir_format constraint_system;
    std::vector<fr> witness;
    generate_ram_constraint_system(constraint_system, witness);

    std::vector<uint8_t> witness_buf;           // = to_buffer(witness);
    std::vector<uint8_t> constraint_system_buf; //
    write(constraint_system_buf, constraint_system);
    write(witness_buf, witness);

    uint32_t total_circuit_size = acir_proofs::get_total_circuit_size(&constraint_system_buf[0]);
    info("total_circuit_size: ", total_circuit_size);
    uint32_t pow2_size = 1 << (numeric::get_msb(total_circuit_size) + 1);
    auto env_crs = std::make_unique<proof_system::EnvReferenceStringFactory>();
    auto verifier_crs = env_crs->get_verifier_crs();

    uint8_t const* pk_buf = nullptr;
    acir_proofs::init_proving_key(&constraint_system_buf[0], &pk_buf);
    barretenberg::g2::affine_element g2x = verifier_crs->get_g2x();

    auto* pippenger_ptr_base = new scalar_multiplication::Pippenger(env_load_prover_crs(pow2_size + 1), pow2_size + 1);
    void* pippenger_ptr = reinterpret_cast<void*>(pippenger_ptr_base);

    std::vector<uint8_t> g2x_buffer(128);
    io::write_g2_elements_to_buffer(&g2x, (char*)(&g2x_buffer[0]), 1);

    uint8_t const* vk_buf = nullptr;
    acir_proofs::init_verification_key(pippenger_ptr, &g2x_buffer[0], pk_buf, &vk_buf);

    uint8_t* proof_data_buf = nullptr;
    size_t proof_length = acir_proofs::new_proof(
        pippenger_ptr, &g2x_buffer[0], pk_buf, &constraint_system_buf[0], &witness_buf[0], &proof_data_buf);

    bool verified = acir_proofs::verify_proof(
        &g2x_buffer[0], vk_buf, &constraint_system_buf[0], proof_data_buf, static_cast<uint32_t>(proof_length));
    EXPECT_EQ(verified, true);
}

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

// TEST(up_ram, TestBlockConstraint)
// {

//     acir_format::BlockConstraint block;
//     std::vector<fr> witness;
//     size_t num_variables = generate_block_constraint(block, witness);
//     acir_format::acir_format constraint_system{
//         .varnum = static_cast<uint32_t>(num_variables),
//         .public_inputs = {},
//         .fixed_base_scalar_mul_constraints = {},
//         .logic_constraints = {},
//         .range_constraints = {},
//         .schnorr_constraints = {},
//         .ecdsa_constraints = {},
//         .sha256_constraints = {},
//         .blake2s_constraints = {},
//         .keccak_constraints = {},
//         .keccak_var_constraints = {},
//         .hash_to_field_constraints = {},
//         .pedersen_constraints = {},
//         .block_constraints = { block },
//         .constraints = {},
//     };

//     std::vector<uint8_t> witness_buf;           // = to_buffer(witness);
//     std::vector<uint8_t> constraint_system_buf; //
//     write(constraint_system_buf, constraint_system);
//     write(witness_buf, witness);

//     uint32_t total_circuit_size = acir_proofs::get_total_circuit_size(&constraint_system_buf[0]);
//     info("total_circuit_size: ", total_circuit_size);
//     uint32_t pow2_size = 1 << (numeric::get_msb(total_circuit_size) + 1);
//     auto env_crs = std::make_unique<proof_system::EnvReferenceStringFactory>();
//     auto verifier_crs = env_crs->get_verifier_crs();

//     uint8_t const* pk_buf = nullptr;
//     acir_proofs::init_proving_key(&constraint_system_buf[0], &pk_buf);
//     barretenberg::g2::affine_element g2x = verifier_crs->get_g2x();

//     auto* pippenger_ptr_base = new scalar_multiplication::Pippenger(env_load_prover_crs(pow2_size + 1), pow2_size +
//     1); void* pippenger_ptr = reinterpret_cast<void*>(pippenger_ptr_base);

//     std::vector<uint8_t> g2x_buffer(128);
//     io::write_g2_elements_to_buffer(&g2x, (char*)(&g2x_buffer[0]), 1);

//     uint8_t const* vk_buf = nullptr;
//     acir_proofs::init_verification_key(pippenger_ptr, &g2x_buffer[0], pk_buf, &vk_buf);

//     uint8_t* proof_data_buf = nullptr;
//     size_t proof_length = acir_proofs::new_proof(
//         pippenger_ptr, &g2x_buffer[0], pk_buf, &constraint_system_buf[0], &witness_buf[0], &proof_data_buf);

//     bool verified = acir_proofs::verify_proof(
//         &g2x_buffer[0], vk_buf, &constraint_system_buf[0], proof_data_buf, static_cast<uint32_t>(proof_length));
//     EXPECT_EQ(verified, true);
// }

TEST(AcirProofs, TestUPRAM2)
{
    acir_format::acir_format constraint_system;
    std::vector<fr> witness;
    generate_ram_constraint_system(constraint_system, witness);

    auto composer = acir_format::create_circuit_with_witness(constraint_system, witness);
    info("composer.circuit_finalised: ", composer.circuit_finalised);
    info("composer num vars: ", composer.get_num_variables());
    auto prover = composer.create_prover();
    info("composer.circuit_finalised: ", composer.circuit_finalised);
    info("composer num vars: ", composer.get_num_variables());
    auto proof = prover.construct_proof();

    // auto vkey = composer.compute_verification_key();
    // auto composer2 = acir_format::Composer(nullptr, vkey);
    // auto composer2 = acir_format::Composer();
    // acir_format::create_circuit(composer2, constraint_system);
    // auto verifier = composer2.create_verifsier();
    auto verifier = composer.create_verifier();
    EXPECT_EQ(verifier.verify_proof(proof), true);
}
