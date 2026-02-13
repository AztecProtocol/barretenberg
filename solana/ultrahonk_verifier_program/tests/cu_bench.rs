#![allow(deprecated)]

use std::{
    fs,
    path::{Path, PathBuf},
};

use solana_account::Account;
use solana_compute_budget_interface::ComputeBudgetInstruction;
use solana_instruction::{AccountMeta, Instruction};
use solana_keypair::Keypair;
use solana_program::keccak::hashv as keccak_hashv;
use solana_program_test::{processor, ProgramTest, ProgramTestContext};
use solana_pubkey::Pubkey;
use solana_signer::Signer;
use solana_transaction::Transaction;
use ultrahonk_verifier_program::{
    compute_proof_hash_bytes, compute_public_inputs_hash_bytes, compute_session_bind_hash_bytes,
    derive_session_pda, encode_instruction, entrypoint_process_instruction, AppendChunkPayload,
    InitSessionPayload, SessionAccount, SessionStatus, VerifierInstruction,
};

const COMPUTE_UNIT_LIMIT: u32 = 1_400_000;
const COMPUTE_HEAP_FRAME_BYTES: u32 = 256 * 1024;
const PROOF_CHUNK_SIZE: usize = 256;
const PUBLIC_INPUTS_CHUNK_SIZE: usize = 64;
const PRIMARY_FIXTURE_ROOT_RELATIVE: &str = "tests/fixtures/bb_generated/main_keccak_nonzk";
const SECONDARY_FIXTURE_ROOT_RELATIVE: &str =
    "tests/fixtures/bb_generated/ecdsa_regression_keccak_nonzk";
const BN254_SCALAR_MODULUS_BE: [u8; 32] = [
    0x30, 0x64, 0x4e, 0x72, 0xe1, 0x31, 0xa0, 0x29, 0xb8, 0x50, 0x45, 0xb6, 0x81, 0x81, 0x58, 0x5d,
    0x28, 0x33, 0xe8, 0x48, 0x79, 0xb9, 0x70, 0x91, 0x43, 0xe1, 0xf5, 0x93, 0xf0, 0x00, 0x00, 0x01,
];
const SPECIAL_PUBLIC_INPUTS_BYTES: usize = 16 * 32;
const COMMITMENT_BYTES: usize = 64;
const FIRST_COMMITMENT_OFFSET: usize = SPECIAL_PUBLIC_INPUTS_BYTES;
const NON_ZK_U256_COMMITMENT_COUNT: usize = 8;
const NON_ZK_U256_BASE_ELEMENTS: usize = 75;
const NON_ZK_U256_PER_ROUND_ELEMENTS: usize = 11;
const NON_ZK_BATCHED_RELATION_PARTIAL_LENGTH: usize = 8;
const COMMITMENT_WINDOW_TRUNCATION_BYTES: usize = 16;

struct FixtureData {
    proof_bytes: Vec<u8>,
    public_inputs_bytes: Vec<u8>,
    vk_hash: [u8; 32],
    source: String,
}

struct StagewiseCuBreakdown {
    init_session: u64,
    append_proof_chunk: Vec<u64>,
    append_public_inputs_chunk: Vec<u64>,
    verify_stage_hash_checks: u64,
    verify_stage_decode_inputs: u64,
    verify_stage_native_verify: u64,
    finalize: u64,
    total_session_flow: u64,
}

fn fixture_dir(relative: &str) -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join(relative)
}

fn read_non_empty_file(path: &Path) -> Option<Vec<u8>> {
    let bytes = fs::read(path).ok()?;
    if bytes.is_empty() {
        return None;
    }
    Some(bytes)
}

fn be_bytes_to_le_u64x4(bytes: &[u8; 32]) -> [u64; 4] {
    [
        u64::from_be_bytes(bytes[24..32].try_into().expect("fixed size slice")),
        u64::from_be_bytes(bytes[16..24].try_into().expect("fixed size slice")),
        u64::from_be_bytes(bytes[8..16].try_into().expect("fixed size slice")),
        u64::from_be_bytes(bytes[0..8].try_into().expect("fixed size slice")),
    ]
}

fn le_u64x4_to_be_bytes(limbs: [u64; 4]) -> [u8; 32] {
    let mut out = [0u8; 32];
    out[0..8].copy_from_slice(&limbs[3].to_be_bytes());
    out[8..16].copy_from_slice(&limbs[2].to_be_bytes());
    out[16..24].copy_from_slice(&limbs[1].to_be_bytes());
    out[24..32].copy_from_slice(&limbs[0].to_be_bytes());
    out
}

fn cmp_le_u64x4(a: [u64; 4], b: [u64; 4]) -> core::cmp::Ordering {
    for idx in (0..4).rev() {
        if a[idx] < b[idx] {
            return core::cmp::Ordering::Less;
        }
        if a[idx] > b[idx] {
            return core::cmp::Ordering::Greater;
        }
    }
    core::cmp::Ordering::Equal
}

fn sub_le_u64x4(a: [u64; 4], b: [u64; 4]) -> Option<[u64; 4]> {
    let mut out = [0u64; 4];
    let mut borrow = 0u64;
    for idx in 0..4 {
        let (tmp, borrow_a) = a[idx].overflowing_sub(b[idx]);
        let (res, borrow_b) = tmp.overflowing_sub(borrow);
        out[idx] = res;
        borrow = (borrow_a as u64) | (borrow_b as u64);
    }
    if borrow != 0 {
        return None;
    }
    Some(out)
}

fn reduce_u256_mod_fr(bytes: &[u8; 32]) -> Option<[u8; 32]> {
    let modulus = be_bytes_to_le_u64x4(&BN254_SCALAR_MODULUS_BE);
    let mut value = be_bytes_to_le_u64x4(bytes);
    while cmp_le_u64x4(value, modulus) != core::cmp::Ordering::Less {
        value = sub_le_u64x4(value, modulus)?;
    }
    Some(le_u64x4_to_be_bytes(value))
}

