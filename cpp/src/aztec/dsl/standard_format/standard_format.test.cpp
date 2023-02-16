// #include <plonk/composer/turbo_composer.hpp>
// #include <plonk/composer/composer_base.hpp>
// #include "standard_format.hpp"
// #include <gtest/gtest.h>
// #include <crypto/sha256/sha256.hpp>
// #include <crypto/blake2s/blake2s.hpp>
// #include <crypto/pedersen/pedersen.hpp>
// #include <stdlib/hash/sha256/sha256.hpp>
// #include <stdlib/hash/blake2s/blake2s.hpp>
// #include <stdlib/merkle_tree/memory_tree.hpp>
// //
// #include <plonk/proof_system/types/polynomial_manifest.hpp>
// #include <plonk/proof_system/commitment_scheme/kate_commitment_scheme.hpp>

// using namespace barretenberg;
// using namespace waffle;

// namespace {
// auto& engine = numeric::random::get_debug_engine();
// }

// // Input :
// 00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000005
// // Expected output : c6481e22c5ff4164af680b8cfaa5e8ed3120eeff89c4f307c4a6faaae059ce10
// TEST(standard_format, sha256_interop)
// {
//     // Create a Turbo compose
//     TurboComposer composer = TurboComposer();

//     // Input to sha256 functions
//     std::vector<uint8_t>
//     in{00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,05
//     };

//     // Compute output for stdlib::sha256 function
//     byte_array_ct input(&composer, in);

//     byte_array_ct output_bits = plonk::stdlib::sha256<Composer>(input);
//             std::cout << "input" << std::endl;
//             std::cout << output_bits << std::endl;
//             std::cout << "input" << std::endl;
//     std::vector<field_ct> output = packed_byte_array_ct(output_bits).to_unverified_byte_slices(1);

//     // Compute output for crypto::sha256
//     std::vector<uint8_t> result = sha256::sha256(in);

//     // // Compare outputs of both functions
//     // for (size_t i = 0; i < 32; ++i) {
//     //     EXPECT_EQ(result[i], output[i].get_value());
//     // }
// }

// TEST(standard_format, dbg_sha_test)
// {
//     standard_format constraint_system{ 4, 0, {}, {}, {}, {} };
//     auto composer = create_circuit_with_witness(constraint_system, { fr(0), fr(5), 0, 0 });

//     create_sha256_constraints(composer, { { 1, 128 }, { 1, 128 }, { 1, 128 }, { 2, 128 } }, 3, 4);

//     std::vector<uint8_t> input{ 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
//     00,
//                                 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
//                                 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 05 };

//     std::vector<uint8_t> result = sha256::sha256(input);

//     std::cout << result << std::endl;

//     auto prover = composer.create_prover();
//     plonk_proof proof = prover.construct_proof();
//     auto verifier = composer.create_verifier();

//     EXPECT_TRUE(verifier.verify_proof(proof));
// }

// TEST(standard_format, sha256_constraint)
// {
//     // The input to SHA256 is 0xBD this was copied from the stdlib::sha256 test

//     // We have the low128 bit of the sha256 result at witness index 1 (public input)
//     // We have the high128 bit of the sha256 result at witness index 2 (public input)
//     // We have the input at witness index 3. Note, that we have an extra parameter which we use to indicate that we
//     // only
//     // want 8 bits from the witness We store the result of the sha256 has in witness indices 4 and 5 We then compare
//     // with the public input values

//     // Construct Sha256 constraint
//     //
//     // First construct the inputs to the sha256 function
//     std::vector<Sha256Input> inputs;

//     // Take the first byte of the witness at index 1
//     inputs.push_back({ .witness = 1, .num_bits = 8 });

//     // constrain the public input to be equal to the result
//     // of the hash function
//     std::vector<poly_triple> eq_constraints;
//     for (uint32_t i = 2; i < 34; i++) {

//         eq_constraints.emplace_back(poly_triple{ i, i + 32, 0, 0, 1, -1, 0, 0 });
//     }

