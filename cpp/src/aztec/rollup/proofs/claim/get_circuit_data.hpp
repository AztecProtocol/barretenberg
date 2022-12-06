#pragma once
#include "claim_tx.hpp"
#include "claim_circuit.hpp"
#include <plonk/proof_system/proving_key/proving_key.hpp>
#include <plonk/proof_system/verification_key/verification_key.hpp>
#include <stdlib/merkle_tree/hash_path.hpp>
#include "../compute_circuit_data.hpp"
#include "../../constants.hpp"

namespace rollup {
namespace proofs {
namespace claim {

using namespace plonk::stdlib::merkle_tree;

using circuit_data = proofs::circuit_data;

inline circuit_data get_circuit_data(std::shared_ptr<waffle::ReferenceStringFactory> const& srs, bool mock = false)
{
    std::cerr << "Getting claim circuit data..." << std::endl;

    auto build_circuit = [&](Composer& composer) {
        claim_tx claim_tx;
        claim_tx.claim_note_path.resize(DATA_TREE_DEPTH);
        claim_tx.defi_interaction_note_path.resize(DEFI_TREE_DEPTH);
        claim_circuit(composer, claim_tx);
    };

    return proofs::get_circuit_data<Composer>(
        "claim", "", srs, "", true, false, false, true, true, false, mock, build_circuit);
}

} // namespace claim
} // namespace proofs
} // namespace rollup