fn compute_vk_hash_from_vk_bytes(vk_bytes: &[u8]) -> Option<[u8; 32]> {
    if vk_bytes.is_empty() || vk_bytes.len() % 32 != 0 {
        return None;
    }
    let mut field_elements = Vec::with_capacity(vk_bytes.len() / 32);
    for chunk in vk_bytes.chunks_exact(32) {
        field_elements.push(<[u8; 32]>::try_from(chunk).ok()?);
    }
    let field_slices: Vec<&[u8]> = field_elements
        .iter()
        .map(|element| element.as_slice())
        .collect();
    reduce_u256_mod_fr(&keccak_hashv(&field_slices).to_bytes())
}

fn load_fixture(relative: &str) -> Option<FixtureData> {
    let fixture_root = fixture_dir(relative);
    let fixture_proof = read_non_empty_file(&fixture_root.join("proof"));
    let fixture_public_inputs = read_non_empty_file(&fixture_root.join("public_inputs"));
    let fixture_vk = read_non_empty_file(&fixture_root.join("vk"));
    let (proof_bytes, public_inputs_bytes, vk_hash) = (
        fixture_proof?,
        fixture_public_inputs?,
        compute_vk_hash_from_vk_bytes(&fixture_vk?)?,
    );
    Some(FixtureData {
        proof_bytes,
        public_inputs_bytes,
        vk_hash,
        source: format!("bb fixture: {}", fixture_root.display()),
    })
}

fn load_primary_fixture_or_fallback() -> FixtureData {
    if let Some(fixture) = load_fixture(PRIMARY_FIXTURE_ROOT_RELATIVE) {
        return fixture;
    }
    FixtureData {
        proof_bytes: sample_proof_bytes(),
        public_inputs_bytes: sample_public_inputs_bytes(),
        vk_hash: [7u8; 32],
        source: "embedded sample proof/public_inputs fallback".to_string(),
    }
}

fn load_secondary_fixture() -> Option<FixtureData> {
    load_fixture(SECONDARY_FIXTURE_ROOT_RELATIVE)
}

fn swap_commitment_sections(
    proof_bytes: &[u8],
    first_commitment_index: usize,
    second_commitment_index: usize,
) -> Option<Vec<u8>> {
    let mut tampered = proof_bytes.to_vec();
    let first_start = FIRST_COMMITMENT_OFFSET
        .checked_add(first_commitment_index.checked_mul(COMMITMENT_BYTES)?)?;
    let second_start = FIRST_COMMITMENT_OFFSET
        .checked_add(second_commitment_index.checked_mul(COMMITMENT_BYTES)?)?;
    let first_end = first_start.checked_add(COMMITMENT_BYTES)?;
    let second_end = second_start.checked_add(COMMITMENT_BYTES)?;
    if first_end > tampered.len() || second_end > tampered.len() {
        return None;
    }
    for idx in 0..COMMITMENT_BYTES {
        tampered.swap(first_start + idx, second_start + idx);
    }
    Some(tampered)
}

fn truncate_commitment_window(proof_bytes: &[u8]) -> Option<Vec<u8>> {
    let start = FIRST_COMMITMENT_OFFSET
        .checked_add(COMMITMENT_BYTES)?
        .checked_sub(COMMITMENT_WINDOW_TRUNCATION_BYTES)?;
    let end = start.checked_add(COMMITMENT_WINDOW_TRUNCATION_BYTES)?;
    if end > proof_bytes.len() {
        return None;
    }

    let mut tampered = Vec::with_capacity(proof_bytes.len());
    tampered.extend_from_slice(&proof_bytes[..start]);
    tampered.extend_from_slice(&proof_bytes[end..]);
    tampered.extend_from_slice(&proof_bytes[start..end]);
    Some(tampered)
}

fn tamper_first_sumcheck_scalar_non_canonical(proof_bytes: &[u8]) -> Option<Vec<u8>> {
    let sumcheck_start = SPECIAL_PUBLIC_INPUTS_BYTES
        .checked_add(NON_ZK_U256_COMMITMENT_COUNT.checked_mul(COMMITMENT_BYTES)?)?;
    let sumcheck_end = sumcheck_start.checked_add(32)?;
    if sumcheck_end > proof_bytes.len() {
        return None;
    }
    let mut tampered = proof_bytes.to_vec();
    tampered[sumcheck_start..sumcheck_end].copy_from_slice(&BN254_SCALAR_MODULUS_BE);
    Some(tampered)
}

fn tamper_first_sumcheck_round_relation(proof_bytes: &[u8]) -> Option<Vec<u8>> {
    let sumcheck_start = SPECIAL_PUBLIC_INPUTS_BYTES
        .checked_add(NON_ZK_U256_COMMITMENT_COUNT.checked_mul(COMMITMENT_BYTES)?)?;
    let sumcheck_end = sumcheck_start.checked_add(32)?;
    if sumcheck_end > proof_bytes.len() {
        return None;
    }
    let mut tampered = proof_bytes.to_vec();
    // Mutate S_0(0) while keeping it canonical to violate the round-0 relation.
    tampered[sumcheck_start + 31] ^= 1;
    Some(tampered)
}

fn tamper_pairing_special_point_invalid(proof_bytes: &[u8]) -> Option<Vec<u8>> {
    if proof_bytes.len() < SPECIAL_PUBLIC_INPUTS_BYTES {
        return None;
    }
    let mut tampered = proof_bytes.to_vec();
    // Pairing point coordinate high limbs must fit in 52 bits.
    // Set x high limb (scalar[3]) to 2^52 to violate width constraints while staying canonical.
    tampered[(3 * 32) + 25] = 0x10;
    Some(tampered)
}

fn tamper_second_sumcheck_round_transition(proof_bytes: &[u8]) -> Option<Vec<u8>> {
    let sumcheck_start = SPECIAL_PUBLIC_INPUTS_BYTES
        .checked_add(NON_ZK_U256_COMMITMENT_COUNT.checked_mul(COMMITMENT_BYTES)?)?;
    let second_round_start =
        sumcheck_start.checked_add(NON_ZK_BATCHED_RELATION_PARTIAL_LENGTH.checked_mul(32)?)?;
    let second_round_end = second_round_start.checked_add(32)?;
    if second_round_end > proof_bytes.len() {
        return None;
    }
    let mut tampered = proof_bytes.to_vec();
    // Mutate S_1(0) while keeping it canonical to violate strict round-transition equality.
    tampered[second_round_start + 31] ^= 1;
    Some(tampered)
}

