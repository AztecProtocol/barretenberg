#pragma once
#include <common/serialize.hpp>
#include <common/assert.hpp>
#include <plonk/composer/composer_base.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include <stdlib/encryption/ecdsa/ecdsa.hpp>
#include <crypto/ecdsa/ecdsa.hpp>
#include <stdlib/types/types.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>
#include <ecc/curves/secp256k1/secp256k1.hpp>

using namespace plonk::stdlib::types;
using namespace barretenberg;

namespace acir_format {

crypto::ecdsa::signature ecdsa_convert_signature(plonk::TurboComposer& composer, std::vector<uint32_t> signature)
{

    crypto::ecdsa::signature signature_cr;

    // Get the witness assignment for each witness index
    // Write the witness assignment to the byte_array

    for (unsigned int i = 0; i < 32; i++) {
        auto witness_index = signature[i];

        std::vector<uint8_t> fr_bytes(sizeof(fr));

        fr value = composer.get_variable(witness_index);

        fr::serialize_to_buffer(value, &fr_bytes[0]);

        signature_cr.r[i] = fr_bytes.back();
    }

    for (unsigned int i = 32; i < 64; i++) {
        auto witness_index = signature[i];

        std::vector<uint8_t> fr_bytes(sizeof(fr));

        fr value = composer.get_variable(witness_index);

        fr::serialize_to_buffer(value, &fr_bytes[0]);

        signature_cr.s[i - 32] = fr_bytes.back();
    }

    return signature_cr;
}

// fq, fr, g1

secp256k1_ct::g1_ct ecdsa_convert_inputs(plonk::TurboComposer* ctx, const secp256k1::g1::affine_element& input)
{
    uint256_t x_u256(input.x);
    uint256_t y_u256(input.y);
    secp256k1_ct::fq_ct x(witness_ct(ctx, barretenberg::fr(x_u256.slice(0, secp256k1_ct::fq_ct::NUM_LIMB_BITS * 2))),
                          witness_ct(ctx,
                                     barretenberg::fr(x_u256.slice(secp256k1_ct::fq_ct::NUM_LIMB_BITS * 2,
                                                                   secp256k1_ct::fq_ct::NUM_LIMB_BITS * 4))));
    secp256k1_ct::fq_ct y(witness_ct(ctx, barretenberg::fr(y_u256.slice(0, secp256k1_ct::fq_ct::NUM_LIMB_BITS * 2))),
                          witness_ct(ctx,
                                     barretenberg::fr(y_u256.slice(secp256k1_ct::fq_ct::NUM_LIMB_BITS * 2,
                                                                   secp256k1_ct::fq_ct::NUM_LIMB_BITS * 4))));

    return secp256k1_ct::g1_ct(x, y);
}

// vector of bytes here, assumes that the witness indices point to a field element which can be represented
// with just a byte.
// notice that this function truncates each field_element to a byte
byte_array_ct ecdsa_vector_of_bytes_to_byte_array(plonk::TurboComposer& composer, std::vector<uint32_t> vector_of_bytes)
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
witness_ct ecdsa_index_to_witness(plonk::TurboComposer& composer, uint32_t index)
{
    fr value = composer.get_variable(index);
    return witness_ct(&composer, value);
}

struct EcdsaSecp256k1Constraint {
    // This is just a bunch of bytes
    // which need to be interpreted as a string
    // Note this must be a bunch of bytes
    std::vector<uint32_t> message;

    // This is the supposed public key which signed the
    // message, giving rise to the signature.
    // Since Fr does not have enough bits to represent
    // the prime field in secp256k1, a byte array is used.
    // Can also use low and hi where lo=128 bits
    std::vector<uint32_t> pub_x_indices;
    std::vector<uint32_t> pub_y_indices;

    // This is the result of verifying the signature
    uint32_t result;

    // This is the computed signature
    //
    std::vector<uint32_t> signature;

    friend bool operator==(EcdsaSecp256k1Constraint const& lhs, EcdsaSecp256k1Constraint const& rhs) = default;
};

void create_ecdsa_verify_constraints(plonk::TurboComposer& composer, const EcdsaSecp256k1Constraint& input)
{

    auto new_sig = ecdsa_convert_signature(composer, input.signature);

    auto message = ecdsa_vector_of_bytes_to_byte_array(composer, input.message);
    auto pub_key_x_byte_arr = ecdsa_vector_of_bytes_to_byte_array(composer, input.pub_x_indices);
    auto pub_key_y_byte_arr = ecdsa_vector_of_bytes_to_byte_array(composer, input.pub_y_indices);

    auto pub_key_x_fq = secp256k1_ct::fq_ct(pub_key_x_byte_arr);
    auto pub_key_y_fq = secp256k1_ct::fq_ct(pub_key_y_byte_arr);

    std::vector<uint8_t> rr(new_sig.r.begin(), new_sig.r.end());
    std::vector<uint8_t> ss(new_sig.s.begin(), new_sig.s.end());

    stdlib::ecdsa::signature<plonk::TurboComposer> sig{ stdlib::byte_array<plonk::TurboComposer>(&composer, rr),
                                                        stdlib::byte_array<plonk::TurboComposer>(&composer, ss) };

    auto pub_key = secp256k1_ct::g1_ct(pub_key_x_fq, pub_key_y_fq);

    composer.copy_from_to(false, input.result);
}

template <typename B> inline void read(B& buf, EcdsaSecp256k1Constraint& constraint)
{
    using serialize::read;
    read(buf, constraint.message);
    read(buf, constraint.signature);
    read(buf, constraint.pub_x_indices);
    read(buf, constraint.pub_y_indices);
    read(buf, constraint.result);
}

template <typename B> inline void write(B& buf, EcdsaSecp256k1Constraint const& constraint)
{
    using serialize::write;
    write(buf, constraint.message);
    write(buf, constraint.signature);
    write(buf, constraint.pub_x_indices);
    write(buf, constraint.pub_y_indices);
    write(buf, constraint.result);
}

} // namespace acir_format
