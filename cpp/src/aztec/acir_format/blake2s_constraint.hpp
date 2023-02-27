#include <common/serialize.hpp>
#include <common/assert.hpp>
#include <plonk/composer/composer_base.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include <stdlib/hash/blake2s/blake2s.hpp>
#include <crypto/blake2s/blake2s.hpp>
#include <stdlib/types/types.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>

using namespace plonk::stdlib::types;
using namespace barretenberg;

namespace acir_format {

struct Blake2sInput {
    uint32_t witness;
    uint32_t num_bits;

    friend bool operator==(Blake2sInput const& lhs, Blake2sInput const& rhs) = default;
};

struct Blake2sConstraint {
    std::vector<Blake2sInput> inputs;
    std::vector<uint32_t> result;

    friend bool operator==(Blake2sConstraint const& lhs, Blake2sConstraint const& rhs) = default;
};

void create_blake2s_constraints(plonk::TurboComposer& composer, const Blake2sConstraint& constraint);

template <typename B> inline void read(B& buf, Blake2sInput& constraint);

template <typename B> inline void write(B& buf, Blake2sInput const& constraint);

template <typename B> inline void read(B& buf, Blake2sConstraint& constraint);

template <typename B> inline void write(B& buf, Blake2sConstraint const& constraint);

} // namespace acir_format
