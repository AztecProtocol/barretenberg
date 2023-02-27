#include <common/serialize.hpp>
#include <common/assert.hpp>
#include <plonk/composer/composer_base.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include <stdlib/encryption/schnorr/schnorr.hpp>
#include <stdlib/types/types.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>

using namespace plonk::stdlib::types;
using namespace barretenberg;

namespace acir_format {

crypto::schnorr::signature convert_signature(plonk::TurboComposer& composer, std::vector<uint32_t> signature)
{

    crypto::schnorr::signature signature_cr;

    // Get the witness assignment for each witness index
    // Write the witness assignment to the byte_array

    for (unsigned int i = 0; i < 32; i++) {
        auto witness_index = signature[i];

        std::vector<uint8_t> fr_bytes(sizeof(fr));

        fr value = composer.get_variable(witness_index);

        fr::serialize_to_buffer(value, &fr_bytes[0]);

        signature_cr.s[i] = fr_bytes.back();
    }

    for (unsigned int i = 32; i < 64; i++) {
        auto witness_index = signature[i];

        std::vector<uint8_t> fr_bytes(sizeof(fr));

        fr value = composer.get_variable(witness_index);

        fr::serialize_to_buffer(value, &fr_bytes[0]);

        signature_cr.e[i - 32] = fr_bytes.back();
    }

    return signature_cr;
}
// vector of bytes here, assumes that the witness indices point to a field element which can be represented
// with just a byte.
// notice that this function truncates each field_element to a byte
byte_array_ct vector_of_bytes_to_byte_array(plonk::TurboComposer& composer, std::vector<uint32_t> vector_of_bytes)
{
    byte_array_ct arr(&composer);

    // Get the witness assignment for each witness index
    // Write the witness assignment to the byte_array
    for (const auto& witness_index : vector_of_bytes) {

        field_ct element = field_ct::from_witness_index(&composer, witness_index);
        size_t num_bytes = 1;

        byte_array_ct element_bytes(element, num_bytes);
        arr.write(element_bytes);
    }
    return arr;
}
witness_ct index_to_witness(plonk::TurboComposer& composer, uint32_t index)
{
    fr value = composer.get_variable(index);
    return witness_ct(&composer, value);
}

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

void create_schnorr_verify_constraints(plonk::TurboComposer& composer, const SchnorrConstraint& input)
{

    auto new_sig = convert_signature(composer, input.signature);
    // From ignorance, you will see me convert a bunch of witnesses from ByteArray -> BitArray
    // This may not be the most efficient way to do it. It is being used as it is known to work,
    // optimisations are welcome!

    // First convert the message of u8 witnesses into a byte_array
    // Do this by taking each element as a u8 and writing it to the byte array

    auto message = vector_of_bytes_to_byte_array(composer, input.message);

    fr pubkey_value_x = composer.get_variable(input.public_key_x);
    fr pubkey_value_y = composer.get_variable(input.public_key_y);

    point_ct pub_key{ witness_ct(&composer, pubkey_value_x), witness_ct(&composer, pubkey_value_y) };

    schnorr::signature_bits sig = stdlib::schnorr::convert_signature(&composer, new_sig);

    // This seems to fail if the signature is not correct.
    // What would be the point of the boolean then?
    bool_ct signature_result = stdlib::schnorr::signature_verification_result(message, pub_key, sig);
    // signature_result does not return a witness, as the whole circuit will
    // fail, given a wrong signature.
    //
    // We therefore necessarily have a witness disconnect here

    // TODO(Blaine): This seems wrong
    auto result_bool = composer.add_variable(signature_result.witness_bool == true);

    composer.copy_from_to(result_bool, input.result);
}

template <typename B> inline void read(B& buf, SchnorrConstraint& constraint)
{
    using serialize::read;
    read(buf, constraint.message);
    read(buf, constraint.signature);
    read(buf, constraint.public_key_x);
    read(buf, constraint.public_key_y);
    read(buf, constraint.result);
}

template <typename B> inline void write(B& buf, SchnorrConstraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.message);
    write(buf, constraint.signature);
    write(buf, constraint.public_key_x);
    write(buf, constraint.public_key_y);
    write(buf, constraint.result);
}

} // namespace acir_format
