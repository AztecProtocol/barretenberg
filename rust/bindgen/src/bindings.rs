use wasm_bindgen::prelude::*;
use wasm_bindgen::JsValue;
use crate::serialize::{BufferDeserializer, NumberDeserializer, VectorDeserializer, BoolDeserializer};
use crate::types::{Fr, Fq, Point, Buffer32, Buffer128};
#[wasm_bindgen]
pub fn pedersenInit() ->  {
    let in_args = js_sys::Array::new();
    let out_types = js_sys::Array::new();
    let result = crate::call_wasm_export::call_wasm_export("pedersen__init", in_args, out_types).unwrap();
}
#[wasm_bindgen]
pub fn pedersenCompressFields(left: Fr, right: Fr) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(left.into());
    in_args.push(right.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen__compress_fields", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn pedersenPlookupCompressFields(left: Fr, right: Fr) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(left.into());
    in_args.push(right.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen_plookup_compress_fields", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn pedersenCompress(inputs_buffer: Fr[]) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(inputs_buffer.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen__compress", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn pedersenPlookupCompress(inputs_buffer: Fr[]) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(inputs_buffer.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen_plookup_compress", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn pedersenCompressWithHashIndex(inputs_buffer: Fr[], hash_index: number) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(inputs_buffer.into());
    in_args.push(hash_index.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen__compress_with_hash_index", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn pedersenCommit(inputs_buffer: Fr[]) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(inputs_buffer.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen__commit", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn pedersenPlookupCommit(inputs_buffer: Fr[]) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(inputs_buffer.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen_plookup_commit", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn pedersenBufferToField(data: Buffer) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(data.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen__buffer_to_field", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn pedersenHashInit() ->  {
    let in_args = js_sys::Array::new();
    let out_types = js_sys::Array::new();
    let result = crate::call_wasm_export::call_wasm_export("pedersen_hash__init", in_args, out_types).unwrap();
}
#[wasm_bindgen]
pub fn pedersenHashPair(left: Fr, right: Fr) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(left.into());
    in_args.push(right.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen__hash_pair", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn pedersenHashMultiple(inputs_buffer: Fr[]) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(inputs_buffer.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen__hash_multiple", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn pedersenHashMultipleWithHashIndex(inputs_buffer: Fr[], hash_index: number) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(inputs_buffer.into());
    in_args.push(hash_index.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen__hash_multiple_with_hash_index", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn pedersenHashToTree(data: Fr[]) -> Fr[] {
    let in_args = js_sys::Array::new();
    in_args.push(data.into());
    let out_types = js_sys::Array::new();
    out_types.push(VectorDeserializer(Fr).into());
    let result = crate::call_wasm_export::call_wasm_export("pedersen__hash_to_tree", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn blake2S(data: Buffer) -> Buffer32 {
    let in_args = js_sys::Array::new();
    in_args.push(data.into());
    let out_types = js_sys::Array::new();
    out_types.push(Buffer32.into());
    let result = crate::call_wasm_export::call_wasm_export("blake2s", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn blake2SToField(data: Buffer) -> Fr {
    let in_args = js_sys::Array::new();
    in_args.push(data.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fr.into());
    let result = crate::call_wasm_export::call_wasm_export("blake2s_to_field", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn schnorrComputePublicKey(private_key: Fr) -> Point {
    let in_args = js_sys::Array::new();
    in_args.push(private_key.into());
    let out_types = js_sys::Array::new();
    out_types.push(Point.into());
    let result = crate::call_wasm_export::call_wasm_export("schnorr__compute_public_key", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn schnorrNegatePublicKey(public_key_buffer: Point) -> Point {
    let in_args = js_sys::Array::new();
    in_args.push(public_key_buffer.into());
    let out_types = js_sys::Array::new();
    out_types.push(Point.into());
    let result = crate::call_wasm_export::call_wasm_export("schnorr__negate_public_key", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn schnorrConstructSignature(message: Buffer, private_key: Fr) -> (Buffer32, Buffer32) {
    let in_args = js_sys::Array::new();
    in_args.push(message.into());
    in_args.push(private_key.into());
    let out_types = js_sys::Array::new();
    out_types.push(Buffer32.into());
    out_types.push(Buffer32.into());
    let result = crate::call_wasm_export::call_wasm_export("schnorr__construct_signature", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn schnorrVerifySignature(message: Buffer, pub_key: Point, sig_s: Buffer32, sig_e: Buffer32) -> boolean {
    let in_args = js_sys::Array::new();
    in_args.push(message.into());
    in_args.push(pub_key.into());
    in_args.push(sig_s.into());
    in_args.push(sig_e.into());
    let out_types = js_sys::Array::new();
    out_types.push(BoolDeserializer().into());
    let result = crate::call_wasm_export::call_wasm_export("schnorr__verify_signature", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn schnorrMultisigCreateMultisigPublicKey(private_key: Fq) -> Buffer128 {
    let in_args = js_sys::Array::new();
    in_args.push(private_key.into());
    let out_types = js_sys::Array::new();
    out_types.push(Buffer128.into());
    let result = crate::call_wasm_export::call_wasm_export("schnorr__multisig_create_multisig_public_key", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn schnorrMultisigValidateAndCombineSignerPubkeys(signer_pubkey_buf: Buffer128[]) -> (Point, boolean) {
    let in_args = js_sys::Array::new();
    in_args.push(signer_pubkey_buf.into());
    let out_types = js_sys::Array::new();
    out_types.push(Point.into());
    out_types.push(BoolDeserializer().into());
    let result = crate::call_wasm_export::call_wasm_export("schnorr__multisig_validate_and_combine_signer_pubkeys", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn schnorrMultisigConstructSignatureRound1() -> (Buffer128, Buffer128) {
    let in_args = js_sys::Array::new();
    let out_types = js_sys::Array::new();
    out_types.push(Buffer128.into());
    out_types.push(Buffer128.into());
    let result = crate::call_wasm_export::call_wasm_export("schnorr__multisig_construct_signature_round_1", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn schnorrMultisigConstructSignatureRound2(message: Buffer, private_key: Fq, signer_round_one_private_buf: Buffer128, signer_pubkeys_buf: Buffer128[], round_one_public_buf: Buffer128[]) -> (Fq, boolean) {
    let in_args = js_sys::Array::new();
    in_args.push(message.into());
    in_args.push(private_key.into());
    in_args.push(signer_round_one_private_buf.into());
    in_args.push(signer_pubkeys_buf.into());
    in_args.push(round_one_public_buf.into());
    let out_types = js_sys::Array::new();
    out_types.push(Fq.into());
    out_types.push(BoolDeserializer().into());
    let result = crate::call_wasm_export::call_wasm_export("schnorr__multisig_construct_signature_round_2", in_args, out_types).unwrap();
    return result;
}
#[wasm_bindgen]
pub fn schnorrMultisigCombineSignatures(message: Buffer, signer_pubkeys_buf: Buffer128[], round_one_buf: Buffer128[], round_two_buf: Fr[]) -> (Buffer32, Buffer32, boolean) {
    let in_args = js_sys::Array::new();
    in_args.push(message.into());
    in_args.push(signer_pubkeys_buf.into());
    in_args.push(round_one_buf.into());
    in_args.push(round_two_buf.into());
    let out_types = js_sys::Array::new();
    out_types.push(Buffer32.into());
    out_types.push(Buffer32.into());
    out_types.push(BoolDeserializer().into());
    let result = crate::call_wasm_export::call_wasm_export("schnorr__multisig_combine_signatures", in_args, out_types).unwrap();
    return result;
}

