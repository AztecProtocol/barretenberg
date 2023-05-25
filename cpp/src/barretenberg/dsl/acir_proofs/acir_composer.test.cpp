// #include <barretenberg/dsl/acir_format/acir_format.hpp>
// #include <barretenberg/srs/io.hpp>
// #include "barretenberg/srs/reference_string/pippenger_reference_string.hpp"
// #include <gtest/gtest.h>
// #include "acir_composer.hpp"
// #include <vector>

// using namespace proof_system::plonk;

// void create_inner_circuit(acir_format::acir_format& constraint_system, std::vector<fr>& witness)
// {
//     /**
//      * constraints produced by Noir program:
//      * fn main(x : u32, y : pub u32) {
//      * let z = x ^ y;
//      *
//      * constrain z != 10;
//      * }
//      **/
//     acir_format::RangeConstraint range_a{
//         .witness = 1,
//         .num_bits = 32,
//     };
//     acir_format::RangeConstraint range_b{
//         .witness = 2,
//         .num_bits = 32,
//     };

//     acir_format::LogicConstraint logic_constraint{
//         .a = 1,
//         .b = 2,
//         .result = 3,
//         .num_bits = 32,
//         .is_xor_gate = 1,
//     };
//     poly_triple expr_a{
//         .a = 3,
//         .b = 4,
//         .c = 0,
//         .q_m = 0,
//         .q_l = 1,
//         .q_r = -1,
//         .q_o = 0,
//         .q_c = -10,
//     };
//     poly_triple expr_b{
//         .a = 4,
//         .b = 5,
//         .c = 6,
//         .q_m = 1,
//         .q_l = 0,
//         .q_r = 0,
//         .q_o = -1,
//         .q_c = 0,
//     };
//     poly_triple expr_c{
//         .a = 4,
//         .b = 6,
//         .c = 4,
//         .q_m = 1,
//         .q_l = 0,
//         .q_r = 0,
//         .q_o = -1,
//         .q_c = 0,

//     };
//     poly_triple expr_d{
//         .a = 6,
//         .b = 0,
//         .c = 0,
//         .q_m = 0,
//         .q_l = -1,
//         .q_r = 0,
//         .q_o = 0,
//         .q_c = 1,
//     };
//     std::vector<poly_triple, ContainerSlabAllocator<poly_triple>> constraints = { expr_a, expr_b, expr_c, expr_d };

//     constraint_system = acir_format::acir_format{
//         .varnum = 7,
//         .public_inputs = { 2 },
//         .fixed_base_scalar_mul_constraints = {},
//         .logic_constraints = { logic_constraint },
//         .range_constraints = { range_a, range_b },
//         .schnorr_constraints = {},
//         .ecdsa_constraints = {},
//         .sha256_constraints = {},
//         .blake2s_constraints = {},
//         .keccak_constraints = {},
//         .hash_to_field_constraints = {},
//         .pedersen_constraints = {},
//         .compute_merkle_root_constraints = {},
//         .block_constraints = {},
//         .recursion_constraints = {},
//         .constraints = constraints,
//     };

//     uint256_t inverse_of_five = fr(5).invert();

//     witness.emplace_back(5);
//     witness.emplace_back(10);
//     witness.emplace_back(15);
//     witness.emplace_back(5);
//     witness.emplace_back(inverse_of_five);
//     witness.emplace_back(1);
// }

// TEST(AcirComposer, TestLogicCircuit)
// {
//     acir_format::acir_format constraint_system;
//     std::vector<fr> witness_inner;
//     create_inner_circuit(constraint_system, witness_inner);

//     std::vector<uint8_t> witness_buf;           // = to_buffer(witness);
//     write(witness_buf, witness_inner);
//     // TODO: get WitnessVector in different way
//     auto witness = from_buffer<std::vector<fr, ContainerSlabAllocator<fr>>>(witness_buf);

//     auto crs_factory = std::shared_ptr<ReferenceStringFactory>(new FileReferenceStringFactory("../srs_db/ignition"));
//     auto g2x = crs_factory.get()->get_verifier_crs()->get_g2x().to_buffer();
//     // uint32_t pow2_size = 1 << 19UL;
//     uint32_t pow2_size = 8192;

//     auto prover_crs = crs_factory.get()->get_prover_crs(pow2_size + 1);
//     auto g1x = prover_crs.get()->get_monomial_points();
//     auto* pippenger_ptr_base = new scalar_multiplication::Pippenger(g1x, pow2_size + 1);
//     void* pippenger_ptr = reinterpret_cast<void*>(pippenger_ptr_base);

//     auto pippenger_factory = std::make_shared<proof_system::PippengerReferenceStringFactory>(
//         reinterpret_cast<barretenberg::scalar_multiplication::Pippenger*>(pippenger_ptr), g2x.data());

//     auto composer = new acir_proofs::AcirComposer(pippenger_factory);

//     composer->init_proving_key(constraint_system);
//     // std::cout << "composer info: " << composer
//     composer->init_verification_key();

//     auto proof = composer->create_proof(constraint_system, witness);

//     auto verified = composer->verify_proof(proof);
//     EXPECT_EQ(verified, true);
// }
