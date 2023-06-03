#pragma once
#include <vector>
#include "barretenberg/dsl/types.hpp"

namespace acir_format {

using namespace proof_system::plonk;

/**
 * @brief RecursionConstraint struct contains information required to recursively verify a proof!
 *
 * Implementation of this constraint can be found on barretenberg master. This branch (`acvm-backend-barretenberg`)
 * is temporary until Noir fully moves to bb.js (and other dynamic backends) as its prover. This struct is defined in
 * order to match up the acir format used by `master` and `acvm-backend-barretenberg` while the temporary branch still
 * exists.
 */
struct RecursionConstraint {
    // An aggregation state is represented by two G1 affine elements. Each G1 point has
    // two field element coordinates (x, y). Thus, four field elements
    static constexpr size_t NUM_AGGREGATION_ELEMENTS = 4;
    // Four limbs are used when simulating a non-native field using the bigfield class
    static constexpr size_t AGGREGATION_OBJECT_SIZE =
        NUM_AGGREGATION_ELEMENTS * NUM_QUOTIENT_PARTS; // 16 field elements
    std::vector<uint32_t> key;
    std::vector<uint32_t> proof;
    std::vector<uint32_t> public_inputs;
    uint32_t key_hash;
    std::array<uint32_t, AGGREGATION_OBJECT_SIZE> input_aggregation_object;
    std::array<uint32_t, AGGREGATION_OBJECT_SIZE> output_aggregation_object;
    std::array<uint32_t, AGGREGATION_OBJECT_SIZE> nested_aggregation_object;

    friend bool operator==(RecursionConstraint const& lhs, RecursionConstraint const& rhs) = default;
};

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