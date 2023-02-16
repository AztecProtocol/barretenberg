#include "../../fixtures/user_context.hpp"
#include "ultra_example.hpp"
#include <common/streams.hpp>
#include <gtest/gtest.h>

using namespace barretenberg;
using namespace rollup::proofs::ultra_example;

TEST(ultra_example_tests, test_ultra_example)
{
    waffle::UltraComposer composer = waffle::UltraComposer("../srs_db/ignition");
    build_circuit(composer);

    waffle::UltraProver prover = composer.create_prover();
    waffle::plonk_proof proof = prover.construct_proof();

    std::cout << "gates: " << composer.get_num_gates() << std::endl;
    std::cout << "proof size: " << proof.proof_data.size() << std::endl;
    std::cout << "public inputs size: " << composer.public_inputs.size() << std::endl;

    auto verifier = composer.create_verifier();
    bool result = verifier.verify_proof(proof);

    EXPECT_TRUE(result);
}