//     standard_format constraint_system{
//         .varnum = 70,
//         .public_inputs = { 2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
//                            18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33 },
//         .fixed_base_scalar_mul_constraints = {},
//         .logic_constraints = {},
//         .range_constraints = {},
//         .schnorr_constraints = {},
//         .ecdsa_constraints = {},
//         .sha256_constraints = { {
//             .inputs = inputs,
//             .result = { 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
//                         50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65 },
//         } },
//         .blake2s_constraints = {},
//         .hash_to_field_constraints = {},
//         .pedersen_constraints = {},
//         .merkle_membership_constraints = {},
//         .constraints = eq_constraints,
//         .merkle_insert_constraints = {},
//     };

//     std::vector<fr> witness_values;

//     witness_values.emplace_back(fr(0xBDULL)); // Add input

//     std::vector<uint8_t> public_input = { 0x68, 0x32, 0x57, 0x20, 0xaa, 0xbd, 0x7c, 0x82, 0xf3, 0x0f, 0x55,
//                                           0x4b, 0x31, 0x3d, 0x05, 0x70, 0xc9, 0x5a, 0xcc, 0xbb, 0x7d, 0xc4,
//                                           0xb5, 0xaa, 0xe1, 0x12, 0x04, 0xc0, 0x8f, 0xfe, 0x73, 0x2b };

//     std::vector<uint8_t> result = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//     std::copy(public_input.begin(), public_input.end(), std::back_inserter(witness_values));
//     std::copy(result.begin(), result.end(), std::back_inserter(witness_values));

//     auto composer = create_circuit_with_witness(constraint_system, witness_values);

//     auto prover = composer.create_prover();

//     plonk_proof proof = prover.construct_proof();

//     auto verifier = composer.create_verifier();
//     EXPECT_TRUE(verifier.verify_proof(proof));
// }

// TEST(standard_format, pedersen_constraint)
// {

//     // Construct Pedersen constraint
//     //
//     std::vector<PedersenConstraint> inputs;

//     // Take the first byte of the witness at index 3
//     inputs.push_back({ .scalars = { 1, 2 }, .result_x = 3, .result_y = 4 });

//     fr x = fr::zero();
//     fr y = fr::one();
//     auto res = crypto::pedersen::commit_native({ x, y });

//     standard_format constraint_system{
//         .varnum = 5,
//         .public_inputs = { 1, 2 },
//         .fixed_base_scalar_mul_constraints = {},
//         .logic_constraints = {},
//         .range_constraints = {},
//         .schnorr_constraints = {},
//         .ecdsa_constraints = {},
//         .sha256_constraints = {},
//         .blake2s_constraints = {},
//         .hash_to_field_constraints = {},
//         .pedersen_constraints = inputs,
//         .merkle_membership_constraints = {},
//         .constraints = { poly_triple{
//             .a = 3,
//             .b = 3,
//             .c = 3,
//             .q_m = 0,
//             .q_l = 0,
//             .q_r = 0,
//             .q_o = 1,
//             .q_c = -res.x,
//         } },
//         .merkle_insert_constraints = {},
//     };

//     // auto res = fr{ 0x108800e84e0f1daf, 0xb9fdf2e4b5b311fd, 0x59b8b08eaf899634, 0xc59cc985b490234b };

//     auto composer = create_circuit_with_witness(constraint_system, { x, y, 0, 0 });

//     auto prover = composer.create_prover();

//     plonk_proof proof = prover.construct_proof();

//     auto verifier = composer.create_verifier();
//     EXPECT_TRUE(verifier.verify_proof(proof));
// }

// TEST(standard_format, composer_with_public_inputs)
// {

//     standard_format constraint_system{
//         .varnum = 4,
//         .public_inputs = { 1, 2 },
//         .fixed_base_scalar_mul_constraints = {},
//         .logic_constraints = {},
//         .range_constraints = {},
//         .schnorr_constraints = {},
//         .ecdsa_constraints = {},
//         .sha256_constraints = {},
//         .blake2s_constraints = {},
//         .hash_to_field_constraints = {},
//         .pedersen_constraints = {},
//         .merkle_membership_constraints = {},
//         .constraints = { poly_triple{
//             .a = 1,
//             .b = 2,
//             .c = 3,
//             .q_m = 0,
//             .q_l = 1,
//             .q_r = 1,
//             .q_o = fr::neg_one(),
//             .q_c = 0,
//         } },
//         .merkle_insert_constraints = {},
//     };

