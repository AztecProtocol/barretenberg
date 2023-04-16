#pragma once
#include <vector>
#include "barretenberg/stdlib/types/types.hpp"
#include "barretenberg/plonk/proof_system//verification_key/verification_key.hpp"

namespace acir_format {

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
 * @param output_aggregation_object Witness indecies of the aggregation object produced by recursive verification
 */
struct RecursionConstraint {
    static constexpr size_t AGGREGATION_OBJECT_SIZE = 16; // 16 field elements
    std::vector<uint32_t> key;
    std::vector<uint32_t> proof;
    bool is_aggregation_object_nonzero;
    uint32_t public_input;
    std::array<uint32_t, AGGREGATION_OBJECT_SIZE> input_aggregation_object;
    std::array<uint32_t, AGGREGATION_OBJECT_SIZE> output_aggregation_object;

    friend bool operator==(RecursionConstraint const& lhs, RecursionConstraint const& rhs) = default;
};

void create_recursion_constraints(plonk::stdlib::types::Composer& composer, const RecursionConstraint& input);

template <typename B> inline void read(B& buf, RecursionConstraint& constraint)
{
    using serialize::read;
    read(buf, constraint.key);
    read(buf, constraint.proof);
    read(buf, constraint.is_aggregation_object_nonzero);
    read(buf, constraint.public_input);
    read(buf, constraint.input_aggregation_object);
    read(buf, constraint.output_aggregation_object);
}

template <typename B> inline void write(B& buf, RecursionConstraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.key);
    write(buf, constraint.proof);
    write(buf, constraint.is_aggregation_object_nonzero);
    write(buf, constraint.public_input);
    write(buf, constraint.input_aggregation_object);
    write(buf, constraint.output_aggregation_object);
}

} // namespace acir_format
