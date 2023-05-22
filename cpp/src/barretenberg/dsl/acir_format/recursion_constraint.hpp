#pragma once
#include <vector>
// #include "barretenberg/stdlib/types/types.hpp"
#include "barretenberg/dsl/types.hpp"
#include "barretenberg/plonk/proof_system/verification_key/verification_key.hpp"

namespace acir_format {

using namespace proof_system::plonk;

/**
 * @brief RecursionConstraint struct contains information required to recursively verify a proof!
 *
 * @details The recursive verifier algorithm produces an 'aggregation object' representing 2 G1 points, expressed as 16
 * witness values. The smart contract Verifier must be aware of this aggregation object in order to complete the full
 * recursive verification. If the circuit verifies more than 1 proof, the recursion algorithm will update a pre-existing
 * aggregation object (`input_aggregation_object`).
 *
 * @details We currently require that the inner circuit being verified only has a single public input. If more are
 * required, the outer circuit can hash them down to 1 input.
 *
 * @param verification_key_data The inner circuit vkey. Is converted into circuit witness values (internal to the
 * backend)
 * @param proof The plonk proof. Is converted into circuit witness values (internal to the backend)
 * @param is_aggregation_object_nonzero A flag to tell us whether the circuit has already recursively verified proofs
 * (and therefore an aggregation object is present)
 * @param public_input The index of the single public input
 * @param input_aggregation_object Witness indices of pre-existing aggregation object (if it exists)
 * @param output_aggregation_object Witness indices of the aggregation object produced by recursive verification
 * @param nested_aggregation_object Public input indices of an aggregation object inside the proof.
 *
 * @note If input_aggregation_object witness indices are all zero, we interpret this to mean that the inner proof does
 * NOT contain a previously recursively verified proof
 * @note nested_aggregation_object is used for cases where the proof being verified contains an aggregation object in
 * its public inputs! If this is the case, we record the public input locations in `nested_aggregation_object`. If the
 * inner proof is of a circuit that does not have a nested aggregation object, these values are all zero.
 *
 * To outline the interaction between the input_aggergation_object and the nested_aggregation_object take the following
 * example: If we have a circuit that verifies 2 proofs A and B, the recursion constraint for B will have an
 * input_aggregation_object that points to the aggregation output produced by verifying A. If circuit B also verifies a
 * proof, in the above example the recursion constraint for verifying B will have a nested object that describes the
 * aggregation object in B’s public inputs as well as an input aggregation object that points to the object produced by
 * the previous recursion constraint in the circuit (the one that verifies A)
 *
 */
struct RecursionConstraint {
    static constexpr size_t AGGREGATION_OBJECT_SIZE = 16; // 16 field elements
    std::vector<uint32_t> key;
    std::vector<uint32_t> proof;
    std::vector<uint32_t> public_inputs;
    uint32_t key_hash;
    std::array<uint32_t, AGGREGATION_OBJECT_SIZE> input_aggregation_object;
    std::array<uint32_t, AGGREGATION_OBJECT_SIZE> output_aggregation_object;
    std::array<uint32_t, AGGREGATION_OBJECT_SIZE> nested_aggregation_object;

    friend bool operator==(RecursionConstraint const& lhs, RecursionConstraint const& rhs) = default;
};

template <bool has_valid_witness_assignment = false>
void create_recursion_constraints(Composer& composer, const RecursionConstraint& input);

extern template void create_recursion_constraints<false>(Composer&, const RecursionConstraint&);
extern template void create_recursion_constraints<true>(Composer&, const RecursionConstraint&);

std::vector<barretenberg::fr> export_key_in_recursion_format(std::shared_ptr<verification_key> const& vkey);
std::vector<barretenberg::fr> export_dummy_key_in_recursion_format(const PolynomialManifest& polynomial_manifest,
                                                                   bool contains_recursive_proof = 0);

std::vector<barretenberg::fr> export_transcript_in_recursion_format(const transcript::StandardTranscript& transcript);
std::vector<barretenberg::fr> export_dummy_transcript_in_recursion_format(const transcript::Manifest& manifest,
                                                                          const bool contains_recursive_proof);

template <typename B> inline void read(B& buf, RecursionConstraint& constraint)
{
    using serialize::read;
    read(buf, constraint.key);
    read(buf, constraint.proof);
    read(buf, constraint.public_inputs);
    read(buf, constraint.key_hash);
    read(buf, constraint.input_aggregation_object);
    read(buf, constraint.output_aggregation_object);
    read(buf, constraint.nested_aggregation_object);
}

template <typename B> inline void write(B& buf, RecursionConstraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.key);
    write(buf, constraint.proof);
    write(buf, constraint.public_inputs);
    write(buf, constraint.key_hash);
    write(buf, constraint.input_aggregation_object);
    write(buf, constraint.output_aggregation_object);
    write(buf, constraint.nested_aggregation_object);
}

} // namespace acir_format
