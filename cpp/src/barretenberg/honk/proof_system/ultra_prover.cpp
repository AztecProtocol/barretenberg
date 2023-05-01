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
    prover_polynomials.q_4 = key->q_4;
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
    prover_polynomials.w_l_shift = key->w_l.shifted();
    prover_polynomials.w_r_shift = key->w_r.shifted();
    prover_polynomials.w_o_shift = key->w_o.shifted();

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
// WORKTODO: No need to commit to sorted polys? We dont in Plonk
template <UltraFlavor Flavor> void UltraProver_<Flavor>::execute_sorted_list_accumulator_round()
{
    // Compute and add eta to relation parameters
    auto eta = transcript.get_challenge("eta");
    relation_parameters.eta = eta;

    // Compute sorted witness-table accumulator and its commitment
    key->sorted_accum = prover_library::compute_sorted_list_accumulator<Flavor>(key, eta);
    queue.add_commitment(key->sorted_accum, commitment_labels.sorted_accum);

    // Finalize fourth wire polynomial by adding lookup memory records, then commit
    prover_library::add_plookup_memory_records_to_wire_4<Flavor>(key, eta);
    queue.add_commitment(key->w_4, commitment_labels.w_4);

    // TODO(luke): Addition of 4th has to wait until here. Move first three to constructor?
    prover_polynomials.sorted_accum_shift = key->sorted_accum.shifted();
    prover_polynomials.sorted_accum = key->sorted_accum;
    prover_polynomials.w_4 = key->w_4;
    prover_polynomials.w_4_shift = key->w_4.shifted();
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
    auto lookup_grand_product_delta = compute_lookup_grand_product_delta(beta, gamma, key->circuit_size);

    relation_parameters.beta = beta;
    relation_parameters.gamma = gamma;
    relation_parameters.public_input_delta = public_input_delta;
    relation_parameters.lookup_grand_product_delta = lookup_grand_product_delta;

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
    auto compare = [](const auto poly1, const auto poly2) {
        for (size_t i = 0; i < poly1.size(); ++i) {
            if (poly1[i] != poly2[i]) {
                return false;
            }
        }
        return true;
    };

    ASSERT(compare(prover_polynomials.q_c, key->q_c));
    ASSERT(compare(prover_polynomials.q_l, key->q_l));
    ASSERT(compare(prover_polynomials.q_r, key->q_r));
    ASSERT(compare(prover_polynomials.q_o, key->q_o));
    ASSERT(compare(prover_polynomials.q_4, key->q_4));
    ASSERT(compare(prover_polynomials.q_m, key->q_m));
    ASSERT(compare(prover_polynomials.q_arith, key->q_arith));
    ASSERT(compare(prover_polynomials.q_sort, key->q_sort));
    ASSERT(compare(prover_polynomials.q_elliptic, key->q_elliptic));
    ASSERT(compare(prover_polynomials.q_aux, key->q_aux));
    ASSERT(compare(prover_polynomials.q_lookup, key->q_lookup));

    ASSERT(compare(prover_polynomials.sigma_1, key->sigma_1));
    ASSERT(compare(prover_polynomials.sigma_2, key->sigma_2));
    ASSERT(compare(prover_polynomials.sigma_3, key->sigma_3));
    ASSERT(compare(prover_polynomials.sigma_4, key->sigma_4));
    ASSERT(compare(prover_polynomials.id_1, key->id_1));
    ASSERT(compare(prover_polynomials.id_2, key->id_2));
    ASSERT(compare(prover_polynomials.id_3, key->id_3));
    ASSERT(compare(prover_polynomials.id_4, key->id_4));

    ASSERT(compare(prover_polynomials.table_1, key->table_1));
    ASSERT(compare(prover_polynomials.table_2, key->table_2));
    ASSERT(compare(prover_polynomials.table_3, key->table_3));
    ASSERT(compare(prover_polynomials.table_4, key->table_4));
    ASSERT(compare(prover_polynomials.lagrange_first, key->lagrange_first));
    ASSERT(compare(prover_polynomials.lagrange_last, key->lagrange_last));

    ASSERT(compare(prover_polynomials.w_l, key->w_l));
    ASSERT(compare(prover_polynomials.w_r, key->w_r));
    ASSERT(compare(prover_polynomials.w_o, key->w_o));
    ASSERT(compare(prover_polynomials.w_4, key->w_4));

    ASSERT(compare(prover_polynomials.sorted_accum, key->sorted_accum));
    ASSERT(compare(prover_polynomials.z_perm, key->z_perm));
    ASSERT(compare(prover_polynomials.z_lookup, key->z_lookup));

    ASSERT(compare(prover_polynomials.table_1_shift, key->table_1.shifted()));
    ASSERT(compare(prover_polynomials.table_2_shift, key->table_2.shifted()));
    ASSERT(compare(prover_polynomials.table_3_shift, key->table_3.shifted()));
    ASSERT(compare(prover_polynomials.table_4_shift, key->table_4.shifted()));
    ASSERT(compare(prover_polynomials.w_l_shift, key->w_l.shifted()));
    ASSERT(compare(prover_polynomials.w_r_shift, key->w_r.shifted()));
    ASSERT(compare(prover_polynomials.w_o_shift, key->w_o.shifted()));
    ASSERT(compare(prover_polynomials.w_4_shift, key->w_4.shifted()));
    ASSERT(compare(prover_polynomials.sorted_accum_shift, key->sorted_accum.shifted()));
    ASSERT(compare(prover_polynomials.z_perm_shift, key->z_perm.shifted()));
    ASSERT(compare(prover_polynomials.z_lookup_shift, key->z_lookup.shifted()));

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
    // Add circuit size public input size and public inputs to transcript.
    execute_preamble_round();

    // Compute first three wire commitments
    execute_wire_commitments_round();
    queue.process_queue();

    // Compute sorted list accumulator and commitment
    execute_sorted_list_accumulator_round();
    queue.process_queue();

    // Fiat-Shamir: beta & gamma
    // Compute grand product(s) and commitments.
    execute_grand_product_computation_round();
    queue.process_queue();

    // Fiat-Shamir: alpha
    // Run sumcheck subprotocol.
    execute_relation_check_rounds();

    return export_proof();
}

template class UltraProver_<honk::flavor::Ultra>;

} // namespace proof_system::honk
