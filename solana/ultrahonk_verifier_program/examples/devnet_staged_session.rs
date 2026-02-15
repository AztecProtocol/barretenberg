use std::{error::Error, fs, path::Path, str::FromStr};

use solana_compute_budget_interface::ComputeBudgetInstruction;
use solana_instruction::{AccountMeta, Instruction};
use solana_keypair::read_keypair_file;
use solana_pubkey::Pubkey;
use solana_rpc_client::rpc_client::RpcClient;
use solana_sdk_ids::system_program;
use solana_signer::Signer;
use solana_transaction::Transaction;
use ultrahonk_verifier_program::{
    compute_proof_hash_bytes, compute_public_inputs_hash_bytes, compute_session_bind_hash_bytes,
    derive_session_pda, encode_instruction, AppendChunkPayload, InitSessionPayload, SessionAccount,
    TOTAL_VERIFY_STAGES, VerifierInstruction,
};

const COMPUTE_UNIT_LIMIT: u32 = 1_400_000;
const COMPUTE_HEAP_FRAME_BYTES: u32 = 256 * 1024;
const CHUNK_SIZE: usize = 512;

fn chunk_bytes(bytes: &[u8], chunk_size: usize) -> Vec<AppendChunkPayload> {
    let mut chunks = Vec::new();
    let mut offset = 0usize;
    let mut index = 0u32;
    while offset < bytes.len() {
        let end = (offset + chunk_size).min(bytes.len());
        chunks.push(AppendChunkPayload {
            chunk_index: index,
            offset: offset as u32,
            chunk_bytes: bytes[offset..end].to_vec(),
        });
        index = index.saturating_add(1);
        offset = end;
    }
    chunks
}

fn send_instruction(
    rpc: &RpcClient,
    authority: &solana_keypair::Keypair,
    instruction: Instruction,
) -> Result<String, Box<dyn Error>> {
    let latest_blockhash = rpc.get_latest_blockhash()?;
    let tx = Transaction::new_signed_with_payer(
        &[
            ComputeBudgetInstruction::set_compute_unit_limit(COMPUTE_UNIT_LIMIT),
            ComputeBudgetInstruction::request_heap_frame(COMPUTE_HEAP_FRAME_BYTES),
            instruction,
        ],
        Some(&authority.pubkey()),
        &[authority],
        latest_blockhash,
    );
    let sig = rpc.send_and_confirm_transaction(&tx)?;
    Ok(sig.to_string())
}

fn make_instruction(
    program_id: Pubkey,
    authority: Pubkey,
    session_pda: Pubkey,
    data: Vec<u8>,
    include_system_program: bool,
) -> Instruction {
    let mut accounts = vec![
        // Writable on init so it can pay rent via CPI when creating session PDA.
        AccountMeta::new(authority, true),
        AccountMeta::new(session_pda, false),
    ];
    if include_system_program {
        accounts.push(AccountMeta::new_readonly(system_program::id(), false));
    }
    Instruction {
        program_id,
        accounts,
        data,
    }
}

