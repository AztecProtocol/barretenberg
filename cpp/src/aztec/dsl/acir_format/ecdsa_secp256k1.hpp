#pragma once
#include <stdlib/types/types.hpp>

namespace acir_format {

struct EcdsaSecp256k1Constraint {
    // This is just a bunch of bytes
    // which need to be interpreted as a string
    // Note this must be a bunch of bytes
    std::vector<uint32_t> message;

    // This is the supposed public key which signed the
    // message, giving rise to the signature.
    // Since Fr does not have enough bits to represent
    // the prime field in secp256k1, a byte array is used.
    // Can also use low and hi where lo=128 bits
    std::vector<uint32_t> pub_x_indices;
    std::vector<uint32_t> pub_y_indices;

    // This is the result of verifying the signature
    uint32_t result;

    // This is the computed signature
    //
    std::vector<uint32_t> signature;

    friend bool operator==(EcdsaSecp256k1Constraint const& lhs, EcdsaSecp256k1Constraint const& rhs) = default;
};

void create_ecdsa_verify_constraints(plonk::TurboComposer& composer, const EcdsaSecp256k1Constraint& input);

template <typename B> inline void read(B& buf, EcdsaSecp256k1Constraint& constraint);

template <typename B> inline void write(B& buf, EcdsaSecp256k1Constraint const& constraint);

} // namespace acir_format
