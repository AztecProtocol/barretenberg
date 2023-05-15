#include "acir_proofs.hpp"
#include "../acir_format/acir_format.hpp"
#include "barretenberg/srs/io.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace proof_system::plonk;

void create_inner_circuit(acir_format::acir_format& constraint_system, std::vector<fr>& witness)
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
        .block_constraints = {},
        .recursion_constraints = {},
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

TEST(AcirProofs, TestSerialization)
{
    acir_format::acir_format constraint_system;
    std::vector<fr> witness;
    create_inner_circuit(constraint_system, witness);

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
        pippenger_ptr, &g2x_buffer[0], pk_buf, &constraint_system_buf[0], &witness_buf[0], &proof_data_buf, true);

    bool verified = acir_proofs::verify_proof(
        &g2x_buffer[0], vk_buf, &constraint_system_buf[0], proof_data_buf, static_cast<uint32_t>(proof_length), true);
    EXPECT_EQ(verified, true);
}

TEST(AcirProofs, TestSerializationWithRecursion)
{
    uint8_t* proof_data_fields = nullptr;
    uint8_t* vk_fields = nullptr;
    uint8_t* vk_hash_buf = nullptr;
    size_t proof_fields_size = 0;
    size_t vk_fields_size = 0;

    // inner circuit
    {
        acir_format::acir_format constraint_system;
        std::vector<fr> witness;
        create_inner_circuit(constraint_system, witness);

        std::vector<uint8_t> witness_buf;           // = to_buffer(witness);
        std::vector<uint8_t> constraint_system_buf; //
        write(constraint_system_buf, constraint_system);
        write(witness_buf, witness);

        uint8_t const* pk_buf = nullptr;
        acir_proofs::init_proving_key(&constraint_system_buf[0], &pk_buf);

        uint32_t total_circuit_size = acir_proofs::get_total_circuit_size(&constraint_system_buf[0]);
        uint32_t pow2_size = 1 << (numeric::get_msb(total_circuit_size) + 1);
        auto env_crs = std::make_unique<proof_system::EnvReferenceStringFactory>();
        auto verifier_crs = env_crs->get_verifier_crs();
        barretenberg::g2::affine_element g2x = verifier_crs->get_g2x();

        auto* pippenger_ptr_base =
            new scalar_multiplication::Pippenger(env_load_prover_crs(pow2_size + 1), pow2_size + 1);
        void* pippenger_ptr = reinterpret_cast<void*>(pippenger_ptr_base);

        std::vector<uint8_t> g2x_buffer(128);
        io::write_g2_elements_to_buffer(&g2x, (char*)(&g2x_buffer[0]), 1);

        uint8_t const* vk_buf = nullptr;
        acir_proofs::init_verification_key(pippenger_ptr, &g2x_buffer[0], pk_buf, &vk_buf);

        uint8_t* proof_data_buf = nullptr;
        size_t proof_length = acir_proofs::new_proof(
            pippenger_ptr, &g2x_buffer[0], pk_buf, &constraint_system_buf[0], &witness_buf[0], &proof_data_buf, true);
        proof_fields_size =
            acir_proofs::serialize_proof_into_field_elements(proof_data_buf, &proof_data_fields, proof_length);
        vk_fields_size = acir_proofs::serialize_verification_key_into_field_elements(
            &g2x_buffer[0], vk_buf, &vk_fields, &vk_hash_buf);

        bool verified = acir_proofs::verify_proof(&g2x_buffer[0],
                                                  vk_buf,
                                                  &constraint_system_buf[0],
                                                  proof_data_buf,
                                                  static_cast<uint32_t>(proof_length),
                                                  true);
        EXPECT_EQ(verified, true);

        delete pippenger_ptr_base;
        free((void*)vk_buf);
        free((void*)pk_buf);
        free((void*)proof_data_buf);
    }
    // outer circuit
    {
        fr vk_hash_value;
        std::vector<fr> proof_witnesses(proof_fields_size / 32);
        std::vector<fr> key_witnesses(vk_fields_size / 32);
        for (size_t i = 0; i < proof_fields_size / 32; i++) {
            proof_witnesses[i] = barretenberg::fr::serialize_from_buffer(&proof_data_fields[i * 32]);
        }
        for (size_t i = 0; i < vk_fields_size / 32; i++) {
            key_witnesses[i] = barretenberg::fr::serialize_from_buffer(&vk_fields[i * 32]);
        }
        vk_hash_value = barretenberg::fr::serialize_from_buffer(vk_hash_buf);

        std::vector<uint32_t> proof_indices;

        const size_t proof_size = proof_witnesses.size();
        for (size_t i = 0; i < proof_size; ++i) {
            proof_indices.emplace_back(static_cast<uint32_t>(i + 19));
        }

        std::vector<uint32_t> key_indices;
        const size_t key_size = key_witnesses.size();
        for (size_t i = 0; i < key_size; ++i) {
            key_indices.emplace_back(static_cast<uint32_t>(i + 19 + proof_size));
        }

        std::array<uint32_t, acir_format::RecursionConstraint::AGGREGATION_OBJECT_SIZE> output_vars;
        for (size_t i = 0; i < acir_format::RecursionConstraint::AGGREGATION_OBJECT_SIZE; ++i) {
            // variable idx 1 = public input
            // variable idx 2-18 = output_vars
            output_vars[i] = (static_cast<uint32_t>(i + 3));
        }
        acir_format::RecursionConstraint recursion_constraint{
            .key = key_indices,
            .proof = proof_indices,
            .public_input = 1,
            .key_hash = 2,
            .input_aggregation_object = {},
            .output_aggregation_object = output_vars,
        };

        // Add a constraint that fixes the vk hash to be the expected value!
        poly_triple vk_equality_constraint{
            .a = recursion_constraint.key_hash,
            .b = 0,
            .c = 0,
            .q_m = 0,
            .q_l = 1,
            .q_r = 0,
            .q_o = 0,
            .q_c = -vk_hash_value,
        };

        std::vector<fr> witness;
        for (size_t i = 0; i < 18; ++i) {
            witness.emplace_back(0);
        }
        for (const auto& wit : proof_witnesses) {
            witness.emplace_back(wit);
        }
        for (const auto& wit : key_witnesses) {
            witness.emplace_back(wit);
        }

        acir_format::acir_format constraint_system{
            .varnum = static_cast<uint32_t>(witness.size() + 1),
            .public_inputs = { 1 },
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
            .compute_merkle_root_constraints = {},
            .recursion_constraints = { recursion_constraint },
            .constraints = { vk_equality_constraint },
        };

        std::vector<uint8_t> witness_buf;
        std::vector<uint8_t> constraint_system_buf;
        write(constraint_system_buf, constraint_system);
        write(witness_buf, witness);
        uint8_t const* pk_buf = nullptr;
        acir_proofs::init_proving_key(&constraint_system_buf[0], &pk_buf);

        uint32_t total_circuit_size = acir_proofs::get_total_circuit_size(&constraint_system_buf[0]);
        uint32_t pow2_size = 1 << (numeric::get_msb(total_circuit_size) + 1);
        auto env_crs = std::make_unique<proof_system::EnvReferenceStringFactory>();
        auto verifier_crs = env_crs->get_verifier_crs();
        barretenberg::g2::affine_element g2x = verifier_crs->get_g2x();

        auto* pippenger_ptr_base =
            new scalar_multiplication::Pippenger(env_load_prover_crs(pow2_size + 1), pow2_size + 1);
        void* pippenger_ptr = reinterpret_cast<void*>(pippenger_ptr_base);

        std::vector<uint8_t> g2x_buffer(128);
        io::write_g2_elements_to_buffer(&g2x, (char*)(&g2x_buffer[0]), 1);

        uint8_t const* vk_buf = nullptr;
        acir_proofs::init_verification_key(pippenger_ptr, &g2x_buffer[0], pk_buf, &vk_buf);

        uint8_t* proof_data_buf = nullptr;
        size_t proof_length = acir_proofs::new_proof(
            pippenger_ptr, &g2x_buffer[0], pk_buf, &constraint_system_buf[0], &witness_buf[0], &proof_data_buf, false);

        bool verified = acir_proofs::verify_proof(&g2x_buffer[0],
                                                  vk_buf,
                                                  &constraint_system_buf[0],
                                                  proof_data_buf,
                                                  static_cast<uint32_t>(proof_length),
                                                  false);
        EXPECT_EQ(verified, true);

        delete pippenger_ptr_base;
        free((void*)vk_buf);
        free((void*)pk_buf);
        free((void*)proof_data_buf);
        free((void*)proof_data_fields);
        free((void*)vk_fields);
    }
}
