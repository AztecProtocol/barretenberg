#include "schnorr_verify.hpp"
#include "barretenberg/stdlib/encryption/schnorr/schnorr.hpp"
#include "barretenberg/crypto/schnorr/schnorr.hpp"

namespace acir_format {

using namespace proof_system::plonk::stdlib;

// vector of bytes here, assumes that the witness indices point to a field element which can be represented
// with just a byte.
// notice that this function truncates each field_element to a byte
byte_array_ct vector_of_bytes_to_byte_array(Composer& composer, std::vector<uint32_t> vector_of_bytes)
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
witness_ct index_to_witness(Composer& composer, uint32_t index)
{
    fr value = composer.get_variable(index);
    return { &composer, value };
}

void create_schnorr_verify_constraints(Composer& composer, const SchnorrConstraint& input)
{
    // From ignorance, you will see me convert a bunch of witnesses from ByteArray -> BitArray
    // This may not be the most efficient way to do it. It is being used as it is known to work,
    // optimisations are welcome!

    // First convert the message of u8 witnesses into a byte_array
    // Do this by taking each element as a u8 and writing it to the byte array

    auto message = vector_of_bytes_to_byte_array(composer, input.message);

    fr pubkey_value_x = composer.get_variable(input.public_key_x);
    fr pubkey_value_y = composer.get_variable(input.public_key_y);

    point_ct pub_key{ witness_ct(&composer, pubkey_value_x), witness_ct(&composer, pubkey_value_y) };

    fr sig_s = composer.get_variable(input.signature_s);
    fr sig_e = composer.get_variable(input.signature_e);

    crypto::schnorr::signature new_sig;
    fr::serialize_to_buffer(sig_s, &new_sig.s[0]);
    fr::serialize_to_buffer(sig_e, &new_sig.e[0]);

    schnorr_signature_bits_ct sig = schnorr::convert_signature(&composer, new_sig);

    bool_ct signature_result = schnorr::signature_verification_result(message, pub_key, sig);

    bool_ct signature_result_normalized = signature_result.normalize();

    composer.assert_equal(signature_result_normalized.witness_index, input.result);
}

} // namespace acir_format
