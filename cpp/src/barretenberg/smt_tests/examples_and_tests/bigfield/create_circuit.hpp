#include "barretenberg/numeric/random/engine.hpp"

#include "barretenberg/ecc/curves/bn254/fq.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"

#include "barretenberg/stdlib/primitives/bigfield/bigfield.hpp"
#include "barretenberg/stdlib/primitives/bool/bool.hpp"
#include "barretenberg/stdlib/primitives/byte_array/byte_array.hpp"
#include "barretenberg/stdlib/primitives/field/field.hpp"

#include "barretenberg/plonk/proof_system/constants.hpp"
#include "barretenberg/plonk/proof_system/prover/prover.hpp"
#include "barretenberg/plonk/proof_system/verifier/verifier.hpp"

#include "barretenberg/stdlib/primitives/circuit_builders/circuit_builders.hpp"
#include "barretenberg/stdlib/primitives/curves/bn254.hpp"

#include "barretenberg/polynomials/polynomial_arithmetic.hpp"
#include <fstream>
#include <memory>
#include <utility>

using namespace barretenberg;
using namespace proof_system::plonk;

using field_ct = proof_system::plonk::stdlib::field_t<StandardCircuitBuilder>;
using witness_t = proof_system::plonk::stdlib::witness_t<StandardCircuitBuilder>;
using pub_witness_t = proof_system::plonk::stdlib::public_witness_t<StandardCircuitBuilder>;

using bn254 = stdlib::bn254<StandardCircuitBuilder>;

using fr_ct = bn254::ScalarField;
using fq_ct = bn254::BaseField;
using public_witness_ct = bn254::public_witness_ct;
using witness_ct = bn254::witness_ct;

auto NUM_LIMB_BITS = NUM_LIMB_BITS_IN_FIELD_SIMULATION;

void create_circuit(const std::string& fname, bool pub_ab);