fn tamper_shplonk_q_commitment_invalid(proof_bytes: &[u8]) -> Option<Vec<u8>> {
    let shplonk_q_start = proof_bytes.len().checked_sub(2 * COMMITMENT_BYTES)?;
    let shplonk_q_end = shplonk_q_start.checked_add(COMMITMENT_BYTES)?;
    if shplonk_q_end > proof_bytes.len() {
        return None;
    }
    let mut tampered = proof_bytes.to_vec();
    tampered[shplonk_q_start..shplonk_q_end].fill(0);
    // (x=1, y=1) is canonical but off-curve for BN254 G1.
    tampered[shplonk_q_start + 31] = 1;
    tampered[shplonk_q_start + 63] = 1;
    Some(tampered)
}

fn infer_non_zk_u256_virtual_log_n(proof_bytes: &[u8]) -> Option<usize> {
    if proof_bytes.is_empty() || proof_bytes.len() % 32 != 0 {
        return None;
    }
    let elements = proof_bytes.len() / 32;
    if elements < NON_ZK_U256_BASE_ELEMENTS {
        return None;
    }
    let delta = elements.checked_sub(NON_ZK_U256_BASE_ELEMENTS)?;
    if delta % NON_ZK_U256_PER_ROUND_ELEMENTS != 0 {
        return None;
    }
    let virtual_log_n = delta / NON_ZK_U256_PER_ROUND_ELEMENTS;
    if virtual_log_n == 0 {
        return None;
    }
    Some(virtual_log_n)
}

fn tamper_first_gemini_fold_commitment_invalid(proof_bytes: &[u8]) -> Option<Vec<u8>> {
    let virtual_log_n = infer_non_zk_u256_virtual_log_n(proof_bytes)?;
    let gemini_commitment_count = virtual_log_n.checked_sub(1)?;
    if gemini_commitment_count == 0 {
        return None;
    }
    let gemini_region_bytes = gemini_commitment_count.checked_mul(COMMITMENT_BYTES)?;
    let gemini_evaluations_bytes = virtual_log_n.checked_mul(32)?;
    let gemini_start = proof_bytes
        .len()
        .checked_sub(gemini_region_bytes)?
        .checked_sub(gemini_evaluations_bytes)?
        .checked_sub(COMMITMENT_BYTES)? // shplonk_q
        .checked_sub(COMMITMENT_BYTES)?; // kzg_w
    let gemini_end = gemini_start.checked_add(COMMITMENT_BYTES)?;
    if gemini_end > proof_bytes.len() {
        return None;
    }
    let mut tampered = proof_bytes.to_vec();
    tampered[gemini_start..gemini_end].fill(0);
    // (x=1, y=1) is canonical but off-curve for BN254 G1.
    tampered[gemini_start + 31] = 1;
    tampered[gemini_start + 63] = 1;
    Some(tampered)
}

fn tamper_first_wire_commitment_invalid(proof_bytes: &[u8]) -> Option<Vec<u8>> {
    let w_l_start = FIRST_COMMITMENT_OFFSET;
    let w_l_end = w_l_start.checked_add(COMMITMENT_BYTES)?;
    if w_l_end > proof_bytes.len() {
        return None;
    }
    let mut tampered = proof_bytes.to_vec();
    tampered[w_l_start..w_l_end].fill(0);
    // (x=1, y=1) is canonical but off-curve for BN254 G1.
    tampered[w_l_start + 31] = 1;
    tampered[w_l_start + 63] = 1;
    Some(tampered)
}

fn tamper_kzg_w_commitment_invalid(proof_bytes: &[u8]) -> Option<Vec<u8>> {
    let kzg_w_start = proof_bytes.len().checked_sub(COMMITMENT_BYTES)?;
    let kzg_w_end = kzg_w_start.checked_add(COMMITMENT_BYTES)?;
    if kzg_w_end > proof_bytes.len() {
        return None;
    }
    let mut tampered = proof_bytes.to_vec();
    tampered[kzg_w_start..kzg_w_end].fill(0);
    // (x=1, y=1) is canonical but off-curve for BN254 G1.
    tampered[kzg_w_start + 31] = 1;
    tampered[kzg_w_start + 63] = 1;
    Some(tampered)
}

fn sample_proof_bytes() -> Vec<u8> {
    fn push_scalar(out: &mut Vec<u8>, scalar: [u8; 32]) {
        out.extend_from_slice(&scalar);
    }
    fn push_commitment(out: &mut Vec<u8>, point: &[u8; 64]) {
        out.extend_from_slice(&point[..32]);
        out.extend_from_slice(&point[32..]);
    }

    const BASE_POINT: [u8; 64] = [
        45, 206, 255, 166, 152, 55, 128, 138, 79, 217, 145, 164, 25, 74, 120, 234, 234, 217, 68,
        149, 162, 44, 133, 120, 184, 205, 12, 44, 175, 98, 168, 172, 20, 24, 216, 15, 209, 175,
        106, 75, 147, 236, 90, 101, 123, 219, 245, 151, 209, 202, 218, 104, 148, 8, 32, 254, 243,
        191, 218, 122, 42, 81, 193, 84,
    ];
    const NEG_POINT: [u8; 64] = [
        45, 206, 255, 166, 152, 55, 128, 138, 79, 217, 145, 164, 25, 74, 120, 234, 234, 217, 68,
        149, 162, 44, 133, 120, 184, 205, 12, 44, 175, 98, 168, 172, 28, 75, 118, 99, 15, 130, 53,
        222, 36, 99, 235, 81, 5, 165, 98, 197, 197, 182, 144, 40, 212, 105, 169, 142, 72, 96, 177,
        156, 174, 43, 59, 243,
    ];

    let mut proof = Vec::new();
    let zero_scalar = [0u8; 32];

    for _ in 0..16 {
        push_scalar(&mut proof, zero_scalar);
    }

    push_commitment(&mut proof, &BASE_POINT); // W_L
    push_commitment(&mut proof, &NEG_POINT); // W_R
    push_commitment(&mut proof, &BASE_POINT); // W_O
    push_commitment(&mut proof, &BASE_POINT); // LOOKUP_READ_COUNTS
    push_commitment(&mut proof, &BASE_POINT); // LOOKUP_READ_TAGS
    push_commitment(&mut proof, &BASE_POINT); // W_4
    push_commitment(&mut proof, &BASE_POINT); // LOOKUP_INVERSES
    push_commitment(&mut proof, &BASE_POINT); // Z_PERM

    for _ in 0..16 {
        push_scalar(&mut proof, zero_scalar);
    }
    for _ in 0..41 {
        push_scalar(&mut proof, zero_scalar);
    }

    push_commitment(&mut proof, &BASE_POINT); // one Gemini fold commitment
    push_scalar(&mut proof, zero_scalar);
    push_scalar(&mut proof, zero_scalar);

    push_commitment(&mut proof, &BASE_POINT); // Shplonk Q
    push_commitment(&mut proof, &BASE_POINT); // KZG W
    proof
}

