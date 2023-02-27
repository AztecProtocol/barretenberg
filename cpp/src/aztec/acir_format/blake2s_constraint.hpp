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

// Rounds a number to the nearest multiple of 8
uint32_t blake_round_to_nearest_mul_8(uint32_t num_bits)
{
    uint32_t remainder = num_bits % 8;
    if (remainder == 0)
        return num_bits;

    return num_bits + 8 - remainder;
}
// Rounds the number of bits to the nearest byte
uint32_t blake_round_to_nearest_byte(uint32_t num_bits)
{
    return blake_round_to_nearest_mul_8(num_bits) / 8;
}

struct Blake2sInput {
    uint32_t witness;
    uint32_t num_bits;
};

struct Blake2sConstraint {
    std::vector<Blake2sInput> inputs;
    std::vector<uint32_t> result;
};

void create_blake2s_constraints(plonk::TurboComposer& composer, const Blake2sConstraint& constraint)
{

    // // Create byte array struct
    byte_array_ct arr(&composer);

    // // Get the witness assignment for each witness index
    // // Write the witness assignment to the byte_array
    for (const auto& witness_index_num_bits : constraint.inputs) {
        auto witness_index = witness_index_num_bits.witness;
        auto num_bits = witness_index_num_bits.num_bits;

        // XXX: The implementation requires us to truncate the element to the nearest byte and not bit
        auto num_bytes = blake_round_to_nearest_byte(num_bits);

        field_ct element = field_ct::from_witness_index(&composer, witness_index);
        byte_array_ct element_bytes(element, num_bytes);

        arr.write(element_bytes);
    }

    byte_array_ct output_bytes = plonk::stdlib::blake2s<plonk::TurboComposer>(arr);

    // Convert byte array to vector of field_t
    auto bytes = output_bytes.bytes();

    for (size_t i = 0; i < bytes.size(); ++i) {
        composer.copy_from_to(bytes[i].normalize().witness_index, constraint.result[i]);
    }
}

template <typename B> inline void read(B& buf, Blake2sInput& constraint)
{
    using serialize::read;
    read(buf, constraint.witness);
    read(buf, constraint.num_bits);
}

template <typename B> inline void write(B& buf, Blake2sInput const& constraint)
{
    using serialize::write;
    write(buf, constraint.witness);
    write(buf, constraint.num_bits);
}

inline bool operator==(Blake2sInput const& lhs, Blake2sInput const& rhs) = default;

template <typename B> inline void read(B& buf, Blake2sConstraint& constraint)
{
    using serialize::read;
    read(buf, constraint.inputs);
    read(buf, constraint.result);
}

template <typename B> inline void write(B& buf, Blake2sConstraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.inputs);
    write(buf, constraint.result);
}

inline bool operator==(Blake2sConstraint const& lhs, Blake2sConstraint const& rhs) = default;
