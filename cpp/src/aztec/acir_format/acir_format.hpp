#include <common/serialize.hpp>
#include <plonk/composer/composer_base.hpp>
#include <plonk/composer/turbo_composer.hpp>
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

using namespace barretenberg;

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

void read_witness(TurboComposer& composer, std::vector<barretenberg::fr> witness);

void create_circuit(TurboComposer& composer, const acir_format& constraint_system);

TurboComposer create_circuit(const acir_format& constraint_system,
                             std::unique_ptr<bonk::ReferenceStringFactory>&& crs_factory);

TurboComposer create_circuit_with_witness(const acir_format& constraint_system,
                                          std::vector<fr> witness,
                                          std::unique_ptr<ReferenceStringFactory>&& crs_factory);

TurboComposer create_circuit_with_witness(const acir_format& constraint_system, std::vector<fr> witness);

void create_circuit_with_witness(TurboComposer& composer,
                                 const acir_format& constraint_system,
                                 std::vector<fr> witness);

// Serialisation
template <typename B> inline void read(B& buf, acir_format& data);

template <typename B> inline void write(B& buf, acir_format const& data);

} // namespace acir_format
