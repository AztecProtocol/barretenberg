#include "acir_format.hpp"
#include "barretenberg/common/log.hpp"
#include "barretenberg/crypto/ecdsa/ecdsa.hpp"

namespace acir_format {

void read_witness(Composer& composer, std::vector<barretenberg::fr> witness)
{
    composer.variables[0] = 0;
    for (size_t i = 0; i < witness.size(); ++i) {
        composer.variables[i + 1] = witness[i];
    }
}
using curve = proof_system::plonk::stdlib::secp256k1<Composer>;

void dummy_ecdsa_constraint(Composer& composer, EcdsaSecp256k1Constraint const& input)
{

    std::vector<uint32_t> pub_x_indices_;
    std::vector<uint32_t> pub_y_indices_;
    std::vector<uint32_t> signature_;
    signature_.resize(64);

    crypto::ecdsa::key_pair<curve::fr, curve::g1> account;
    account.private_key = 10;
    account.public_key = curve::g1::one * account.private_key;
    uint256_t pub_x_value = account.public_key.x;
    uint256_t pub_y_value = account.public_key.y;
    std::string message_string = "Instructions unclear, ask again later.";
    crypto::ecdsa::signature signature =
        crypto::ecdsa::construct_signature<Sha256Hasher, curve::fq, curve::fr, curve::g1>(message_string, account);
    for (size_t i = 0; i < 32; ++i) {
        uint32_t x_wit = composer.add_variable(pub_x_value.slice(248 - i * 8, 256 - i * 8));
        uint32_t y_wit = composer.add_variable(pub_y_value.slice(248 - i * 8, 256 - i * 8));
        uint32_t r_wit = composer.add_variable(signature.r[i]);
        uint32_t s_wit = composer.add_variable(signature.s[i]);
        pub_x_indices_.emplace_back(x_wit);
        pub_y_indices_.emplace_back(y_wit);
        signature_[i] = r_wit;
        signature_[i + 32] = s_wit;
    }

    for (size_t i = 0; i < input.pub_x_indices.size(); ++i) {
        composer.assert_equal(pub_x_indices_[i], input.pub_x_indices[i]);
    }
    for (size_t i = 0; i < input.pub_y_indices.size(); ++i) {
        composer.assert_equal(pub_y_indices_[i], input.pub_y_indices[i]);
    }
    for (size_t i = 0; i < input.signature.size(); ++i) {
        composer.assert_equal(signature_[i], input.signature[i]);
    }
}

void create_circuit(Composer& composer, const acir_format& constraint_system)
{
    if (constraint_system.public_inputs.size() > constraint_system.varnum) {
        info("create_circuit: too many public inputs!");
    }

    for (size_t i = 1; i < constraint_system.varnum; ++i) {
        // If the index is in the public inputs vector, then we add it as a public input

        if (std::find(constraint_system.public_inputs.begin(), constraint_system.public_inputs.end(), i) !=
            constraint_system.public_inputs.end()) {
            composer.add_public_variable(0);

        } else {
            composer.add_variable(0);
        }
    }

    // Add arithmetic gates
    for (const auto& constraint : constraint_system.constraints) {
        composer.create_poly_gate(constraint);
    }

    // Add and constraint
    for (const auto& constraint : constraint_system.logic_constraints) {
        create_logic_gate(
            composer, constraint.a, constraint.b, constraint.result, constraint.num_bits, constraint.is_xor_gate);
    }

    // Add range constraint
    for (const auto& constraint : constraint_system.range_constraints) {
        composer.create_range_constraint(constraint.witness, constraint.num_bits, "");
    }

    // Add sha256 constraints
    for (const auto& constraint : constraint_system.sha256_constraints) {
        create_sha256_constraints(composer, constraint);
    }

    // Add compute merkle root constraints
    for (const auto& constraint : constraint_system.compute_merkle_root_constraints) {
        create_compute_merkle_root_constraint(composer, constraint);
    }

    // Add schnorr constraints
    for (const auto& constraint : constraint_system.schnorr_constraints) {
        create_schnorr_verify_constraints(composer, constraint);
    }

    // Add ECDSA constraints
    for (const auto& constraint : constraint_system.ecdsa_constraints) {
        dummy_ecdsa_constraint(composer, constraint);
        create_ecdsa_verify_constraints(composer, constraint);
    }

    // Add blake2s constraints
    for (const auto& constraint : constraint_system.blake2s_constraints) {
        create_blake2s_constraints(composer, constraint);
    }

    // Add keccak constraints
    for (const auto& constraint : constraint_system.keccak_constraints) {
        create_keccak_constraints(composer, constraint);
    }

    // Add pedersen constraints
    for (const auto& constraint : constraint_system.pedersen_constraints) {
        create_pedersen_constraint(composer, constraint);
    }

    // Add fixed base scalar mul constraints
    for (const auto& constraint : constraint_system.fixed_base_scalar_mul_constraints) {
        create_fixed_base_constraint(composer, constraint);
    }

    // Add hash to field constraints
    for (const auto& constraint : constraint_system.hash_to_field_constraints) {
        create_hash_to_field_constraints(composer, constraint);
    }
}

Composer create_circuit(const acir_format& constraint_system,
                        std::unique_ptr<proof_system::ReferenceStringFactory>&& crs_factory)
{
    if (constraint_system.public_inputs.size() > constraint_system.varnum) {
        info("create_circuit: too many public inputs!");
    }

    Composer composer(std::move(crs_factory));

    for (size_t i = 1; i < constraint_system.varnum; ++i) {
        // If the index is in the public inputs vector, then we add it as a public input

        if (std::find(constraint_system.public_inputs.begin(), constraint_system.public_inputs.end(), i) !=
            constraint_system.public_inputs.end()) {

            composer.add_public_variable(0);

        } else {
            composer.add_variable(0);
        }
    }
    // Add arithmetic gates
    for (const auto& constraint : constraint_system.constraints) {
        composer.create_poly_gate(constraint);
    }

    // Add logic constraint
    for (const auto& constraint : constraint_system.logic_constraints) {
        create_logic_gate(
            composer, constraint.a, constraint.b, constraint.result, constraint.num_bits, constraint.is_xor_gate);
    }

    // Add range constraint
    for (const auto& constraint : constraint_system.range_constraints) {
        composer.create_range_constraint(constraint.witness, constraint.num_bits, "");
    }

    // Add sha256 constraints
    for (const auto& constraint : constraint_system.sha256_constraints) {
        create_sha256_constraints(composer, constraint);
    }

    // Add compute merkle root constraints
    for (const auto& constraint : constraint_system.compute_merkle_root_constraints) {
        create_compute_merkle_root_constraint(composer, constraint);
    }

    // Add schnorr constraints
    for (const auto& constraint : constraint_system.schnorr_constraints) {
        create_schnorr_verify_constraints(composer, constraint);
    }

    // Add ECDSA constraints
    for (const auto& constraint : constraint_system.ecdsa_constraints) {
        dummy_ecdsa_constraint(composer, constraint);
        create_ecdsa_verify_constraints(composer, constraint);
    }

    // Add blake2s constraints
    for (const auto& constraint : constraint_system.blake2s_constraints) {
        create_blake2s_constraints(composer, constraint);
    }

    // Add keccak constraints
    for (const auto& constraint : constraint_system.keccak_constraints) {
        create_keccak_constraints(composer, constraint);
    }

    // Add pedersen constraints
    for (const auto& constraint : constraint_system.pedersen_constraints) {
        create_pedersen_constraint(composer, constraint);
    }

    // Add fixed base scalar mul constraints
    for (const auto& constraint : constraint_system.fixed_base_scalar_mul_constraints) {
        create_fixed_base_constraint(composer, constraint);
    }

    // Add hash to field constraints
    for (const auto& constraint : constraint_system.hash_to_field_constraints) {
        create_hash_to_field_constraints(composer, constraint);
    }

    return composer;
}

Composer create_circuit_with_witness(const acir_format& constraint_system,
                                     std::vector<fr> witness,
                                     std::unique_ptr<ReferenceStringFactory>&& crs_factory)
{
    if (constraint_system.public_inputs.size() > constraint_system.varnum) {
        info("create_circuit_with_witness: too many public inputs!");
    }

    Composer composer(std::move(crs_factory));

    for (size_t i = 1; i < constraint_system.varnum; ++i) {
        // If the index is in the public inputs vector, then we add it as a public input

        if (std::find(constraint_system.public_inputs.begin(), constraint_system.public_inputs.end(), i) !=
            constraint_system.public_inputs.end()) {

            composer.add_public_variable(0);

        } else {
            composer.add_variable(0);
        }
    }

    read_witness(composer, witness);

    // Add arithmetic gates
    for (const auto& constraint : constraint_system.constraints) {
        composer.create_poly_gate(constraint);
    }

    // Add logic constraint
    for (const auto& constraint : constraint_system.logic_constraints) {
        create_logic_gate(
            composer, constraint.a, constraint.b, constraint.result, constraint.num_bits, constraint.is_xor_gate);
    }

    // Add range constraint
    for (const auto& constraint : constraint_system.range_constraints) {
        composer.create_range_constraint(constraint.witness, constraint.num_bits, "");
    }

    // Add sha256 constraints
    for (const auto& constraint : constraint_system.sha256_constraints) {
        create_sha256_constraints(composer, constraint);
    }

    // Add compute merkle root constraints
    for (const auto& constraint : constraint_system.compute_merkle_root_constraints) {
        create_compute_merkle_root_constraint(composer, constraint);
    }

    // Add schnorr constraints
    for (const auto& constraint : constraint_system.schnorr_constraints) {
        create_schnorr_verify_constraints(composer, constraint);
    }

    // Add ECDSA constraints
    for (const auto& constraint : constraint_system.ecdsa_constraints) {
        create_ecdsa_verify_constraints(composer, constraint);
    }

    // Add blake2s constraints
    for (const auto& constraint : constraint_system.blake2s_constraints) {
        create_blake2s_constraints(composer, constraint);
    }

    // Add keccak constraints
    for (const auto& constraint : constraint_system.keccak_constraints) {
        create_keccak_constraints(composer, constraint);
    }

    // Add pedersen constraints
    for (const auto& constraint : constraint_system.pedersen_constraints) {
        create_pedersen_constraint(composer, constraint);
    }

    // Add fixed base scalar mul constraints
    for (const auto& constraint : constraint_system.fixed_base_scalar_mul_constraints) {
        create_fixed_base_constraint(composer, constraint);
    }

    // Add hash to field constraints
    for (const auto& constraint : constraint_system.hash_to_field_constraints) {
        create_hash_to_field_constraints(composer, constraint);
    }

    return composer;
}
Composer create_circuit_with_witness(const acir_format& constraint_system, std::vector<fr> witness)
{
    if (constraint_system.public_inputs.size() > constraint_system.varnum) {
        info("create_circuit_with_witness: too many public inputs!");
    }

    auto composer = Composer();

    for (size_t i = 1; i < constraint_system.varnum; ++i) {
        // If the index is in the public inputs vector, then we add it as a public input

        if (std::find(constraint_system.public_inputs.begin(), constraint_system.public_inputs.end(), i) !=
            constraint_system.public_inputs.end()) {

            composer.add_public_variable(0);

        } else {
            composer.add_variable(0);
        }
    }

    read_witness(composer, witness);

    // Add arithmetic gates
    for (const auto& constraint : constraint_system.constraints) {
        composer.create_poly_gate(constraint);
    }

    // Add logic constraint
    for (const auto& constraint : constraint_system.logic_constraints) {
        create_logic_gate(
            composer, constraint.a, constraint.b, constraint.result, constraint.num_bits, constraint.is_xor_gate);
    }

    // Add range constraint
    for (const auto& constraint : constraint_system.range_constraints) {
        composer.create_range_constraint(constraint.witness, constraint.num_bits, "");
    }

    // Add sha256 constraints
    for (const auto& constraint : constraint_system.sha256_constraints) {
        create_sha256_constraints(composer, constraint);
    }

    // Add compute merkle root constraints
    for (const auto& constraint : constraint_system.compute_merkle_root_constraints) {
        create_compute_merkle_root_constraint(composer, constraint);
    }

    // Add schnorr constraints
    for (const auto& constraint : constraint_system.schnorr_constraints) {
        create_schnorr_verify_constraints(composer, constraint);
    }

    // Add ECDSA constraints
    for (const auto& constraint : constraint_system.ecdsa_constraints) {
        create_ecdsa_verify_constraints(composer, constraint);
    }

    // Add blake2s constraints
    for (const auto& constraint : constraint_system.blake2s_constraints) {
        create_blake2s_constraints(composer, constraint);
    }

    // Add keccak constraints
    for (const auto& constraint : constraint_system.keccak_constraints) {
        create_keccak_constraints(composer, constraint);
    }

    // Add pedersen constraints
    for (const auto& constraint : constraint_system.pedersen_constraints) {
        create_pedersen_constraint(composer, constraint);
    }

    // Add fixed base scalar mul constraints
    for (const auto& constraint : constraint_system.fixed_base_scalar_mul_constraints) {
        create_fixed_base_constraint(composer, constraint);
    }

    // Add hash to field constraints
    for (const auto& constraint : constraint_system.hash_to_field_constraints) {
        create_hash_to_field_constraints(composer, constraint);
    }

    return composer;
}
void create_circuit_with_witness(Composer& composer, const acir_format& constraint_system, std::vector<fr> witness)
{
    if (constraint_system.public_inputs.size() > constraint_system.varnum) {
        info("create_circuit_with_witness: too many public inputs!");
    }

    for (size_t i = 1; i < constraint_system.varnum; ++i) {
        // If the index is in the public inputs vector, then we add it as a public input

        if (std::find(constraint_system.public_inputs.begin(), constraint_system.public_inputs.end(), i) !=
            constraint_system.public_inputs.end()) {

            composer.add_public_variable(0);

        } else {
            composer.add_variable(0);
        }
    }

    read_witness(composer, witness);

    // Add arithmetic gates
    for (const auto& constraint : constraint_system.constraints) {
        composer.create_poly_gate(constraint);
    }

    // Add logic constraint
    for (const auto& constraint : constraint_system.logic_constraints) {
        create_logic_gate(
            composer, constraint.a, constraint.b, constraint.result, constraint.num_bits, constraint.is_xor_gate);
    }

    // Add range constraint
    for (const auto& constraint : constraint_system.range_constraints) {
        composer.create_range_constraint(constraint.witness, constraint.num_bits, "");
    }

    // Add sha256 constraints
    for (const auto& constraint : constraint_system.sha256_constraints) {
        create_sha256_constraints(composer, constraint);
    }

    // Add compute merkle root constraints
    for (const auto& constraint : constraint_system.compute_merkle_root_constraints) {
        create_compute_merkle_root_constraint(composer, constraint);
    }

    // Add schnorr constraints
    for (const auto& constraint : constraint_system.schnorr_constraints) {
        create_schnorr_verify_constraints(composer, constraint);
    }

    // Add ECDSA constraints
    for (const auto& constraint : constraint_system.ecdsa_constraints) {
        create_ecdsa_verify_constraints(composer, constraint);
    }

    // Add blake2s constraints
    for (const auto& constraint : constraint_system.blake2s_constraints) {
        create_blake2s_constraints(composer, constraint);
    }

    // Add keccak constraints
    for (const auto& constraint : constraint_system.keccak_constraints) {
        create_keccak_constraints(composer, constraint);
    }

    // Add pedersen constraints
    for (const auto& constraint : constraint_system.pedersen_constraints) {
        create_pedersen_constraint(composer, constraint);
    }

    // Add fixed base scalar mul constraints
    for (const auto& constraint : constraint_system.fixed_base_scalar_mul_constraints) {
        create_fixed_base_constraint(composer, constraint);
    }

    // Add hash to field constraints
    for (const auto& constraint : constraint_system.hash_to_field_constraints) {
        create_hash_to_field_constraints(composer, constraint);
    }
}

} // namespace acir_format