//     auto composer = create_circuit_with_witness(constraint_system, { 1, 2, 3 });

//     auto prover = composer.create_prover();

//     plonk_proof proof = prover.construct_proof();

//     auto verifier = composer.create_verifier();
//     std::cout << proof.proof_data << std::endl;
//     EXPECT_TRUE(verifier.verify_proof(proof));
// }

// TEST(standard_format, debug_hash_to_field)
// {

//     auto composer = TurboComposer();

//     std::string input = "abc";
//     std::vector<uint8_t> input_v(input.begin(), input.end());
//     std::vector<uint8_t> output;
//     std::cout << input_v << std::endl;
//     output = blake2::blake2s(input_v);

//     std::cout << fr::serialize_from_buffer(output.data()) << std::endl;
// }

// TEST(standard_format, merkle_membership_constraint)
// {
//     // This test will check whether a leaf is in a merkle set
//     //
//     // This is done with three objects:
//     //  - The merkle root
//     //  - A path from the current item to the merkle root
//     //  - Index of the leaf we want to prove membership fo
//     //
//     // What is Public and What is Private?
//     //
//     // The index and path must be witnesses.
//     // The root can be Public, however in the code it is a Witness also. Ariel/Zac: Why?
//     // What if Verifier wants to ensure that the root is correct? Extra constraint?
//     //
//     //
//     // The tree will have 8 leaves.
//     // The depth is therefore log_2(8) = 3
//     size_t depth = 3;

//     //////////// Start Noi
//     // First compute the root using MemoryTree.
//     // Note: All leaves in MemoryTree are the same; the blake2s hash of 64 bytes of zeroes
//     //
//     MemoryTree mem_tree(depth);
//     auto root = mem_tree.root();
//     //
//     // Now declare the position of the leaf we are trying to prove membership fo
//     // For this test, it doesn't matter as all leaves are the same
//     //
//     // Why do we need usize and fr?
//     // usize is needed when we want to ask the MemoryTree to "give the hashpath at index 0"
//     // fr is needed for the actual gadget proof
//     size_t index_as_usize = 0;
//     fr index_as_fr = fr::zero();
//     //
//     // Compute the leaf value
//     std::vector<uint8_t> zero_element(64, 0);
//     auto leaf = hash_value_native(zero_element);
//     //
//     // Compute the hash path
//     // These are the field element pairs which make up the hashpath
//     // Noir assumes that `get_hash_path` comes from somewhere. It is out of scope for Noir as to where.
//     //
//     // Note that we have field elements here, in Noir these will need to be witnesses
//     fr_hash_path hash_path_field_pairs = mem_tree.get_hash_path(index_as_usize);
//     // Lets make this into a Vec<fr>, as this is what the standard format uses
//     // XXX: Maybe we can use pairs, I just have not tried serialising it to Rust, so not sure how.
//     std::vector<fr> hash_path;
//     for (const auto& pair : hash_path_field_pairs) {
//         hash_path.emplace_back(pair.first);
//         hash_path.emplace_back(pair.second);
//     }
//     //////////// End Noi

//     // Construct merkle_root_constraint
//     auto merkle_membership_constraint = MerkleMembershipConstraint{
//         .hash_path = { 1, 2, 3, 4, 5, 6 }, // Our depth is 3, so each level will contain 3 pairs. We have a total of
//                                            // 6 nodes.
//         .root = 7,
//         .leaf = 8,
//         .result = 9,
//         .index = 10,
//     };

