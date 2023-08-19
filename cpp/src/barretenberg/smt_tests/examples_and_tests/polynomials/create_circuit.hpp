#include "barretenberg/crypto/generators/generator_data.hpp"
#include "barretenberg/crypto/pedersen_commitment/pedersen.hpp"
#include "barretenberg/proof_system/circuit_builder/standard_circuit_builder.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include "barretenberg/stdlib/primitives/field/field.hpp"

using namespace barretenberg;
using namespace proof_system;

auto& engine = numeric::random::get_debug_engine();

using field_ct = proof_system::plonk::stdlib::field_t<StandardCircuitBuilder>;
using witness_t = proof_system::plonk::stdlib::witness_t<StandardCircuitBuilder>;
using pub_witness_t = proof_system::plonk::stdlib::public_witness_t<StandardCircuitBuilder>;

void create_circuit(const std::string& fname, size_t n, bool pub_coeffs);