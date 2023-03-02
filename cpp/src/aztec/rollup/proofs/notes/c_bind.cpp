#include "native/index.hpp"
#include <ecc/curves/grumpkin/grumpkin.hpp>
#include <crypto/sha256/sha256.hpp>
#include <crypto/aes128/aes128.hpp>

using namespace barretenberg;
using namespace rollup::proofs::notes::native;

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void notes__value_note_partial_commitment(uint8_t const* note_secret_buffer,
                                                      uint8_t const* public_key_buffer,
                                                      uint8_t const* creator_pubkey_buffer,
                                                      bool account_required,
                                                      uint8_t* output)
{
    auto note_secret = from_buffer<fr>(note_secret_buffer);
    auto public_key = from_buffer<grumpkin::g1::affine_element>(public_key_buffer);
    auto creator_pubkey = from_buffer<fr>(creator_pubkey_buffer);
    auto partial_state = value::create_partial_commitment(note_secret, public_key, account_required, creator_pubkey);
    write(output, partial_state);
}

WASM_EXPORT void notes__value_note_commitment(uint8_t const* note_buffer, uint8_t* output)
{
    auto note = from_buffer<value::value_note>(note_buffer);
    auto note_commitment = note.commit();
    write(output, note_commitment);
}

WASM_EXPORT void notes__value_note_nullifier(uint8_t const* commitment_buffer,
                                             uint8_t* acc_pk_buffer,
                                             bool is_real,
                                             uint8_t* output)
{
    auto commitment = from_buffer<grumpkin::fq>(commitment_buffer);
    auto acc_pk = from_buffer<uint256_t>(acc_pk_buffer);
    auto nullifier = compute_nullifier(commitment, acc_pk, is_real);
    write(output, nullifier);
}

WASM_EXPORT void notes__claim_note_partial_commitment(uint8_t const* note_buffer, uint8_t* output)
{
    auto note = from_buffer<claim::claim_note>(note_buffer);
    auto note_commitment = note.partial_commit();
    write(output, note_commitment);
}

WASM_EXPORT void notes__claim_note_nullifier(uint8_t const* commitment_buffer, uint8_t* output)
{
    auto commitment = from_buffer<grumpkin::fq>(commitment_buffer);
    auto nullifier = claim::compute_nullifier(commitment);
    write(output, nullifier);
}

WASM_EXPORT void notes__claim_note_complete_partial_commitment(uint8_t const* commitment_buffer,
                                                               uint32_t interaction_nonce,
                                                               uint8_t* fee,
                                                               uint8_t* output)
{
    auto commitment = from_buffer<grumpkin::fq>(commitment_buffer);
    auto claim_fee = from_buffer<uint256_t>(fee);
    auto enc_note = claim::complete_partial_commitment(commitment, interaction_nonce, claim_fee);
    write(output, enc_note);
}

WASM_EXPORT void notes__defi_interaction_note_commitment(uint8_t const* note_buffer, uint8_t* output)
{
    auto note = from_buffer<defi_interaction::note>(note_buffer);
    auto commitment = note.commit();
    write(output, commitment);
}

/**
 * This decrypts the AES encryption of the notes using the private keys of a user.
 * The notes owned by a user are stored in two forms:
 *   (i) a Pedersen commitment to the note which is inserted in the data tree
 *  (ii) an AES encryption of the note data
 * We need the AES encryption of the note to allow users to "view" the notes owned by them.
 */
WASM_EXPORT void notes__batch_decrypt_notes(uint8_t const* encrypted_notes_buffer,
                                            uint8_t* private_key_buffer,
                                            uint32_t numKeys,
                                            uint8_t* output)
{
    constexpr size_t AES_CIPHERTEXT_LENGTH = 80;
    std::vector<uint8_t> aes_messages(AES_CIPHERTEXT_LENGTH * numKeys);
    std::vector<grumpkin::g1::affine_element> ephemeral_public_keys;
    ephemeral_public_keys.reserve(numKeys);
    grumpkin::fr private_key = from_buffer<grumpkin::fr>(private_key_buffer);

    uint8_t const* note_ptr = encrypted_notes_buffer;
    uint8_t* aes_ptr = &aes_messages[0];
    std::vector<bool> key_on_curve;
    key_on_curve.reserve(numKeys);
    for (size_t i = 0; i < numKeys; ++i) {
        auto pubkey = from_buffer<grumpkin::g1::affine_element>(note_ptr + AES_CIPHERTEXT_LENGTH);
        key_on_curve.push_back(pubkey.on_curve());
        ephemeral_public_keys.emplace_back(pubkey);
        memcpy(aes_ptr, note_ptr, AES_CIPHERTEXT_LENGTH);
        note_ptr += (AES_CIPHERTEXT_LENGTH + 64);
        aes_ptr += AES_CIPHERTEXT_LENGTH;
    }

    const auto shared_secrets = grumpkin::g1::element::batch_mul_with_endomorphism(ephemeral_public_keys, private_key);

    uint8_t* output_ptr = output;
    for (size_t i = 0; i < numKeys; ++i) {
        if (key_on_curve[i]) {
            std::vector<uint8_t> secret_buffer = to_buffer<grumpkin::g1::affine_element>(shared_secrets[i]);
            secret_buffer.emplace_back(1); // we append 1 to the shared secret buffer when deriving aes decryption keys

            auto secret_hash = sha256::sha256(secret_buffer);

            uint8_t* aes_key = &secret_hash[0];
            uint8_t aes_iv[16];
            // copy the aes_iv out of secret_hash. We need it for later and `decrypt_buffer_cbc` will mutate the iv
            memcpy(&aes_iv[0], &secret_hash[16], 16);
            uint8_t* aes_message = &aes_messages[i * AES_CIPHERTEXT_LENGTH];

            crypto::aes128::decrypt_buffer_cbc(aes_message, &aes_iv[0], aes_key, AES_CIPHERTEXT_LENGTH);

            bool iv_match = true;
            for (size_t j = 0; j < 8; ++j) {
                iv_match = iv_match && (aes_message[j] == secret_hash[j + 16]);
            }
            output_ptr[0] = iv_match ? 1 : 0;
            memcpy(output_ptr + 1, aes_message + 8, 72);
        } else {
            memset(output_ptr, 0, 73);
        }
        output_ptr += 73;
    }
}

WASM_EXPORT void notes__account_note_commitment(uint8_t const* account_alias_hash_buffer,
                                                uint8_t const* owner_key_buf,
                                                uint8_t const* signing_key_buf,
                                                uint8_t* output)
{
    auto account_alias_hash = from_buffer<barretenberg::fr>(account_alias_hash_buffer);
    auto owner_key = from_buffer<grumpkin::g1::affine_element>(owner_key_buf);
    auto signing_key = from_buffer<grumpkin::g1::affine_element>(signing_key_buf);
    auto note_commitment = account::generate_account_commitment(account_alias_hash, owner_key.x, signing_key.x);
    write(output, note_commitment);
}

WASM_EXPORT void notes__compute_account_alias_hash_nullifier(uint8_t const* id_buffer, uint8_t* output)
{
    auto account_alias_hash = from_buffer<barretenberg::fr>(id_buffer);
    auto nullifier = account::compute_account_alias_hash_nullifier(account_alias_hash);
    write(output, nullifier);
}

WASM_EXPORT void notes__compute_account_public_key_nullifier(uint8_t const* public_key_buffer, uint8_t* output)
{
    auto account_public_key = from_buffer<grumpkin::g1::affine_element>(public_key_buffer);
    auto nullifier = account::compute_account_public_key_nullifier(account_public_key);
    write(output, nullifier);
}
}