//     standard_format constraint_system{
//         .varnum = 11,
//         .public_inputs = {},
//         .fixed_base_scalar_mul_constraints = {},
//         .logic_constraints = {},
//         .range_constraints = {},
//         .schnorr_constraints = {},
//         .ecdsa_constraints = {},
//         .sha256_constraints = {},
//         .blake2s_constraints = {},
//         .hash_to_field_constraints = {},
//         .pedersen_constraints = {},
//         .merkle_membership_constraints = { merkle_membership_constraint },
//         .constraints = {},
//         .merkle_insert_constraints = {},
//     };

//     auto composer = create_circuit_with_witness(constraint_system,
//                                                 { // First 6 values are the hash path
//                                                   hash_path[0],
//                                                   hash_path[1],
//                                                   hash_path[2],
//                                                   hash_path[3],
//                                                   hash_path[4],
//                                                   hash_path[5],
//                                                   // Next value is the root of the merkle tree
//                                                   root,
//                                                   // Next value is the leaf,
//                                                   leaf,
//                                                   // The result of the merkle_tree check will be a boolean.
//                                                   // set the value to zero and the merkle constrain will set it to
//                                                   //  the
//                                                   // output of the gadget
//                                                   fr::zero(),
//                                                   // Index that this value is located amongst the leaves
//                                                   index_as_fr });

//     auto prover = composer.create_prover();

//     plonk_proof proof = prover.construct_proof();

//     auto verifier = composer.create_verifier();
//     EXPECT_TRUE(verifier.verify_proof(proof));
// }

// TEST(standard_format, dbg_value)
// {
//     auto pub = grumpkin::g1::one * grumpkin::fr::one();
//     EXPECT_EQ(pub.x, barretenberg::fr::one());
// }

// TEST(standard_format, schnorr_verify_constraint)
// {

//     //////////// Start Noi

//     // // erm...what do we do now?
//     std::string message_string = "hello";

//     crypto::schnorr::key_pair<grumpkin::fr, grumpkin::g1> account;
//     account.private_key = grumpkin::fr::random_element();
//     account.public_key = grumpkin::g1::one * account.private_key;

//     crypto::schnorr::signature signature =
//         crypto::schnorr::construct_signature<Blake2sHasher, grumpkin::fq, grumpkin::fr, grumpkin::g1>(message_string,
//                                                                                                       account);
//     bool first_result = crypto::schnorr::verify_signature<Blake2sHasher, grumpkin::fq, grumpkin::fr, grumpkin::g1>(
//         message_string, account.public_key, signature);

//     EXPECT_EQ(first_result, true);

//     // Lets prepare the input, as if Noir was sending it over WASM
//     //
//     // Public key preparation
//     auto public_key_x = account.public_key.x;
//     auto public_key_y = account.public_key.y;

//     // We do not use the private key in the circuit!
//     // It's only signature verification

//     // Signature preparation
//     std::vector<fr> signature_bytes_as_fr;

//     for (const auto& sig_byte : signature.s) {
//         signature_bytes_as_fr.emplace_back(fr(sig_byte));
//     }

//     for (const auto& sig_byte : signature.e) {
//         signature_bytes_as_fr.emplace_back(fr(sig_byte));
//     }

//     // Prepare the message
//     //
//     std::vector<uint8_t> message_bytes(message_string.begin(), message_string.end());
//     std::vector<fr> message_bytes_as_fr;

//     for (const auto& msg_byte : message_bytes) {
//         message_bytes_as_fr.emplace_back(fr(msg_byte));
//     }
//     //////////// End Noi

//     std::vector<uint32_t> signature_indices;
//     for (uint32_t i = 9; i < 9 + 64; i++) {
//         signature_indices.emplace_back(i);
//     }

//     auto schnorr_constraint = SchnorrConstraint{
//         .message = { 1, 2, 3, 4, 5 }, // Message is "hello" which has 5 bytes
//         .public_key_x = 6,
//         .public_key_y = 7,
//         .result = 8,
//         .signature = signature_indices,
//     };
//     (void)schnorr_constraint;