fn sample_public_inputs_bytes() -> Vec<u8> {
    let mut out = vec![0u8; 64];
    out[31] = 1;
    out[63] = 2;
    out
}

fn chunk_bytes(bytes: &[u8], chunk_size: usize) -> Vec<AppendChunkPayload> {
    bytes
        .chunks(chunk_size)
        .enumerate()
        .map(|(index, chunk)| AppendChunkPayload {
            chunk_index: index as u32,
            offset: (index * chunk_size) as u32,
            chunk_bytes: chunk.to_vec(),
        })
        .collect()
}

fn build_init_payload(
    authority: &Pubkey,
    proof_bytes: &[u8],
    public_inputs_bytes: &[u8],
    vk_hash: [u8; 32],
) -> InitSessionPayload {
    let proof_hash = compute_proof_hash_bytes(proof_bytes);
    let public_inputs_hash = compute_public_inputs_hash_bytes(public_inputs_bytes);
    let session_bind_hash = compute_session_bind_hash_bytes(
        &vk_hash,
        &proof_hash,
        &public_inputs_hash,
        &authority.to_bytes(),
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

fn build_instruction(
    program_id: Pubkey,
    authority: Pubkey,
    session_pda: Pubkey,
    data: Vec<u8>,
) -> Instruction {
    Instruction {
        program_id,
        accounts: vec![
            AccountMeta::new_readonly(authority, true),
            AccountMeta::new(session_pda, false),
        ],
        data,
    }
}

async fn simulate_then_process(
    context: &mut ProgramTestContext,
    authority: &Keypair,
    instruction: Instruction,
) -> u64 {
    let tx = Transaction::new_signed_with_payer(
        &[
            ComputeBudgetInstruction::set_compute_unit_limit(COMPUTE_UNIT_LIMIT),
            ComputeBudgetInstruction::request_heap_frame(COMPUTE_HEAP_FRAME_BYTES),
            instruction,
        ],
        Some(&context.payer.pubkey()),
        &[&context.payer, authority],
        context.last_blockhash,
    );

    let sim = context
        .banks_client
        .simulate_transaction(tx.clone())
        .await
        .expect("simulation RPC call failed");
    if let Some(Err(err)) = sim.result {
        let logs = sim
            .simulation_details
            .as_ref()
            .map(|details| details.logs.join("\n"))
            .unwrap_or_else(|| "<no simulation logs>".to_string());
        panic!("simulation returned transaction error: {err:?}\nlogs:\n{logs}");
    }

    let units = sim
        .simulation_details
        .expect("missing simulation details")
        .units_consumed;

    context
        .banks_client
        .process_transaction(tx)
        .await
        .expect("process transaction failed");
    context.last_blockhash = context
        .banks_client
        .get_latest_blockhash()
        .await
        .expect("latest blockhash");

    units
}

async fn process_expect_failure(
    context: &mut ProgramTestContext,
    authority: &Keypair,
    instruction: Instruction,
) {
    let tx = Transaction::new_signed_with_payer(
        &[
            ComputeBudgetInstruction::set_compute_unit_limit(COMPUTE_UNIT_LIMIT),
            ComputeBudgetInstruction::request_heap_frame(COMPUTE_HEAP_FRAME_BYTES),
            instruction,
        ],
        Some(&context.payer.pubkey()),
        &[&context.payer, authority],
        context.last_blockhash,
    );

    let sim = context
        .banks_client
        .simulate_transaction(tx)
        .await
        .expect("simulation RPC call failed");
    assert!(
        matches!(sim.result, Some(Err(_))),
        "transaction unexpectedly succeeded in simulation"
    );
    context.last_blockhash = context
        .banks_client
        .get_latest_blockhash()
        .await
        .expect("latest blockhash");
}

async fn fetch_session(context: &mut ProgramTestContext, session_pda: Pubkey) -> SessionAccount {
    let session_account = context
        .banks_client
        .get_account(session_pda)
        .await
        .expect("fetch session account")
        .expect("session account must exist");
    SessionAccount::deserialize_from_account_data(&session_account.data)
        .expect("deserialize session")
}

async fn run_full_session_status(
    proof_bytes: &[u8],
    public_inputs_bytes: &[u8],
    vk_hash: [u8; 32],
) -> SessionStatus {
    let program_id = Pubkey::new_unique();
    let authority = Keypair::new();
    let init_payload = build_init_payload(
        &authority.pubkey(),
        proof_bytes,
        public_inputs_bytes,
        vk_hash,
    );
    let (session_pda, _bump) = derive_session_pda(
        &program_id,
        &authority.pubkey(),
        &init_payload.session_bind_hash,
    );
    let required_session_size = SessionAccount::serialized_size_for_totals(
        init_payload.proof_total_len,
        init_payload.public_inputs_total_len,
    )
    .expect("session size");
    let mut session_account_data = vec![0u8; required_session_size];
    SessionAccount::default()
        .serialize_into_account_data(&mut session_account_data)
        .expect("seed session serialization");

    let mut program_test = ProgramTest::new(
        "ultrahonk_verifier_program",
        program_id,
        processor!(entrypoint_process_instruction),
    );
    program_test.prefer_bpf(true);
    program_test.add_account(
        authority.pubkey(),
        Account {
            lamports: 1_000_000_000,
            data: vec![],
            owner: Pubkey::new_unique(),
            executable: false,
            rent_epoch: 0,
        },
    );
    program_test.add_account(
        session_pda,
        Account {
            lamports: 1_000_000_000,
            data: session_account_data,
            owner: program_id,
            executable: false,
            rent_epoch: 0,
        },
    );
    let mut context = program_test.start_with_context().await;

    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::InitSession(init_payload.clone())),
        ),
    )
    .await;

    for payload in chunk_bytes(proof_bytes, PROOF_CHUNK_SIZE) {
        let _ = simulate_then_process(
            &mut context,
            &authority,
            build_instruction(
                program_id,
                authority.pubkey(),
                session_pda,
                encode_instruction(&VerifierInstruction::AppendProofChunk(payload)),
            ),
        )
        .await;
    }

    for payload in chunk_bytes(public_inputs_bytes, PUBLIC_INPUTS_CHUNK_SIZE) {
        let _ = simulate_then_process(
            &mut context,
            &authority,
            build_instruction(
                program_id,
                authority.pubkey(),
                session_pda,
                encode_instruction(&VerifierInstruction::AppendPublicInputsChunk(payload)),
            ),
        )
        .await;
    }

    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 0,
            }),
        ),
    )
    .await;
    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 1,
            }),
        ),
    )
    .await;
    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 2,
            }),
        ),
    )
    .await;
    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::Finalize),
        ),
    )
    .await;

    let session_account = context
        .banks_client
        .get_account(session_pda)
        .await
        .expect("fetch session account")
        .expect("session account must exist");
    SessionAccount::deserialize_from_account_data(&session_account.data)
        .expect("deserialize session")
        .status
}

