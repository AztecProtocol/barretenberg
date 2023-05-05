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
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .compute_merkle_root_constraints = {},
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

struct RecursiveCircuitData {
    std::vector<barretenberg::fr> key_witnesses;
    std::vector<barretenberg::fr> proof_witnesses;
    size_t num_public_inputs;
};

RecursiveCircuitData fetch_recursive_circuit_data(acir_format::acir_format constraint_system, std::vector<fr> witness)
{
    uint8_t* proof_data_fields = nullptr;
    uint8_t* vk_fields = nullptr;
    uint8_t* vk_hash_buf = nullptr;
    size_t proof_fields_size = 0;
    size_t vk_fields_size = 0;

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

    auto* pippenger_ptr_base = new scalar_multiplication::Pippenger(env_load_prover_crs(pow2_size + 1), pow2_size + 1);
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
    vk_fields_size =
        acir_proofs::serialize_verification_key_into_field_elements(&g2x_buffer[0], vk_buf, &vk_fields, &vk_hash_buf);

    bool verified = acir_proofs::verify_proof(
        &g2x_buffer[0], vk_buf, &constraint_system_buf[0], proof_data_buf, static_cast<uint32_t>(proof_length), true);
    EXPECT_EQ(verified, true);

    delete pippenger_ptr_base;
    free((void*)vk_buf);
    free((void*)pk_buf);
    free((void*)proof_data_buf);

    fr vk_hash_value;
    std::vector<fr> proof_witnesses(proof_fields_size / 32);
    std::vector<fr> key_witnesses((vk_fields_size / 32) + 1);
    for (size_t i = 0; i < proof_fields_size / 32; i++) {
        proof_witnesses[i] = barretenberg::fr::serialize_from_buffer(&proof_data_fields[i * 32]);
    }
    for (size_t i = 0; i < vk_fields_size / 32; i++) {
        key_witnesses[i] = barretenberg::fr::serialize_from_buffer(&vk_fields[i * 32]);
    }

    free((void*)proof_data_fields);
    free((void*)vk_fields);

    vk_hash_value = barretenberg::fr::serialize_from_buffer(vk_hash_buf);
    key_witnesses[vk_fields_size / 32] = vk_hash_value;
    auto inner_circuit = RecursiveCircuitData{ key_witnesses, proof_witnesses, constraint_system.public_inputs.size() };
    return inner_circuit;
}

RecursiveCircuitData fetch_inner_circuit_data()
{
    acir_format::acir_format constraint_system;
    std::vector<fr> witness;
    create_inner_circuit(constraint_system, witness);

    return fetch_recursive_circuit_data(constraint_system, witness);
}

/**
 * @brief Create a circuit that recursively verifies one or more inner circuits
 *
 * @param inner_composers
 * @return acir_format::Composer
 */
