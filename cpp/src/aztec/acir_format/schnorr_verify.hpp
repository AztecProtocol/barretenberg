#include <stdlib/types/types.hpp>

namespace acir_format {

struct SchnorrConstraint {
    // This is just a bunch of bytes
    // which need to be interpreted as a string
    // Note this must be a bunch of bytes
    std::vector<uint32_t> message;

    // This is the supposed public key which signed the
    // message, giving rise to the signature
    uint32_t public_key_x;
    uint32_t public_key_y;

    // This is the result of verifying the signature
    uint32_t result;

    // This is the computed signature
    //
    std::vector<uint32_t> signature;

    friend bool operator==(SchnorrConstraint const& lhs, SchnorrConstraint const& rhs) = default;
};

void create_schnorr_verify_constraints(plonk::TurboComposer& composer, const SchnorrConstraint& input);

template <typename B> inline void read(B& buf, SchnorrConstraint& constraint);

template <typename B> inline void write(B& buf, SchnorrConstraint const& constraint);

} // namespace acir_format
