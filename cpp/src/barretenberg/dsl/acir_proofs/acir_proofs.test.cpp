#include "acir_proofs.hpp"
#include "../acir_format/acir_format.hpp"
#include "barretenberg/srs/io.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace proof_system::plonk;

void generate_block_constraint_system(acir_format::acir_format& constraint_system, std::vector<fr>& witness_values)
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
    acir_format::BlockConstraint block = acir_format::BlockConstraint{
        .init = { a0, a1 },
        .trace = { op1, op2 },
        .type = acir_format::BlockType::ROM,
    };

    constraint_system = acir_format::acir_format{
        .varnum = static_cast<uint32_t>(witness_len),
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
}

TEST(AcirProofs, TestRAMSerialization)
{
    acir_format::acir_format constraint_system;
    std::vector<fr> witness;
    generate_block_constraint_system(constraint_system, witness);

    std::vector<uint8_t> witness_buf;           // = to_buffer(witness);
    std::vector<uint8_t> constraint_system_buf; //
    write(constraint_system_buf, constraint_system);
    write(witness_buf, witness);

    uint32_t total_circuit_size = acir_proofs::get_total_circuit_size(&constraint_system_buf[0]);
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