async fn run_session_until_stage0(
    committed_proof_bytes: &[u8],
    committed_public_inputs_bytes: &[u8],
    ingested_proof_bytes: &[u8],
    ingested_public_inputs_bytes: &[u8],
    vk_hash: [u8; 32],
) -> SessionAccount {
    let program_id = Pubkey::new_unique();
    let authority = Keypair::new();
    let init_payload = build_init_payload(
        &authority.pubkey(),
        committed_proof_bytes,
        committed_public_inputs_bytes,
        vk_hash,
    );
    let (session_pda, _bump) = derive_session_pda(
        &program_id,
        &authority.pubkey(),
        &init_payload.session_bind_hash,
    );
    let required_session_size = SessionAccount::serialized_size_for_totals(
        init_payload.proof_total_len,
        init_payload.public_inputs_total_len,
    )
    .expect("session size");
    let mut session_account_data = vec![0u8; required_session_size];
    SessionAccount::default()
        .serialize_into_account_data(&mut session_account_data)
        .expect("seed session serialization");

    let mut program_test = ProgramTest::new(
        "ultrahonk_verifier_program",
        program_id,
        processor!(entrypoint_process_instruction),
    );
    program_test.prefer_bpf(true);
    program_test.add_account(
        authority.pubkey(),
        Account {
            lamports: 1_000_000_000,
            data: vec![],
            owner: Pubkey::new_unique(),
            executable: false,
            rent_epoch: 0,
        },
    );
    program_test.add_account(
        session_pda,
        Account {
            lamports: 1_000_000_000,
            data: session_account_data,
            owner: program_id,
            executable: false,
            rent_epoch: 0,
        },
    );
    let mut context = program_test.start_with_context().await;

    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::InitSession(init_payload.clone())),
        ),
    )
    .await;

    for payload in chunk_bytes(ingested_proof_bytes, PROOF_CHUNK_SIZE) {
        let _ = simulate_then_process(
            &mut context,
            &authority,
            build_instruction(
                program_id,
                authority.pubkey(),
                session_pda,
                encode_instruction(&VerifierInstruction::AppendProofChunk(payload)),
            ),
        )
        .await;
    }

    for payload in chunk_bytes(ingested_public_inputs_bytes, PUBLIC_INPUTS_CHUNK_SIZE) {
        let _ = simulate_then_process(
            &mut context,
            &authority,
            build_instruction(
                program_id,
                authority.pubkey(),
                session_pda,
                encode_instruction(&VerifierInstruction::AppendPublicInputsChunk(payload)),
            ),
        )
        .await;
    }

    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 0,
            }),
        ),
    )
    .await;

    let session_account = context
        .banks_client
        .get_account(session_pda)
        .await
        .expect("fetch session account")
        .expect("session account must exist");
    SessionAccount::deserialize_from_account_data(&session_account.data)
        .expect("deserialize session")
}

fn metric_key(metric_prefix: &str, metric_name: &str) -> String {
    if metric_prefix.is_empty() {
        metric_name.to_string()
    } else {
        format!("{metric_prefix}_{metric_name}")
    }
}

fn print_metric_u64(metric_prefix: &str, metric_name: &str, value: u64) {
    println!("{}={value}", metric_key(metric_prefix, metric_name));
}

fn print_metric_str(metric_prefix: &str, metric_name: &str, value: &str) {
    println!("{}={value}", metric_key(metric_prefix, metric_name));
}

