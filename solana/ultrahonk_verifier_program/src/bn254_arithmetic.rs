use solana_bn254::{
    prelude::{
        alt_bn128_g1_addition_be, alt_bn128_pairing_be, ALT_BN128_G1_ADDITION_INPUT_SIZE,
        ALT_BN128_G1_POINT_SIZE, ALT_BN128_PAIRING_ELEMENT_SIZE, ALT_BN128_PAIRING_OUTPUT_SIZE,
    },
    AltBn128Error,
};
#[cfg(test)]
use solana_bn254::prelude::{
    alt_bn128_g1_multiplication_be, ALT_BN128_G1_MULTIPLICATION_INPUT_SIZE,
};
use solana_program::keccak::hashv;

// BN254 scalar field modulus (Fr) - 1 in big-endian form.
#[cfg(test)]
const SCALAR_NEG_ONE_BE: [u8; 32] = [
    0x30, 0x64, 0x4e, 0x72, 0xe1, 0x31, 0xa0, 0x29, 0xb8, 0x50, 0x45, 0xb6, 0x81, 0x81, 0x58, 0x5d,
    0x28, 0x33, 0xe8, 0x48, 0x79, 0xb9, 0x70, 0x91, 0x43, 0xe1, 0xf5, 0x93, 0xf0, 0x00, 0x00, 0x00,
];

// BN254 scalar modulus (Fr) for canonical field checks in special public inputs.
const BN254_SCALAR_MODULUS_BE: [u8; 32] = [
    0x30, 0x64, 0x4e, 0x72, 0xe1, 0x31, 0xa0, 0x29, 0xb8, 0x50, 0x45, 0xb6, 0x81, 0x81, 0x58, 0x5d,
    0x28, 0x33, 0xe8, 0x48, 0x79, 0xb9, 0x70, 0x91, 0x43, 0xe1, 0xf5, 0x93, 0xf0, 0x00, 0x00, 0x01,
];
const BN254_SCALAR_MODULUS_LE_U64X4: [u64; 4] = [
    0x43e1_f593_f000_0001,
    0x2833_e848_79b9_7091,
    0xb850_45b6_8181_585d,
    0x3064_4e72_e131_a029,
];
const BN254_BASE_MODULUS_BE: [u8; 32] = [
    0x30, 0x64, 0x4e, 0x72, 0xe1, 0x31, 0xa0, 0x29, 0xb8, 0x50, 0x45, 0xb6, 0x81, 0x81, 0x58, 0x5d,
    0x97, 0x81, 0x6a, 0x91, 0x68, 0x71, 0xca, 0x8d, 0x3c, 0x20, 0x8c, 0x16, 0xd8, 0x7c, 0xfd, 0x47,
];
const BN254_BASE_MODULUS_LE_U64X4: [u64; 4] = [
    0x3c20_8c16_d87c_fd47,
    0x9781_6a91_6871_ca8d,
    0xb850_45b6_8181_585d,
    0x3064_4e72_e131_a029,
];
// mu = floor(2^512 / Fr_modulus), little-endian 64-bit limbs.
const FR_BARRETT_MU_LE_U64X5: [u64; 5] = [
    0x2070_3a6b_e1de_9259,
    0x1448_5200_9e88_0ae6,
    0xb074_a586_8073_0147,
    0x4a47_4626_23a0_4a7a,
    0x0000_0000_0000_0005,
];

// Fixed G2 generator (big-endian, EIP-197 encoding) used for pairing checks.
const PAIRING_REFERENCE_G2_POINT_BE: [u8; 128] = [
    25, 142, 147, 147, 146, 13, 72, 58, 114, 96, 191, 183, 49, 251, 93, 37, 241, 170, 73, 51, 53,
    169, 231, 18, 151, 228, 133, 183, 174, 243, 18, 194, 24, 0, 222, 239, 18, 31, 30, 118, 66, 106,
    0, 102, 94, 92, 68, 121, 103, 67, 34, 212, 247, 94, 218, 221, 70, 222, 189, 92, 217, 146, 246,
    237, 9, 6, 137, 208, 88, 95, 240, 117, 236, 158, 153, 173, 105, 12, 51, 149, 188, 75, 49, 51,
    112, 179, 142, 243, 85, 172, 218, 220, 209, 34, 151, 91, 18, 200, 94, 165, 219, 140, 109, 235,
    74, 171, 113, 128, 141, 203, 64, 143, 227, 209, 231, 105, 12, 67, 211, 123, 76, 230, 204, 1,
    102, 250, 125, 170,
];

const SPECIAL_PUBLIC_INPUTS_SIZE: usize = 16;
const NUM_SMALL_IPA_EVALUATIONS: usize = 4;
const MAX_VIRTUAL_LOG_N: usize = 32;

