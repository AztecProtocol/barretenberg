#pragma once
#include "logic_constraint.hpp"
#include "range_constraint.hpp"
#include "sha256_constraint.hpp"
#include "blake2s_constraint.hpp"
#include "fixed_base_scalar_mul.hpp"
#include "schnorr_verify.hpp"
#include "ecdsa_secp256k1.hpp"
#include "merkle_membership_constraint.hpp"
#include "pedersen.hpp"
#include "hash_to_field.hpp"
#include "barretenberg/stdlib/types/types.hpp"

namespace acir_format {

struct acir_format {
    // The number of witnesses in the circuit
    uint32_t varnum;

    std::vector<uint32_t> public_inputs;

    std::vector<FixedBaseScalarMul> fixed_base_scalar_mul_constraints;
    std::vector<LogicConstraint> logic_constraints;
    std::vector<RangeConstraint> range_constraints;
    std::vector<SchnorrConstraint> schnorr_constraints;
    std::vector<EcdsaSecp256k1Constraint> ecdsa_constraints;
    std::vector<Sha256Constraint> sha256_constraints;
    std::vector<Blake2sConstraint> blake2s_constraints;
    std::vector<HashToFieldConstraint> hash_to_field_constraints;
    std::vector<PedersenConstraint> pedersen_constraints;
    std::vector<MerkleMembershipConstraint> merkle_membership_constraints;
    // A standard plonk arithmetic constraint, as defined in the poly_triple struct, consists of selector values
    // for q_M,q_L,q_R,q_O,q_C and indices of three variables taking the role of left, right and output wire
    std::vector<poly_triple> constraints;

    friend bool operator==(acir_format const& lhs, acir_format const& rhs) = default;
};

void read_witness(plonk::stdlib::types::Composer& composer, std::vector<barretenberg::fr> witness);

void create_circuit(plonk::stdlib::types::Composer& composer, const acir_format& constraint_system);

plonk::stdlib::types::Composer create_circuit(const acir_format& constraint_system,
                                              std::unique_ptr<bonk::ReferenceStringFactory>&& crs_factory);

plonk::stdlib::types::Composer create_circuit_with_witness(const acir_format& constraint_system,
                                                           std::vector<fr> witness,
                                                           std::unique_ptr<ReferenceStringFactory>&& crs_factory);

plonk::stdlib::types::Composer create_circuit_with_witness(const acir_format& constraint_system,
                                                           std::vector<fr> witness);

void create_circuit_with_witness(plonk::stdlib::types::Composer& composer,
                                 const acir_format& constraint_system,
                                 std::vector<fr> witness);

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
