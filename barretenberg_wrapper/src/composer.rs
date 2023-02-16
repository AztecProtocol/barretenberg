use crate::*;

/// # Safety
/// pippenger must point to a valid Pippenger object
pub unsafe fn smart_contract(
    pippenger: *mut ::std::os::raw::c_void,
    g2_ptr: &[u8],
    cs_ptr: &[u8],
    output_buf: *mut *mut u8,
) -> u32 {
    composer__smart_contract(
        pippenger,
        g2_ptr.as_ptr() as *const u8,
        cs_ptr.as_ptr() as *const u8,
        output_buf,
    )
}

/// # Safety
/// cs_prt must point to a valid constraints system structure of type standard_format
pub unsafe fn get_circuit_size(cs_prt: *const u8) -> u32 {
    composer__get_circuit_size(cs_prt)
    // TODO test with a circuit of size 2^19 cf: https://github.com/noir-lang/noir/issues/12
}

/// # Safety
/// cs_prt must point to a valid constraints system structure of type standard_format
pub unsafe fn get_exact_circuit_size(cs_prt: *const u8) -> u32 {
    standard_example__get_exact_circuit_size(cs_prt)
}

/// # Safety
/// pippenger must point to a valid Pippenger object
pub unsafe fn create_proof(
    pippenger: *mut ::std::os::raw::c_void,
    cs_ptr: &[u8],
    g2_ptr: &[u8],
    witness_ptr: &[u8],
    proof_data_ptr: *mut *mut u8,
) -> u64 {
    composer__new_proof(
        pippenger,
        g2_ptr.as_ptr() as *const u8,
        cs_ptr.as_ptr() as *const u8,
        witness_ptr.as_ptr() as *const u8,
        proof_data_ptr as *const *mut u8 as *mut *mut u8,
    )
}
/// # Safety
/// pippenger must point to a valid Pippenger object
pub unsafe fn verify(
    // XXX: Important: This assumes that the proof does not have the public inputs pre-pended to it
    // This is not the case, if you take the proof directly from Barretenberg
    pippenger: *mut ::std::os::raw::c_void,
    proof: &[u8],
    cs_ptr: &[u8],
    g2_ptr: &[u8],
) -> bool {
    let proof_ptr = proof.as_ptr() as *const u8;

    composer__verify_proof(
        pippenger,
        g2_ptr.as_ptr() as *const u8,
        cs_ptr.as_ptr() as *const u8,
        proof_ptr as *mut u8,
        proof.len() as u32,
    )
}

/// # Safety
/// cs_prt must point to a valid constraints system structure of type standard_format
pub unsafe fn init_proving_key(cs_ptr: &[u8], pk_data_ptr: *mut *mut u8) -> u64 {
    let cs_ptr = cs_ptr.as_ptr() as *const u8;
    c_init_proving_key(cs_ptr, pk_data_ptr as *const *mut u8 as *mut *const u8)
}

/// # Safety
/// pippenger must point to a valid Pippenger object
pub unsafe fn init_verification_key(
    pippenger: *mut ::std::os::raw::c_void,
    g2_ptr: &[u8],
    pk_ptr: &[u8],
    vk_data_ptr: *mut *mut u8,
) -> u64 {
    c_init_verification_key(
        pippenger,
        g2_ptr.as_ptr() as *const u8,
        pk_ptr.as_ptr() as *const u8,
        vk_data_ptr as *const *mut u8 as *mut *const u8,
    )
}

/// # Safety
/// pippenger must point to a valid Pippenger object
pub unsafe fn create_proof_with_pk(
    pippenger: *mut ::std::os::raw::c_void,
    g2_ptr: &[u8],
    pk_ptr: &[u8],
    cs_ptr: &[u8],
    witness_ptr: &[u8],
    proof_data_ptr: *mut *mut u8,
) -> u64 {
    let cs_ptr = cs_ptr.as_ptr() as *const u8;
    let pk_ptr = pk_ptr.as_ptr() as *const u8;
    c_new_proof(
        pippenger,
        g2_ptr.as_ptr() as *const u8,
        pk_ptr,
        cs_ptr,
        witness_ptr.as_ptr() as *const u8,
        proof_data_ptr as *const *mut u8 as *mut *mut u8,
    )
}

/// # Safety
/// cs_prt must point to a valid constraints system structure of type standard_format
pub unsafe fn verify_with_vk(g2_ptr: &[u8], vk_ptr: &[u8], cs_ptr: &[u8], proof: &[u8]) -> bool {
    let proof_ptr = proof.as_ptr() as *const u8;

    c_verify_proof(
        g2_ptr.as_ptr() as *const u8,
        vk_ptr.as_ptr() as *const u8,
        cs_ptr.as_ptr() as *const u8,
        proof_ptr as *mut u8,
        proof.len() as u32,
    )
}