//     standard_format constraint_system{
//         .varnum = 75,
//         .public_inputs = {},
//         .fixed_base_scalar_mul_constraints = {},
//         .logic_constraints = {},
//         .range_constraints = {},
//         .schnorr_constraints = { schnorr_constraint },
//         .ecdsa_constraints = {},
//         .sha256_constraints = {},
//         .blake2s_constraints = {},
//         .hash_to_field_constraints = {},
//         .pedersen_constraints = {},
//         .merkle_membership_constraints = {},
//         .constraints = { poly_triple{
//             // Result should be 1
//             .a = 8,
//             .b = 8,
//             .c = 8,
//             .q_m = 0,
//             .q_l = 0,
//             .q_r = 0,
//             .q_o = 1,
//             .q_c = -1,
//         } },
//         .merkle_insert_constraints = {},
//     };

//     std::vector<fr> witness_values;
//     std::copy(message_bytes_as_fr.begin(), message_bytes_as_fr.end(), std::back_inserter(witness_values));
//     witness_values.emplace_back(public_key_x);
//     witness_values.emplace_back(public_key_y);
//     witness_values.emplace_back(fr::zero());

//     std::copy(signature_bytes_as_fr.begin(), signature_bytes_as_fr.end(), std::back_inserter(witness_values));

//     auto composer = create_circuit_with_witness(constraint_system, witness_values);
//     std::cout << "the number of schnorr gates: " << composer.get_num_gates() << std::endl;

//     auto prover = composer.create_prover();

//     plonk_proof proof = prover.construct_proof();

//     auto verifier = composer.create_verifier();
//     EXPECT_TRUE(verifier.verify_proof(proof));
// }
// TEST(standard_format, fixed_base_scalar_mul)
// {
//     crypto::schnorr::key_pair<grumpkin::fr, grumpkin::g1> account;
//     account.private_key = grumpkin::fr::random_element();
//     account.public_key = grumpkin::g1::one * account.private_key;

//     auto scalar_mul_constraint = FixedBaseScalarMul{
//         .scalar = 1,
//         .pub_key_x = 2,
//         .pub_key_y = 3,
//     };

//     standard_format constraint_system{
//         .varnum = 6,
//         .public_inputs = { 4, 5 },
//         .fixed_base_scalar_mul_constraints = { scalar_mul_constraint },
//         .logic_constraints = {},
//         .range_constraints = {},
//         .schnorr_constraints = {},
//         .ecdsa_constraints = {},
//         .sha256_constraints = {},
//         .blake2s_constraints = {},
//         .hash_to_field_constraints = {},
//         .pedersen_constraints = {},
//         .merkle_membership_constraints = {},
//         .constraints = { poly_triple{
//                              // pub_key_x_pub == computed_x_pub
//                              .a = 2,
//                              .b = 4,
//                              .c = 4,
//                              .q_m = 0,
//                              .q_l = 1,
//                              .q_r = -1,
//                              .q_o = 0,
//                              .q_c = 0,
//                          },
//                          poly_triple{
//                              // pub_key_y_pub == computed_y_pub
//                              .a = 3,
//                              .b = 5,
//                              .c = 5,
//                              .q_m = 0,
//                              .q_l = 1,
//                              .q_r = -1,
//                              .q_o = 0,
//                              .q_c = 0,
//                          }

//         },
//         .merkle_insert_constraints = {},
//     };

//     std::vector<fr> witness_values;
//     witness_values.emplace_back(account.private_key);
//     witness_values.emplace_back(0);
//     witness_values.emplace_back(0);
//     witness_values.emplace_back(account.public_key.x);
//     witness_values.emplace_back(account.public_key.y);

//     auto composer = create_circuit_with_witness(constraint_system, witness_values);

//     auto prover = composer.create_prover();

//     plonk_proof proof = prover.construct_proof();

//     auto verifier = composer.create_verifier();
//     EXPECT_TRUE(verifier.verify_proof(proof));
// }

// TEST(standard_format, ecdsa_verify_constraint)
// {

//     //////////// Start Noi

//     // // erm...what do we do now?
//     std::string message_string = "hello";

//     crypto::ecdsa::key_pair<secp256k1::fr, secp256k1::g1> account;
//     account.private_key = secp256k1::fr::random_element();
//     account.public_key = secp256k1::g1::one * account.private_key;