async fn benchmark_fixture_stagewise_and_full_session_cu(
    fixture: &FixtureData,
    metric_prefix: &str,
) -> StagewiseCuBreakdown {
    let program_id = Pubkey::new_unique();
    let authority = Keypair::new();

    print_metric_str(metric_prefix, "fixture_source", &fixture.source);
    let proof_bytes = &fixture.proof_bytes;
    let public_inputs_bytes = &fixture.public_inputs_bytes;
    let init_payload = build_init_payload(
        &authority.pubkey(),
        proof_bytes,
        public_inputs_bytes,
        fixture.vk_hash,
    );
    let (session_pda, _bump) = derive_session_pda(
        &program_id,
        &authority.pubkey(),
        &init_payload.session_bind_hash,
    );
    let required_session_size = SessionAccount::serialized_size_for_totals(
        init_payload.proof_total_len,
        init_payload.public_inputs_total_len,
    )
    .expect("session size");
    let mut session_account_data = vec![0u8; required_session_size];
    SessionAccount::default()
        .serialize_into_account_data(&mut session_account_data)
        .expect("seed session serialization");

    let mut program_test = ProgramTest::new(
        "ultrahonk_verifier_program",
        program_id,
        processor!(entrypoint_process_instruction),
    );
    program_test.prefer_bpf(true);
    program_test.add_account(
        authority.pubkey(),
        Account {
            lamports: 1_000_000_000,
            data: vec![],
            owner: Pubkey::new_unique(),
            executable: false,
            rent_epoch: 0,
        },
    );
    program_test.add_account(
        session_pda,
        Account {
            lamports: 1_000_000_000,
            data: session_account_data,
            owner: program_id,
            executable: false,
            rent_epoch: 0,
        },
    );

    let mut context = program_test.start_with_context().await;
    let mut total_units: u64 = 0;

    let init_units = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::InitSession(init_payload.clone())),
        ),
    )
    .await;
    total_units += init_units;
    print_metric_u64(metric_prefix, "cu_init_session", init_units);

    let mut append_proof_units = Vec::new();
    for payload in chunk_bytes(proof_bytes, PROOF_CHUNK_SIZE) {
        let units = simulate_then_process(
            &mut context,
            &authority,
            build_instruction(
                program_id,
                authority.pubkey(),
                session_pda,
                encode_instruction(&VerifierInstruction::AppendProofChunk(payload)),
            ),
        )
        .await;
        total_units += units;
        append_proof_units.push(units);
        print_metric_u64(metric_prefix, "cu_append_proof_chunk", units);
    }

    let mut append_public_inputs_units = Vec::new();
    for payload in chunk_bytes(public_inputs_bytes, PUBLIC_INPUTS_CHUNK_SIZE) {
        let units = simulate_then_process(
            &mut context,
            &authority,
            build_instruction(
                program_id,
                authority.pubkey(),
                session_pda,
                encode_instruction(&VerifierInstruction::AppendPublicInputsChunk(payload)),
            ),
        )
        .await;
        total_units += units;
        append_public_inputs_units.push(units);
        print_metric_u64(metric_prefix, "cu_append_public_inputs_chunk", units);
    }

    let stage0_units = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 0,
            }),
        ),
    )
    .await;
    total_units += stage0_units;
    print_metric_u64(metric_prefix, "cu_verify_stage_hash_checks", stage0_units);

    let stage1_units = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 1,
            }),
        ),
    )
    .await;
    total_units += stage1_units;
    print_metric_u64(metric_prefix, "cu_verify_stage_decode_inputs", stage1_units);

    let stage2_units = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 2,
            }),
        ),
    )
    .await;
    total_units += stage2_units;
    print_metric_u64(metric_prefix, "cu_verify_stage_native_verify", stage2_units);

    let finalize_units = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::Finalize),
        ),
    )
    .await;
    total_units += finalize_units;
    print_metric_u64(metric_prefix, "cu_finalize", finalize_units);

    let session_account = context
        .banks_client
        .get_account(session_pda)
        .await
        .expect("fetch session account")
        .expect("session account must exist");
    let session = SessionAccount::deserialize_from_account_data(&session_account.data)
        .expect("deserialize session");
    assert_eq!(session.status, SessionStatus::Verified);

    print_metric_u64(metric_prefix, "cu_total_session_flow", total_units);

    StagewiseCuBreakdown {
        init_session: init_units,
        append_proof_chunk: append_proof_units,
        append_public_inputs_chunk: append_public_inputs_units,
        verify_stage_hash_checks: stage0_units,
        verify_stage_decode_inputs: stage1_units,
        verify_stage_native_verify: stage2_units,
        finalize: finalize_units,
        total_session_flow: total_units,
    }
}

#[tokio::test]
#[ignore = "Run with cargo-test-sbf --features sbf-entrypoint --test cu_bench -- --ignored --nocapture for SBF CU measurements"]
async fn benchmark_stagewise_and_full_session_cu() {
    let fixture = load_primary_fixture_or_fallback();
    let _ = benchmark_fixture_stagewise_and_full_session_cu(&fixture, "").await;
}

#[tokio::test]
#[ignore = "Run with cargo-test-sbf --features sbf-entrypoint --test cu_bench secondary_fixture_stagewise_and_full_session_cu_with_deltas -- --ignored --nocapture --test-threads=1 for secondary fixture CU capture and delta metrics"]
async fn secondary_fixture_stagewise_and_full_session_cu_with_deltas() {
    let primary_fixture = load_primary_fixture_or_fallback();
    let primary = benchmark_fixture_stagewise_and_full_session_cu(&primary_fixture, "primary").await;

    let secondary_fixture = load_secondary_fixture().expect("secondary bb fixture must exist");
    let secondary = benchmark_fixture_stagewise_and_full_session_cu(&secondary_fixture, "secondary").await;

    let delta_stage2 =
        i128::from(secondary.verify_stage_native_verify) - i128::from(primary.verify_stage_native_verify);
    let delta_total =
        i128::from(secondary.total_session_flow) - i128::from(primary.total_session_flow);
    let delta_stage0 = i128::from(secondary.verify_stage_hash_checks) - i128::from(primary.verify_stage_hash_checks);
    let delta_stage1 =
        i128::from(secondary.verify_stage_decode_inputs) - i128::from(primary.verify_stage_decode_inputs);
    let delta_init = i128::from(secondary.init_session) - i128::from(primary.init_session);
    let delta_finalize = i128::from(secondary.finalize) - i128::from(primary.finalize);

    println!("cu_delta_secondary_minus_primary_init_session={delta_init}");
    println!("cu_delta_secondary_minus_primary_verify_stage_hash_checks={delta_stage0}");
    println!("cu_delta_secondary_minus_primary_verify_stage_decode_inputs={delta_stage1}");
    println!("cu_delta_secondary_minus_primary_verify_stage_native_verify={delta_stage2}");
    println!("cu_delta_secondary_minus_primary_finalize={delta_finalize}");
    println!("cu_delta_secondary_minus_primary_total_session_flow={delta_total}");
    println!(
        "secondary_append_proof_chunk_count={}",
        secondary.append_proof_chunk.len()
    );
    println!(
        "secondary_append_public_inputs_chunk_count={}",
        secondary.append_public_inputs_chunk.len()
    );
}

