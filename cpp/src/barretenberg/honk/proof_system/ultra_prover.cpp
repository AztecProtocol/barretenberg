#include "ultra_prover.hpp"
#include <algorithm>
#include <cstddef>
#include "barretenberg/honk/proof_system/prover_library.hpp"
#include "barretenberg/honk/sumcheck/sumcheck.hpp"
#include <array>
#include "barretenberg/honk/sumcheck/polynomials/univariate.hpp" // will go away
#include "barretenberg/honk/utils/power_polynomial.hpp"
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include <memory>
#include <span>
#include <utility>
#include <vector>
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/honk/sumcheck/relations/arithmetic_relation.hpp"
#include "barretenberg/honk/sumcheck/relations/grand_product_computation_relation.hpp"
#include "barretenberg/honk/sumcheck/relations/grand_product_initialization_relation.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/transcript/transcript_wrappers.hpp"
#include <string>
#include "barretenberg/honk/pcs/claim.hpp"

namespace proof_system::honk {

/**
 * Create UltraHonkProver from proving key, witness and manifest.
 *
 * @param input_key Proving key.
 * @param input_manifest Input manifest
 *
 * @tparam settings Settings class.
 * */
template <typename Flavor>
UltraHonkProver<Flavor>::UltraHonkProver(std::shared_ptr<typename Flavor::ProvingKey> input_key)
    : key(input_key)
    , queue(input_key->circuit_size, transcript)
{}

template <typename settings> plonk::proof& UltraHonkProver<settings>::export_proof()
{
    proof.proof_data = transcript.proof_data;
    return proof;
}

template <typename settings> plonk::proof& UltraHonkProver<settings>::construct_proof()
{
    return export_proof();
}

template class UltraHonkProver<honk::flavor::Ultra>;

} // namespace proof_system::honk