const NON_ZK_BATCHED_RELATION_PARTIAL_LENGTH: usize = 8;
const ZK_BATCHED_RELATION_PARTIAL_LENGTH: usize = 9;
const NON_ZK_NUM_ALL_ENTITIES: usize = 41;
const ZK_NUM_ALL_ENTITIES: usize = 42;
const MAX_SUMCHECK_ROUND_WIDTH: usize = ZK_BATCHED_RELATION_PARTIAL_LENGTH;
const INVERSE_FACTORIALS_BE: [[u8; 32]; MAX_SUMCHECK_ROUND_WIDTH] = [
    [
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01,
    ],
    [
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01,
    ],
    [
        0x18, 0x32, 0x27, 0x39, 0x70, 0x98, 0xd0, 0x14, 0xdc, 0x28, 0x22, 0xdb, 0x40, 0xc0, 0xac,
        0x2e, 0x94, 0x19, 0xf4, 0x24, 0x3c, 0xdc, 0xb8, 0x48, 0xa1, 0xf0, 0xfa, 0xc9, 0xf8, 0x00,
        0x00, 0x01,
    ],
    [
        0x28, 0x53, 0x96, 0xb5, 0x10, 0xfe, 0xb0, 0x22, 0xc4, 0x42, 0xe4, 0xc2, 0xc1, 0x41, 0x1e,
        0xf8, 0x4c, 0x2b, 0x41, 0x91, 0xba, 0xc5, 0x33, 0x23, 0xb8, 0x91, 0xa1, 0xfb, 0x48, 0x00,
        0x00, 0x01,
    ],
    [
        0x2e, 0x60, 0x20, 0x83, 0x6d, 0x24, 0xe4, 0x27, 0xfb, 0x4c, 0xed, 0x79, 0x91, 0x71, 0x4a,
        0x03, 0xf1, 0x31, 0xbe, 0x9a, 0xc9, 0xfc, 0x61, 0x35, 0xe1, 0x0d, 0xe0, 0xad, 0xc6, 0x00,
        0x00, 0x01,
    ],
    [
        0x12, 0xf4, 0x16, 0x31, 0x42, 0xde, 0x1a, 0x76, 0xbd, 0x85, 0xd7, 0x09, 0x9d, 0x63, 0xba,
        0x13, 0x6b, 0x7a, 0xba, 0xfa, 0x40, 0xbd, 0xf6, 0xc1, 0x6d, 0xc9, 0x91, 0x40, 0x57, 0x99,
        0x99, 0x9a,
    ],
    [
        0x13, 0x4a, 0x1d, 0xd9, 0x2b, 0x8a, 0xe4, 0x77, 0x07, 0xb1, 0x10, 0x69, 0x1a, 0xbb, 0xbc,
        0x77, 0x9f, 0x50, 0x6c, 0x97, 0x33, 0x5d, 0x79, 0x50, 0xa8, 0xec, 0xea, 0x11, 0x5e, 0x99,
        0x99, 0x9a,
    ],
    [
        0x2c, 0x3b, 0xfe, 0x5c, 0xeb, 0xd0, 0xa9, 0xeb, 0x9f, 0x14, 0xf4, 0xf4, 0x97, 0x65, 0x1d,
        0x85, 0x82, 0x5c, 0x8d, 0x78, 0x4b, 0x1a, 0x04, 0x1a, 0x52, 0x51, 0x3d, 0x13, 0x92, 0x15,
        0xf1, 0x60,
    ],
    [
        0x05, 0x87, 0x7f, 0xcb, 0x9d, 0x7a, 0x15, 0x3d, 0x73, 0xe2, 0x9e, 0x9e, 0x92, 0xec, 0xa3,
        0xb0, 0xb0, 0x4b, 0x91, 0xaf, 0x09, 0x63, 0x40, 0x83, 0x4a, 0x4a, 0x27, 0xa2, 0x72, 0x42,
        0xbe, 0x2c,
    ],
];

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum CommitmentEncoding {
    U256,
    FrSplit,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ProofLayoutProfile {
    pub has_zk: bool,
    pub commitment_encoding: CommitmentEncoding,
    pub virtual_log_n: usize,
}

#[derive(Debug, Clone)]
pub struct UltraHonkProofSections {
    pub profile: ProofLayoutProfile,
    pub special_public_inputs: [[u8; 32]; SPECIAL_PUBLIC_INPUTS_SIZE],
    pub hiding_polynomial_commitment: Option<[u8; ALT_BN128_G1_POINT_SIZE]>,
    pub w_l_commitment: [u8; ALT_BN128_G1_POINT_SIZE],
    pub w_r_commitment: [u8; ALT_BN128_G1_POINT_SIZE],
    pub w_o_commitment: [u8; ALT_BN128_G1_POINT_SIZE],
    pub lookup_read_counts_commitment: [u8; ALT_BN128_G1_POINT_SIZE],
    pub lookup_read_tags_commitment: [u8; ALT_BN128_G1_POINT_SIZE],
    pub w_4_commitment: [u8; ALT_BN128_G1_POINT_SIZE],
    pub lookup_inverses_commitment: [u8; ALT_BN128_G1_POINT_SIZE],
    pub z_perm_commitment: [u8; ALT_BN128_G1_POINT_SIZE],
    pub libra_concatenation_commitment: Option<[u8; ALT_BN128_G1_POINT_SIZE]>,
    pub libra_sum: Option<[u8; 32]>,
    pub sumcheck_univariates: Vec<[u8; 32]>,
    pub libra_claimed_evaluation: Option<[u8; 32]>,
    pub sumcheck_evaluations_count: usize,
    pub libra_grand_sum_commitment: Option<[u8; ALT_BN128_G1_POINT_SIZE]>,
    pub libra_quotient_commitment: Option<[u8; ALT_BN128_G1_POINT_SIZE]>,
    pub gemini_fold_commitments: Vec<[u8; ALT_BN128_G1_POINT_SIZE]>,
    pub gemini_fold_evaluations: Vec<[u8; 32]>,
    pub libra_evaluations: Vec<[u8; 32]>,
    pub shplonk_q_commitment: [u8; ALT_BN128_G1_POINT_SIZE],
    pub kzg_w_commitment: [u8; ALT_BN128_G1_POINT_SIZE],
}

fn is_be_one(bytes: &[u8]) -> bool {
    if bytes.len() != ALT_BN128_PAIRING_OUTPUT_SIZE {
        return false;
    }
    bytes[..ALT_BN128_PAIRING_OUTPUT_SIZE - 1]
        .iter()
        .all(|byte| *byte == 0)
        && bytes[ALT_BN128_PAIRING_OUTPUT_SIZE - 1] == 1
}

fn is_scalar_canonical_be(bytes: &[u8; 32]) -> bool {
    bytes < &BN254_SCALAR_MODULUS_BE
}

fn is_base_field_canonical_be(bytes: &[u8; 32]) -> bool {
    bytes < &BN254_BASE_MODULUS_BE
}

fn is_within_bits_be(bytes: &[u8; 32], bits: usize) -> bool {
    let used_bytes = bits.div_ceil(8);
    let leading_zero_bytes = 32usize.saturating_sub(used_bytes);
    if bytes[..leading_zero_bytes].iter().any(|byte| *byte != 0) {
        return false;
    }
    let rem_bits = bits % 8;
    if rem_bits == 0 || leading_zero_bytes >= 32 {
        return true;
    }
    let mask: u8 = 0xff << rem_bits;
    (bytes[leading_zero_bytes] & mask) == 0
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

fn add_le_u64x4(a: [u64; 4], b: [u64; 4]) -> Option<[u64; 4]> {
    let mut out = [0u64; 4];
    let mut carry = 0u128;
    for idx in 0..4 {
        let sum = a[idx] as u128 + b[idx] as u128 + carry;
        out[idx] = sum as u64;
        carry = sum >> 64;
    }
    if carry != 0 {
        return None;
    }
    Some(out)
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

fn shl_le_u64x4(a: [u64; 4], shift: usize) -> [u64; 4] {
    if shift == 0 {
        return a;
    }
    if shift >= 256 {
        return [0u64; 4];
    }
    let word_shift = shift / 64;
    let bit_shift = shift % 64;
    let mut out = [0u64; 4];
    for (idx, limb) in a.iter().enumerate() {
        let target = idx + word_shift;
        if target < 4 {
            out[target] |= *limb << bit_shift;
        }
        if bit_shift != 0 && target + 1 < 4 {
            out[target + 1] |= *limb >> (64 - bit_shift);
        }
    }
    out
}

fn shr_le_u64x4(a: [u64; 4], shift: usize) -> [u64; 4] {
    if shift == 0 {
        return a;
    }
    if shift >= 256 {
        return [0u64; 4];
    }
    let word_shift = shift / 64;
    let bit_shift = shift % 64;
    let mut out = [0u64; 4];
    for idx in word_shift..4 {
        let source = a[idx];
        let target = idx - word_shift;
        out[target] |= source >> bit_shift;
        if bit_shift != 0 && idx + 1 < 4 {
            out[target] |= a[idx + 1] << (64 - bit_shift);
        }
    }
    out
}

fn reduce_u256_mod_fr(bytes: &[u8; 32]) -> Option<[u8; 32]> {
    let modulus = BN254_SCALAR_MODULUS_LE_U64X4;
    let mut value = be_bytes_to_le_u64x4(bytes);
    while cmp_le_u64x4(value, modulus) != core::cmp::Ordering::Less {
        value = sub_le_u64x4(value, modulus)?;
    }
    Some(le_u64x4_to_be_bytes(value))
}

fn split_duplex_challenge(challenge_be: &[u8; 32]) -> ([u8; 32], [u8; 32]) {
    let value = be_bytes_to_le_u64x4(challenge_be);

    // lo = challenge[0..127)
    let mut lo = value;
    lo[1] &= 0x7fff_ffff_ffff_ffff;
    lo[2] = 0;
    lo[3] = 0;

    // hi = challenge[127..254)
    let mut hi = shr_le_u64x4(value, 127);
    hi[1] &= 0x7fff_ffff_ffff_ffff;
    hi[2] = 0;
    hi[3] = 0;

    (le_u64x4_to_be_bytes(lo), le_u64x4_to_be_bytes(hi))
}

fn append_commitment_field_slices<'a>(
    out: &mut Vec<&'a [u8]>,
    point: &'a [u8; ALT_BN128_G1_POINT_SIZE],
) -> Option<()> {
    out.push(point.get(..32)?);
    out.push(point.get(32..)?);
    Some(())
}

fn public_inputs_count_if_canonical(public_inputs_bytes: &[u8]) -> Option<usize> {
    if public_inputs_bytes.len() % 32 != 0 {
        return None;
    }
    for chunk in public_inputs_bytes.chunks_exact(32) {
        let scalar = <[u8; 32]>::try_from(chunk).ok()?;
        if !is_scalar_canonical_be(&scalar) {
            return None;
        }
    }
    Some(public_inputs_bytes.len() / 32)
}

fn next_duplex_challenge(
    is_first_challenge: &mut bool,
    previous_challenge: &mut [u8; 32],
    payload_slices: &[&[u8]],
) -> Option<([u8; 32], [u8; 32])> {
    let include_previous_challenge = !*is_first_challenge;
    if *is_first_challenge && payload_slices.is_empty() {
        return None;
    }
    if *is_first_challenge {
        *is_first_challenge = false;
    }

    let digest = if include_previous_challenge {
        let mut hash_input_slices = Vec::with_capacity(payload_slices.len().saturating_add(1));
        hash_input_slices.push(previous_challenge.as_slice());
        hash_input_slices.extend_from_slice(payload_slices);
        hashv(&hash_input_slices).to_bytes()
    } else {
        hashv(payload_slices).to_bytes()
    };
    let reduced = reduce_u256_mod_fr(&digest)?;
    *previous_challenge = reduced;
    Some(split_duplex_challenge(&reduced))
}

#[derive(Debug, Clone)]
struct SumcheckChallengeSchedule {
    sumcheck_round_challenges: Vec<[u8; 32]>,
    libra_challenge: Option<[u8; 32]>,
    final_previous_challenge: [u8; 32],
}

fn derive_sumcheck_round_challenges_keccak(
    sections: &UltraHonkProofSections,
    vk_hash: &[u8; 32],
    public_inputs_bytes: &[u8],
    public_inputs_count: usize,
) -> Option<SumcheckChallengeSchedule> {
    let round_width = if sections.profile.has_zk {
        ZK_BATCHED_RELATION_PARTIAL_LENGTH
    } else {
        NON_ZK_BATCHED_RELATION_PARTIAL_LENGTH
    };
    let expected_round_elements = sections.profile.virtual_log_n.checked_mul(round_width)?;
    if sections.sumcheck_univariates.len() != expected_round_elements {
        return None;
    }
    let mut is_first_challenge = true;
    let mut previous_challenge = [0u8; 32];
    let first_round_payload_elements = 1usize
        .checked_add(public_inputs_count)?
        .checked_add(SPECIAL_PUBLIC_INPUTS_SIZE)?
        .checked_add(if sections.profile.has_zk { 8 } else { 6 })?;
    let mut payload_slices: Vec<&[u8]> = Vec::with_capacity(first_round_payload_elements);
    payload_slices.push(vk_hash.as_slice());
    payload_slices.extend(public_inputs_bytes.chunks_exact(32));
    for scalar in sections.special_public_inputs.iter() {
        payload_slices.push(scalar.as_slice());
    }
    if sections.profile.has_zk {
        let hiding_polynomial_commitment = sections.hiding_polynomial_commitment.as_ref()?;
        append_commitment_field_slices(
            &mut payload_slices,
            hiding_polynomial_commitment,
        )?;
    }
    append_commitment_field_slices(&mut payload_slices, &sections.w_l_commitment)?;
    append_commitment_field_slices(&mut payload_slices, &sections.w_r_commitment)?;
    append_commitment_field_slices(&mut payload_slices, &sections.w_o_commitment)?;

    // eta, eta_two
    let _ = next_duplex_challenge(
        &mut is_first_challenge,
        &mut previous_challenge,
        &payload_slices,
    )?;
    payload_slices.clear();
    // eta_three
    let _ = next_duplex_challenge(
        &mut is_first_challenge,
        &mut previous_challenge,
        &payload_slices,
    )?;

    append_commitment_field_slices(
        &mut payload_slices,
        &sections.lookup_read_counts_commitment,
    )?;
    append_commitment_field_slices(
        &mut payload_slices,
        &sections.lookup_read_tags_commitment,
    )?;
    append_commitment_field_slices(&mut payload_slices, &sections.w_4_commitment)?;

    // beta, gamma
    let _ = next_duplex_challenge(
        &mut is_first_challenge,
        &mut previous_challenge,
        &payload_slices,
    )?;
    payload_slices.clear();

    append_commitment_field_slices(
        &mut payload_slices,
        &sections.lookup_inverses_commitment,
    )?;
    append_commitment_field_slices(&mut payload_slices, &sections.z_perm_commitment)?;

    // alpha
    let _ = next_duplex_challenge(
        &mut is_first_challenge,
        &mut previous_challenge,
        &payload_slices,
    )?;
    payload_slices.clear();
    // Sumcheck:gate_challenge
    let _ = next_duplex_challenge(
        &mut is_first_challenge,
        &mut previous_challenge,
        &payload_slices,
    )?;

    let mut libra_challenge = None;
    if sections.profile.has_zk {
        let libra_concatenation_commitment = sections.libra_concatenation_commitment.as_ref()?;
        append_commitment_field_slices(
            &mut payload_slices,
            libra_concatenation_commitment,
        )?;
        let libra_sum = sections.libra_sum.as_ref()?;
        payload_slices.push(libra_sum.as_slice());
        // Libra:Challenge
        let (libra_challenge_candidate, _unused) = next_duplex_challenge(
            &mut is_first_challenge,
            &mut previous_challenge,
            &payload_slices,
        )?;
        libra_challenge = Some(libra_challenge_candidate);
        payload_slices.clear();
    }

    let mut sumcheck_round_challenges = Vec::with_capacity(sections.profile.virtual_log_n);
    for round_idx in 0..sections.profile.virtual_log_n {
        let start = round_idx * round_width;
        let end = start + round_width;
        payload_slices.extend(
            sections.sumcheck_univariates[start..end]
                .iter()
                .map(|scalar| scalar.as_slice()),
        );
        let (round_challenge, _second_challenge) = next_duplex_challenge(
            &mut is_first_challenge,
            &mut previous_challenge,
            &payload_slices,
        )?;
        payload_slices.clear();
        sumcheck_round_challenges.push(round_challenge);
    }

    Some(SumcheckChallengeSchedule {
        sumcheck_round_challenges,
        libra_challenge,
        final_previous_challenge: previous_challenge,
    })
}

fn verify_opening_transcript_challenge_chain(
    sections: &UltraHonkProofSections,
    sumcheck_schedule: &SumcheckChallengeSchedule,
) -> bool {
    let Some(gemini_commitment_count) = sections.profile.virtual_log_n.checked_sub(1) else {
        return false;
    };
    if sections.gemini_fold_commitments.len() != gemini_commitment_count {
        return false;
    }
    if sections.gemini_fold_evaluations.len() != sections.profile.virtual_log_n {
        return false;
    }
    if sections.profile.has_zk {
        if sections.libra_evaluations.len() != NUM_SMALL_IPA_EVALUATIONS {
            return false;
        }
    } else if !sections.libra_evaluations.is_empty() {
        return false;
    }

    let mut is_first_challenge = false;
    let mut previous_challenge = sumcheck_schedule.final_previous_challenge;
    let mut payload_slices: Vec<&[u8]> =
        Vec::with_capacity(sections.profile.virtual_log_n + NUM_SMALL_IPA_EVALUATIONS);

    // Gemini:r
    for fold_commitment in sections.gemini_fold_commitments.iter() {
        if append_commitment_field_slices(&mut payload_slices, fold_commitment).is_none() {
            return false;
        }
    }
    let Some((_gemini_r, _unused_gemini_r)) = next_duplex_challenge(
        &mut is_first_challenge,
        &mut previous_challenge,
        &payload_slices,
    ) else {
        return false;
    };
    payload_slices.clear();

    // Shplonk:nu
    payload_slices.extend(
        sections
            .gemini_fold_evaluations
            .iter()
            .map(|scalar| scalar.as_slice()),
    );
    if sections.profile.has_zk {
        payload_slices.extend(
            sections
                .libra_evaluations
                .iter()
                .map(|scalar| scalar.as_slice()),
        );
    }
    let Some((_shplonk_nu, _unused_shplonk_nu)) = next_duplex_challenge(
        &mut is_first_challenge,
        &mut previous_challenge,
        &payload_slices,
    ) else {
        return false;
    };
    payload_slices.clear();

    // Shplonk:z
    if append_commitment_field_slices(&mut payload_slices, &sections.shplonk_q_commitment).is_none()
    {
        return false;
    }
    let Some((_shplonk_z, _unused_shplonk_z)) = next_duplex_challenge(
        &mut is_first_challenge,
        &mut previous_challenge,
        &payload_slices,
    ) else {
        return false;
    };
    true
}

fn decode_fr_split_base_field(low_limb: &[u8; 32], high_limb: &[u8; 32]) -> Option<[u8; 32]> {
    // FrCodec packs base field values as: value = limb0 + (limb1 << 136)
    // with limb0 < 2^136 and limb1 < 2^118.
    if !is_within_bits_be(low_limb, 136) || !is_within_bits_be(high_limb, 118) {
        return None;
    }
    let low = be_bytes_to_le_u64x4(low_limb);
    let high = be_bytes_to_le_u64x4(high_limb);
    let high_shifted = shl_le_u64x4(high, 136);
    let value = add_le_u64x4(low, high_shifted)?;
    Some(le_u64x4_to_be_bytes(value))
}

fn decode_pairing_point_coordinate(
    low_limb_0: &[u8; 32],
    low_limb_1: &[u8; 32],
    low_limb_2: &[u8; 32],
    high_limb: &[u8; 32],
) -> Option<[u8; 32]> {
    // Pairing points are encoded as 4 limbs in radix-2^68:
    // value = limb0 + (limb1 << 68) + (limb2 << 136) + (limb3 << 204)
    // The top limb must fit in 52 bits to avoid truncation in 256-bit reconstruction.
    if !is_within_bits_be(low_limb_0, 68)
        || !is_within_bits_be(low_limb_1, 68)
        || !is_within_bits_be(low_limb_2, 68)
        || !is_within_bits_be(high_limb, 52)
    {
        return None;
    }

    let mut value = be_bytes_to_le_u64x4(low_limb_0);
    value = add_le_u64x4(value, shl_le_u64x4(be_bytes_to_le_u64x4(low_limb_1), 68))?;
    value = add_le_u64x4(value, shl_le_u64x4(be_bytes_to_le_u64x4(low_limb_2), 136))?;
    value = add_le_u64x4(value, shl_le_u64x4(be_bytes_to_le_u64x4(high_limb), 204))?;

    let coordinate = le_u64x4_to_be_bytes(value);
    if !is_base_field_canonical_be(&coordinate) {
        return None;
    }
    Some(coordinate)
}

fn decode_pairing_special_points(
    special_public_inputs: &[[u8; 32]; SPECIAL_PUBLIC_INPUTS_SIZE],
) -> Option<[[u8; ALT_BN128_G1_POINT_SIZE]; 2]> {
    let mut points = [[0u8; ALT_BN128_G1_POINT_SIZE]; 2];
    for point_idx in 0..2 {
        let base = point_idx * 8;
        let x = decode_pairing_point_coordinate(
            &special_public_inputs[base],
            &special_public_inputs[base + 1],
            &special_public_inputs[base + 2],
            &special_public_inputs[base + 3],
        )?;
        let y = decode_pairing_point_coordinate(
            &special_public_inputs[base + 4],
            &special_public_inputs[base + 5],
            &special_public_inputs[base + 6],
            &special_public_inputs[base + 7],
        )?;
        points[point_idx][..32].copy_from_slice(&x);
        points[point_idx][32..].copy_from_slice(&y);
    }
    Some(points)
}

fn fr_add_mod(a: &[u8; 32], b: &[u8; 32]) -> Option<[u8; 32]> {
    let reduced = fr_add_mod_limbs(be_bytes_to_le_u64x4(a), be_bytes_to_le_u64x4(b))?;
    Some(le_u64x4_to_be_bytes(reduced))
}

fn fr_add_mod_limbs(a: [u64; 4], b: [u64; 4]) -> Option<[u64; 4]> {
    let modulus = BN254_SCALAR_MODULUS_LE_U64X4;
    let sum = add_le_u64x4(a, b)?;
    let reduced = if cmp_le_u64x4(sum, modulus) != core::cmp::Ordering::Less {
        sub_le_u64x4(sum, modulus)?
    } else {
        sum
    };
    Some(reduced)
}

fn fr_sub_mod_limbs(a: [u64; 4], b: [u64; 4]) -> Option<[u64; 4]> {
    let modulus = BN254_SCALAR_MODULUS_LE_U64X4;
    let reduced = match cmp_le_u64x4(a, b) {
        core::cmp::Ordering::Greater => sub_le_u64x4(a, b)?,
        core::cmp::Ordering::Equal => [0u64; 4],
        core::cmp::Ordering::Less => {
            let delta = sub_le_u64x4(b, a)?;
            sub_le_u64x4(modulus, delta)?
        }
    };
    Some(reduced)
}

fn mul_le_u64x4(a: [u64; 4], b: [u64; 4]) -> [u64; 8] {
    let mut out = [0u64; 8];
    for (i, a_limb) in a.iter().enumerate() {
        let mut carry = 0u128;
        for (j, b_limb) in b.iter().enumerate() {
            let idx = i + j;
            let accum = (*a_limb as u128) * (*b_limb as u128) + (out[idx] as u128) + carry;
            out[idx] = accum as u64;
            carry = accum >> 64;
        }
        let mut idx = i + 4;
        while carry != 0 {
            let accum = (out[idx] as u128) + carry;
            out[idx] = accum as u64;
            carry = accum >> 64;
            idx += 1;
        }
    }
    out
}

fn mul_le_u64x5_by_u64x5(a: [u64; 5], b: [u64; 5]) -> [u64; 10] {
    let mut out = [0u64; 10];
    for (i, a_limb) in a.iter().enumerate() {
        let mut carry = 0u128;
        for (j, b_limb) in b.iter().enumerate() {
            let idx = i + j;
            let accum = (*a_limb as u128) * (*b_limb as u128) + (out[idx] as u128) + carry;
            out[idx] = accum as u64;
            carry = accum >> 64;
        }
        let mut idx = i + 5;
        while carry != 0 {
            let accum = (out[idx] as u128) + carry;
            out[idx] = accum as u64;
            carry = accum >> 64;
            idx += 1;
        }
    }
    out
}

fn mul_le_u64x5_by_u64x4(a: [u64; 5], b: [u64; 4]) -> [u64; 9] {
    let mut out = [0u64; 9];
    for (i, a_limb) in a.iter().enumerate() {
        let mut carry = 0u128;
        for (j, b_limb) in b.iter().enumerate() {
            let idx = i + j;
            let accum = (*a_limb as u128) * (*b_limb as u128) + (out[idx] as u128) + carry;
            out[idx] = accum as u64;
            carry = accum >> 64;
        }
        let mut idx = i + 4;
        while carry != 0 {
            let accum = (out[idx] as u128) + carry;
            out[idx] = accum as u64;
            carry = accum >> 64;
            idx += 1;
        }
    }
    out
}

fn sub_wrap_le_u64x5(a: [u64; 5], b: [u64; 5]) -> [u64; 5] {
    let mut out = [0u64; 5];
    let mut borrow = 0u64;
    for idx in 0..5 {
        let (tmp, borrow_a) = a[idx].overflowing_sub(b[idx]);
        let (res, borrow_b) = tmp.overflowing_sub(borrow);
        out[idx] = res;
        borrow = (borrow_a as u64) | (borrow_b as u64);
    }
    out
}

fn sub_modulus_from_le_u64x5(value: [u64; 5]) -> [u64; 5] {
    let mut out = value;
    let mut borrow = 0u64;
    for idx in 0..4 {
        let (tmp, borrow_a) = out[idx].overflowing_sub(BN254_SCALAR_MODULUS_LE_U64X4[idx]);
        let (res, borrow_b) = tmp.overflowing_sub(borrow);
        out[idx] = res;
        borrow = (borrow_a as u64) | (borrow_b as u64);
    }
    if borrow != 0 {
        let (res, _borrow_out) = out[4].overflowing_sub(borrow);
        out[4] = res;
    }
    out
}

fn reduce_u512_mod_fr_limbs(product: [u64; 8]) -> [u64; 4] {
    // Barrett reduction with base 2^64 and k = 4 limbs.
    // q1 = floor(product / 2^(64*(k-1)))      -> 5 limbs
    // q2 = q1 * mu                             -> 10 limbs
    // q3 = floor(q2 / 2^(64*(k+1)))            -> 5 limbs
    // r  = (product mod 2^(64*(k+1))) - (q3*n mod 2^(64*(k+1)))
    let q1 = [product[3], product[4], product[5], product[6], product[7]];
    let q2 = mul_le_u64x5_by_u64x5(q1, FR_BARRETT_MU_LE_U64X5);
    let q3 = [q2[5], q2[6], q2[7], q2[8], q2[9]];

    let r1 = [product[0], product[1], product[2], product[3], product[4]];
    let q3_times_modulus = mul_le_u64x5_by_u64x4(q3, BN254_SCALAR_MODULUS_LE_U64X4);
    let r2 = [
        q3_times_modulus[0],
        q3_times_modulus[1],
        q3_times_modulus[2],
        q3_times_modulus[3],
        q3_times_modulus[4],
    ];
    let mut r = sub_wrap_le_u64x5(r1, r2);

    while r[4] != 0
        || cmp_le_u64x4([r[0], r[1], r[2], r[3]], BN254_SCALAR_MODULUS_LE_U64X4)
            != core::cmp::Ordering::Less
    {
        r = sub_modulus_from_le_u64x5(r);
    }

    [r[0], r[1], r[2], r[3]]
}

fn fr_mul_mod_limbs(a: [u64; 4], b: [u64; 4]) -> [u64; 4] {
    let product = mul_le_u64x4(a, b);
    reduce_u512_mod_fr_limbs(product)
}

fn fr_mul_mod(a: &[u8; 32], b: &[u8; 32]) -> Option<[u8; 32]> {
    Some(le_u64x4_to_be_bytes(fr_mul_mod_limbs(
        be_bytes_to_le_u64x4(a),
        be_bytes_to_le_u64x4(b),
    )))
}

fn fr_is_zero(value: &[u8; 32]) -> bool {
    value.iter().all(|byte| *byte == 0)
}

fn evaluate_sumcheck_round_univariate(
    round_univariate: &[[u8; 32]],
    round_challenge: &[u8; 32],
) -> Option<[u8; 32]> {
    if round_univariate.is_empty() || round_univariate.len() > MAX_SUMCHECK_ROUND_WIDTH {
        return None;
    }

    let round_width = round_univariate.len();
    let mut forward_differences = [[0u64; 4]; MAX_SUMCHECK_ROUND_WIDTH];
    let mut divided_differences = [[0u64; 4]; MAX_SUMCHECK_ROUND_WIDTH];
    for idx in 0..round_width {
        forward_differences[idx] = be_bytes_to_le_u64x4(&round_univariate[idx]);
    }
    divided_differences[0] = forward_differences[0];

    for order in 1..round_width {
        for idx in 0..(round_width - order) {
            forward_differences[idx] =
                fr_sub_mod_limbs(forward_differences[idx + 1], forward_differences[idx])?;
        }
        // inverse_factorial(1) == 1
        divided_differences[order] = if order == 1 {
            forward_differences[0]
        } else {
            fr_mul_mod_limbs(
                forward_differences[0],
                be_bytes_to_le_u64x4(&INVERSE_FACTORIALS_BE[order]),
            )
        };
    }

    let mut target_sum = divided_differences[round_width - 1];
    let challenge_limbs = be_bytes_to_le_u64x4(round_challenge);
    for order in (0..(round_width - 1)).rev() {
        let challenge_minus_domain_point =
            fr_sub_mod_limbs(challenge_limbs, [order as u64, 0, 0, 0])?;
        let folded = fr_mul_mod_limbs(challenge_minus_domain_point, target_sum);
        target_sum = fr_add_mod_limbs(divided_differences[order], folded)?;
    }
    Some(le_u64x4_to_be_bytes(target_sum))
}

#[cfg(test)]
fn g1_mul_input(
    point: &[u8; ALT_BN128_G1_POINT_SIZE],
    scalar: &[u8; 32],
) -> [u8; ALT_BN128_G1_MULTIPLICATION_INPUT_SIZE] {
    let mut input = [0u8; ALT_BN128_G1_MULTIPLICATION_INPUT_SIZE];
    input[..ALT_BN128_G1_POINT_SIZE].copy_from_slice(point);
    input[ALT_BN128_G1_POINT_SIZE..].copy_from_slice(scalar);
    input
}

fn g1_add_input(
    left: &[u8; ALT_BN128_G1_POINT_SIZE],
    right: &[u8; ALT_BN128_G1_POINT_SIZE],
) -> [u8; ALT_BN128_G1_ADDITION_INPUT_SIZE] {
    let mut input = [0u8; ALT_BN128_G1_ADDITION_INPUT_SIZE];
    input[..ALT_BN128_G1_POINT_SIZE].copy_from_slice(left);
    input[ALT_BN128_G1_POINT_SIZE..].copy_from_slice(right);
    input
}

#[cfg(test)]
fn g1_mul_point(
    point: &[u8; ALT_BN128_G1_POINT_SIZE],
    scalar: &[u8; 32],
) -> Result<[u8; ALT_BN128_G1_POINT_SIZE], AltBn128Error> {
    let output = alt_bn128_g1_multiplication_be(&g1_mul_input(point, scalar))?;
    <[u8; ALT_BN128_G1_POINT_SIZE]>::try_from(output.as_slice())
        .map_err(|_| AltBn128Error::UnexpectedError)
}

fn g1_add_points(
    left: &[u8; ALT_BN128_G1_POINT_SIZE],
    right: &[u8; ALT_BN128_G1_POINT_SIZE],
) -> Result<[u8; ALT_BN128_G1_POINT_SIZE], AltBn128Error> {
    let output = alt_bn128_g1_addition_be(&g1_add_input(left, right))?;
    <[u8; ALT_BN128_G1_POINT_SIZE]>::try_from(output.as_slice())
        .map_err(|_| AltBn128Error::UnexpectedError)
}

fn validate_g1_point(point: &[u8; ALT_BN128_G1_POINT_SIZE]) -> Result<(), AltBn128Error> {
    // Using addition with identity for curve validation is materially cheaper than mul-by-1,
    // while still fail-closing invalid encodings through the syscall decoder.
    let mut input = [0u8; ALT_BN128_G1_ADDITION_INPUT_SIZE];
    input[..ALT_BN128_G1_POINT_SIZE].copy_from_slice(point);
    let output = alt_bn128_g1_addition_be(&input)?;
    if output.as_slice() != point {
        return Err(AltBn128Error::UnexpectedError);
    }
    Ok(())
}

fn g1_negate_point(
    point: &[u8; ALT_BN128_G1_POINT_SIZE],
) -> Option<[u8; ALT_BN128_G1_POINT_SIZE]> {
    let x = <[u8; 32]>::try_from(point.get(..32)?).ok()?;
    let y = <[u8; 32]>::try_from(point.get(32..)?).ok()?;
    if !is_base_field_canonical_be(&x) || !is_base_field_canonical_be(&y) {
        return None;
    }
    let y_neg = if y == [0u8; 32] {
        [0u8; 32]
    } else {
        let y_limbs = be_bytes_to_le_u64x4(&y);
        let neg_limbs = sub_le_u64x4(BN254_BASE_MODULUS_LE_U64X4, y_limbs)?;
        le_u64x4_to_be_bytes(neg_limbs)
    };
    let mut out = [0u8; ALT_BN128_G1_POINT_SIZE];
    out[..32].copy_from_slice(&x);
    out[32..].copy_from_slice(&y_neg);
    Some(out)
}

struct ProofCursor<'a> {
    proof_bytes: &'a [u8],
    total_elements: usize,
    cursor: usize,
}

impl<'a> ProofCursor<'a> {
    fn new(proof_bytes: &'a [u8]) -> Option<Self> {
        if proof_bytes.is_empty() || proof_bytes.len() % 32 != 0 {
            return None;
        }
        Some(Self {
            proof_bytes,
            total_elements: proof_bytes.len() / 32,
            cursor: 0,
        })
    }

    fn take_scalar(&mut self) -> Option<[u8; 32]> {
        if self.cursor >= self.total_elements {
            return None;
        }
        let start = self.cursor.checked_mul(32)?;
        let end = start.checked_add(32)?;
        let out = <[u8; 32]>::try_from(self.proof_bytes.get(start..end)?).ok()?;
        self.cursor += 1;
        Some(out)
    }

    fn take_scalar_canonical(&mut self) -> Option<[u8; 32]> {
        let scalar = self.take_scalar()?;
        if !is_scalar_canonical_be(&scalar) {
            return None;
        }
        Some(scalar)
    }

    fn take_scalars_canonical(&mut self, count: usize) -> Option<Vec<[u8; 32]>> {
        let end_cursor = self.cursor.checked_add(count)?;
        if end_cursor > self.total_elements {
            return None;
        }
        let start = self.cursor.checked_mul(32)?;
        let end = end_cursor.checked_mul(32)?;
        let mut out = Vec::with_capacity(count);
        for chunk in self.proof_bytes.get(start..end)?.chunks_exact(32) {
            let scalar = <[u8; 32]>::try_from(chunk).ok()?;
            if !is_scalar_canonical_be(&scalar) {
                return None;
            }
            out.push(scalar);
        }
        self.cursor = end_cursor;
        Some(out)
    }

    fn skip_scalars_canonical(&mut self, count: usize) -> Option<()> {
        let end_cursor = self.cursor.checked_add(count)?;
        if end_cursor > self.total_elements {
            return None;
        }
        let start = self.cursor.checked_mul(32)?;
        let end = end_cursor.checked_mul(32)?;
        for chunk in self.proof_bytes.get(start..end)?.chunks_exact(32) {
            let scalar = <[u8; 32]>::try_from(chunk).ok()?;
            if !is_scalar_canonical_be(&scalar) {
                return None;
            }
        }
        self.cursor = end_cursor;
        Some(())
    }

    fn take_commitment(
        &mut self,
        encoding: CommitmentEncoding,
    ) -> Option<[u8; ALT_BN128_G1_POINT_SIZE]> {
        let mut point = [0u8; ALT_BN128_G1_POINT_SIZE];
        match encoding {
            CommitmentEncoding::U256 => {
                let x = self.take_scalar()?;
                let y = self.take_scalar()?;
                if !is_base_field_canonical_be(&x) || !is_base_field_canonical_be(&y) {
                    return None;
                }
                point[..32].copy_from_slice(&x);
                point[32..].copy_from_slice(&y);
            }
            CommitmentEncoding::FrSplit => {
                let x0 = self.take_scalar()?;
                let x1 = self.take_scalar()?;
                let y0 = self.take_scalar()?;
                let y1 = self.take_scalar()?;
                let x = decode_fr_split_base_field(&x0, &x1)?;
                let y = decode_fr_split_base_field(&y0, &y1)?;
                if !is_base_field_canonical_be(&x) || !is_base_field_canonical_be(&y) {
                    return None;
                }
                point[..32].copy_from_slice(&x);
                point[32..].copy_from_slice(&y);
            }
        }
        if validate_g1_point(&point).is_err() {
            return None;
        }
        Some(point)
    }
}

fn parse_proof_sections_for_profile(
    proof_bytes: &[u8],
    profile: ProofLayoutProfile,
) -> Option<Box<UltraHonkProofSections>> {
    let mut cursor = ProofCursor::new(proof_bytes)?;

    let mut special_public_inputs = [[0u8; 32]; SPECIAL_PUBLIC_INPUTS_SIZE];
    for scalar in special_public_inputs.iter_mut() {
        *scalar = cursor.take_scalar_canonical()?;
    }
    decode_pairing_special_points(&special_public_inputs)?;

    let hiding_polynomial_commitment = if profile.has_zk {
        Some(cursor.take_commitment(profile.commitment_encoding)?)
    } else {
        None
    };

    let w_l_commitment = cursor.take_commitment(profile.commitment_encoding)?;
    let w_r_commitment = cursor.take_commitment(profile.commitment_encoding)?;
    let w_o_commitment = cursor.take_commitment(profile.commitment_encoding)?;
    let lookup_read_counts_commitment = cursor.take_commitment(profile.commitment_encoding)?;
    let lookup_read_tags_commitment = cursor.take_commitment(profile.commitment_encoding)?;
    let w_4_commitment = cursor.take_commitment(profile.commitment_encoding)?;
    let lookup_inverses_commitment = cursor.take_commitment(profile.commitment_encoding)?;
    let z_perm_commitment = cursor.take_commitment(profile.commitment_encoding)?;

    let libra_concatenation_commitment = if profile.has_zk {
        Some(cursor.take_commitment(profile.commitment_encoding)?)
    } else {
        None
    };
    let libra_sum = if profile.has_zk {
        Some(cursor.take_scalar_canonical()?)
    } else {
        None
    };

    let batched_relation_partial_length = if profile.has_zk {
        ZK_BATCHED_RELATION_PARTIAL_LENGTH
    } else {
        NON_ZK_BATCHED_RELATION_PARTIAL_LENGTH
    };
    let sumcheck_univariate_count = profile
        .virtual_log_n
        .checked_mul(batched_relation_partial_length)?;
    let sumcheck_univariates = cursor.take_scalars_canonical(sumcheck_univariate_count)?;

    let libra_claimed_evaluation = if profile.has_zk {
        Some(cursor.take_scalar_canonical()?)
    } else {
        None
    };

    let num_all_entities = if profile.has_zk {
        ZK_NUM_ALL_ENTITIES
    } else {
        NON_ZK_NUM_ALL_ENTITIES
    };
    cursor.skip_scalars_canonical(num_all_entities)?;

    let libra_grand_sum_commitment = if profile.has_zk {
        Some(cursor.take_commitment(profile.commitment_encoding)?)
    } else {
        None
    };
    let libra_quotient_commitment = if profile.has_zk {
        Some(cursor.take_commitment(profile.commitment_encoding)?)
    } else {
        None
    };

    let gemini_fold_commitments = {
        let count = profile.virtual_log_n.checked_sub(1)?;
        let mut out = Vec::with_capacity(count);
        for _ in 0..count {
            out.push(cursor.take_commitment(profile.commitment_encoding)?);
        }
        out
    };
    let gemini_fold_evaluations = cursor.take_scalars_canonical(profile.virtual_log_n)?;

    let libra_evaluations = if profile.has_zk {
        cursor.take_scalars_canonical(NUM_SMALL_IPA_EVALUATIONS)?
    } else {
        Vec::new()
    };

    let shplonk_q_commitment = cursor.take_commitment(profile.commitment_encoding)?;
    let kzg_w_commitment = cursor.take_commitment(profile.commitment_encoding)?;

    if cursor.cursor != cursor.total_elements {
        return None;
    }

    Some(Box::new(UltraHonkProofSections {
        profile,
        special_public_inputs,
        hiding_polynomial_commitment,
        w_l_commitment,
        w_r_commitment,
        w_o_commitment,
        lookup_read_counts_commitment,
        lookup_read_tags_commitment,
        w_4_commitment,
        lookup_inverses_commitment,
        z_perm_commitment,
        libra_concatenation_commitment,
        libra_sum,
        sumcheck_univariates,
        libra_claimed_evaluation,
        sumcheck_evaluations_count: num_all_entities,
        libra_grand_sum_commitment,
        libra_quotient_commitment,
        gemini_fold_commitments,
        gemini_fold_evaluations,
        libra_evaluations,
        shplonk_q_commitment,
        kzg_w_commitment,
    }))
}

fn infer_profile_from_element_count(
    element_count: usize,
    has_zk: bool,
    commitment_encoding: CommitmentEncoding,
) -> Option<ProofLayoutProfile> {
    let (base_elements, per_round_elements) = match (has_zk, commitment_encoding) {
        (false, CommitmentEncoding::U256) => (75usize, 11usize),
        (false, CommitmentEncoding::FrSplit) => (93usize, 13usize),
        (true, CommitmentEncoding::U256) => (90usize, 12usize),
        (true, CommitmentEncoding::FrSplit) => (116usize, 14usize),
    };
    if element_count < base_elements {
        return None;
    }
    let delta = element_count.checked_sub(base_elements)?;
    if delta % per_round_elements != 0 {
        return None;
    }
    let virtual_log_n = delta / per_round_elements;
    if !(1..=MAX_VIRTUAL_LOG_N).contains(&virtual_log_n) {
        return None;
    }
    Some(ProofLayoutProfile {
        has_zk,
        commitment_encoding,
        virtual_log_n,
    })
}

#[inline(never)]
pub fn parse_ultrahonk_proof_sections(proof_bytes: &[u8]) -> Option<Box<UltraHonkProofSections>> {
    if proof_bytes.is_empty() || proof_bytes.len() % 32 != 0 {
        return None;
    }
    let element_count = proof_bytes.len() / 32;
    let profile_candidates = [
        (false, CommitmentEncoding::U256),
        (false, CommitmentEncoding::FrSplit),
        (true, CommitmentEncoding::U256),
        (true, CommitmentEncoding::FrSplit),
    ];
    for (has_zk, commitment_encoding) in profile_candidates {
        let Some(profile) =
            infer_profile_from_element_count(element_count, has_zk, commitment_encoding)
        else {
            continue;
        };
        if let Some(sections) = parse_proof_sections_for_profile(proof_bytes, profile) {
            return Some(sections);
        }
    }
    None
}

#[inline(never)]
fn verify_commitment_linearity(
    p: &[u8; ALT_BN128_G1_POINT_SIZE],
    q: &[u8; ALT_BN128_G1_POINT_SIZE],
) -> Result<bool, AltBn128Error> {
    let p_plus_q = g1_add_points(p, q)?;
    let two_p = g1_add_points(p, p)?;
    let two_q = g1_add_points(q, q)?;
    let lhs = g1_add_points(&two_p, &two_q)?;
    let rhs = g1_add_points(&p_plus_q, &p_plus_q)?;
    Ok(lhs == rhs)
}

#[inline(never)]
fn verify_kzg_pairing_identity(w: &[u8; ALT_BN128_G1_POINT_SIZE]) -> Result<bool, AltBn128Error> {
    let neg_w = g1_negate_point(w).ok_or(AltBn128Error::UnexpectedError)?;
    let mut pairing_input = [0u8; ALT_BN128_PAIRING_ELEMENT_SIZE * 2];
    pairing_input[..ALT_BN128_G1_POINT_SIZE].copy_from_slice(w);
    pairing_input[ALT_BN128_G1_POINT_SIZE..ALT_BN128_PAIRING_ELEMENT_SIZE]
        .copy_from_slice(&PAIRING_REFERENCE_G2_POINT_BE);
    pairing_input[ALT_BN128_PAIRING_ELEMENT_SIZE
        ..ALT_BN128_PAIRING_ELEMENT_SIZE + ALT_BN128_G1_POINT_SIZE]
        .copy_from_slice(&neg_w);
    pairing_input[ALT_BN128_PAIRING_ELEMENT_SIZE + ALT_BN128_G1_POINT_SIZE..]
        .copy_from_slice(&PAIRING_REFERENCE_G2_POINT_BE);

    let pairing_output = alt_bn128_pairing_be(&pairing_input)?;
    Ok(is_be_one(&pairing_output))
}

#[inline(never)]
fn verify_sumcheck_transcript_round_consistency(
    sections: &UltraHonkProofSections,
    vk_hash: &[u8; 32],
    public_inputs_bytes: &[u8],
    public_inputs_count: usize,
) -> Option<SumcheckChallengeSchedule> {
    let round_width = if sections.profile.has_zk {
        ZK_BATCHED_RELATION_PARTIAL_LENGTH
    } else {
        NON_ZK_BATCHED_RELATION_PARTIAL_LENGTH
    };
    let expected_round_elements = sections.profile.virtual_log_n.saturating_mul(round_width);
    if sections.sumcheck_univariates.len() != expected_round_elements {
        return None;
    }
    if sections.profile.has_zk
        && (sections.libra_sum.is_none() || sections.libra_claimed_evaluation.is_none())
    {
        return None;
    }

    let Some(sumcheck_challenge_schedule) = derive_sumcheck_round_challenges_keccak(
        sections,
        vk_hash,
        public_inputs_bytes,
        public_inputs_count,
    )
    else {
        return None;
    };
    if sumcheck_challenge_schedule.sumcheck_round_challenges.len() != sections.profile.virtual_log_n
    {
        return None;
    }
    let mut round_target_sum = if sections.profile.has_zk {
        let Some(libra_sum) = sections.libra_sum else {
            return None;
        };
        let Some(libra_challenge) = sumcheck_challenge_schedule.libra_challenge else {
            return None;
        };
        if fr_is_zero(&libra_challenge) {
            return None;
        }
        let Some(target) = fr_mul_mod(&libra_challenge, &libra_sum) else {
            return None;
        };
        target
    } else {
        [0u8; 32]
    };

    for round_idx in 0..sections.profile.virtual_log_n {
        let start = round_idx * round_width;
        let end = start + round_width;
        let round = &sections.sumcheck_univariates[start..end];
        let Some(round_sum) = fr_add_mod(&round[0], &round[1]) else {
            return None;
        };
        if round_sum != round_target_sum {
            return None;
        }
        let round_challenge = sumcheck_challenge_schedule.sumcheck_round_challenges[round_idx];
        if fr_is_zero(&round_challenge) {
            return None;
        }
        let Some(next_target_sum) = evaluate_sumcheck_round_univariate(round, &round_challenge)
        else {
            return None;
        };
        round_target_sum = next_target_sum;
    }

    Some(sumcheck_challenge_schedule)
}

#[inline(never)]
pub fn evaluate_ultrahonk_arithmetic_checks_with_public_inputs_count(
    proof_bytes: &[u8],
    vk_hash: &[u8; 32],
    public_inputs_bytes: &[u8],
    public_inputs_count: usize,
) -> Result<bool, AltBn128Error> {
    let Some(sections) = parse_ultrahonk_proof_sections(proof_bytes) else {
        return Ok(false);
    };
    let Some(sumcheck_schedule) = verify_sumcheck_transcript_round_consistency(
        &sections,
        vk_hash,
        public_inputs_bytes,
        public_inputs_count,
    ) else {
        return Ok(false);
    };
    if !verify_opening_transcript_challenge_chain(&sections, &sumcheck_schedule) {
        return Ok(false);
    }
    let commitment_linearity_ok =
        verify_commitment_linearity(&sections.w_l_commitment, &sections.w_r_commitment)?;
    if !commitment_linearity_ok {
        return Ok(false);
    }
    let pairing_identity_ok = verify_kzg_pairing_identity(&sections.kzg_w_commitment)?;
    if !pairing_identity_ok {
        return Ok(false);
    }

    Ok(true)
}

#[inline(never)]
pub fn evaluate_ultrahonk_arithmetic_checks(
    proof_bytes: &[u8],
    vk_hash: &[u8; 32],
    public_inputs_bytes: &[u8],
) -> Result<bool, AltBn128Error> {
    let Some(public_inputs_count) = public_inputs_count_if_canonical(public_inputs_bytes) else {
        return Ok(false);
    };
    evaluate_ultrahonk_arithmetic_checks_with_public_inputs_count(
        proof_bytes,
        vk_hash,
        public_inputs_bytes,
        public_inputs_count,
    )
}

#[cfg(test)]
pub fn runtime_preflight_check() -> Result<(), AltBn128Error> {
    // Check G1 addition path with identity points.
    let g1_add_output = alt_bn128_g1_addition_be(&[0u8; ALT_BN128_G1_ADDITION_INPUT_SIZE])?;
    if g1_add_output.len() != ALT_BN128_G1_POINT_SIZE || g1_add_output.iter().any(|byte| *byte != 0)
    {
        return Err(AltBn128Error::UnexpectedError);
    }

    // Check G1 multiplication path with identity point and scalar 0.
    let g1_mul_output =
        alt_bn128_g1_multiplication_be(&[0u8; ALT_BN128_G1_MULTIPLICATION_INPUT_SIZE])?;
    if g1_mul_output.len() != ALT_BN128_G1_POINT_SIZE || g1_mul_output.iter().any(|byte| *byte != 0)
    {
        return Err(AltBn128Error::UnexpectedError);
    }

    // Check pairing path with identity pair.
    let pairing_output = alt_bn128_pairing_be(&[0u8; ALT_BN128_PAIRING_ELEMENT_SIZE])?;
    if !is_be_one(&pairing_output) {
        return Err(AltBn128Error::UnexpectedError);
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    const TEST_G1_POINT_BE: [u8; 64] = [
        45, 206, 255, 166, 152, 55, 128, 138, 79, 217, 145, 164, 25, 74, 120, 234, 234, 217, 68,
        149, 162, 44, 133, 120, 184, 205, 12, 44, 175, 98, 168, 172, 20, 24, 216, 15, 209, 175,
        106, 75, 147, 236, 90, 101, 123, 219, 245, 151, 209, 202, 218, 104, 148, 8, 32, 254, 243,
        191, 218, 122, 42, 81, 193, 84,
    ];
    const TEST_G1_NEG_POINT_BE: [u8; 64] = [
        45, 206, 255, 166, 152, 55, 128, 138, 79, 217, 145, 164, 25, 74, 120, 234, 234, 217, 68,
        149, 162, 44, 133, 120, 184, 205, 12, 44, 175, 98, 168, 172, 28, 75, 118, 99, 15, 130, 53,
        222, 36, 99, 235, 81, 5, 165, 98, 197, 197, 182, 144, 40, 212, 105, 169, 142, 72, 96, 177,
        156, 174, 43, 59, 243,
    ];

    fn hex32(hex: &str) -> [u8; 32] {
        let trimmed = hex.strip_prefix("0x").unwrap_or(hex);
        assert!(trimmed.len() <= 64);
        let mut padded = String::with_capacity(64);
        for _ in 0..(64 - trimmed.len()) {
            padded.push('0');
        }
        padded.push_str(trimmed);

        let mut out = [0u8; 32];
        for idx in 0..32 {
            out[idx] =
                u8::from_str_radix(&padded[idx * 2..(idx * 2) + 2], 16).expect("valid hex byte");
        }
        out
    }

    fn push_scalar(out: &mut Vec<u8>, scalar: [u8; 32]) {
        out.extend_from_slice(&scalar);
    }

    fn push_commitment(out: &mut Vec<u8>, point: &[u8; ALT_BN128_G1_POINT_SIZE]) {
        out.extend_from_slice(&point[..32]);
        out.extend_from_slice(&point[32..]);
    }

    fn sample_valid_non_zk_keccak_proof_bytes() -> Vec<u8> {
        const LOG_N: usize = 2;
        let mut proof = Vec::new();

        // 16 special public inputs (pairing points). Zero limbs encode identity points.
        for _ in 0..SPECIAL_PUBLIC_INPUTS_SIZE {
            push_scalar(&mut proof, [0u8; 32]);
        }

        // Core commitments.
        push_commitment(&mut proof, &TEST_G1_POINT_BE); // W_L
        push_commitment(&mut proof, &TEST_G1_NEG_POINT_BE); // W_R
        push_commitment(&mut proof, &TEST_G1_POINT_BE); // W_O
        push_commitment(&mut proof, &TEST_G1_POINT_BE); // LOOKUP_READ_COUNTS
        push_commitment(&mut proof, &TEST_G1_POINT_BE); // LOOKUP_READ_TAGS
        push_commitment(&mut proof, &TEST_G1_POINT_BE); // W_4
        push_commitment(&mut proof, &TEST_G1_POINT_BE); // LOOKUP_INVERSES
        push_commitment(&mut proof, &TEST_G1_POINT_BE); // Z_PERM

        // Sumcheck univariates.
        for _ in 0..(LOG_N * NON_ZK_BATCHED_RELATION_PARTIAL_LENGTH) {
            push_scalar(&mut proof, [0u8; 32]);
        }

        // Sumcheck evaluations.
        for _ in 0..NON_ZK_NUM_ALL_ENTITIES {
            push_scalar(&mut proof, [0u8; 32]);
        }

        // Gemini folds and evals.
        push_commitment(&mut proof, &TEST_G1_POINT_BE); // one fold commitment
        push_scalar(&mut proof, [0u8; 32]);
        push_scalar(&mut proof, [0u8; 32]);

        // Shplonk Q and KZG W.
        push_commitment(&mut proof, &TEST_G1_POINT_BE);
        push_commitment(&mut proof, &TEST_G1_POINT_BE);
        proof
    }

    #[test]
    fn bn254_runtime_preflight_succeeds() {
        runtime_preflight_check().expect("bn254 runtime preflight should succeed");
    }

    #[test]
    fn parse_ultrahonk_sections_accepts_sample_non_zk_keccak_layout() {
        let proof = sample_valid_non_zk_keccak_proof_bytes();
        let parsed = parse_ultrahonk_proof_sections(&proof).expect("proof should parse");
        assert!(!parsed.profile.has_zk);
        assert_eq!(parsed.profile.commitment_encoding, CommitmentEncoding::U256);
        assert_eq!(parsed.profile.virtual_log_n, 2);
        assert_eq!(parsed.sumcheck_univariates.len(), 16);
        assert_eq!(parsed.sumcheck_evaluations_count, NON_ZK_NUM_ALL_ENTITIES);
    }

    #[test]
    fn fr_mul_mod_matches_known_vectors() {
        let vectors = [
            (
                "0x08e9dc468133287637ebdcd9e87a1613e443df789558867f5ba91faf7a024204",
                "0x0967d0cae6f4590b9a164106cf6a659eb4862b21fb97d43588561712e8e5216a",
                "0x06a6c4347d79851b3736792906b97ff21d31be2384d246b9b981ffb4713ff30c",
            ),
            (
                "0x1521f387af19922ad9b8a714e61a441c12e0c8b2bad640fb19488dec4f65d4d9",
                "0x28fcb26f9c6316b950f244556f25e2a25a92118719c78df48f4ff31e78de5857",
                "0x2e769c5be13fe335da129ce4f86c7d901f9bcaa53d397e4a083062dfd15ffd9a",
            ),
            (
                "0x28045af9ab0c1681c8f8e3d0d3290a4cb5d32b1666194cb1d71037d1b83e90ec",
                "0x2ebce25e3e70f16a55485822de1b372ad3fbf47a7e5b1e7f9ca5499d004ae545",
                "0x2a72a7867b5eb248015a24e2d782e2f50df86f911ac3f23c3f5313da97182e6f",
            ),
        ];

        for (left, right, expected) in vectors {
            let lhs = hex32(left);
            let rhs = hex32(right);
            let expected_out = hex32(expected);
            let product = fr_mul_mod(&lhs, &rhs).expect("Fr multiplication should succeed");
            assert_eq!(product, expected_out);
        }
    }

    #[test]
    fn fr_mul_mod_obeys_identity_and_commutativity() {
        let value = hex32("0x215ddba6dd84f39e71545a137a1d50068d723104f77383c13458a748e9bb17bc");
        let one = hex32("0x0000000000000000000000000000000000000000000000000000000000000001");
        let zero = hex32("0x0000000000000000000000000000000000000000000000000000000000000000");
        let other = hex32("0x05d67b7072ae22448b0163c1cd9d2b7d247a8333f7b0b7d2cda8056c3d15eef7");

        assert_eq!(
            fr_mul_mod(&value, &one).expect("Fr multiplication should succeed"),
            value,
        );
        assert_eq!(
            fr_mul_mod(&value, &zero).expect("Fr multiplication should succeed"),
            zero,
        );
        assert_eq!(
            fr_mul_mod(&value, &other).expect("Fr multiplication should succeed"),
            fr_mul_mod(&other, &value).expect("Fr multiplication should succeed"),
        );
    }

    #[test]
    fn ultrahonk_arithmetic_checks_accept_consistent_decoded_proof() {
        let proof = sample_valid_non_zk_keccak_proof_bytes();
        let vk_hash = [7u8; 32];
        let sections = parse_ultrahonk_proof_sections(&proof).expect("proof should parse");
        let public_inputs_count = public_inputs_count_if_canonical(&[])
            .expect("empty public inputs are canonical");
        let challenge_schedule =
            derive_sumcheck_round_challenges_keccak(&sections, &vk_hash, &[], public_inputs_count);
        assert!(challenge_schedule.is_some());
        assert_eq!(
            challenge_schedule
                .expect("sumcheck challenge schedule")
                .sumcheck_round_challenges
                .len(),
            sections.profile.virtual_log_n
        );
        let sumcheck_schedule = verify_sumcheck_transcript_round_consistency(
            &sections,
            &vk_hash,
            &[],
            public_inputs_count,
        )
        .expect("sumcheck transcript checks should pass");
        assert!(verify_opening_transcript_challenge_chain(
            &sections,
            &sumcheck_schedule
        ));
        let commitment_linearity =
            verify_commitment_linearity(&sections.w_l_commitment, &sections.w_r_commitment)
                .expect("linearity check should execute");
        assert!(commitment_linearity);
        let pairing_identity = verify_kzg_pairing_identity(&sections.kzg_w_commitment)
            .expect("pairing check should execute");
        assert!(pairing_identity);
        let verified = evaluate_ultrahonk_arithmetic_checks(&proof, &vk_hash, &[])
            .expect("evaluation should not error");
        assert!(verified);
    }

    #[test]
    fn ultrahonk_arithmetic_checks_reject_truncated_proof() {
        let mut proof = sample_valid_non_zk_keccak_proof_bytes();
        proof.truncate(proof.len().saturating_sub(32));
        let vk_hash = [7u8; 32];
        let verified = evaluate_ultrahonk_arithmetic_checks(&proof, &vk_hash, &[])
            .expect("evaluation should not error");
        assert!(!verified);
    }

    #[test]
    fn ultrahonk_arithmetic_checks_reject_non_canonical_sumcheck_scalar() {
        let mut proof = sample_valid_non_zk_keccak_proof_bytes();
        let sumcheck_start = (SPECIAL_PUBLIC_INPUTS_SIZE * 32) + (8 * ALT_BN128_G1_POINT_SIZE);
        proof[sumcheck_start..sumcheck_start + 32].copy_from_slice(&BN254_SCALAR_MODULUS_BE);
        let vk_hash = [7u8; 32];
        let verified = evaluate_ultrahonk_arithmetic_checks(&proof, &vk_hash, &[])
            .expect("evaluation should not error");
        assert!(!verified);
    }

    #[test]
    fn ultrahonk_arithmetic_checks_reject_invalid_round0_sumcheck_relation() {
        let mut proof = sample_valid_non_zk_keccak_proof_bytes();
        let sumcheck_start = (SPECIAL_PUBLIC_INPUTS_SIZE * 32) + (8 * ALT_BN128_G1_POINT_SIZE);
        proof[sumcheck_start + 31] = 1;
        let vk_hash = [7u8; 32];
        let verified = evaluate_ultrahonk_arithmetic_checks(&proof, &vk_hash, &[])
            .expect("evaluation should not error");
        assert!(!verified);
    }

    #[test]
    fn ultrahonk_arithmetic_checks_reject_invalid_round_transition_relation() {
        let mut proof = sample_valid_non_zk_keccak_proof_bytes();
        let sumcheck_start = (SPECIAL_PUBLIC_INPUTS_SIZE * 32) + (8 * ALT_BN128_G1_POINT_SIZE);
        let round_1_first_evaluation =
            sumcheck_start + (NON_ZK_BATCHED_RELATION_PARTIAL_LENGTH * 32);
        proof[round_1_first_evaluation + 31] = 1;
        let vk_hash = [7u8; 32];
        let verified = evaluate_ultrahonk_arithmetic_checks(&proof, &vk_hash, &[])
            .expect("evaluation should not error");
        assert!(!verified);
    }

    #[test]
    fn ultrahonk_arithmetic_checks_reject_invalid_pairing_special_limb_width() {
        let mut proof = sample_valid_non_zk_keccak_proof_bytes();
        // Pairing coordinate high limbs must fit in 52 bits.
        // Set x high limb (scalar[3]) to 2^52, which violates this bound while remaining canonical.
        proof[(3 * 32) + 25] = 0x10;
        let vk_hash = [7u8; 32];
        let verified = evaluate_ultrahonk_arithmetic_checks(&proof, &vk_hash, &[])
            .expect("evaluation should not error");
        assert!(!verified);
    }

    #[test]
    fn ultrahonk_arithmetic_checks_reject_invalid_shplonk_q_commitment() {
        let mut proof = sample_valid_non_zk_keccak_proof_bytes();
        let shplonk_q_start = proof
            .len()
            .checked_sub(2 * ALT_BN128_G1_POINT_SIZE)
            .expect("proof should contain final Shplonk/KZG commitments");
        let shplonk_q_end = shplonk_q_start + ALT_BN128_G1_POINT_SIZE;
        proof[shplonk_q_start..shplonk_q_end].fill(0);
        // Set affine point to (x=1, y=1): canonical field elements that are not on BN254 G1.
        proof[shplonk_q_start + 31] = 1;
        proof[shplonk_q_start + 63] = 1;
        let vk_hash = [7u8; 32];
        let verified = evaluate_ultrahonk_arithmetic_checks(&proof, &vk_hash, &[])
            .expect("evaluation should not error");
        assert!(!verified);
    }

    #[test]
    fn ultrahonk_arithmetic_checks_reject_invalid_gemini_fold_commitment() {
        let mut proof = sample_valid_non_zk_keccak_proof_bytes();
        // Sample non-zk proof has virtual_log_n=2 => one Gemini fold commitment.
        let gemini_fold_start = proof
            .len()
            .checked_sub(4 * ALT_BN128_G1_POINT_SIZE)
            .expect("proof should contain Gemini fold + Shplonk/KZG tail");
        let gemini_fold_end = gemini_fold_start + ALT_BN128_G1_POINT_SIZE;
        proof[gemini_fold_start..gemini_fold_end].fill(0);
        // Set x to base-field modulus (non-canonical Fq limb), which must fail decode checks.
        proof[gemini_fold_start..gemini_fold_start + 32].copy_from_slice(&BN254_BASE_MODULUS_BE);
        proof[gemini_fold_start + 63] = 1;
        let vk_hash = [7u8; 32];
        let verified = evaluate_ultrahonk_arithmetic_checks(&proof, &vk_hash, &[])
            .expect("evaluation should not error");
        assert!(!verified);
    }

    #[test]
    fn ultrahonk_arithmetic_checks_reject_invalid_w_l_commitment() {
        let mut proof = sample_valid_non_zk_keccak_proof_bytes();
        let w_l_start = SPECIAL_PUBLIC_INPUTS_SIZE * 32;
        let w_l_end = w_l_start + ALT_BN128_G1_POINT_SIZE;
        proof[w_l_start..w_l_end].fill(0);
        // Set affine point to (x=1, y=1): canonical limbs but off-curve for BN254 G1.
        proof[w_l_start + 31] = 1;
        proof[w_l_start + 63] = 1;
        let vk_hash = [7u8; 32];
        let verified = evaluate_ultrahonk_arithmetic_checks(&proof, &vk_hash, &[])
            .expect("evaluation should not error");
        assert!(!verified);
    }

    #[test]
    fn ultrahonk_arithmetic_checks_reject_invalid_kzg_w_commitment() {
        let mut proof = sample_valid_non_zk_keccak_proof_bytes();
        let kzg_w_start = proof
            .len()
            .checked_sub(ALT_BN128_G1_POINT_SIZE)
            .expect("proof should contain final KZG witness commitment");
        let kzg_w_end = kzg_w_start + ALT_BN128_G1_POINT_SIZE;
        proof[kzg_w_start..kzg_w_end].fill(0);
        // Set affine point to (x=1, y=1): canonical limbs but off-curve for BN254 G1.
        proof[kzg_w_start + 31] = 1;
        proof[kzg_w_start + 63] = 1;
        let vk_hash = [7u8; 32];
        let verified = evaluate_ultrahonk_arithmetic_checks(&proof, &vk_hash, &[])
            .expect("evaluation should not error");
        assert!(!verified);
    }

    #[test]
    fn test_g1_negation_scalar_matches_expected_point() {
        let neg = g1_mul_point(&TEST_G1_POINT_BE, &SCALAR_NEG_ONE_BE)
            .expect("negation multiplication should succeed");
        assert_eq!(neg, TEST_G1_NEG_POINT_BE);
        let sum = g1_add_points(&TEST_G1_POINT_BE, &neg).expect("addition should succeed");
        assert_eq!(sum, [0u8; ALT_BN128_G1_POINT_SIZE]);
    }

    #[test]
    fn pairing_identity_holds_for_test_point_and_its_negation() {
        let mut pairing_input = [0u8; ALT_BN128_PAIRING_ELEMENT_SIZE * 2];
        pairing_input[..ALT_BN128_G1_POINT_SIZE].copy_from_slice(&TEST_G1_POINT_BE);
        pairing_input[ALT_BN128_G1_POINT_SIZE..ALT_BN128_PAIRING_ELEMENT_SIZE]
            .copy_from_slice(&PAIRING_REFERENCE_G2_POINT_BE);
        pairing_input[ALT_BN128_PAIRING_ELEMENT_SIZE
            ..ALT_BN128_PAIRING_ELEMENT_SIZE + ALT_BN128_G1_POINT_SIZE]
            .copy_from_slice(&TEST_G1_NEG_POINT_BE);
        pairing_input[ALT_BN128_PAIRING_ELEMENT_SIZE + ALT_BN128_G1_POINT_SIZE..]
            .copy_from_slice(&PAIRING_REFERENCE_G2_POINT_BE);

        let pairing_output =
            alt_bn128_pairing_be(&pairing_input).expect("pairing should succeed for valid points");
        assert!(is_be_one(&pairing_output));
    }
}