//     crypto::ecdsa::signature signature =
//         crypto::ecdsa::construct_signature<Blake2sHasher, secp256k1::fq, secp256k1::fr,
//         secp256k1::g1>(message_string,
//                                                                                                        account);

//     bool first_result = crypto::ecdsa::verify_signature<Blake2sHasher, secp256k1::fq, secp256k1::fr, secp256k1::g1>(
//         message_string, account.public_key, signature);
//     EXPECT_TRUE(first_result);

//     /////////

//     // Lets prepare the input, as if Noir was sending it over WASM
//     //
//     // Public key preparation

//     auto pub_key_bytes = to_buffer(account.public_key);

//     std::vector<fr> pub_key_x_bytes(pub_key_bytes.begin(),
//                                     pub_key_bytes.begin() + (long int)(pub_key_bytes.size() / 2));
//     std::vector<fr> pub_key_y_bytes(pub_key_bytes.begin() + (long int)(pub_key_bytes.size() / 2),
//     pub_key_bytes.end());

//     // We do not use the private key in the circuit!
//     // It's only signature verification

//     // Signature preparation
//     std::vector<fr> signature_bytes_as_fr;

//     for (const auto& sig_byte : signature.r) {
//         signature_bytes_as_fr.emplace_back(fr(sig_byte));
//     }

//     for (const auto& sig_byte : signature.s) {
//         signature_bytes_as_fr.emplace_back(fr(sig_byte));
//     }

//     // Prepare the message
//     //
//     std::vector<fr> hashed_message_bytes_as_fr;

//     std::vector<uint8_t> input(message_string.begin(), message_string.end());
//     auto hashed_message = blake2::blake2s(input);
//     for (const auto& msg_byte : hashed_message) {
//         hashed_message_bytes_as_fr.emplace_back(fr(msg_byte));
//     }
//     //////////// End Noir

//     // The first index is reserved

//     // 32 indices are for the hashed message
//     uint32_t msg_start = 1;
//     uint32_t msg_len = 32;
//     uint32_t msg_end = msg_start + msg_len;
//     std::vector<uint32_t> hashed_message_indices;
//     for (uint32_t i = msg_start; i < msg_end; i++) {
//         hashed_message_indices.emplace_back(i);
//     }

//     // The 33rd index is for the result
//     uint32_t result_index = msg_end;

//     uint32_t sig_start = msg_end + 1;
//     uint32_t sig_len = 64;
//     uint32_t sig_end = sig_start + sig_len;
//     std::vector<uint32_t> signature_indices;
//     for (uint32_t i = sig_start; i < sig_end; i++) {
//         signature_indices.emplace_back(i);
//     }

//     uint32_t pub_key_x_start = sig_end;
//     uint32_t pub_key_len = 32;
//     uint32_t pub_key_x_end = pub_key_x_start + pub_key_len;
//     std::vector<uint32_t> pubkey_x_indices;
//     for (uint32_t i = pub_key_x_start; i < pub_key_x_start + pub_key_len; i++) {
//         pubkey_x_indices.emplace_back(i);
//     }

//     uint32_t pub_key_y_start = pub_key_x_end;
//     uint32_t pub_key_y_end = pub_key_y_start + pub_key_len;
//     std::vector<uint32_t> pubkey_y_indices;
//     for (uint32_t i = pub_key_y_start; i < pub_key_y_end; i++) {
//         pubkey_y_indices.emplace_back(i);
//     }

//     // There are two occurrences of +1:
//     // - The first is for the result_index
//     // - The second is for the reserved zero index

//     uint32_t total_number_indices = (uint32_t)hashed_message_indices.size() + 1 + (uint32_t)signature_indices.size()
//     +
//                                     (uint32_t)pubkey_x_indices.size() + (uint32_t)pubkey_y_indices.size() + 1;

//     auto ecdsa_constraint = EcdsaSecp256k1Constraint{
//         .message = hashed_message_indices,
//         .pub_x_indices = pubkey_x_indices,
//         .pub_y_indices = pubkey_y_indices,
//         .result = result_index,
//         .signature = signature_indices,
//         // .pub_key = account.public_key,
//         // .sig = signature,
//     };

