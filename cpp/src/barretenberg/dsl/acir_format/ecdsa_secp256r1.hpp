// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#pragma once
#include "barretenberg/common/serialize.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace acir_format {

struct EcdsaSecp256r1Constraint {
    // This is the byte representation of the hashed message.
    std::array<uint32_t, 32> hashed_message;

    // This is the supposed public key which signed the
    // message, giving rise to the signature.
    // Since Fr does not have enough bits to represent
    // the prime field in secp256r1, a byte array is used.
    // Can also use low and hi where lo=128 bits
    std::array<uint32_t, 32> pub_x_indices;
    std::array<uint32_t, 32> pub_y_indices;

    // This is the result of verifying the signature
    uint32_t result;

    // This is the computed signature
    //
    std::array<uint32_t, 64> signature;

    friend bool operator==(EcdsaSecp256r1Constraint const& lhs, EcdsaSecp256r1Constraint const& rhs) = default;
};

template <typename Builder>
void create_ecdsa_r1_verify_constraints(Builder& builder,
                                        const EcdsaSecp256r1Constraint& input,
                                        bool has_valid_witness_assignments = true);

template <typename Builder> void dummy_ecdsa_constraint(Builder& builder, EcdsaSecp256r1Constraint const& input);

template <typename B> inline void read(B& buf, EcdsaSecp256r1Constraint& constraint)
{
    using serialize::read;
    read(buf, constraint.hashed_message);
    read(buf, constraint.signature);
    read(buf, constraint.pub_x_indices);
    read(buf, constraint.pub_y_indices);
    read(buf, constraint.result);
}

template <typename B> inline void write(B& buf, EcdsaSecp256r1Constraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.hashed_message);
    write(buf, constraint.signature);
    write(buf, constraint.pub_x_indices);
    write(buf, constraint.pub_y_indices);
    write(buf, constraint.result);
}

} // namespace acir_format
