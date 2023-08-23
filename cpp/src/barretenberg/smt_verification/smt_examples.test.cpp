#include "barretenberg/crypto/generators/generator_data.hpp"
#include "barretenberg/crypto/pedersen_commitment/pedersen.hpp"
#include "barretenberg/proof_system/circuit_builder/standard_circuit_builder.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "barretenberg/stdlib/primitives/field/field.hpp"

#include "barretenberg/smt_verification/circuit/circuit.hpp"

using namespace barretenberg;
using namespace proof_system;

namespace {
auto& engine = numeric::random::get_debug_engine();
}

using field_t = proof_system::plonk::stdlib::field_t<StandardCircuitBuilder>;
using witness_t = proof_system::plonk::stdlib::witness_t<StandardCircuitBuilder>;
using pub_witness_t = proof_system::plonk::stdlib::public_witness_t<StandardCircuitBuilder>;

const std::string p = "21888242871839275222246405745257275088548364400416034343698204186575808495617";

TEST(circuit_verification, multiplication_true)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();

    field_t a(witness_t(&builder, fr::random_element()));
    field_t b(witness_t(&builder, fr::random_element()));
    field_t c = (a + a) / (b + b + b);

    builder.set_variable_name(a.witness_index, "a");
    builder.set_variable_name(b.witness_index, "b");
    builder.set_variable_name(c.witness_index, "c");
    ASSERT_TRUE(builder.check_circuit());

    auto buf = builder.export_circuit();

    smt_circuit::CircuitSchema circuit_info = smt_circuit::unpack_from_buffer(buf);
    smt_solver::Solver s(p, true, 10);
    smt_circuit::Circuit circuit(circuit_info, &s);
    smt_terms::FFTerm a1 = circuit["a"];
    smt_terms::FFTerm b1 = circuit["b"];
    smt_terms::FFTerm c1 = circuit["c"];
    smt_terms::FFTerm two = smt_terms::Const("2", &s, 10);
    smt_terms::FFTerm thr = smt_terms::Const("3", &s, 10);
    smt_terms::FFTerm cr = smt_terms::Var("cr", &s);
    cr = (two * a1) / (thr * b1);
    c1 != cr;

    bool res = s.check();
    ASSERT_FALSE(res);
}

TEST(circuit_verification, multiplication_false)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();

    field_t a(witness_t(&builder, fr::random_element()));
    field_t b(witness_t(&builder, fr::random_element()));
    field_t c = (a) / (b + b + b); // mistake was here

    builder.set_variable_name(a.witness_index, "a");
    builder.set_variable_name(b.witness_index, "b");
    builder.set_variable_name(c.witness_index, "c");
    ASSERT_TRUE(builder.check_circuit());

    auto buf = builder.export_circuit();

    smt_circuit::CircuitSchema circuit_info = smt_circuit::unpack_from_buffer(buf);
    smt_solver::Solver s(p, true, 10);
    smt_circuit::Circuit circuit(circuit_info, &s);

    smt_terms::FFTerm a1 = circuit["a"];
    smt_terms::FFTerm b1 = circuit["b"];
    smt_terms::FFTerm c1 = circuit["c"];

    smt_terms::FFTerm two = smt_terms::Const("2", &s, 10);
    smt_terms::FFTerm thr = smt_terms::Const("3", &s, 10);
    smt_terms::FFTerm cr = smt_terms::Var("cr", &s);
    cr = (two * a1) / (thr * b1);
    c1 != cr;

    bool res = s.check();
    ASSERT_TRUE(res);

    std::unordered_map<std::string, cvc5::Term> terms({ { "a", a1 }, { "b", b1 }, { "c", c1 }, { "cr", cr } });

    std::unordered_map<std::string, std::string> vals = s.model(terms);

    info("a = ", vals["a"]);
    info("b = ", vals["b"]);
    info("c = ", vals["c"]);
    info("c_res = ", vals["cr"]);
}

TEST(circuit_verifiaction, unique_witness)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();

    field_t a(witness_t(&builder, fr::random_element()));
    field_t b(witness_t(&builder, fr::random_element()));
    builder.set_variable_name(a.witness_index, "a");
    builder.set_variable_name(b.witness_index, "b");

    field_t c;
    for (size_t i = 0; i < 20; i++) {
        c = a + b;
        a = b;
        b = c;
    }

    builder.set_variable_name(a.witness_index, "pre_res");
    builder.set_variable_name(b.witness_index, "res");

    auto buf = builder.export_circuit();

    smt_solver::Solver s(p, true, 10);
    smt_circuit::CircuitSchema circuit_info = smt_circuit::unpack_from_buffer(buf);

    std::pair<smt_circuit::Circuit, smt_circuit::Circuit> cirs =
        smt_circuit::unique_witness(circuit_info, &s, { "a", "b", "res" }, { "pre_res" });

    bool res = s.check();
    ASSERT_TRUE(res);
}