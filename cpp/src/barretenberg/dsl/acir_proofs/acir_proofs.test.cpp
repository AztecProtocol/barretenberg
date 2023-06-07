#include "acir_proofs.hpp"
#include "../acir_format/acir_format.hpp"
#include "barretenberg/srs/io.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace proof_system::plonk;

void generate_ram_constraint_system(acir_format::acir_format& constraint_system, std::vector<fr>& witness_values)
{
    barretenberg::fr m_3 = fr::neg_one() + fr::neg_one() + fr::neg_one();
    info(m_3);
    acir_format::BlockConstraint block;
    // a =[x1;2x1;3x1;x1+3;x1+4] => x1
    // z witness => 3
    // a[z]           =>y
    // a[z-3];         x3;

    size_t witness_len = 1;
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