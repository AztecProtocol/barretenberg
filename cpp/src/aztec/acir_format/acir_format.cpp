#include "acir_format.hpp"

using namespace barretenberg;

namespace acir_format {

void read_witness(TurboComposer& composer, std::vector<barretenberg::fr> witness)
{
    composer.variables[0] = 0;
    for (size_t i = 0; i < witness.size(); ++i) {
        composer.variables[i + 1] = witness[i];
    }
}

void create_circuit(TurboComposer& composer, const acir_format& constraint_system)
{
    if (constraint_system.public_inputs.size() > constraint_system.varnum) {
        std::cout << "too many public inputs!" << std::endl;
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
        composer.decompose_into_base4_accumulators(constraint.witness, constraint.num_bits, "");
    }

    // Add sha256 constraints
    for (const auto& constraint : constraint_system.sha256_constraints) {
        create_sha256_constraints(composer, constraint);
    }

    // Add merkle membership constraints
    for (const auto& constraint : constraint_system.merkle_membership_constraints) {
        create_merkle_check_membership_constraint(composer, constraint);
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

TurboComposer create_circuit(const acir_format& constraint_system,
                             std::unique_ptr<bonk::ReferenceStringFactory>&& crs_factory)
{
    if (constraint_system.public_inputs.size() > constraint_system.varnum) {
        std::cout << "too many public inputs!" << std::endl;
    }

    TurboComposer composer(std::move(crs_factory));

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
        composer.decompose_into_base4_accumulators(constraint.witness, constraint.num_bits, "");
    }

    // Add sha256 constraints
    for (const auto& constraint : constraint_system.sha256_constraints) {
        create_sha256_constraints(composer, constraint);
    }

    // Add merkle membership constraints
    for (const auto& constraint : constraint_system.merkle_membership_constraints) {
        create_merkle_check_membership_constraint(composer, constraint);
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

TurboComposer create_circuit_with_witness(const acir_format& constraint_system,
                                          std::vector<fr> witness,
                                          std::unique_ptr<ReferenceStringFactory>&& crs_factory)
{
    if (constraint_system.public_inputs.size() > constraint_system.varnum) {
        std::cout << "too many public inputs!" << std::endl;
    }

    TurboComposer composer(std::move(crs_factory));

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
        composer.decompose_into_base4_accumulators(constraint.witness, constraint.num_bits, "");
    }

    // Add sha256 constraints
    for (const auto& constraint : constraint_system.sha256_constraints) {
        create_sha256_constraints(composer, constraint);
    }

    // Add merkle membership constraints
    for (const auto& constraint : constraint_system.merkle_membership_constraints) {
        create_merkle_check_membership_constraint(composer, constraint);
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
TurboComposer create_circuit_with_witness(const acir_format& constraint_system, std::vector<fr> witness)
{
    if (constraint_system.public_inputs.size() > constraint_system.varnum) {
        std::cout << "too many public inputs!" << std::endl;
    }

    auto composer = TurboComposer();

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
        composer.decompose_into_base4_accumulators(constraint.witness, constraint.num_bits, "");
    }

    // Add sha256 constraints
    for (const auto& constraint : constraint_system.sha256_constraints) {
        create_sha256_constraints(composer, constraint);
    }

    // Add merkle membership constraints
    for (const auto& constraint : constraint_system.merkle_membership_constraints) {
        create_merkle_check_membership_constraint(composer, constraint);
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
void create_circuit_with_witness(TurboComposer& composer, const acir_format& constraint_system, std::vector<fr> witness)
{
    if (constraint_system.public_inputs.size() > constraint_system.varnum) {
        std::cout << "too many public inputs!" << std::endl;
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
        composer.decompose_into_base4_accumulators(constraint.witness, constraint.num_bits, "");
    }

    // Add sha256 constraints
    for (const auto& constraint : constraint_system.sha256_constraints) {
        create_sha256_constraints(composer, constraint);
    }

    // Add merkle membership constraints
    for (const auto& constraint : constraint_system.merkle_membership_constraints) {
        create_merkle_check_membership_constraint(composer, constraint);
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

// Serialisation
template <typename B> inline void read(B& buf, acir_format& data)
{
    using serialize::read;
    read(buf, data.varnum);
    read(buf, data.public_inputs);
    read(buf, data.logic_constraints);
    read(buf, data.range_constraints);
    read(buf, data.sha256_constraints);
    read(buf, data.merkle_membership_constraints);
    read(buf, data.schnorr_constraints);
    read(buf, data.ecdsa_constraints);
    read(buf, data.blake2s_constraints);
    read(buf, data.pedersen_constraints);
    read(buf, data.hash_to_field_constraints);
    read(buf, data.fixed_base_scalar_mul_constraints);
    read(buf, data.constraints);
}

template <typename B> inline void write(B& buf, acir_format const& data)
{
    using serialize::write;
    write(buf, data.varnum);
    write(buf, data.public_inputs);
    write(buf, data.logic_constraints);
    write(buf, data.range_constraints);
    write(buf, data.sha256_constraints);
    write(buf, data.merkle_membership_constraints);
    write(buf, data.schnorr_constraints);
    write(buf, data.ecdsa_constraints);
    write(buf, data.blake2s_constraints);
    write(buf, data.pedersen_constraints);
    write(buf, data.hash_to_field_constraints);
    write(buf, data.fixed_base_scalar_mul_constraints);
    write(buf, data.constraints);
}

} // namespace acir_format