fn run_case(
    rpc: &RpcClient,
    authority: &solana_keypair::Keypair,
    program_id: Pubkey,
    vk_hash: [u8; 32],
    proof_bytes: &[u8],
    public_inputs_bytes: &[u8],
    label: &str,
) -> Result<(), Box<dyn Error>> {
    let authority_pubkey = authority.pubkey();
    let proof_hash = compute_proof_hash_bytes(proof_bytes);
    let public_inputs_hash = compute_public_inputs_hash_bytes(public_inputs_bytes);
    let session_bind_hash = compute_session_bind_hash_bytes(
        &vk_hash,
        &proof_hash,
        &public_inputs_hash,
        &authority_pubkey.to_bytes(),
        proof_bytes.len() as u64,
        public_inputs_bytes.len() as u64,
    );
    let (session_pda, _) = derive_session_pda(&program_id, &authority_pubkey, &session_bind_hash);

    println!("--- {label} ---");
    println!("session_pda={session_pda}");

    let init_payload = InitSessionPayload {
        vk_hash,
        proof_total_len: proof_bytes.len() as u64,
        public_inputs_total_len: public_inputs_bytes.len() as u64,
        proof_hash,
        public_inputs_hash,
        session_bind_hash,
    };
    let init_ix = make_instruction(
        program_id,
        authority_pubkey,
        session_pda,
        encode_instruction(&VerifierInstruction::InitSession(init_payload)),
        true,
    );
    let init_sig = send_instruction(rpc, authority, init_ix)?;
    println!("init_sig={init_sig}");

    for chunk in chunk_bytes(proof_bytes, CHUNK_SIZE) {
        let ix = make_instruction(
            program_id,
            authority_pubkey,
            session_pda,
            encode_instruction(&VerifierInstruction::AppendProofChunk(chunk)),
            false,
        );
        let sig = send_instruction(rpc, authority, ix)?;
        println!("append_proof_sig={sig}");
    }

    for chunk in chunk_bytes(public_inputs_bytes, CHUNK_SIZE) {
        let ix = make_instruction(
            program_id,
            authority_pubkey,
            session_pda,
            encode_instruction(&VerifierInstruction::AppendPublicInputsChunk(chunk)),
            false,
        );
        let sig = send_instruction(rpc, authority, ix)?;
        println!("append_public_inputs_sig={sig}");
    }

    for stage in 0..TOTAL_VERIFY_STAGES {
        let ix = make_instruction(
            program_id,
            authority_pubkey,
            session_pda,
            encode_instruction(&VerifierInstruction::VerifyStep {
                expected_stage_index: stage,
            }),
            false,
        );
        let sig = send_instruction(rpc, authority, ix)?;
        println!("verify_stage_{stage}_sig={sig}");
    }

    let finalize_ix = make_instruction(
        program_id,
        authority_pubkey,
        session_pda,
        encode_instruction(&VerifierInstruction::Finalize),
        false,
    );
    let finalize_sig = send_instruction(rpc, authority, finalize_ix)?;
    println!("finalize_sig={finalize_sig}");

    let account_data = rpc.get_account_data(&session_pda)?;
    let session = SessionAccount::deserialize_from_account_data(&account_data)
        .map_err(|err| format!("failed to decode session account: {err:?}"))?;
    println!(
        "final_status={:?} stage_failed={} stage_index={}",
        session.status, session.stage_failed, session.stage_index
    );
    println!();

    Ok(())
}

fn parse_vk_hash(path: &Path) -> Result<[u8; 32], Box<dyn Error>> {
    let bytes = fs::read(path)?;
    let vk_hash: [u8; 32] = bytes
        .as_slice()
        .try_into()
        .map_err(|_| "vk_hash file must be exactly 32 bytes")?;
    Ok(vk_hash)
}

fn x5_public_input_bytes() -> [u8; 32] {
    let mut bytes = [0u8; 32];
    bytes[31] = 5;
    bytes
}

fn main() -> Result<(), Box<dyn Error>> {
    let args: Vec<String> = std::env::args().collect();
    if args.len() != 6 {
        return Err(format!(
            "usage: {} <rpc_url> <program_id> <authority_keypair.json> <proof_path> <public_inputs_path>",
            args[0]
        )
        .into());
    }

    let rpc_url = &args[1];
    let program_id = Pubkey::from_str(&args[2])?;
    let authority = read_keypair_file(&args[3])?;
    let proof_path = Path::new(&args[4]);
    let public_inputs_path = Path::new(&args[5]);
    let vk_hash_path = public_inputs_path
        .parent()
        .ok_or("public_inputs_path must have parent directory")?
        .join("vk_hash");

    let rpc = RpcClient::new(rpc_url.to_string());
    let authority_balance = rpc.get_balance(&authority.pubkey())?;
    println!("authority={}", authority.pubkey());
    println!("authority_balance_lamports={authority_balance}");
    println!("program_id={program_id}");
    println!();

    let proof_bytes = fs::read(proof_path)?;
    let valid_public_inputs_bytes = fs::read(public_inputs_path)?;
    let vk_hash = parse_vk_hash(&vk_hash_path)?;

    run_case(
        &rpc,
        &authority,
        program_id,
        vk_hash,
        &proof_bytes,
        &valid_public_inputs_bytes,
        "x=6_valid",
    )?;

    let invalid_public_inputs = x5_public_input_bytes();
    run_case(
        &rpc,
        &authority,
        program_id,
        vk_hash,
        &proof_bytes,
        &invalid_public_inputs,
        "x=5_invalid",
    )?;

    Ok(())
}
