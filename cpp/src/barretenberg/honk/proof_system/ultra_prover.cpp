#include "ultra_prover.hpp"
#include <algorithm>
#include <cstddef>
#include "barretenberg/honk/proof_system/prover_library.hpp"
#include "barretenberg/honk/sumcheck/relations/ultra_arithmetic_relation.hpp"
#include "barretenberg/honk/sumcheck/relations/ultra_arithmetic_relation_secondary.hpp"
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
#include "barretenberg/honk/sumcheck/relations/lookup_grand_product_relation.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/transcript/transcript_wrappers.hpp"
#include <string>
#include "barretenberg/honk/pcs/claim.hpp"

namespace proof_system::honk {

/**
 * Create UltraProver_ from proving key, witness and manifest.
 *
 * @param input_key Proving key.
 * @param input_manifest Input manifest
 *
 * @tparam settings Settings class.
 * */
template <UltraFlavor Flavor>
UltraProver_<Flavor>::UltraProver_(std::shared_ptr<typename Flavor::ProvingKey> input_key)
    : key(input_key)
    , queue(input_key->circuit_size, transcript)
{
    prover_polynomials.q_c = key->q_c;
    prover_polynomials.q_l = key->q_l;
    prover_polynomials.q_r = key->q_r;
    prover_polynomials.q_o = key->q_o;
    prover_polynomials.q_m = key->q_m;
    prover_polynomials.q_arith = key->q_arith;
    prover_polynomials.q_sort = key->q_sort;
    prover_polynomials.q_elliptic = key->q_elliptic;
    prover_polynomials.q_aux = key->q_aux;
    prover_polynomials.q_lookup = key->q_lookup;
    prover_polynomials.sigma_1 = key->sigma_1;
    prover_polynomials.sigma_2 = key->sigma_2;
    prover_polynomials.sigma_3 = key->sigma_3;
    prover_polynomials.sigma_4 = key->sigma_4;
    prover_polynomials.id_1 = key->id_1;
    prover_polynomials.id_2 = key->id_2;
    prover_polynomials.id_3 = key->id_3;
    prover_polynomials.id_4 = key->id_4;
    prover_polynomials.table_1 = key->table_1;
    prover_polynomials.table_2 = key->table_2;
    prover_polynomials.table_3 = key->table_3;
    prover_polynomials.table_4 = key->table_4;
    prover_polynomials.table_1_shift = key->table_1.shifted();
    prover_polynomials.table_2_shift = key->table_2.shifted();
    prover_polynomials.table_3_shift = key->table_3.shifted();
    prover_polynomials.table_4_shift = key->table_4.shifted();
    prover_polynomials.lagrange_first = key->lagrange_first;
    prover_polynomials.lagrange_last = key->lagrange_last;
    prover_polynomials.w_l = key->w_l;
    prover_polynomials.w_r = key->w_r;
    prover_polynomials.w_o = key->w_o;
    prover_polynomials.w_4 = key->w_4;
    prover_polynomials.sorted_1 = key->sorted_1;
    prover_polynomials.sorted_2 = key->sorted_2;
    prover_polynomials.sorted_3 = key->sorted_3;
    prover_polynomials.sorted_4 = key->sorted_4;

    // Add public inputs to transcript from the second wire polynomial
    std::span<FF> public_wires_source = prover_polynomials.w_r;

    for (size_t i = 0; i < key->num_public_inputs; ++i) {
        public_inputs.emplace_back(public_wires_source[i]);
    }
}

/**
 * @brief Commit to the first three wires only
 *
 */
template <UltraFlavor Flavor> void UltraProver_<Flavor>::compute_wire_commitments()
{
    auto wire_polys = key->get_wires();
    auto labels = commitment_labels.get_wires();
    for (size_t idx = 0; idx < 3; ++idx) {
        queue.add_commitment(wire_polys[idx], labels[idx]);
    }
}

/**
 * @brief Add circuit size, public input size, and public inputs to transcript
 *
 */
template <UltraFlavor Flavor> void UltraProver_<Flavor>::execute_preamble_round()
{
    const auto circuit_size = static_cast<uint32_t>(key->circuit_size);
    const auto num_public_inputs = static_cast<uint32_t>(key->num_public_inputs);

    transcript.send_to_verifier("circuit_size", circuit_size);
    transcript.send_to_verifier("public_input_size", num_public_inputs);

    for (size_t i = 0; i < key->num_public_inputs; ++i) {
        auto public_input_i = public_inputs[i];
        transcript.send_to_verifier("public_input_" + std::to_string(i), public_input_i);
    }
}

/**
 * @brief Compute commitments to the first three wires
 *
 */
template <UltraFlavor Flavor> void UltraProver_<Flavor>::execute_wire_commitments_round()
{
    auto wire_polys = key->get_wires();
    auto labels = commitment_labels.get_wires();
    for (size_t idx = 0; idx < 3; ++idx) {
        queue.add_commitment(wire_polys[idx], labels[idx]);
    }
}

/**
 * @brief Compute sorted witness-table accumulator
 *
 */
template <UltraFlavor Flavor> void UltraProver_<Flavor>::execute_sorted_list_accumulator_round()
{
    // Compute and add eta to relation parameters
    auto eta = transcript.get_challenge("eta");
    relation_parameters.eta = eta;

    // Compute sorted witness-table accumulator and its commmitment
    key->sorted_accum = prover_library::compute_sorted_list_accumulator<Flavor>(key, eta);

    // Finalize wire 4 by adding lookup memory records then commit to it
    prover_library::add_plookup_memory_records_to_wire_4<Flavor>(key, eta);
    queue.add_commitment(key->w_4, commitment_labels.w_4);
}

/**
 * @brief Compute permutation and lookup grand product polynomials and commitments
 *
 */
template <UltraFlavor Flavor> void UltraProver_<Flavor>::execute_grand_product_computation_round()
{
    // Compute and store parameters required by relations in Sumcheck
    auto [beta, gamma] = transcript.get_challenges("beta", "gamma");

    auto public_input_delta = compute_public_input_delta<FF>(public_inputs, beta, gamma, key->circuit_size);

    relation_parameters.beta = beta;
    relation_parameters.gamma = gamma;
    relation_parameters.public_input_delta = public_input_delta;

    // Compute permutation grand product and its commitment
    key->z_perm = prover_library::compute_permutation_grand_product<Flavor>(key, beta, gamma);
    queue.add_commitment(key->z_perm, commitment_labels.z_perm);

    // Compute lookup grand product and its commitment
    key->z_lookup = prover_library::compute_lookup_grand_product<Flavor>(key, relation_parameters.eta, beta, gamma);
    queue.add_commitment(key->z_lookup, commitment_labels.z_lookup);

    prover_polynomials.z_perm = key->z_perm;
    prover_polynomials.z_perm_shift = key->z_perm.shifted();
    prover_polynomials.z_lookup = key->z_lookup;
    prover_polynomials.z_lookup_shift = key->z_lookup.shifted();
}

/**
 * @brief Run Sumcheck resulting in u = (u_1,...,u_d) challenges and all evaluations at u being calculated.
 *
 */
template <UltraFlavor Flavor> void UltraProver_<Flavor>::execute_relation_check_rounds()
{
    using Sumcheck = sumcheck::Sumcheck<Flavor,
                                        ProverTranscript<FF>,
                                        sumcheck::UltraArithmeticRelation,
                                        sumcheck::UltraArithmeticRelationSecondary,
                                        sumcheck::UltraGrandProductComputationRelation,
                                        sumcheck::UltraGrandProductInitializationRelation,
                                        sumcheck::LookupGrandProductComputationRelation,
                                        sumcheck::LookupGrandProductInitializationRelation>;

    auto sumcheck = Sumcheck(key->circuit_size, transcript);

    sumcheck_output = sumcheck.execute_prover(prover_polynomials, relation_parameters);
}

template <UltraFlavor Flavor> plonk::proof& UltraProver_<Flavor>::export_proof()
{
    proof.proof_data = transcript.proof_data;
    return proof;
}

template <UltraFlavor Flavor> plonk::proof& UltraProver_<Flavor>::construct_proof()
{
    return export_proof();
}

template class UltraProver_<honk::flavor::Ultra>;

} // namespace proof_system::honk
