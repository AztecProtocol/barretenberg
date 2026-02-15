#![forbid(unsafe_code)]
#![cfg_attr(feature = "sbf-entrypoint", allow(unexpected_cfgs))]

mod bn254_arithmetic;
mod native_verify_kernel;

#[cfg(feature = "sbf-entrypoint")]
use solana_program::entrypoint;
use solana_program::{
    account_info::{next_account_info, AccountInfo},
    entrypoint::ProgramResult,
    hash::hashv,
    msg,
    program::{invoke_signed, set_return_data},
    program_error::ProgramError,
    pubkey::Pubkey,
    sysvar::{rent::Rent, Sysvar},
};
use solana_sdk_ids::system_program;
use solana_system_interface::instruction as system_instruction;

pub const SESSION_ABI_VERSION: u32 = 2;
pub const TOTAL_VERIFY_STAGES: u32 = 3;
const SESSION_ACCOUNT_DISCRIMINATOR: &[u8; 8] = b"UHSESSN2";

pub const INIT_SESSION_DISCRIMINANT: u8 = 0;
pub const APPEND_PROOF_CHUNK_DISCRIMINANT: u8 = 1;
pub const APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT: u8 = 2;
pub const VERIFY_STEP_DISCRIMINANT: u8 = 3;
pub const FINALIZE_DISCRIMINANT: u8 = 4;

pub const PROOF_HASH_DOMAIN_TAG: &[u8] = b"bb.solana.ultrahonk.proof.v1";
pub const PUBLIC_INPUTS_HASH_DOMAIN_TAG: &[u8] = b"bb.solana.ultrahonk.public_inputs.v1";
pub const SESSION_BIND_HASH_DOMAIN_TAG: &[u8] = b"bb.solana.ultrahonk.session_bind.v1";

// BN254 scalar field modulus in canonical 32-byte big-endian form.
const BN254_SCALAR_MODULUS_BE: [u8; 32] = [
    0x30, 0x64, 0x4e, 0x72, 0xe1, 0x31, 0xa0, 0x29, 0xb8, 0x50, 0x45, 0xb6, 0x81, 0x81, 0x58, 0x5d,
    0x28, 0x33, 0xe8, 0x48, 0x79, 0xb9, 0x70, 0x91, 0x43, 0xe1, 0xf5, 0x93, 0xf0, 0x00, 0x00, 0x01,
];

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(u8)]
pub enum SessionStatus {
    CollectingProof = 0,
    CollectingPublicInputs = 1,
    ReadyToVerify = 2,
    Verifying = 3,
    Verified = 4,
    Failed = 5,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(u32)]
