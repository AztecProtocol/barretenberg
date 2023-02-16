use crate::*;
use std::convert::TryInto;
// TODO: Behaviour has changed since the old barretenberg
// TODO: so test vectors wont work.
//
// TODO: We should check what we need
pub fn compress_native(left: &[u8; 32], right: &[u8; 32]) -> [u8; 32] {
    // unsafe { pedersen__init() };
    let mut result = [0_u8; 32];

    unsafe {
        pedersen__compress_fields(
            left.as_ptr() as *const u8,
            right.as_ptr() as *const u8,
            result.as_mut_ptr(),
        );
    }
    result
}

pub fn compress_many(inputs: &[[u8; 32]]) -> [u8; 32] {
    // unsafe { pedersen__init() };
    //convert inputs into one buffer: length + data
    let mut buffer = Vec::new();
    let witness_len = inputs.len() as u32;
    buffer.extend_from_slice(&witness_len.to_be_bytes());
    for assignment in inputs {
        buffer.extend_from_slice(assignment);
    }

    let mut result = [0_u8; 32];
    unsafe {
        pedersen__compress(buffer.as_ptr() as *const u8, result.as_mut_ptr());
    }
    result
}

pub fn encrypt(inputs_buffer: &[[u8; 32]]) -> ([u8; 32], [u8; 32]) {
    let mut buffer = Vec::new();
    let buffer_len = inputs_buffer.len() as u32;
    let mut result = [0_u8; 64];
    buffer.extend_from_slice(&buffer_len.to_be_bytes());
    for e in inputs_buffer {
        buffer.extend_from_slice(e);
    }

    unsafe {
        pedersen__commit(buffer.as_ptr() as *const u8, result.as_mut_ptr());
    }
    let s: [u8; 32] = (result[0..32]).try_into().unwrap();
    let e: [u8; 32] = (result[32..64]).try_into().unwrap();
    (s, e)
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn basic_interop() {
        // Expected values were taken from Barretenberg by running `crypto::pedersen::compress_native`
        // printing the result in hex to `std::cout` and copying
        struct Test<'a> {
            input_left: [u8; 32],
            input_right: [u8; 32],
            expected_hex: &'a str,
        }
        let f_zero = [0_u8; 32];
        let mut f_one = [0_u8; 32];
        f_one[31] = 1;

        let tests = vec![
            Test {
                input_left: f_zero,
                input_right: f_one,
                expected_hex: "229fb88be21cec523e9223a21324f2e305aea8bff9cdbcb3d0c6bba384666ea1",
            },
            Test {
                input_left: f_one,
                input_right: f_one,
                expected_hex: "26425ddf29b4af6ee91008e8dbcbee975653170eee849efd75abf8301dee114e",
            },
            Test {
                input_left: f_one,
                input_right: f_zero,
                expected_hex: "08f3cb4f0fdd7a9ef130c6d4590af6750b1475161020a198a56eced45078ccf2",
            },
        ];

        for test in tests {
            let got = compress_native(&test.input_left, &test.input_right);
            let mut many_intputs: Vec<[u8; 32]> = Vec::new();
            many_intputs.push(test.input_left);
            many_intputs.push(test.input_right);
            let got_many = compress_many(&many_intputs);
            assert_eq!(hex::encode(got), test.expected_hex);
            assert_eq!(got, got_many);
        }
    }

    #[test]
    fn pedersen_hash_to_point() {
        let f_zero = [0_u8; 32];
        let mut f_one = [0_u8; 32];
        f_one[31] = 1;
        let mut inputs: Vec<[u8; 32]> = Vec::new();
        inputs.push(f_zero);
        inputs.push(f_one);

        let (x, y) = encrypt(&inputs);
        let expected_x = "229fb88be21cec523e9223a21324f2e305aea8bff9cdbcb3d0c6bba384666ea1";
        let expected_y = "296b4b4605e586a91caa3202baad557628a8c56d0a1d6dff1a7ca35aed3029d5";
        assert_eq!(expected_x, hex::encode(x));
        assert_eq!(expected_y, hex::encode(y));
    }
}