//     std::cout << pubkey_x_indices.size() << std::endl;
//     std::cout << pubkey_y_indices.size() << std::endl;
//     std::cout << signature_indices.size() << std::endl;

//     standard_format constraint_system{
//         .varnum = total_number_indices,
//         .public_inputs = {},
//         .fixed_base_scalar_mul_constraints = {},
//         .logic_constraints = {},
//         .range_constraints = {},
//         .schnorr_constraints = {},
//         .ecdsa_constraints = { ecdsa_constraint },
//         .sha256_constraints = {},
//         .blake2s_constraints = {},
//         .hash_to_field_constraints = {},
//         .pedersen_constraints = {},
//         .merkle_membership_constraints = {},
//         .constraints = { poly_triple{
//             // Result should be 1
//             .a = result_index,
//             .b = result_index,
//             .c = result_index,
//             .q_m = 0,
//             .q_l = 0,
//             .q_r = 0,
//             .q_o = 1,
//             .q_c = -1,
//         } },
//         .merkle_insert_constraints = {},
//     };

//     std::vector<fr> witness_values;
//     std::copy(hashed_message_bytes_as_fr.begin(), hashed_message_bytes_as_fr.end(),
//     std::back_inserter(witness_values));
//     // witness_values.emplace_back(public_key_x);
//     // witness_values.emplace_back(public_key_y);
//     witness_values.emplace_back(fr::zero());

//     std::copy(signature_bytes_as_fr.begin(), signature_bytes_as_fr.end(), std::back_inserter(witness_values));
//     std::copy(pub_key_x_bytes.begin(), pub_key_x_bytes.end(), std::back_inserter(witness_values));
//     std::copy(pub_key_y_bytes.begin(), pub_key_y_bytes.end(), std::back_inserter(witness_values));

//     auto composer = create_circuit_with_witness(constraint_system, witness_values);
//     auto prover = composer.create_prover();
//     std::cout << "the number of gates: " << composer.get_num_gates() << std::endl;

//     plonk_proof proof = prover.construct_proof();

//     auto verifier = composer.create_verifier();
//     EXPECT_TRUE(verifier.verify_proof(proof));
// }

// // Forget tests below

// // TEST(standard_format, prove_1_constraint)
// // {
// //     standard_format constraint_system{ 3, 0,{}, {{1,32}}, {},{ { 1, 2, 3, 0, 1, 1, -1, 0 } }};
// //     auto composer = create_circuit_with_witness(constraint_system, {200,3,203});

// //     auto prover = composer.create_prover();
// //     plonk_proof proof = prover.construct_proof();
// //     auto verifier = composer.create_verifier();

// //     EXPECT_TRUE(verifier.verify_proof(proof));
// // }
// // TEST(standard_format, prove_stuff)
// // {
// //     standard_format constraint_system{ 5, 0,{{1,4,5,32, false}}, {{1,32}}, {},{ { 1, 2, 3, 0, 1, 1, -1, 0 } }};
// //     auto composer = create_circuit_with_witness(constraint_system, {200,3,203});

// //     auto prover = composer.create_prover();
// //     plonk_proof proof = prover.construct_proof();

// //     auto verifier = composer.create_verifier();

// //     EXPECT_TRUE(verifier.verify_proof(proof));
// // }

// // TEST(standard_format, prove_2_constraint)
// // {
// //     standard_format constraint_system{ 3, 0,{}, { {1,32} }, {},  { { 1, 2, 3, 0, 1, 1, -1, 0 } }};
// //     auto composer = create_circuit_with_witness(constraint_system, {5 , 0, 5});

// //     auto prover = composer.create_prover();
// //     plonk_proof proof = prover.construct_proof();

// //     auto verifier = composer.create_verifier();

// //     EXPECT_TRUE(verifier.verify_proof(proof));
// // }