pub enum VerifyStage {
    HashChecks = 0,
    DecodeInputs = 1,
    NativeVerify = 2,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct InitSessionPayload {
    pub vk_hash: [u8; 32],
    pub proof_total_len: u64,
    pub public_inputs_total_len: u64,
    pub proof_hash: [u8; 32],
    pub public_inputs_hash: [u8; 32],
    pub session_bind_hash: [u8; 32],
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct AppendChunkPayload {
    pub chunk_index: u32,
    pub offset: u32,
    pub chunk_bytes: Vec<u8>,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum VerifierInstruction {
    InitSession(InitSessionPayload),
    AppendProofChunk(AppendChunkPayload),
    AppendPublicInputsChunk(AppendChunkPayload),
    VerifyStep { expected_stage_index: u32 },
    Finalize,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct SessionAccount {
    pub initialized: bool,
    pub authority: [u8; 32],
    pub vk_hash: [u8; 32],
    pub proof_hash: [u8; 32],
    pub public_inputs_hash: [u8; 32],
    pub session_bind_hash: [u8; 32],
    pub proof_total_len: u64,
    pub public_inputs_total_len: u64,
    pub proof_received_len: u64,
    pub public_inputs_received_len: u64,
    pub next_proof_chunk_index: u32,
    pub next_public_inputs_chunk_index: u32,
    pub stage_index: u32,
    pub status: SessionStatus,
    pub stage_failed: bool,
    pub native_verify_result_hint: bool,
    pub proof_store: Vec<u8>,
    pub public_inputs_store: Vec<u8>,
}

impl Default for SessionAccount {
    fn default() -> Self {
        Self {
            initialized: false,
            authority: [0u8; 32],
            vk_hash: [0u8; 32],
            proof_hash: [0u8; 32],
            public_inputs_hash: [0u8; 32],
            session_bind_hash: [0u8; 32],
            proof_total_len: 0,
            public_inputs_total_len: 0,
            proof_received_len: 0,
            public_inputs_received_len: 0,
            next_proof_chunk_index: 0,
            next_public_inputs_chunk_index: 0,
            stage_index: 0,
            status: SessionStatus::CollectingProof,
            stage_failed: false,
            native_verify_result_hint: false,
            proof_store: Vec::new(),
            public_inputs_store: Vec::new(),
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum SessionError {
    TruncatedInstruction,
    InvalidInstructionDiscriminant(u8),
    InvalidLengthPrefix,
    InvalidSessionStatus(u8),
    SessionNotInitialized,
    SessionAlreadyInitialized,
    InvalidAuthority,
    MissingRequiredSignature,
    MissingRequiredAccount(&'static str),
    InvalidAccountOwner,
    InvalidSessionAccountPda,
    SessionAccountNotWritable,
    InvalidVkAccountData,
    InvalidVkHashBinding,
    InvalidSessionAccountData,
    SessionAccountDataTooSmall {
        required: usize,
        actual: usize,
    },
    InvalidSessionState {
        expected: SessionStatus,
        got: SessionStatus,
    },
    NonContiguousChunkIndex {
        expected: u32,
        got: u32,
    },
    OffsetMismatch {
        expected: u64,
        got: u32,
    },
    ChunkOverflow,
    LengthTooLarge,
    VerifyStageMismatch {
        expected: u32,
        got: u32,
    },
    VerifyStageOutOfRange(u32),
    FinalizeBeforeFullIngest,
    FinalizeBeforeAllStages,
    AuthorityAccountNotWritable,
    MissingSystemProgramAccount,
    InvalidSystemProgramAccount,
    SessionAccountAlreadyExists,
}

impl SessionStatus {
    fn from_u8(value: u8) -> Result<Self, SessionError> {
        match value {
            0 => Ok(Self::CollectingProof),
            1 => Ok(Self::CollectingPublicInputs),
            2 => Ok(Self::ReadyToVerify),
            3 => Ok(Self::Verifying),
            4 => Ok(Self::Verified),
            5 => Ok(Self::Failed),
            other => Err(SessionError::InvalidSessionStatus(other)),
        }
    }
}

impl SessionError {
    fn code(&self) -> u32 {
        match self {
            SessionError::TruncatedInstruction => 0,
            SessionError::InvalidInstructionDiscriminant(_) => 1,
            SessionError::InvalidLengthPrefix => 2,
            SessionError::InvalidSessionStatus(_) => 3,
            SessionError::SessionNotInitialized => 4,
            SessionError::SessionAlreadyInitialized => 5,
            SessionError::InvalidAuthority => 6,
            SessionError::MissingRequiredSignature => 7,
            SessionError::MissingRequiredAccount(_) => 8,
            SessionError::InvalidAccountOwner => 9,
            SessionError::InvalidSessionAccountPda => 10,
            SessionError::SessionAccountNotWritable => 11,
            SessionError::InvalidVkAccountData => 12,
            SessionError::InvalidVkHashBinding => 13,
            SessionError::InvalidSessionAccountData => 14,
            SessionError::SessionAccountDataTooSmall { .. } => 15,
            SessionError::InvalidSessionState { .. } => 16,
            SessionError::NonContiguousChunkIndex { .. } => 17,
            SessionError::OffsetMismatch { .. } => 18,
            SessionError::ChunkOverflow => 19,
            SessionError::LengthTooLarge => 20,
            SessionError::VerifyStageMismatch { .. } => 21,
            SessionError::VerifyStageOutOfRange(_) => 22,
            SessionError::FinalizeBeforeFullIngest => 23,
            SessionError::FinalizeBeforeAllStages => 24,
            SessionError::AuthorityAccountNotWritable => 25,
            SessionError::MissingSystemProgramAccount => 26,
            SessionError::InvalidSystemProgramAccount => 27,
            SessionError::SessionAccountAlreadyExists => 28,
        }
    }
}

impl From<SessionError> for ProgramError {
    fn from(value: SessionError) -> Self {
        ProgramError::Custom(value.code())
    }
}

fn take_u8(input: &[u8], cursor: &mut usize) -> Result<u8, SessionError> {
    if *cursor >= input.len() {
        return Err(SessionError::TruncatedInstruction);
    }
    let out = input[*cursor];
    *cursor += 1;
    Ok(out)
}

fn take_bool(input: &[u8], cursor: &mut usize) -> Result<bool, SessionError> {
    Ok(take_u8(input, cursor)? != 0)
}

fn take_u32_le(input: &[u8], cursor: &mut usize) -> Result<u32, SessionError> {
    if input.len().saturating_sub(*cursor) < 4 {
        return Err(SessionError::TruncatedInstruction);
    }
    let bytes = <[u8; 4]>::try_from(&input[*cursor..*cursor + 4])
        .map_err(|_| SessionError::TruncatedInstruction)?;
    *cursor += 4;
    Ok(u32::from_le_bytes(bytes))
}

fn take_u64_le(input: &[u8], cursor: &mut usize) -> Result<u64, SessionError> {
    if input.len().saturating_sub(*cursor) < 8 {
        return Err(SessionError::TruncatedInstruction);
    }
    let bytes = <[u8; 8]>::try_from(&input[*cursor..*cursor + 8])
        .map_err(|_| SessionError::TruncatedInstruction)?;
    *cursor += 8;
    Ok(u64::from_le_bytes(bytes))
}

fn take_bytes32(input: &[u8], cursor: &mut usize) -> Result<[u8; 32], SessionError> {
    if input.len().saturating_sub(*cursor) < 32 {
        return Err(SessionError::TruncatedInstruction);
    }
    let out = <[u8; 32]>::try_from(&input[*cursor..*cursor + 32])
        .map_err(|_| SessionError::TruncatedInstruction)?;
    *cursor += 32;
    Ok(out)
}

fn take_len_prefixed_bytes(input: &[u8], cursor: &mut usize) -> Result<Vec<u8>, SessionError> {
    let len = take_u32_le(input, cursor)? as usize;
    if input.len().saturating_sub(*cursor) < len {
        return Err(SessionError::InvalidLengthPrefix);
    }
    let out = input[*cursor..*cursor + len].to_vec();
    *cursor += len;
    Ok(out)
}

fn put_u8(out: &mut [u8], cursor: &mut usize, value: u8) -> Result<(), SessionError> {
    if out.len().saturating_sub(*cursor) < 1 {
        return Err(SessionError::SessionAccountDataTooSmall {
            required: cursor.saturating_add(1),
            actual: out.len(),
        });
    }
    out[*cursor] = value;
    *cursor += 1;
    Ok(())
}

fn put_u32_le(out: &mut [u8], cursor: &mut usize, value: u32) -> Result<(), SessionError> {
    if out.len().saturating_sub(*cursor) < 4 {
        return Err(SessionError::SessionAccountDataTooSmall {
            required: cursor.saturating_add(4),
            actual: out.len(),
        });
    }
    out[*cursor..*cursor + 4].copy_from_slice(&value.to_le_bytes());
    *cursor += 4;
    Ok(())
}

fn put_u64_le(out: &mut [u8], cursor: &mut usize, value: u64) -> Result<(), SessionError> {
    if out.len().saturating_sub(*cursor) < 8 {
        return Err(SessionError::SessionAccountDataTooSmall {
            required: cursor.saturating_add(8),
            actual: out.len(),
        });
    }
    out[*cursor..*cursor + 8].copy_from_slice(&value.to_le_bytes());
    *cursor += 8;
    Ok(())
}

fn put_bytes(out: &mut [u8], cursor: &mut usize, value: &[u8]) -> Result<(), SessionError> {
    if out.len().saturating_sub(*cursor) < value.len() {
        return Err(SessionError::SessionAccountDataTooSmall {
            required: cursor.saturating_add(value.len()),
            actual: out.len(),
        });
    }
    out[*cursor..*cursor + value.len()].copy_from_slice(value);
    *cursor += value.len();
    Ok(())
}

fn bytes32_from_hash(hash: solana_program::hash::Hash) -> [u8; 32] {
    hash.to_bytes()
}

pub fn compute_proof_hash_bytes(proof_bytes: &[u8]) -> [u8; 32] {
    bytes32_from_hash(hashv(&[PROOF_HASH_DOMAIN_TAG, proof_bytes]))
}

pub fn compute_public_inputs_hash_bytes(public_inputs_bytes: &[u8]) -> [u8; 32] {
    bytes32_from_hash(hashv(&[PUBLIC_INPUTS_HASH_DOMAIN_TAG, public_inputs_bytes]))
}

pub fn compute_session_bind_hash_bytes(
    vk_hash: &[u8; 32],
    proof_hash: &[u8; 32],
    public_inputs_hash: &[u8; 32],
    authority: &[u8; 32],
    proof_total_len: u64,
    public_inputs_total_len: u64,
) -> [u8; 32] {
    let proof_total_len_bytes = proof_total_len.to_le_bytes();
    let public_inputs_total_len_bytes = public_inputs_total_len.to_le_bytes();
    bytes32_from_hash(hashv(&[
        SESSION_BIND_HASH_DOMAIN_TAG,
        vk_hash,
        proof_hash,
        public_inputs_hash,
        authority,
        &proof_total_len_bytes,
        &public_inputs_total_len_bytes,
    ]))
}

fn is_canonical_field_element_be(element: &[u8]) -> bool {
    element < BN254_SCALAR_MODULUS_BE.as_slice()
}

fn canonical_public_inputs_count(public_inputs_bytes: &[u8]) -> Option<usize> {
    if public_inputs_bytes.len() % 32 != 0 {
        return None;
    }
    if !public_inputs_bytes
        .chunks_exact(32)
        .all(is_canonical_field_element_be)
    {
        return None;
    }
    Some(public_inputs_bytes.len() / 32)
}

fn has_canonical_public_inputs_encoding(public_inputs_bytes: &[u8]) -> bool {
    canonical_public_inputs_count(public_inputs_bytes).is_some()
}

fn build_native_verify_input(
    session: &SessionAccount,
) -> Option<native_verify_kernel::NativeVerifyInput<'_>> {
    if session.proof_store.len() != session.proof_total_len as usize
        || session.public_inputs_store.len() != session.public_inputs_total_len as usize
    {
        return None;
    }
    if session.proof_store.is_empty() {
        return None;
    }
    if session.public_inputs_store.len() % 32 != 0 {
        return None;
    }
    let public_inputs_count = session.public_inputs_store.len() / 32;
    Some(native_verify_kernel::NativeVerifyInput {
        vk_hash: session.vk_hash,
        proof_hash: session.proof_hash,
        public_inputs_hash: session.public_inputs_hash,
        session_bind_hash: session.session_bind_hash,
        authority: session.authority,
        proof_total_len: session.proof_total_len,
        public_inputs_total_len: session.public_inputs_total_len,
        public_inputs_count,
        proof_bytes: &session.proof_store,
        public_inputs_bytes: &session.public_inputs_store,
    })
}

#[inline(never)]
fn evaluate_native_verify_stage(session: &SessionAccount) -> bool {
    let Some(input) = build_native_verify_input(session) else {
        return false;
    };

    native_verify_kernel::evaluate(&input).verified
}

pub fn decode_instruction(input: &[u8]) -> Result<VerifierInstruction, SessionError> {
    let mut cursor = 0usize;
    let discriminant = take_u8(input, &mut cursor)?;
    let instruction = match discriminant {
        INIT_SESSION_DISCRIMINANT => {
            let payload = InitSessionPayload {
                vk_hash: take_bytes32(input, &mut cursor)?,
                proof_total_len: take_u64_le(input, &mut cursor)?,
                public_inputs_total_len: take_u64_le(input, &mut cursor)?,
                proof_hash: take_bytes32(input, &mut cursor)?,
                public_inputs_hash: take_bytes32(input, &mut cursor)?,
                session_bind_hash: take_bytes32(input, &mut cursor)?,
            };
            VerifierInstruction::InitSession(payload)
        }
        APPEND_PROOF_CHUNK_DISCRIMINANT => {
            let payload = AppendChunkPayload {
                chunk_index: take_u32_le(input, &mut cursor)?,
                offset: take_u32_le(input, &mut cursor)?,
                chunk_bytes: take_len_prefixed_bytes(input, &mut cursor)?,
            };
            VerifierInstruction::AppendProofChunk(payload)
        }
        APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT => {
            let payload = AppendChunkPayload {
                chunk_index: take_u32_le(input, &mut cursor)?,
                offset: take_u32_le(input, &mut cursor)?,
                chunk_bytes: take_len_prefixed_bytes(input, &mut cursor)?,
            };
            VerifierInstruction::AppendPublicInputsChunk(payload)
        }
        VERIFY_STEP_DISCRIMINANT => VerifierInstruction::VerifyStep {
            expected_stage_index: take_u32_le(input, &mut cursor)?,
        },
        FINALIZE_DISCRIMINANT => VerifierInstruction::Finalize,
        other => return Err(SessionError::InvalidInstructionDiscriminant(other)),
    };

    if cursor != input.len() {
        return Err(SessionError::InvalidLengthPrefix);
    }
    Ok(instruction)
}

fn encode_u32(out: &mut Vec<u8>, v: u32) {
    out.extend_from_slice(&v.to_le_bytes());
}

fn encode_u64(out: &mut Vec<u8>, v: u64) {
    out.extend_from_slice(&v.to_le_bytes());
}

pub fn encode_instruction(instruction: &VerifierInstruction) -> Vec<u8> {
    let mut out = Vec::new();
    match instruction {
        VerifierInstruction::InitSession(payload) => {
            out.push(INIT_SESSION_DISCRIMINANT);
            out.extend_from_slice(&payload.vk_hash);
            encode_u64(&mut out, payload.proof_total_len);
            encode_u64(&mut out, payload.public_inputs_total_len);
            out.extend_from_slice(&payload.proof_hash);
            out.extend_from_slice(&payload.public_inputs_hash);
            out.extend_from_slice(&payload.session_bind_hash);
        }
        VerifierInstruction::AppendProofChunk(payload) => {
            out.push(APPEND_PROOF_CHUNK_DISCRIMINANT);
            encode_u32(&mut out, payload.chunk_index);
            encode_u32(&mut out, payload.offset);
            encode_u32(&mut out, payload.chunk_bytes.len() as u32);
            out.extend_from_slice(&payload.chunk_bytes);
        }
        VerifierInstruction::AppendPublicInputsChunk(payload) => {
            out.push(APPEND_PUBLIC_INPUTS_CHUNK_DISCRIMINANT);
            encode_u32(&mut out, payload.chunk_index);
            encode_u32(&mut out, payload.offset);
            encode_u32(&mut out, payload.chunk_bytes.len() as u32);
            out.extend_from_slice(&payload.chunk_bytes);
        }
        VerifierInstruction::VerifyStep {
            expected_stage_index,
        } => {
            out.push(VERIFY_STEP_DISCRIMINANT);
            encode_u32(&mut out, *expected_stage_index);
        }
        VerifierInstruction::Finalize => out.push(FINALIZE_DISCRIMINANT),
    }
    out
}

impl SessionAccount {
    fn base_serialized_size() -> usize {
        8  // account discriminator
        + 4 // abi version
        + 1 // initialized
        + 32 // authority
        + 32 // vk_hash
        + 32 // proof_hash
        + 32 // public_inputs_hash
        + 32 // session_bind_hash
        + 8 // proof_total_len
        + 8 // public_inputs_total_len
        + 8 // proof_received_len
        + 8 // public_inputs_received_len
        + 4 // next_proof_chunk_index
        + 4 // next_public_inputs_chunk_index
        + 4 // stage_index
        + 1 // status
        + 1 // stage_failed
        + 1 // native_verify_result_hint
        + 4 // proof_store length prefix
        + 4 // public_inputs_store length prefix
    }

    pub fn serialized_size_for_totals(
        proof_total_len: u64,
        public_inputs_total_len: u64,
    ) -> Result<usize, SessionError> {
        let proof_len =
            usize::try_from(proof_total_len).map_err(|_| SessionError::LengthTooLarge)?;
        let public_inputs_len =
            usize::try_from(public_inputs_total_len).map_err(|_| SessionError::LengthTooLarge)?;
        Self::base_serialized_size()
            .checked_add(proof_len)
            .and_then(|v| v.checked_add(public_inputs_len))
            .ok_or(SessionError::LengthTooLarge)
    }

    fn serialized_size(&self) -> Result<usize, SessionError> {
        let proof_len_u32 =
            u32::try_from(self.proof_store.len()).map_err(|_| SessionError::LengthTooLarge)?;
        let public_inputs_len_u32 = u32::try_from(self.public_inputs_store.len())
            .map_err(|_| SessionError::LengthTooLarge)?;
        let _ = (proof_len_u32, public_inputs_len_u32);
        Self::base_serialized_size()
            .checked_add(self.proof_store.len())
            .and_then(|v| v.checked_add(self.public_inputs_store.len()))
            .ok_or(SessionError::LengthTooLarge)
    }

    pub fn deserialize_from_account_data(input: &[u8]) -> Result<Self, SessionError> {
        if input.iter().all(|byte| *byte == 0) {
            return Ok(Self::default());
        }
        if input.len() < Self::base_serialized_size() {
            return Err(SessionError::InvalidSessionAccountData);
        }
        if &input[..8] != SESSION_ACCOUNT_DISCRIMINATOR {
            return Err(SessionError::InvalidSessionAccountData);
        }

        let mut cursor = 8usize;
        let abi_version = take_u32_le(input, &mut cursor)?;
        if abi_version != SESSION_ABI_VERSION {
            return Err(SessionError::InvalidSessionAccountData);
        }

        let initialized = take_bool(input, &mut cursor)?;
        let authority = take_bytes32(input, &mut cursor)?;
        let vk_hash = take_bytes32(input, &mut cursor)?;
        let proof_hash = take_bytes32(input, &mut cursor)?;
        let public_inputs_hash = take_bytes32(input, &mut cursor)?;
        let session_bind_hash = take_bytes32(input, &mut cursor)?;
        let proof_total_len = take_u64_le(input, &mut cursor)?;
        let public_inputs_total_len = take_u64_le(input, &mut cursor)?;
        let proof_received_len = take_u64_le(input, &mut cursor)?;
        let public_inputs_received_len = take_u64_le(input, &mut cursor)?;
        let next_proof_chunk_index = take_u32_le(input, &mut cursor)?;
        let next_public_inputs_chunk_index = take_u32_le(input, &mut cursor)?;
        let stage_index = take_u32_le(input, &mut cursor)?;
        let status = SessionStatus::from_u8(take_u8(input, &mut cursor)?)?;
        let stage_failed = take_bool(input, &mut cursor)?;
        let native_verify_result_hint = take_bool(input, &mut cursor)?;
        let proof_store = take_len_prefixed_bytes(input, &mut cursor)?;
        let public_inputs_store = take_len_prefixed_bytes(input, &mut cursor)?;

        Ok(Self {
            initialized,
            authority,
            vk_hash,
            proof_hash,
            public_inputs_hash,
            session_bind_hash,
            proof_total_len,
            public_inputs_total_len,
            proof_received_len,
            public_inputs_received_len,
            next_proof_chunk_index,
            next_public_inputs_chunk_index,
            stage_index,
            status,
            stage_failed,
            native_verify_result_hint,
            proof_store,
            public_inputs_store,
        })
    }

    pub fn serialize_into_account_data(&self, out: &mut [u8]) -> Result<(), SessionError> {
        let required = self.serialized_size()?;
        if out.len() < required {
            return Err(SessionError::SessionAccountDataTooSmall {
                required,
                actual: out.len(),
            });
        }

        out.fill(0);
        let mut cursor = 0usize;
        put_bytes(out, &mut cursor, SESSION_ACCOUNT_DISCRIMINATOR)?;
        put_u32_le(out, &mut cursor, SESSION_ABI_VERSION)?;
        put_u8(out, &mut cursor, u8::from(self.initialized))?;
        put_bytes(out, &mut cursor, &self.authority)?;
        put_bytes(out, &mut cursor, &self.vk_hash)?;
        put_bytes(out, &mut cursor, &self.proof_hash)?;
        put_bytes(out, &mut cursor, &self.public_inputs_hash)?;
        put_bytes(out, &mut cursor, &self.session_bind_hash)?;
        put_u64_le(out, &mut cursor, self.proof_total_len)?;
        put_u64_le(out, &mut cursor, self.public_inputs_total_len)?;
        put_u64_le(out, &mut cursor, self.proof_received_len)?;
        put_u64_le(out, &mut cursor, self.public_inputs_received_len)?;
        put_u32_le(out, &mut cursor, self.next_proof_chunk_index)?;
        put_u32_le(out, &mut cursor, self.next_public_inputs_chunk_index)?;
        put_u32_le(out, &mut cursor, self.stage_index)?;
        put_u8(out, &mut cursor, self.status as u8)?;
        put_u8(out, &mut cursor, u8::from(self.stage_failed))?;
        put_u8(out, &mut cursor, u8::from(self.native_verify_result_hint))?;
        put_u32_le(
            out,
            &mut cursor,
            u32::try_from(self.proof_store.len()).map_err(|_| SessionError::LengthTooLarge)?,
        )?;
        put_bytes(out, &mut cursor, &self.proof_store)?;
        put_u32_le(
            out,
            &mut cursor,
            u32::try_from(self.public_inputs_store.len())
                .map_err(|_| SessionError::LengthTooLarge)?,
        )?;
        put_bytes(out, &mut cursor, &self.public_inputs_store)?;
        Ok(())
    }

    fn ensure_initialized(&self) -> Result<(), SessionError> {
        if self.initialized {
            Ok(())
        } else {
            Err(SessionError::SessionNotInitialized)
        }
    }

    fn ensure_authority(&self, authority: &[u8; 32]) -> Result<(), SessionError> {
        if &self.authority == authority {
            Ok(())
        } else {
            Err(SessionError::InvalidAuthority)
        }
    }

    pub fn init_session(
        &mut self,
        authority: [u8; 32],
        payload: InitSessionPayload,
    ) -> Result<(), SessionError> {
        if self.initialized {
            return Err(SessionError::SessionAlreadyInitialized);
        }

        let proof_len =
            usize::try_from(payload.proof_total_len).map_err(|_| SessionError::LengthTooLarge)?;
        let public_inputs_len = usize::try_from(payload.public_inputs_total_len)
            .map_err(|_| SessionError::LengthTooLarge)?;

        self.initialized = true;
        self.authority = authority;
        self.vk_hash = payload.vk_hash;
        self.proof_hash = payload.proof_hash;
        self.public_inputs_hash = payload.public_inputs_hash;
        self.session_bind_hash = payload.session_bind_hash;
        self.proof_total_len = payload.proof_total_len;
        self.public_inputs_total_len = payload.public_inputs_total_len;
        self.proof_received_len = 0;
        self.public_inputs_received_len = 0;
        self.next_proof_chunk_index = 0;
        self.next_public_inputs_chunk_index = 0;
        self.stage_index = 0;
        self.stage_failed = false;
        self.proof_store = vec![0u8; proof_len];
        self.public_inputs_store = vec![0u8; public_inputs_len];

        self.status = if self.proof_total_len == 0 && self.public_inputs_total_len == 0 {
            SessionStatus::ReadyToVerify
        } else if self.proof_total_len == 0 {
            SessionStatus::CollectingPublicInputs
        } else {
            SessionStatus::CollectingProof
        };
        Ok(())
    }

    pub fn append_proof_chunk(&mut self, payload: AppendChunkPayload) -> Result<(), SessionError> {
        self.ensure_initialized()?;
        if self.status != SessionStatus::CollectingProof {
            return Err(SessionError::InvalidSessionState {
                expected: SessionStatus::CollectingProof,
                got: self.status,
            });
        }
        if payload.chunk_index != self.next_proof_chunk_index {
            return Err(SessionError::NonContiguousChunkIndex {
                expected: self.next_proof_chunk_index,
                got: payload.chunk_index,
            });
        }
        if u64::from(payload.offset) != self.proof_received_len {
            return Err(SessionError::OffsetMismatch {
                expected: self.proof_received_len,
                got: payload.offset,
            });
        }

        let chunk_len =
            u64::try_from(payload.chunk_bytes.len()).map_err(|_| SessionError::LengthTooLarge)?;
        let end = self
            .proof_received_len
            .checked_add(chunk_len)
            .ok_or(SessionError::ChunkOverflow)?;
        if end > self.proof_total_len {
            return Err(SessionError::ChunkOverflow);
        }

        let start_usize =
            usize::try_from(self.proof_received_len).map_err(|_| SessionError::LengthTooLarge)?;
        let end_usize = usize::try_from(end).map_err(|_| SessionError::LengthTooLarge)?;
        self.proof_store[start_usize..end_usize].copy_from_slice(&payload.chunk_bytes);

        self.proof_received_len = end;
        self.next_proof_chunk_index = self.next_proof_chunk_index.saturating_add(1);

        if self.proof_received_len == self.proof_total_len {
            self.status = if self.public_inputs_total_len == 0 {
                SessionStatus::ReadyToVerify
            } else {
                SessionStatus::CollectingPublicInputs
            };
        }
        Ok(())
    }

    pub fn append_public_inputs_chunk(
        &mut self,
        payload: AppendChunkPayload,
    ) -> Result<(), SessionError> {
        self.ensure_initialized()?;
        if self.status != SessionStatus::CollectingPublicInputs {
            return Err(SessionError::InvalidSessionState {
                expected: SessionStatus::CollectingPublicInputs,
                got: self.status,
            });
        }
        if payload.chunk_index != self.next_public_inputs_chunk_index {
            return Err(SessionError::NonContiguousChunkIndex {
                expected: self.next_public_inputs_chunk_index,
                got: payload.chunk_index,
            });
        }
        if u64::from(payload.offset) != self.public_inputs_received_len {
            return Err(SessionError::OffsetMismatch {
                expected: self.public_inputs_received_len,
                got: payload.offset,
            });
        }

        let chunk_len =
            u64::try_from(payload.chunk_bytes.len()).map_err(|_| SessionError::LengthTooLarge)?;
        let end = self
            .public_inputs_received_len
            .checked_add(chunk_len)
            .ok_or(SessionError::ChunkOverflow)?;
        if end > self.public_inputs_total_len {
            return Err(SessionError::ChunkOverflow);
        }

        let start_usize = usize::try_from(self.public_inputs_received_len)
            .map_err(|_| SessionError::LengthTooLarge)?;
        let end_usize = usize::try_from(end).map_err(|_| SessionError::LengthTooLarge)?;
        self.public_inputs_store[start_usize..end_usize].copy_from_slice(&payload.chunk_bytes);

        self.public_inputs_received_len = end;
        self.next_public_inputs_chunk_index = self.next_public_inputs_chunk_index.saturating_add(1);

        if self.public_inputs_received_len == self.public_inputs_total_len {
            self.status = SessionStatus::ReadyToVerify;
        }
        Ok(())
    }

    pub fn verify_step(&mut self, expected_stage_index: u32) -> Result<(), SessionError> {
        self.ensure_initialized()?;
        if self.status != SessionStatus::ReadyToVerify && self.status != SessionStatus::Verifying {
            return Err(SessionError::InvalidSessionState {
                expected: SessionStatus::ReadyToVerify,
                got: self.status,
            });
        }
        if expected_stage_index != self.stage_index {
            return Err(SessionError::VerifyStageMismatch {
                expected: self.stage_index,
                got: expected_stage_index,
            });
        }
        if self.stage_index >= TOTAL_VERIFY_STAGES {
            return Err(SessionError::VerifyStageOutOfRange(self.stage_index));
        }

        self.status = SessionStatus::Verifying;
        match self.stage_index {
            x if x == VerifyStage::HashChecks as u32 => {
                let fully_ingested = self.proof_received_len == self.proof_total_len
                    && self.public_inputs_received_len == self.public_inputs_total_len;
                if !fully_ingested {
                    self.stage_failed = true;
                } else {
                    let computed_proof_hash = compute_proof_hash_bytes(&self.proof_store);
                    let computed_public_inputs_hash =
                        compute_public_inputs_hash_bytes(&self.public_inputs_store);
                    let computed_session_bind_hash = compute_session_bind_hash_bytes(
                        &self.vk_hash,
                        &computed_proof_hash,
                        &computed_public_inputs_hash,
                        &self.authority,
                        self.proof_total_len,
                        self.public_inputs_total_len,
                    );

                    if computed_proof_hash != self.proof_hash
                        || computed_public_inputs_hash != self.public_inputs_hash
                        || computed_session_bind_hash != self.session_bind_hash
                    {
                        self.stage_failed = true;
                    }
                }
            }
            x if x == VerifyStage::DecodeInputs as u32 => {
                if self.proof_store.len() != self.proof_total_len as usize
                    || self.public_inputs_store.len() != self.public_inputs_total_len as usize
                    || self.proof_store.is_empty()
                    || !has_canonical_public_inputs_encoding(&self.public_inputs_store)
                {
                    self.stage_failed = true;
                }
            }
            x if x == VerifyStage::NativeVerify as u32 => {
                if !evaluate_native_verify_stage(self) {
                    self.stage_failed = true;
                }
            }
            other => return Err(SessionError::VerifyStageOutOfRange(other)),
        }
        self.stage_index = self.stage_index.saturating_add(1);
        Ok(())
    }

    pub fn finalize(&mut self) -> Result<bool, SessionError> {
        self.ensure_initialized()?;
        if self.status != SessionStatus::Verifying {
            return Err(SessionError::InvalidSessionState {
                expected: SessionStatus::Verifying,
                got: self.status,
            });
        }
        if self.proof_received_len != self.proof_total_len
            || self.public_inputs_received_len != self.public_inputs_total_len
        {
            return Err(SessionError::FinalizeBeforeFullIngest);
        }
        if self.stage_index < TOTAL_VERIFY_STAGES {
            return Err(SessionError::FinalizeBeforeAllStages);
        }
        let verified = !self.stage_failed;
        self.status = if verified {
            SessionStatus::Verified
        } else {
            SessionStatus::Failed
        };
        Ok(verified)
    }
}

fn process_decoded_instruction(
    session: &mut SessionAccount,
    authority: [u8; 32],
    instruction: VerifierInstruction,
) -> Result<Option<bool>, SessionError> {
    match instruction {
        VerifierInstruction::InitSession(payload) => {
            session.init_session(authority, payload)?;
            Ok(None)
        }
        VerifierInstruction::AppendProofChunk(payload) => {
            session.ensure_initialized()?;
            session.ensure_authority(&authority)?;
            session.append_proof_chunk(payload)?;
            Ok(None)
        }
        VerifierInstruction::AppendPublicInputsChunk(payload) => {
            session.ensure_initialized()?;
            session.ensure_authority(&authority)?;
            session.append_public_inputs_chunk(payload)?;
            Ok(None)
        }
        VerifierInstruction::VerifyStep {
            expected_stage_index,
        } => {
            session.ensure_initialized()?;
            session.ensure_authority(&authority)?;
            session.verify_step(expected_stage_index)?;
            Ok(None)
        }
        VerifierInstruction::Finalize => {
            session.ensure_initialized()?;
            session.ensure_authority(&authority)?;
            let result = session.finalize()?;
            Ok(Some(result))
        }
    }
}

pub fn process_instruction(
    session: &mut SessionAccount,
    authority: [u8; 32],
    instruction_data: &[u8],
) -> Result<Option<bool>, SessionError> {
    let instruction = decode_instruction(instruction_data)?;
    process_decoded_instruction(session, authority, instruction)
}

pub fn derive_session_pda(
    program_id: &Pubkey,
    authority: &Pubkey,
    session_bind_hash: &[u8; 32],
) -> (Pubkey, u8) {
    Pubkey::find_program_address(
        &[b"session", authority.as_ref(), session_bind_hash],
        program_id,
    )
}

fn validate_vk_binding_if_present(
    instruction: &VerifierInstruction,
    vk_account: Option<&AccountInfo>,
    program_id: &Pubkey,
) -> Result<(), ProgramError> {
    let VerifierInstruction::InitSession(payload) = instruction else {
        return Ok(());
    };

    let Some(vk_account) = vk_account else {
        return Ok(());
    };

    if vk_account.owner != program_id {
        return Err(SessionError::InvalidAccountOwner.into());
    }
    let vk_data = vk_account
        .try_borrow_data()
        .map_err(|_| SessionError::InvalidVkAccountData)?;
    if vk_data.len() < 32 {
        return Err(SessionError::InvalidVkAccountData.into());
    }
    if vk_data[..32] != payload.vk_hash {
        return Err(SessionError::InvalidVkHashBinding.into());
    }
    Ok(())
}

fn maybe_create_session_account_for_init<'a>(
    program_id: &Pubkey,
    authority_account: &AccountInfo<'a>,
    session_account: &AccountInfo<'a>,
    system_program_account: Option<&AccountInfo<'a>>,
    payload: &InitSessionPayload,
) -> ProgramResult {
    if session_account.owner == program_id {
        return Ok(());
    }
    if session_account.lamports() != 0 || session_account.data_len() != 0 {
        if session_account.owner != &system_program::id() {
            return Err(SessionError::InvalidAccountOwner.into());
        }
        return Err(SessionError::SessionAccountAlreadyExists.into());
    }
    if !authority_account.is_writable {
        return Err(SessionError::AuthorityAccountNotWritable.into());
    }

    let system_program_account =
        system_program_account.ok_or(SessionError::MissingSystemProgramAccount)?;
    if system_program_account.key != &system_program::id() {
        return Err(SessionError::InvalidSystemProgramAccount.into());
    }

    let required = SessionAccount::serialized_size_for_totals(
        payload.proof_total_len,
        payload.public_inputs_total_len,
    )?;
    let required_u64 = u64::try_from(required).map_err(|_| SessionError::LengthTooLarge)?;
    let rent_lamports = Rent::get()?.minimum_balance(required);

    let (_, bump) = derive_session_pda(program_id, authority_account.key, &payload.session_bind_hash);
    let bump_seed = [bump];
    let signer_seeds: [&[u8]; 4] = [
        b"session",
        authority_account.key.as_ref(),
        payload.session_bind_hash.as_ref(),
        &bump_seed,
    ];

    let create_ix = system_instruction::create_account(
        authority_account.key,
        session_account.key,
        rent_lamports,
        required_u64,
        program_id,
    );
    invoke_signed(
        &create_ix,
        &[
            authority_account.clone(),
            session_account.clone(),
            system_program_account.clone(),
        ],
        &[&signer_seeds],
    )?;

    Ok(())
}

pub fn process_instruction_accounts(
    program_id: &Pubkey,
    accounts: &[AccountInfo],
    instruction_data: &[u8],
) -> ProgramResult {
    let instruction = decode_instruction(instruction_data)?;
    let mut account_iter = accounts.iter();

    let authority_account = next_account_info(&mut account_iter)
        .map_err(|_| SessionError::MissingRequiredAccount("authority"))?;
    let session_account = next_account_info(&mut account_iter)
        .map_err(|_| SessionError::MissingRequiredAccount("session"))?;
    let third_account = account_iter.next();

    if !authority_account.is_signer {
        return Err(SessionError::MissingRequiredSignature.into());
    }
    if !session_account.is_writable {
        return Err(SessionError::SessionAccountNotWritable.into());
    }
    let vk_account: Option<&AccountInfo>;
    let mut preloaded_session = None;
    let expected_bind_hash = match &instruction {
        VerifierInstruction::InitSession(payload) => {
            let (expected_pda, _) =
                derive_session_pda(program_id, authority_account.key, &payload.session_bind_hash);
            if expected_pda != *session_account.key {
                return Err(SessionError::InvalidSessionAccountPda.into());
            }

            if session_account.owner == program_id {
                vk_account = third_account;
            } else {
                maybe_create_session_account_for_init(
                    program_id,
                    authority_account,
                    session_account,
                    third_account,
                    payload,
                )?;
                vk_account = account_iter.next();
            }
            payload.session_bind_hash
        }
        _ => {
            vk_account = third_account;
            if session_account.owner != program_id {
                return Err(SessionError::InvalidAccountOwner.into());
            }
            let session = {
                let data_ref = session_account
                    .try_borrow_data()
                    .map_err(|_| SessionError::InvalidSessionAccountData)?;
                SessionAccount::deserialize_from_account_data(&data_ref)?
            };
            if !session.initialized {
                return Err(SessionError::SessionNotInitialized.into());
            }
            let bind_hash = session.session_bind_hash;
            preloaded_session = Some(session);
            bind_hash
        }
    };

    let (expected_pda, _) =
        derive_session_pda(program_id, authority_account.key, &expected_bind_hash);
    if expected_pda != *session_account.key {
        return Err(SessionError::InvalidSessionAccountPda.into());
    }

    if let VerifierInstruction::InitSession(payload) = &instruction {
        let required = SessionAccount::serialized_size_for_totals(
            payload.proof_total_len,
            payload.public_inputs_total_len,
        )?;
        if session_account.data_len() < required {
            return Err(SessionError::SessionAccountDataTooSmall {
                required,
                actual: session_account.data_len(),
            }
            .into());
        }
    }

    validate_vk_binding_if_present(&instruction, vk_account, program_id)?;

    let mut session = if let Some(session) = preloaded_session {
        session
    } else {
        let data_ref = session_account
            .try_borrow_data()
            .map_err(|_| SessionError::InvalidSessionAccountData)?;
        SessionAccount::deserialize_from_account_data(&data_ref)?
    };

    let result =
        process_decoded_instruction(&mut session, authority_account.key.to_bytes(), instruction)?;

    {
        let mut data_ref = session_account
            .try_borrow_mut_data()
            .map_err(|_| SessionError::InvalidSessionAccountData)?;
        session.serialize_into_account_data(&mut data_ref)?;
    }

    if let Some(value) = result {
        set_return_data(&[u8::from(value)]);
    }

    Ok(())
}

#[cfg(feature = "sbf-entrypoint")]
entrypoint!(entrypoint_process_instruction);
pub fn entrypoint_process_instruction(
    program_id: &Pubkey,
    accounts: &[AccountInfo],
    instruction_data: &[u8],
) -> ProgramResult {
    match process_instruction_accounts(program_id, accounts, instruction_data) {
        Ok(()) => Ok(()),
        Err(err) => {
            msg!("ultrahonk verifier error: {:?}", err);
            Err(err)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn bytes32(byte: u8) -> [u8; 32] {
        [byte; 32]
    }

    fn init_payload(proof_len: u64, public_inputs_len: u64) -> InitSessionPayload {
        InitSessionPayload {
            vk_hash: bytes32(1),
            proof_total_len: proof_len,
            public_inputs_total_len: public_inputs_len,
            proof_hash: bytes32(2),
            public_inputs_hash: bytes32(3),
            session_bind_hash: bytes32(4),
        }
    }

    fn valid_init_payload(
        authority: [u8; 32],
        proof_bytes: &[u8],
        public_inputs_bytes: &[u8],
    ) -> InitSessionPayload {
        let vk_hash = bytes32(1);
        let proof_hash = compute_proof_hash_bytes(proof_bytes);
        let public_inputs_hash = compute_public_inputs_hash_bytes(public_inputs_bytes);
        let session_bind_hash = compute_session_bind_hash_bytes(
            &vk_hash,
            &proof_hash,
            &public_inputs_hash,
            &authority,
            proof_bytes.len() as u64,
            public_inputs_bytes.len() as u64,
        );
        InitSessionPayload {
            vk_hash,
            proof_total_len: proof_bytes.len() as u64,
            public_inputs_total_len: public_inputs_bytes.len() as u64,
            proof_hash,
            public_inputs_hash,
            session_bind_hash,
        }
    }

    fn sample_canonical_proof_bytes() -> Vec<u8> {
        fn push_scalar(out: &mut Vec<u8>, scalar: [u8; 32]) {
            out.extend_from_slice(&scalar);
        }
        fn push_commitment(out: &mut Vec<u8>, point: &[u8; 64]) {
            out.extend_from_slice(&point[..32]);
            out.extend_from_slice(&point[32..]);
        }

        const BASE_POINT: [u8; 64] = [
            45, 206, 255, 166, 152, 55, 128, 138, 79, 217, 145, 164, 25, 74, 120, 234, 234, 217,
            68, 149, 162, 44, 133, 120, 184, 205, 12, 44, 175, 98, 168, 172, 20, 24, 216, 15, 209,
            175, 106, 75, 147, 236, 90, 101, 123, 219, 245, 151, 209, 202, 218, 104, 148, 8, 32,
            254, 243, 191, 218, 122, 42, 81, 193, 84,
        ];
        const NEG_POINT: [u8; 64] = [
            45, 206, 255, 166, 152, 55, 128, 138, 79, 217, 145, 164, 25, 74, 120, 234, 234, 217,
            68, 149, 162, 44, 133, 120, 184, 205, 12, 44, 175, 98, 168, 172, 28, 75, 118, 99, 15,
            130, 53, 222, 36, 99, 235, 81, 5, 165, 98, 197, 197, 182, 144, 40, 212, 105, 169, 142,
            72, 96, 177, 156, 174, 43, 59, 243,
        ];

        let mut proof = Vec::new();
        let zero_scalar = [0u8; 32];

        // 16 special public input field elements (two pairing points, all-zero = infinity).
        for _ in 0..16 {
            push_scalar(&mut proof, zero_scalar);
        }

        // 8 commitments before sumcheck.
        push_commitment(&mut proof, &BASE_POINT); // W_L
        push_commitment(&mut proof, &NEG_POINT); // W_R
        push_commitment(&mut proof, &BASE_POINT); // W_O
        push_commitment(&mut proof, &BASE_POINT); // LOOKUP_READ_COUNTS
        push_commitment(&mut proof, &BASE_POINT); // LOOKUP_READ_TAGS
        push_commitment(&mut proof, &BASE_POINT); // W_4
        push_commitment(&mut proof, &BASE_POINT); // LOOKUP_INVERSES
        push_commitment(&mut proof, &BASE_POINT); // Z_PERM

        // Non-ZK sample uses log_n=2:
        // sumcheck_univariates: 2 * 8
        for _ in 0..16 {
            push_scalar(&mut proof, zero_scalar);
        }
        // sumcheck_evaluations: 41
        for _ in 0..41 {
            push_scalar(&mut proof, zero_scalar);
        }

        // Gemini fold commitments: log_n - 1 = 1
        push_commitment(&mut proof, &BASE_POINT);
        // Gemini fold evaluations: log_n = 2
        push_scalar(&mut proof, zero_scalar);
        push_scalar(&mut proof, zero_scalar);

        // Shplonk Q and KZG W commitments.
        push_commitment(&mut proof, &BASE_POINT);
        push_commitment(&mut proof, &BASE_POINT);
        proof
    }

    #[test]
    fn decode_roundtrip() {
        let instruction = VerifierInstruction::AppendProofChunk(AppendChunkPayload {
            chunk_index: 1,
            offset: 8,
            chunk_bytes: vec![1, 2, 3, 4],
        });
        let encoded = encode_instruction(&instruction);
        let decoded = decode_instruction(&encoded).expect("decode should succeed");
        assert_eq!(decoded, instruction);
    }

    #[test]
    fn happy_path_staged_session() {
        let authority = bytes32(7);
        let proof_bytes = sample_canonical_proof_bytes();
        let public_inputs_bytes = {
            let mut bytes = vec![0u8; 32];
            bytes[31] = 1;
            bytes
        };
        let mut session = SessionAccount::default();

        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::InitSession(valid_init_payload(
                authority,
                &proof_bytes,
                &public_inputs_bytes,
            ))),
        )
        .expect("init should succeed");

        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::AppendProofChunk(AppendChunkPayload {
                chunk_index: 0,
                offset: 0,
                chunk_bytes: proof_bytes.clone(),
            })),
        )
        .expect("append proof");

        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::AppendPublicInputsChunk(
                AppendChunkPayload {
                    chunk_index: 0,
                    offset: 0,
                    chunk_bytes: public_inputs_bytes.clone(),
                },
            )),
        )
        .expect("append public inputs");

        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 0,
            }),
        )
        .expect("stage 0");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 1,
            }),
        )
        .expect("stage 1");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 2,
            }),
        )
        .expect("stage 2");

        let result = process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::Finalize),
        )
        .expect("finalize should succeed");
        assert_eq!(result, Some(true));
        assert_eq!(session.status, SessionStatus::Verified);
    }

    #[test]
    fn rejects_non_contiguous_chunk_index() {
        let authority = bytes32(9);
        let mut session = SessionAccount::default();
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::InitSession(init_payload(8, 0))),
        )
        .expect("init should succeed");

        let err = process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::AppendProofChunk(AppendChunkPayload {
                chunk_index: 1,
                offset: 0,
                chunk_bytes: vec![1, 2, 3, 4],
            })),
        )
        .expect_err("must reject non-contiguous index");
        assert_eq!(
            err,
            SessionError::NonContiguousChunkIndex {
                expected: 0,
                got: 1
            }
        );
    }

    #[test]
    fn rejects_offset_tampering() {
        let authority = bytes32(8);
        let mut session = SessionAccount::default();
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::InitSession(init_payload(8, 0))),
        )
        .expect("init should succeed");

        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::AppendProofChunk(AppendChunkPayload {
                chunk_index: 0,
                offset: 0,
                chunk_bytes: vec![1, 2, 3, 4],
            })),
        )
        .expect("first proof chunk must succeed");

        let err = process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::AppendProofChunk(AppendChunkPayload {
                chunk_index: 1,
                offset: 6,
                chunk_bytes: vec![5, 6, 7, 8],
            })),
        )
        .expect_err("must reject mismatched offset");
        assert_eq!(
            err,
            SessionError::OffsetMismatch {
                expected: 4,
                got: 6
            }
        );
    }

    #[test]
    fn hash_checks_stage_fails_when_reassembled_proof_hash_mismatches() {
        let authority = bytes32(3);
        let proof_bytes = sample_canonical_proof_bytes();
        let mut public_inputs_bytes = vec![0u8; 32];
        public_inputs_bytes[31] = 5;

        let mut session = SessionAccount::default();
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::InitSession(valid_init_payload(
                authority,
                &proof_bytes,
                &public_inputs_bytes,
            ))),
        )
        .expect("init");

        // Inject a tampered proof chunk while preserving chunking rules.
        let mut tampered_proof = proof_bytes.clone();
        tampered_proof[0] ^= 0xFF;
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::AppendProofChunk(AppendChunkPayload {
                chunk_index: 0,
                offset: 0,
                chunk_bytes: tampered_proof,
            })),
        )
        .expect("proof chunk 0");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::AppendPublicInputsChunk(
                AppendChunkPayload {
                    chunk_index: 0,
                    offset: 0,
                    chunk_bytes: public_inputs_bytes.clone(),
                },
            )),
        )
        .expect("public inputs");

        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 0,
            }),
        )
        .expect("stage 0");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 1,
            }),
        )
        .expect("stage 1");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 2,
            }),
        )
        .expect("stage 2");

        let result = process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::Finalize),
        )
        .expect("finalize");
        assert_eq!(result, Some(false));
        assert_eq!(session.status, SessionStatus::Failed);
    }

    #[test]
    fn decode_stage_fails_on_non_canonical_public_input_elements() {
        let authority = bytes32(4);
        let proof_bytes = sample_canonical_proof_bytes();
        // Equal to BN254 modulus, which is non-canonical as a field element.
        let invalid_public_inputs = BN254_SCALAR_MODULUS_BE.to_vec();
        let mut session = SessionAccount::default();

        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::InitSession(valid_init_payload(
                authority,
                &proof_bytes,
                &invalid_public_inputs,
            ))),
        )
        .expect("init");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::AppendProofChunk(AppendChunkPayload {
                chunk_index: 0,
                offset: 0,
                chunk_bytes: proof_bytes,
            })),
        )
        .expect("proof chunk 0");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::AppendPublicInputsChunk(
                AppendChunkPayload {
                    chunk_index: 0,
                    offset: 0,
                    chunk_bytes: invalid_public_inputs,
                },
            )),
        )
        .expect("public inputs");

        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 0,
            }),
        )
        .expect("stage 0");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 1,
            }),
        )
        .expect("stage 1");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 2,
            }),
        )
        .expect("stage 2");

        let result = process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::Finalize),
        )
        .expect("finalize");
        assert_eq!(result, Some(false));
        assert_eq!(session.status, SessionStatus::Failed);
    }

    #[test]
    fn decode_stage_rejects_public_input_limb_outside_fr_but_inside_fq() {
        let authority = bytes32(5);
        let proof_bytes = sample_canonical_proof_bytes();
        // This limb is canonical in BN254 base field Fq, but not in scalar field Fr.
        let invalid_public_inputs = vec![
            0x30, 0x64, 0x4e, 0x72, 0xe1, 0x31, 0xa0, 0x29, 0xb8, 0x50, 0x45, 0xb6, 0x81, 0x81,
            0x58, 0x5d, 0x97, 0x81, 0x6a, 0x91, 0x68, 0x71, 0xca, 0x8d, 0x3c, 0x20, 0x8c, 0x16,
            0xd8, 0x7c, 0xfd, 0x46,
        ];
        let mut session = SessionAccount::default();

        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::InitSession(valid_init_payload(
                authority,
                &proof_bytes,
                &invalid_public_inputs,
            ))),
        )
        .expect("init");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::AppendProofChunk(AppendChunkPayload {
                chunk_index: 0,
                offset: 0,
                chunk_bytes: proof_bytes,
            })),
        )
        .expect("proof chunk 0");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::AppendPublicInputsChunk(
                AppendChunkPayload {
                    chunk_index: 0,
                    offset: 0,
                    chunk_bytes: invalid_public_inputs,
                },
            )),
        )
        .expect("public inputs");

        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 0,
            }),
        )
        .expect("stage 0");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 1,
            }),
        )
        .expect("stage 1");
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 2,
            }),
        )
        .expect("stage 2");

        let result = process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::Finalize),
        )
        .expect("finalize");
        assert_eq!(result, Some(false));
        assert_eq!(session.status, SessionStatus::Failed);
    }

    #[test]
    fn rejects_finalize_before_all_stages() {
        let authority = bytes32(1);
        let mut session = SessionAccount::default();
        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::InitSession(init_payload(0, 0))),
        )
        .expect("init should succeed");

        process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 0,
            }),
        )
        .expect("first stage");

        let err = process_instruction(
            &mut session,
            authority,
            &encode_instruction(&VerifierInstruction::Finalize),
        )
        .expect_err("must reject finalize before all stages");
        assert_eq!(err, SessionError::FinalizeBeforeAllStages);
    }

    #[test]
    fn native_verify_stage_boundary_rejects_mismatched_buffer_lengths() {
        let mut session = SessionAccount::default();
        session.proof_total_len = 8;
        session.public_inputs_total_len = 32;
        session.proof_store = vec![1u8; 4];
        session.public_inputs_store = {
            let mut bytes = vec![0u8; 32];
            bytes[31] = 1;
            bytes
        };
        assert!(!evaluate_native_verify_stage(&session));
    }

    #[test]
    fn native_verify_stage_boundary_accepts_well_formed_matching_commitments() {
        let mut session = SessionAccount::default();
        session.proof_store = sample_canonical_proof_bytes();
        session.public_inputs_store = {
            let mut bytes = vec![0u8; 32];
            bytes[31] = 1;
            bytes
        };
        session.proof_total_len = session.proof_store.len() as u64;
        session.public_inputs_total_len = session.public_inputs_store.len() as u64;
        session.vk_hash = bytes32(1);
        session.proof_hash = compute_proof_hash_bytes(&session.proof_store);
        session.public_inputs_hash = compute_public_inputs_hash_bytes(&session.public_inputs_store);
        session.authority = bytes32(3);
        session.session_bind_hash = compute_session_bind_hash_bytes(
            &session.vk_hash,
            &session.proof_hash,
            &session.public_inputs_hash,
            &session.authority,
            session.proof_total_len,
            session.public_inputs_total_len,
        );

        assert!(evaluate_native_verify_stage(&session));
    }

    #[test]
    fn native_verify_stage_boundary_does_not_recheck_hash_commitments() {
        let mut session = SessionAccount::default();
        session.proof_store = sample_canonical_proof_bytes();
        session.public_inputs_store = {
            let mut bytes = vec![0u8; 32];
            bytes[31] = 1;
            bytes
        };
        session.proof_total_len = session.proof_store.len() as u64;
        session.public_inputs_total_len = session.public_inputs_store.len() as u64;
        session.vk_hash = bytes32(1);
        session.proof_hash = bytes32(99);
        session.public_inputs_hash = compute_public_inputs_hash_bytes(&session.public_inputs_store);
        session.authority = bytes32(3);
        session.session_bind_hash = compute_session_bind_hash_bytes(
            &session.vk_hash,
            &compute_proof_hash_bytes(&session.proof_store),
            &session.public_inputs_hash,
            &session.authority,
            session.proof_total_len,
            session.public_inputs_total_len,
        );

        assert!(evaluate_native_verify_stage(&session));
    }

    fn make_account_info<'a>(
        key: &'a Pubkey,
        is_signer: bool,
        is_writable: bool,
        lamports: &'a mut u64,
        data: &'a mut [u8],
        owner: &'a Pubkey,
    ) -> AccountInfo<'a> {
        AccountInfo::new(key, is_signer, is_writable, lamports, data, owner, false)
    }

    #[test]
    fn runtime_init_persists_session_state() {
        let program_id = Pubkey::new_unique();
        let authority_key = Pubkey::new_unique();
        let session_bind_hash = bytes32(9);
        let (session_pda, _) = derive_session_pda(&program_id, &authority_key, &session_bind_hash);

        let payload = InitSessionPayload {
            session_bind_hash,
            ..init_payload(8, 4)
        };
        let init_ix = encode_instruction(&VerifierInstruction::InitSession(payload.clone()));

        let mut authority_lamports = 0u64;
        let mut authority_data = [];
        let mut session_lamports = 0u64;
        let required = SessionAccount::serialized_size_for_totals(8, 4).expect("required size");
        let mut session_data = vec![0u8; required + 64];

        let authority_owner = Pubkey::new_unique();
        let authority_ai = make_account_info(
            &authority_key,
            true,
            false,
            &mut authority_lamports,
            &mut authority_data,
            &authority_owner,
        );
        let session_ai = make_account_info(
            &session_pda,
            false,
            true,
            &mut session_lamports,
            &mut session_data,
            &program_id,
        );
        let accounts = vec![authority_ai, session_ai];

        process_instruction_accounts(&program_id, &accounts, &init_ix)
            .expect("runtime init should succeed");

        let stored = {
            let data = accounts[1].try_borrow_data().expect("borrow session data");
            SessionAccount::deserialize_from_account_data(&data)
                .expect("deserialize stored session")
        };
        assert!(stored.initialized);
        assert_eq!(stored.authority, authority_key.to_bytes());
        assert_eq!(stored.session_bind_hash, session_bind_hash);
        assert_eq!(stored.proof_store.len(), 8);
        assert_eq!(stored.public_inputs_store.len(), 4);
    }

    #[test]
    fn runtime_rejects_non_pda_session_account() {
        let program_id = Pubkey::new_unique();
        let authority_key = Pubkey::new_unique();
        let session_bind_hash = bytes32(10);
        let payload = InitSessionPayload {
            session_bind_hash,
            ..init_payload(0, 0)
        };
        let init_ix = encode_instruction(&VerifierInstruction::InitSession(payload));

        let mut authority_lamports = 0u64;
        let mut authority_data = [];
        let mut session_lamports = 0u64;
        let mut session_data =
            vec![0u8; SessionAccount::serialized_size_for_totals(0, 0).expect("size")];

        let authority_owner = Pubkey::new_unique();
        let authority_ai = make_account_info(
            &authority_key,
            true,
            false,
            &mut authority_lamports,
            &mut authority_data,
            &authority_owner,
        );
        let wrong_session_key = Pubkey::new_unique();
        let session_ai = make_account_info(
            &wrong_session_key,
            false,
            true,
            &mut session_lamports,
            &mut session_data,
            &program_id,
        );
        let accounts = vec![authority_ai, session_ai];

        let err = process_instruction_accounts(&program_id, &accounts, &init_ix)
            .expect_err("should reject non-PDA account");
        assert_eq!(
            err,
            ProgramError::Custom(SessionError::InvalidSessionAccountPda.code())
        );
    }

    #[test]
    fn runtime_rejects_session_owner_mismatch() {
        let program_id = Pubkey::new_unique();
        let authority_key = Pubkey::new_unique();
        let session_bind_hash = bytes32(11);
        let (session_pda, _) = derive_session_pda(&program_id, &authority_key, &session_bind_hash);

        let payload = InitSessionPayload {
            session_bind_hash,
            ..init_payload(0, 0)
        };
        let init_ix = encode_instruction(&VerifierInstruction::InitSession(payload));

        let mut authority_lamports = 0u64;
        let mut authority_data = [];
        let mut session_lamports = 0u64;
        let mut session_data =
            vec![0u8; SessionAccount::serialized_size_for_totals(0, 0).expect("size")];

        let authority_owner = Pubkey::new_unique();
        let authority_ai = make_account_info(
            &authority_key,
            true,
            false,
            &mut authority_lamports,
            &mut authority_data,
            &authority_owner,
        );
        let wrong_owner = Pubkey::new_unique();
        let session_ai = make_account_info(
            &session_pda,
            false,
            true,
            &mut session_lamports,
            &mut session_data,
            &wrong_owner,
        );
        let accounts = vec![authority_ai, session_ai];

        let err = process_instruction_accounts(&program_id, &accounts, &init_ix)
            .expect_err("owner mismatch must fail");
        assert_eq!(
            err,
            ProgramError::Custom(SessionError::InvalidAccountOwner.code())
        );
    }

    #[test]
    fn runtime_rejects_missing_authority_signature() {
        let program_id = Pubkey::new_unique();
        let authority_key = Pubkey::new_unique();
        let session_bind_hash = bytes32(12);
        let (session_pda, _) = derive_session_pda(&program_id, &authority_key, &session_bind_hash);

        let payload = InitSessionPayload {
            session_bind_hash,
            ..init_payload(0, 0)
        };
        let init_ix = encode_instruction(&VerifierInstruction::InitSession(payload));

        let mut authority_lamports = 0u64;
        let mut authority_data = [];
        let mut session_lamports = 0u64;
        let mut session_data =
            vec![0u8; SessionAccount::serialized_size_for_totals(0, 0).expect("size")];

        let authority_owner = Pubkey::new_unique();
        let authority_ai = make_account_info(
            &authority_key,
            false,
            false,
            &mut authority_lamports,
            &mut authority_data,
            &authority_owner,
        );
        let session_ai = make_account_info(
            &session_pda,
            false,
            true,
            &mut session_lamports,
            &mut session_data,
            &program_id,
        );
        let accounts = vec![authority_ai, session_ai];

        let err = process_instruction_accounts(&program_id, &accounts, &init_ix)
            .expect_err("unsigned authority must fail");
        assert_eq!(
            err,
            ProgramError::Custom(SessionError::MissingRequiredSignature.code())
        );
    }

    #[test]
    fn runtime_rejects_vk_hash_mismatch_when_vk_provided() {
        let program_id = Pubkey::new_unique();
        let authority_key = Pubkey::new_unique();
        let session_bind_hash = bytes32(13);
        let (session_pda, _) = derive_session_pda(&program_id, &authority_key, &session_bind_hash);

        let payload = InitSessionPayload {
            session_bind_hash,
            ..init_payload(0, 0)
        };
        let init_ix = encode_instruction(&VerifierInstruction::InitSession(payload));

        let mut authority_lamports = 0u64;
        let mut authority_data = [];
        let mut session_lamports = 0u64;
        let mut session_data =
            vec![0u8; SessionAccount::serialized_size_for_totals(0, 0).expect("size")];
        let mut vk_lamports = 0u64;
        let mut vk_data = vec![0u8; 64];
        vk_data[..32].copy_from_slice(&bytes32(99));

        let authority_owner = Pubkey::new_unique();
        let authority_ai = make_account_info(
            &authority_key,
            true,
            false,
            &mut authority_lamports,
            &mut authority_data,
            &authority_owner,
        );
        let session_ai = make_account_info(
            &session_pda,
            false,
            true,
            &mut session_lamports,
            &mut session_data,
            &program_id,
        );
        let vk_key = Pubkey::new_unique();
        let vk_ai = make_account_info(
            &vk_key,
            false,
            false,
            &mut vk_lamports,
            &mut vk_data,
            &program_id,
        );
        let accounts = vec![authority_ai, session_ai, vk_ai];

        let err = process_instruction_accounts(&program_id, &accounts, &init_ix)
            .expect_err("vk hash mismatch must fail");
        assert_eq!(
            err,
            ProgramError::Custom(SessionError::InvalidVkHashBinding.code())
        );
    }
}