#[tokio::test]
#[ignore = "Run with cargo-test-sbf --features sbf-entrypoint --test cu_bench -- --ignored --nocapture for malformed-vector staged-session checks"]
async fn malformed_vectors_fail_staged_verification() {
    let fixture = load_primary_fixture_or_fallback();
    println!("fixture_source={}", fixture.source);
    let proof_bytes = fixture.proof_bytes;
    let public_inputs_bytes = fixture.public_inputs_bytes;

    let mut non_canonical_public_inputs = public_inputs_bytes.clone();
    if non_canonical_public_inputs.len() < 32 {
        non_canonical_public_inputs.resize(32, 0);
    }
    non_canonical_public_inputs[..32].copy_from_slice(&BN254_SCALAR_MODULUS_BE);
    let status =
        run_full_session_status(&proof_bytes, &non_canonical_public_inputs, fixture.vk_hash).await;
    assert_eq!(status, SessionStatus::Failed);

    let mut non_aligned_proof = proof_bytes.clone();
    non_aligned_proof
        .pop()
        .expect("proof fixture should have at least one byte");
    let status =
        run_full_session_status(&non_aligned_proof, &public_inputs_bytes, fixture.vk_hash).await;
    assert_eq!(status, SessionStatus::Failed);

    let empty_proof = Vec::new();
    let status = run_full_session_status(&empty_proof, &public_inputs_bytes, fixture.vk_hash).await;
    assert_eq!(status, SessionStatus::Failed);

    let non_canonical_sumcheck_proof = tamper_first_sumcheck_scalar_non_canonical(&proof_bytes)
        .expect("proof fixture should include sumcheck scalars");
    let status = run_full_session_status(
        &non_canonical_sumcheck_proof,
        &public_inputs_bytes,
        fixture.vk_hash,
    )
    .await;
    assert_eq!(status, SessionStatus::Failed);

    let invalid_round0_relation_proof = tamper_first_sumcheck_round_relation(&proof_bytes)
        .expect("proof fixture should include round-0 sumcheck data");
    let status = run_full_session_status(
        &invalid_round0_relation_proof,
        &public_inputs_bytes,
        fixture.vk_hash,
    )
    .await;
    assert_eq!(status, SessionStatus::Failed);

    let invalid_pairing_special_point_proof = tamper_pairing_special_point_invalid(&proof_bytes)
        .expect("proof fixture should include pairing special-input limbs");
    let status = run_full_session_status(
        &invalid_pairing_special_point_proof,
        &public_inputs_bytes,
        fixture.vk_hash,
    )
    .await;
    assert_eq!(status, SessionStatus::Failed);

    let invalid_round_transition_proof = tamper_second_sumcheck_round_transition(&proof_bytes)
        .expect("proof fixture should include round-1 sumcheck data");
    let status = run_full_session_status(
        &invalid_round_transition_proof,
        &public_inputs_bytes,
        fixture.vk_hash,
    )
    .await;
    assert_eq!(status, SessionStatus::Failed);

    let invalid_shplonk_q_proof = tamper_shplonk_q_commitment_invalid(&proof_bytes)
        .expect("proof fixture should include final Shplonk commitment");
    let status = run_full_session_status(
        &invalid_shplonk_q_proof,
        &public_inputs_bytes,
        fixture.vk_hash,
    )
    .await;
    assert_eq!(status, SessionStatus::Failed);

    let invalid_gemini_fold_proof = tamper_first_gemini_fold_commitment_invalid(&proof_bytes)
        .expect("proof fixture should include Gemini fold commitments");
    let status = run_full_session_status(
        &invalid_gemini_fold_proof,
        &public_inputs_bytes,
        fixture.vk_hash,
    )
    .await;
    assert_eq!(status, SessionStatus::Failed);

    let invalid_first_wire_proof =
        tamper_first_wire_commitment_invalid(&proof_bytes).expect("proof should include W_L");
    let status = run_full_session_status(
        &invalid_first_wire_proof,
        &public_inputs_bytes,
        fixture.vk_hash,
    )
    .await;
    assert_eq!(status, SessionStatus::Failed);

    let invalid_kzg_w_proof =
        tamper_kzg_w_commitment_invalid(&proof_bytes).expect("proof should include KZG witness");
    let status =
        run_full_session_status(&invalid_kzg_w_proof, &public_inputs_bytes, fixture.vk_hash).await;
    assert_eq!(status, SessionStatus::Failed);
}

#[tokio::test]
#[ignore = "Run with cargo-test-sbf --features sbf-entrypoint --test cu_bench -- --ignored --nocapture for stage-0 tamper checks without trap-prone stage-2 execution"]
async fn malformed_vectors_fail_hash_binding_at_stage0() {
    let fixture = load_primary_fixture_or_fallback();
    println!("fixture_source={}", fixture.source);
    let proof_bytes = fixture.proof_bytes;
    let public_inputs_bytes = fixture.public_inputs_bytes;

    let reordered_proof =
        swap_commitment_sections(&proof_bytes, 0, 2).expect("proof should include commitments");
    let session = run_session_until_stage0(
        &proof_bytes,
        &public_inputs_bytes,
        &reordered_proof,
        &public_inputs_bytes,
        fixture.vk_hash,
    )
    .await;
    assert!(session.stage_failed);
    assert_eq!(session.stage_index, 1);

    let truncated_window_proof =
        truncate_commitment_window(&proof_bytes).expect("proof should support truncation window");
    let session = run_session_until_stage0(
        &proof_bytes,
        &public_inputs_bytes,
        &truncated_window_proof,
        &public_inputs_bytes,
        fixture.vk_hash,
    )
    .await;
    assert!(session.stage_failed);
    assert_eq!(session.stage_index, 1);
}

#[tokio::test]
#[ignore = "Run with cargo test --test cu_bench secondary_fixture_full_session_verifies -- --ignored --nocapture (host) or cargo-test-sbf --features sbf-entrypoint --test cu_bench secondary_fixture_full_session_verifies -- --ignored --nocapture (SBF) for secondary bb fixture full-session coverage"]
async fn secondary_fixture_full_session_verifies() {
    let fixture = load_secondary_fixture().expect("secondary bb fixture must exist");
    println!("fixture_source={}", fixture.source);
    let status = run_full_session_status(
        &fixture.proof_bytes,
        &fixture.public_inputs_bytes,
        fixture.vk_hash,
    )
    .await;
    assert_eq!(status, SessionStatus::Verified);
}

