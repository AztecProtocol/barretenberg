use crate::bn254_arithmetic;

#[derive(Debug, Clone, Copy)]
pub struct NativeVerifyInput<'a> {
    pub vk_hash: [u8; 32],
    pub proof_hash: [u8; 32],
    pub public_inputs_hash: [u8; 32],
    pub session_bind_hash: [u8; 32],
    pub authority: [u8; 32],
    pub proof_total_len: u64,
    pub public_inputs_total_len: u64,
    pub public_inputs_count: usize,
    pub proof_bytes: &'a [u8],
    pub public_inputs_bytes: &'a [u8],
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct NativeVerifyResult {
    pub verified: bool,
    #[allow(dead_code)]
    pub transcript_digest: [u8; 32],
}

#[inline(never)]
pub fn evaluate(input: &NativeVerifyInput<'_>) -> NativeVerifyResult {
    // Stage-0 already checks proof/public-input/session-bind hashes; stage-2 focuses on
    // arithmetic consistency to keep CU bounded.

    let arithmetic_checks_ok =
        match bn254_arithmetic::evaluate_ultrahonk_arithmetic_checks_with_public_inputs_count(
        input.proof_bytes,
        &input.vk_hash,
        input.public_inputs_bytes,
        input.public_inputs_count,
    ) {
        Ok(verified) => verified,
        Err(_) => false,
    };

    let verified = !input.proof_bytes.is_empty() && arithmetic_checks_ok;

    NativeVerifyResult {
        verified,
        transcript_digest: input.session_bind_hash,
    }
}
