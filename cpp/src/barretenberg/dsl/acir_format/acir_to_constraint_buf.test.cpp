#include "../acir_proofs/acir_composer.hpp"
#include "acir_to_constraint_buf.hpp"
#include <barretenberg/bb/file_io.hpp>
#include <barretenberg/bb/get_bytecode.hpp>
#include <barretenberg/bb/get_witness.hpp>
#include <barretenberg/common/container.hpp>
#include <barretenberg/common/log.hpp>
#include <barretenberg/serialize/cbind.hpp>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

namespace acir_format::tests {

TEST(AcirToConstraintBuf, DeserializesCircuit)
{
    barretenberg::srs::init_crs_factory("../srs_db/ignition");

    auto bytecode =
        get_bytecode("/mnt/user-data/charlie/aztec-repos/barretenberg/acir_tests/acir_tests/1_mul/target/main.json");
    info("bytecode length: ", bytecode.size());
    auto witness_buf = get_witness_data(
        "/mnt/user-data/charlie/aztec-repos/barretenberg/acir_tests/acir_tests/1_mul/target/witness.tr");
    info("witness length: ", witness_buf.size());

    auto constraint_buf = circuit_buf_to_acir_format(bytecode);
    info("here");
    auto witness = witness_buf_to_witness_data(witness_buf);
    info("here");

    acir_proofs::AcirComposer ac(0, true);
    auto proof = ac.create_proof(barretenberg::srs::get_crs_factory(), constraint_buf, witness, false);

    EXPECT_TRUE(ac.verify_proof(proof, false));
}

} // namespace acir_format::tests