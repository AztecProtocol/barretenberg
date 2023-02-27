#include "hash_to_field.hpp"
#include "round.hpp"

using namespace plonk::stdlib::types;
using namespace barretenberg;

namespace acir_format {

void create_hash_to_field_constraints(plonk::TurboComposer& composer, const HashToFieldConstraint constraint)
{

    // // Create byte array struct
    byte_array_ct arr(&composer);

    // // Get the witness assignment for each witness index
    // // Write the witness assignment to the byte_array
    for (const auto& witness_index_num_bits : constraint.inputs) {
        auto witness_index = witness_index_num_bits.witness;
        auto num_bits = witness_index_num_bits.num_bits;

        // XXX: The implementation requires us to truncate the element to the nearest byte and not bit
        auto num_bytes = round_to_nearest_byte(num_bits);

        field_ct element = field_ct::from_witness_index(&composer, witness_index);
        byte_array_ct element_bytes(element, num_bytes);
        byte_array_ct reversed_bytes = element_bytes.reverse();

        arr.write(reversed_bytes);
    }

    // Hash To Field using blake2s.
    // Note: It does not need to be blake2s in the future

    byte_array_ct out_bytes = plonk::stdlib::blake2s<plonk::TurboComposer>(arr);
    // TODO(Blaine): Switched this from field_t to field_ct, is it correct?
    field_ct out(out_bytes);
    // TODO(Blaine): Switched this from field_t to field_ct, is it correct?
    field_ct normalised_out = out.normalize();

    composer.copy_from_to(normalised_out.witness_index, constraint.result);
}

template <typename B> inline void read(B& buf, HashToFieldInput& constraint)
{
    using serialize::read;
    read(buf, constraint.witness);
    read(buf, constraint.num_bits);
}

template <typename B> inline void write(B& buf, HashToFieldInput const& constraint)
{
    using serialize::write;
    write(buf, constraint.witness);
    write(buf, constraint.num_bits);
}

template <typename B> inline void read(B& buf, HashToFieldConstraint& constraint)
{
    using serialize::read;
    read(buf, constraint.inputs);
    read(buf, constraint.result);
}

template <typename B> inline void write(B& buf, HashToFieldConstraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.inputs);
    write(buf, constraint.result);
}

} // namespace acir_format