std::pair<acir_format::acir_format, std::vector<fr>> create_outer_circuit(
    std::vector<RecursiveCircuitData>& inner_circuits)
{
    std::vector<acir_format::RecursionConstraint> recursion_constraints;

    // witness count starts at 1 (Composer reserves 1st witness to be the zero-valued zero_idx)
    size_t witness_offset = 1;
    std::array<uint32_t, acir_format::RecursionConstraint::AGGREGATION_OBJECT_SIZE> output_aggregation_object;
    std::vector<fr> witness;

    for (size_t i = 0; i < inner_circuits.size(); ++i) {
        const bool has_input_aggregation_object = i > 0;

        auto& inner_circuit = inner_circuits[i];
        const std::vector<barretenberg::fr> proof_witnesses = inner_circuit.proof_witnesses;
        const std::vector<barretenberg::fr> key_witnesses = inner_circuit.key_witnesses;

        const bool has_nested_proof = uint32_t(key_witnesses[5]);

        std::cout << "contains_recursive_proof: " << has_nested_proof << std::endl;
        const size_t num_inner_public_inputs = inner_circuit.num_public_inputs;

        const uint32_t key_hash_start_idx = static_cast<uint32_t>(witness_offset);
        const uint32_t public_input_start_idx = key_hash_start_idx + 1;
        // TODO test nested proofs
        // const uint32_t output_aggregation_object_start_idx =
        //     static_cast<uint32_t>(public_input_start_idx + num_inner_public_inputs);
        const uint32_t output_aggregation_object_start_idx =
            static_cast<uint32_t>(public_input_start_idx + num_inner_public_inputs + (has_nested_proof ? 16 : 0));
        const uint32_t proof_indices_start_idx = output_aggregation_object_start_idx + 16;
        const uint32_t key_indices_start_idx = static_cast<uint32_t>(proof_indices_start_idx + proof_witnesses.size());

        std::vector<uint32_t> proof_indices;
        std::vector<uint32_t> key_indices;
        std::vector<uint32_t> inner_public_inputs;
        std::array<uint32_t, acir_format::RecursionConstraint::AGGREGATION_OBJECT_SIZE> input_aggregation_object = {};
        std::array<uint32_t, acir_format::RecursionConstraint::AGGREGATION_OBJECT_SIZE> nested_aggregation_object = {};
        if (has_input_aggregation_object) {
            input_aggregation_object = output_aggregation_object;
        }
        for (size_t i = 0; i < 16; ++i) {
            output_aggregation_object[i] = (static_cast<uint32_t>(i + output_aggregation_object_start_idx));
        }
        if (has_nested_proof) {
            for (size_t i = 6; i < 22; ++i) {
                nested_aggregation_object[i] = uint32_t(key_witnesses[i]);
            }
        }
        for (size_t i = 0; i < proof_witnesses.size(); ++i) {
            proof_indices.emplace_back(static_cast<uint32_t>(i + proof_indices_start_idx));
        }
        const size_t key_size = key_witnesses.size();
        for (size_t i = 0; i < key_size; ++i) {
            key_indices.emplace_back(static_cast<uint32_t>(i + key_indices_start_idx));
        }
        for (size_t i = 0; i < num_inner_public_inputs; ++i) {
            inner_public_inputs.push_back(static_cast<uint32_t>(i + public_input_start_idx));
        }

        acir_format::RecursionConstraint recursion_constraint{
            .key = key_indices,
            .proof = proof_indices,
            .public_inputs = inner_public_inputs,
            .key_hash = key_hash_start_idx,
            .input_aggregation_object = input_aggregation_object,
            .output_aggregation_object = output_aggregation_object,
            .nested_aggregation_object = nested_aggregation_object,
        };
        recursion_constraints.push_back(recursion_constraint);
        for (size_t i = 0; i < proof_indices_start_idx - witness_offset; ++i) {
            witness.emplace_back(0);
        }
        for (const auto& wit : proof_witnesses) {
            witness.emplace_back(wit);
        }
        for (const auto& wit : key_witnesses) {
            witness.emplace_back(wit);
        }
        witness_offset = key_indices_start_idx + key_witnesses.size();
    }

    std::vector<uint32_t> public_inputs(output_aggregation_object.begin(), output_aggregation_object.end());

    acir_format::acir_format constraint_system{
        .varnum = static_cast<uint32_t>(witness.size() + 1),
        .public_inputs = public_inputs,
        .fixed_base_scalar_mul_constraints = {},
        .logic_constraints = {},
        .range_constraints = {},
        .schnorr_constraints = {},
        .ecdsa_constraints = {},
        .sha256_constraints = {},
        .blake2s_constraints = {},
        .hash_to_field_constraints = {},
        .pedersen_constraints = {},
        .compute_merkle_root_constraints = {},
        .recursion_constraints = recursion_constraints,
        .constraints = {},
    };

    // NOTE: This passes, so the circuit is good with a witness but something is broken with the dummy key and dummy
    // proof This NOTE was fixed by using g1 affine two for the dummy key commitments rather than affine one auto
    // composer = acir_format::create_circuit_with_witness(constraint_system, witness); auto prover =
    // composer.create_ultra_with_keccak_prover(); std::cout << "prover gates = " << prover.circuit_size << std::endl;
    // auto proof = prover.construct_proof();
    // auto verifier = composer.create_ultra_with_keccak_verifier();
    // EXPECT_EQ(verifier.verify_proof(proof), true);
    // std::cout << "done create_circuit_with_witness, composer.contains_recursive_proof: " <<
    // composer.contains_recursive_proof << std::endl;

    return std::make_pair(constraint_system, witness);
}