// // TEST(standard_format, prove_1_constraint_wrong_witness_fails)
// // {
// //     standard_format constraint_system{ 3, 0,{},{}, {},{ { 1, 2, 3, 0, 1, 1, -1, 0 } } };
// //     auto composer = create_circuit(constraint_system);
// //     read_witness(composer, { 1, 3, 7 });

// //     auto prover = composer.create_prover();
// //     plonk_proof proof = prover.construct_proof();

// //     auto verifier = composer.create_verifier();

// //     EXPECT_FALSE(verifier.verify_proof(proof));
// // }

// // TEST(standard_format, prove_1_constraint_with_public_inputs)
// // {
// //     standard_format constraint_system{ 3, 2,{},  {}, {},{ { 1, 2, 3, 0, 1, 1, -1, 0 } } };
// //     auto composer = create_circuit(constraint_system);
// //     read_witness(composer, { 1, 3, 4 });

// //     auto prover = composer.create_prover();
// //     plonk_proof proof = prover.construct_proof();

// //     auto verification_key = composer.compute_verification_key();

// //     std::vector<fr> public_inputs = { 1, 3 };
// //     TurboVerifier verifier(verification_key, TurboComposer::create_manifest(2), &public_inputs);

// //     EXPECT_TRUE(verifier.verify_proof(proof));
// // }

// // TEST(standard_format, prove_1_constraint_with_wrong_public_inputs_fails)
// // {
// //     standard_format constraint_system{ 3, 2,{},  {}, {}, { { 1, 2, 3, 0, 1, 1, -1, 0 } } };
// //     auto composer = create_circuit(constraint_system);
// //     read_witness(composer, { 1, 3, 4 });

// //     auto prover = composer.create_prover();
// //     plonk_proof proof = prover.construct_proof();

// //     auto verification_key = composer.compute_verification_key();

// //     std::vector<fr> public_inputs = { 1, 2 };
// //     TurboVerifier verifier(verification_key, TurboComposer::create_manifest(2), &public_inputs);

// //     EXPECT_FALSE(verifier.verify_proof(proof));
// // }

// // TEST(standard_format, prove_1_constraint_with_wrong_number_of_public_inputs_fails)
// // {
// //     standard_format constraint_system{ 3, 2,{}, {}, {}, { { 1, 2, 3, 0, 1, 1, -1, 0 } } };
// //     auto composer = create_circuit(constraint_system);
// //     read_witness(composer, { 1, 3, 4 });

// //     auto prover = composer.create_prover();
// //     plonk_proof proof = prover.construct_proof();

// //     auto verification_key = composer.compute_verification_key();

// //     std::vector<fr> public_inputs = { 1 };
// //     TurboVerifier verifier(verification_key, TurboComposer::create_manifest(2), &public_inputs);
// //     EXPECT_FALSE(verifier.verify_proof(proof));

// //     std::vector<fr> public_inputs2 = { 1, 3, 4 };
// //     TurboVerifier verifier2(verification_key, TurboComposer::create_manifest(2), &public_inputs2);
// //     EXPECT_FALSE(verifier2.verify_proof(proof));
// // }

// // TEST(standard_format, prove_2_constraints)
// // {
// //     standard_format constraint_system{ 4, 1,{}, {}, {}, { { 1, 2, 3, 0, 1, 1, -1, 0 }, { 2, 3, 4, 1, 0, 0, -1, 1 }
// }
// //     }; auto composer = create_circuit(constraint_system); read_witness(composer, { 1, 1, 2, 3 });

// //     auto prover = composer.create_prover();
// //     plonk_proof proof = prover.construct_proof();

// //     auto verifier = composer.create_verifier();

// //     EXPECT_TRUE(verifier.verify_proof(proof));
// // }

// // TEST(standard_format, serialize)
// // {
// //     standard_format constraint_system{ 4, 1,{}, {}, {}, { { 1, 2, 3, 0, 1, 1, -1, 0 }, { 2, 3, 4, 1, 6, 17, 28, 39
// }
// //     }};

// //     auto buf = to_buffer(constraint_system);
// //     auto result = from_buffer<standard_format>(buf);

// //     EXPECT_EQ(result, constraint_system);
// // }
