#include "ecdsa.hpp"
#include <barretenberg/common/streams.hpp>
#include <barretenberg/ecc/curves/secp256k1/secp256k1.hpp>

using namespace bb;
using namespace bb::crypto;

WASM_EXPORT void ecdsa__compute_public_key(uint8_t const* private_key, uint8_t* public_key_buf)
{
    auto priv_key = from_buffer<secp256k1::fr>(private_key);
    secp256k1::g1::affine_element pub_key = secp256k1::g1::one * priv_key;
    write(public_key_buf, pub_key);
}

WASM_EXPORT void ecdsa__construct_signature(uint8_t const* message,
                                            size_t msg_len,
                                            uint8_t const* private_key,
                                            uint8_t* output_sig_r,
                                            uint8_t* output_sig_s,
                                            uint8_t* output_sig_v)
{
    using serialize::write;
    auto priv_key = from_buffer<secp256k1::fr>(private_key);
    secp256k1::g1::affine_element pub_key = secp256k1::g1::one * priv_key;
    ecdsa_key_pair<secp256k1::fr, secp256k1::g1> key_pair = { priv_key, pub_key };

    auto sig = ecdsa_construct_signature<Sha256Hasher, secp256k1::fq, secp256k1::fr, secp256k1::g1>(
        std::string((char*)message, msg_len), key_pair);
    write(output_sig_r, sig.r);
    write(output_sig_s, sig.s);
    write(output_sig_v, sig.v);
}

WASM_EXPORT void ecdsa__recover_public_key_from_signature(uint8_t const* message,
                                                          size_t msg_len,
                                                          uint8_t const* sig_r,
                                                          uint8_t const* sig_s,
                                                          uint8_t* sig_v,
                                                          uint8_t* output_pub_key)
{
    std::array<uint8_t, 32> r, s;
    std::copy(sig_r, sig_r + 32, r.begin());
    std::copy(sig_s, sig_s + 32, s.begin());
    const uint8_t v = *sig_v;

    ecdsa_signature sig = { r, s, v };
    auto recovered_pub_key = ecdsa_recover_public_key<Sha256Hasher, secp256k1::fq, secp256k1::fr, secp256k1::g1>(
        std::string((char*)message, msg_len), sig);
    write(output_pub_key, recovered_pub_key);
}

WASM_EXPORT bool ecdsa__verify_signature(uint8_t const* message,
                                         size_t msg_len,
                                         uint8_t const* pub_key,
                                         uint8_t const* sig_r,
                                         uint8_t const* sig_s,
                                         uint8_t const* sig_v)
{
    auto pubk = from_buffer<secp256k1::g1::affine_element>(pub_key);
    std::array<uint8_t, 32> r, s;
    std::copy(sig_r, sig_r + 32, r.begin());
    std::copy(sig_s, sig_s + 32, s.begin());
    const uint8_t v = *sig_v;

    ecdsa_signature sig = { r, s, v };
    return ecdsa_verify_signature<Sha256Hasher, secp256k1::fq, secp256k1::fr, secp256k1::g1>(
        std::string((char*)message, msg_len), pubk, sig);
}

WASM_EXPORT void blackbox_secp256k1_verify_signature(uint256_t const* hashed_message,
                                                     size_t,
                                                     uint256_t const* pub_key_x,
                                                     uint256_t const* pub_key_y,
                                                     uint256_t const* sig,
                                                     uint256_t* result)
{
    // Create pub key.
    std::array<uint8_t, 32> hm_arr, x_arr, y_arr;
    for (size_t i = 0; i < 32; ++i) {
        hm_arr[i] = static_cast<uint8_t>(hashed_message[i].data[0]);
        x_arr[i] = static_cast<uint8_t>(pub_key_x[i].data[0]);
        y_arr[i] = static_cast<uint8_t>(pub_key_y[i].data[0]);
    }
    auto x = from_buffer<secp256k1::fq>(x_arr.data());
    auto y = from_buffer<secp256k1::fq>(y_arr.data());
    secp256k1::g1::affine_element pub_key(x, y);

    // Create signature.
    std::array<uint8_t, 32> r, s;
    for (size_t i = 0; i < 32; ++i) {
        r[i] = static_cast<uint8_t>(sig[i].data[0]);
        s[i] = static_cast<uint8_t>(sig[i + 32].data[0]);
    }
    const uint8_t v = 0;
    ecdsa_signature signature = { r, s, v };

    *result = ecdsa_verify_signature<secp256k1::fq, secp256k1::fr, secp256k1::g1>(hm_arr, pub_key, signature);
}

WASM_EXPORT void blackbox_secp256r1_verify_signature(uint256_t const* hashed_message,
                                                     size_t,
                                                     uint256_t const* pub_key_x,
                                                     uint256_t const* pub_key_y,
                                                     uint256_t const* sig,
                                                     uint256_t* result)
{
    // Create pub key.
    std::array<uint8_t, 32> hm_arr, x_arr, y_arr;
    for (size_t i = 0; i < 32; ++i) {
        hm_arr[i] = static_cast<uint8_t>(hashed_message[i].data[0]);
        x_arr[i] = static_cast<uint8_t>(pub_key_x[i].data[0]);
        y_arr[i] = static_cast<uint8_t>(pub_key_y[i].data[0]);
    }
    auto x = from_buffer<secp256r1::fq>(x_arr.data());
    auto y = from_buffer<secp256r1::fq>(y_arr.data());
    secp256r1::g1::affine_element pub_key(x, y);

    // Create signature.
    std::array<uint8_t, 32> r, s;
    for (size_t i = 0; i < 32; ++i) {
        r[i] = static_cast<uint8_t>(sig[i].data[0]);
        s[i] = static_cast<uint8_t>(sig[i + 32].data[0]);
    }
    const uint8_t v = 0;
    ecdsa_signature signature = { r, s, v };

    *result = ecdsa_verify_signature<secp256r1::fq, secp256r1::fr, secp256r1::g1>(hm_arr, pub_key, signature);
}