#[tokio::test]
#[ignore = "Run with cargo-test-sbf --features sbf-entrypoint --test cu_bench -- --ignored --nocapture for secondary bb fixture stage-0 coverage"]
async fn secondary_fixture_stage0_hash_checks_pass() {
    let fixture = load_secondary_fixture().expect("secondary bb fixture must exist");
    println!("fixture_source={}", fixture.source);
    let session = run_session_until_stage0(
        &fixture.proof_bytes,
        &fixture.public_inputs_bytes,
        &fixture.proof_bytes,
        &fixture.public_inputs_bytes,
        fixture.vk_hash,
    )
    .await;
    assert!(!session.stage_failed);
    assert_eq!(session.stage_index, 1);
}

#[tokio::test]
#[ignore = "Run with cargo-test-sbf --features sbf-entrypoint --test cu_bench -- --ignored --nocapture for replay/order staged-session checks"]
async fn replay_and_order_violations_fail_fast() {
    let program_id = Pubkey::new_unique();
    let authority = Keypair::new();
    let fixture = load_primary_fixture_or_fallback();
    println!("fixture_source={}", fixture.source);

    let proof_chunks = chunk_bytes(&fixture.proof_bytes, PROOF_CHUNK_SIZE);
    assert!(
        proof_chunks.len() >= 2,
        "fixture must have at least two proof chunks for replay/order checks"
    );
    let public_inputs_chunks = chunk_bytes(&fixture.public_inputs_bytes, PUBLIC_INPUTS_CHUNK_SIZE);
    assert!(
        !public_inputs_chunks.is_empty(),
        "fixture must have at least one public-input chunk for replay/order checks"
    );

    let init_payload = build_init_payload(
        &authority.pubkey(),
        &fixture.proof_bytes,
        &fixture.public_inputs_bytes,
        fixture.vk_hash,
    );
    let (session_pda, _bump) = derive_session_pda(
        &program_id,
        &authority.pubkey(),
        &init_payload.session_bind_hash,
    );
    let required_session_size = SessionAccount::serialized_size_for_totals(
        init_payload.proof_total_len,
        init_payload.public_inputs_total_len,
    )
    .expect("session size");
    let mut session_account_data = vec![0u8; required_session_size];
    SessionAccount::default()
        .serialize_into_account_data(&mut session_account_data)
        .expect("seed session serialization");

    let mut program_test = ProgramTest::new(
        "ultrahonk_verifier_program",
        program_id,
        processor!(entrypoint_process_instruction),
    );
    program_test.prefer_bpf(true);
    program_test.add_account(
        authority.pubkey(),
        Account {
            lamports: 1_000_000_000,
            data: vec![],
            owner: Pubkey::new_unique(),
            executable: false,
            rent_epoch: 0,
        },
    );
    program_test.add_account(
        session_pda,
        Account {
            lamports: 1_000_000_000,
            data: session_account_data,
            owner: program_id,
            executable: false,
            rent_epoch: 0,
        },
    );
    let mut context = program_test.start_with_context().await;

    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::InitSession(init_payload)),
        ),
    )
    .await;

    let first_chunk = proof_chunks[0].clone();
    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::AppendProofChunk(first_chunk.clone())),
        ),
    )
    .await;

    process_expect_failure(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::AppendProofChunk(first_chunk)),
        ),
    )
    .await;

    let mut wrong_offset_chunk = proof_chunks[1].clone();
    wrong_offset_chunk.offset = wrong_offset_chunk.offset.saturating_add(1);
    process_expect_failure(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::AppendProofChunk(wrong_offset_chunk)),
        ),
    )
    .await;

    process_expect_failure(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 0,
            }),
        ),
    )
    .await;

    let session_after_early_failures = fetch_session(&mut context, session_pda).await;
    assert_eq!(session_after_early_failures.stage_index, 0);
    assert!(!session_after_early_failures.stage_failed);
    assert_eq!(session_after_early_failures.status, SessionStatus::CollectingProof);
    assert_eq!(session_after_early_failures.next_proof_chunk_index, 1);

    for payload in proof_chunks.into_iter().skip(1) {
        let _ = simulate_then_process(
            &mut context,
            &authority,
            build_instruction(
                program_id,
                authority.pubkey(),
                session_pda,
                encode_instruction(&VerifierInstruction::AppendProofChunk(payload)),
            ),
        )
        .await;
    }

    for payload in public_inputs_chunks {
        let _ = simulate_then_process(
            &mut context,
            &authority,
            build_instruction(
                program_id,
                authority.pubkey(),
                session_pda,
                encode_instruction(&VerifierInstruction::AppendPublicInputsChunk(payload)),
            ),
        )
        .await;
    }

    process_expect_failure(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 1,
            }),
        ),
    )
    .await;

    let session_after_stage_order_failure = fetch_session(&mut context, session_pda).await;
    assert_eq!(session_after_stage_order_failure.stage_index, 0);
    assert!(!session_after_stage_order_failure.stage_failed);
    assert_eq!(session_after_stage_order_failure.status, SessionStatus::ReadyToVerify);

    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 0,
            }),
        ),
    )
    .await;

    process_expect_failure(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 0,
            }),
        ),
    )
    .await;

    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 1,
            }),
        ),
    )
    .await;
    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: 2,
            }),
        ),
    )
    .await;
    let _ = simulate_then_process(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::Finalize),
        ),
    )
    .await;

    process_expect_failure(
        &mut context,
        &authority,
        build_instruction(
            program_id,
            authority.pubkey(),
            session_pda,
            encode_instruction(&VerifierInstruction::Finalize),
        ),
    )
    .await;

    let final_session = fetch_session(&mut context, session_pda).await;
    assert_eq!(final_session.status, SessionStatus::Verified);
    assert_eq!(final_session.stage_index, 3);
    assert!(!final_session.stage_failed);
}