// // Working double recursion test
TEST(AcirProofs, TestSerializationWithBasicDoubleRecursion)
{
    std::vector<RecursiveCircuitData> inner_circuits;
    auto a = fetch_inner_circuit_data();
    inner_circuits.push_back(a);

    auto b = fetch_inner_circuit_data();
    inner_circuits.push_back(b);

    auto c = create_outer_circuit(inner_circuits);

    std::vector<uint8_t> witness_buf;
    std::vector<uint8_t> constraint_system_buf;
    write(constraint_system_buf, c.first);
    write(witness_buf, c.second);
    uint8_t const* pk_buf = nullptr;
    acir_proofs::init_proving_key(&constraint_system_buf[0], &pk_buf);

    uint32_t total_circuit_size = acir_proofs::get_total_circuit_size(&constraint_system_buf[0]);
    uint32_t pow2_size = 1 << (numeric::get_msb(total_circuit_size) + 1);
    auto env_crs = std::make_unique<proof_system::EnvReferenceStringFactory>();
    auto verifier_crs = env_crs->get_verifier_crs();
    barretenberg::g2::affine_element g2x = verifier_crs->get_g2x();

    auto* pippenger_ptr_base = new scalar_multiplication::Pippenger(env_load_prover_crs(pow2_size + 1), pow2_size + 1);
    void* pippenger_ptr = reinterpret_cast<void*>(pippenger_ptr_base);

    std::vector<uint8_t> g2x_buffer(128);
    io::write_g2_elements_to_buffer(&g2x, (char*)(&g2x_buffer[0]), 1);

    uint8_t const* vk_buf = nullptr;
    acir_proofs::init_verification_key(pippenger_ptr, &g2x_buffer[0], pk_buf, &vk_buf);

    uint8_t* proof_data_buf = nullptr;
    size_t proof_length = acir_proofs::new_proof(
        pippenger_ptr, &g2x_buffer[0], pk_buf, &constraint_system_buf[0], &witness_buf[0], &proof_data_buf, false);

    bool verified = acir_proofs::verify_proof(
        &g2x_buffer[0], vk_buf, &constraint_system_buf[0], proof_data_buf, static_cast<uint32_t>(proof_length), false);
    EXPECT_EQ(verified, true);

    delete pippenger_ptr_base;
    free((void*)vk_buf);
    free((void*)pk_buf);
    free((void*)proof_data_buf);
}

TEST(AcirProofs, TestSerializationWithFullRecursiveComposition)
{
    std::vector<RecursiveCircuitData> layer_1_circuits;
    auto a_data_circuit = fetch_inner_circuit_data();
    layer_1_circuits.push_back(a_data_circuit);

    std::vector<RecursiveCircuitData> layer_2_circuits;
    auto a2_circuit = fetch_inner_circuit_data();
    layer_2_circuits.push_back(a2_circuit);

    auto b_cs_and_witness = create_outer_circuit(layer_1_circuits);
    auto b_circuit = fetch_recursive_circuit_data(b_cs_and_witness.first, b_cs_and_witness.second);
    layer_2_circuits.push_back(b_circuit);

    // std::vector<RecursiveCircuitData> layer_3_circuits;
    auto c_cs_and_witness = create_outer_circuit(layer_2_circuits);
    // auto c_circuit = fetch_recursive_circuit_data(c_cs_and_witness.first, c_cs_and_witness.second);
    // layer_3_circuits.push_back(c_circuit);

    std::vector<uint8_t> witness_buf;
    std::vector<uint8_t> constraint_system_buf;
    write(constraint_system_buf, c_cs_and_witness.first);
    write(witness_buf, c_cs_and_witness.second);
    uint8_t const* pk_buf = nullptr;
    acir_proofs::init_proving_key(&constraint_system_buf[0], &pk_buf);

    uint32_t total_circuit_size = acir_proofs::get_total_circuit_size(&constraint_system_buf[0]);
    uint32_t pow2_size = 1 << (numeric::get_msb(total_circuit_size) + 1);
    auto env_crs = std::make_unique<proof_system::EnvReferenceStringFactory>();
    auto verifier_crs = env_crs->get_verifier_crs();
    barretenberg::g2::affine_element g2x = verifier_crs->get_g2x();

    auto* pippenger_ptr_base = new scalar_multiplication::Pippenger(env_load_prover_crs(pow2_size + 1), pow2_size + 1);
    void* pippenger_ptr = reinterpret_cast<void*>(pippenger_ptr_base);

    std::vector<uint8_t> g2x_buffer(128);
    io::write_g2_elements_to_buffer(&g2x, (char*)(&g2x_buffer[0]), 1);

    uint8_t const* vk_buf = nullptr;
    acir_proofs::init_verification_key(pippenger_ptr, &g2x_buffer[0], pk_buf, &vk_buf);

    uint8_t* proof_data_buf = nullptr;
    size_t proof_length = acir_proofs::new_proof(
        pippenger_ptr, &g2x_buffer[0], pk_buf, &constraint_system_buf[0], &witness_buf[0], &proof_data_buf, false);

    bool verified = acir_proofs::verify_proof(
        &g2x_buffer[0], vk_buf, &constraint_system_buf[0], proof_data_buf, static_cast<uint32_t>(proof_length), false);
    EXPECT_EQ(verified, true);

    delete pippenger_ptr_base;
    free((void*)vk_buf);
    free((void*)pk_buf);
    free((void*)proof_data_buf);
}
