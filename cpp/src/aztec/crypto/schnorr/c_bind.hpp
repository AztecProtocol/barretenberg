#include <stddef.h>
#include <stdint.h>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void compute_public_key(uint8_t const* private_key, uint8_t* public_key_buf);

WASM_EXPORT void construct_signature(
    uint8_t const* message, size_t msg_len, uint8_t const* private_key, uint8_t* s, uint8_t* e);

WASM_EXPORT bool verify_signature(
    uint8_t const* message, size_t msg_len, uint8_t const* pub_key, uint8_t const* sig_s, uint8_t const* sig_e);

WASM_EXPORT void multisig_create_multisig_public_key(uint8_t const* private_key, uint8_t* multisig_pubkey_buf);

WASM_EXPORT void multisig_validate_and_combine_signer_pubkeys(uint8_t const* signer_pubkey_buf,
                                                              uint8_t* combined_key_buf);
WASM_EXPORT void multisig_construct_signature_round_1(uint8_t* round_one_public_output_buf,
                                                      uint8_t* round_one_private_output_buf);
WASM_EXPORT void multisig_construct_signature_round_2(uint8_t const* message,
                                                      size_t msg_len,
                                                      uint8_t* const private_key,
                                                      uint8_t* const signer_round_one_private_buf,
                                                      uint8_t* const signer_pubkeys_buf,
                                                      uint8_t* const round_one_public_buf,
                                                      uint8_t* round_two_buf);
WASM_EXPORT void multisig_combine_signatures(uint8_t const* message,
                                             size_t msg_len,
                                             uint8_t* const signer_pubkeys_buf,
                                             uint8_t* const round_one_buf,
                                             uint8_t* const round_two_buf,
                                             uint8_t* s,
                                             uint8_t* e);
}