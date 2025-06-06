// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#pragma once
#include "barretenberg/crypto/poseidon2/poseidon2_params.hpp"
#include "barretenberg/stdlib/hash/poseidon2/sponge/sponge.hpp"
#include "barretenberg/stdlib/primitives/byte_array/byte_array.hpp"
#include "barretenberg/stdlib/primitives/field/field.hpp"

#include "../../primitives/circuit_builders/circuit_builders.hpp"

namespace bb::stdlib {

/**
 * @brief stdlib class that evaluates in-circuit poseidon2 hashes, consistent with behavior in
 * crypto::poseidon2
 *
 * @tparam Builder
 */
template <typename Builder> class poseidon2 {

  private:
    using field_ct = stdlib::field_t<Builder>;
    using bool_ct = stdlib::bool_t<Builder>;
    using Params = crypto::Poseidon2Bn254ScalarFieldParams;
    using Permutation = Poseidon2Permutation<Params, Builder>;
    // We choose our rate to be t-1 and capacity to be 1.
    using Sponge = FieldSponge<Params::t - 1, 1, Params::t, Permutation, Builder>;

  public:
    static field_ct hash(Builder& builder, const std::vector<field_ct>& in);
    static field_ct hash_buffer(Builder& builder, const stdlib::byte_array<Builder>& input);
};

} // namespace bb::stdlib
