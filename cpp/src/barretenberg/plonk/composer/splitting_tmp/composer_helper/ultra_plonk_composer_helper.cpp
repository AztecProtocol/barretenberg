#include "ultra_plonk_composer_helper.hpp"
#include "barretenberg/proof_system/circuit_constructors/ultra_circuit_constructor.hpp"
#include "barretenberg/proof_system/composer/permutation_helper.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace plonk {

// template <typename CircuitConstructor>
// std::shared_ptr<proving_key> UltraPlonkComposerHelper<CircuitConstructor>::compute_proving_key(
//     const CircuitConstructor& circuit_constructor)
// {
//     if (circuit_proving_key) {
//         return circuit_proving_key;
//     }

//     size_t tables_size = 0;
//     size_t lookups_size = 0;
//     for (const auto& table : circuit_constructor.lookup_tables) {
//         tables_size += table.size;
//         lookups_size += table.lookup_gates.size();
//     }

//     const size_t minimum_circuit_size = tables_size + lookups_size;
//     const size_t num_randomized_gates = NUM_RANDOMIZED_GATES;
//     // Initialize circuit_proving_key
//     // TODO(#229)(Kesha): replace composer types.
//     circuit_proving_key = initialize_proving_key(circuit_constructor,
//                                                  crs_factory_.get(),
//                                                  minimum_circuit_size,
//                                                  num_randomized_gates,
//                                                  plonk::ComposerType::PLOOKUP);

//     construct_lagrange_selector_forms(circuit_constructor, circuit_proving_key.get());

//     enforce_nonzero_polynomial_selectors(circuit_constructor, circuit_proving_key.get());

//     compute_monomial_and_coset_selector_forms(circuit_proving_key.get(), ultra_selector_properties());

//     // // Compute sigma polynomials (TODO(kesha): we should update that late)
//     // bonk::compute_standard_plonk_sigma_permutations<CircuitConstructor::program_width>(circuit_constructor,
//     //                                                                                    circuit_proving_key.get());

//     const size_t subgroup_size = circuit_proving_key->circuit_size;

//     polynomial poly_q_table_column_1(subgroup_size);
//     polynomial poly_q_table_column_2(subgroup_size);
//     polynomial poly_q_table_column_3(subgroup_size);
//     polynomial poly_q_table_column_4(subgroup_size);

//     size_t offset = subgroup_size - tables_size - s_randomness - 1;

//     // Create lookup selector polynomials which interpolate each table column.
//     // Our selector polys always need to interpolate the full subgroup size, so here we offset so as to
//     // put the table column's values at the end. (The first gates are for non-lookup constraints).
//     // [0, ..., 0, ...table, 0, 0, 0, x]
//     //  ^^^^^^^^^  ^^^^^^^^  ^^^^^^^  ^nonzero to ensure uniqueness and to avoid infinity commitments
//     //  |          table     randomness
//     //  ignored, as used for regular constraints and padding to the next power of 2.

//     for (size_t i = 0; i < offset; ++i) {
//         poly_q_table_column_1[i] = 0;
//         poly_q_table_column_2[i] = 0;
//         poly_q_table_column_3[i] = 0;
//         poly_q_table_column_4[i] = 0;
//     }

//     for (const auto& table : circuit_constructor.lookup_tables) {
//         const fr table_index(table.table_index);

//         for (size_t i = 0; i < table.size; ++i) {
//             poly_q_table_column_1[offset] = table.column_1[i];
//             poly_q_table_column_2[offset] = table.column_2[i];
//             poly_q_table_column_3[offset] = table.column_3[i];
//             poly_q_table_column_4[offset] = table_index;
//             ++offset;
//         }
//     }

//     // Initialise the last `s_randomness` positions in table polynomials with 0. We don't need to actually randomise
//     // the table polynomials.
//     for (size_t i = 0; i < s_randomness; ++i) {
//         poly_q_table_column_1[offset] = 0;
//         poly_q_table_column_2[offset] = 0;
//         poly_q_table_column_3[offset] = 0;
//         poly_q_table_column_4[offset] = 0;
//         ++offset;
//     }

//     // In the case of using UltraPlonkComposer for a circuit which does _not_ make use of any lookup tables, all four
//     // table columns would be all zeros. This would result in these polys' commitments all being the point at
//     infinity
//     // (which is bad because our point arithmetic assumes we'll never operate on the point at infinity). To avoid
//     this,
//     // we set the last evaluation of each poly to be nonzero. The last `num_roots_cut_out_of_vanishing_poly = 4`
//     // evaluations are ignored by constraint checks; we arbitrarily choose the very-last evaluation to be nonzero.
//     See
//     // ComposerBase::compute_proving_key_base for further explanation, as a similar trick is done there. We could
//     // have chosen `1` for each such evaluation here, but that would have resulted in identical commitments for
//     // all four columns. We don't want to have equal commitments, because biggroup operations assume no points are
//     // equal, so if we tried to verify an ultra proof in a circuit, the biggroup operations would fail. To combat
//     // this, we just choose distinct values:
//     ASSERT(offset == subgroup_size - 1);
//     auto unique_last_value =
//         get_num_selectors() + 1; // Note: in compute_proving_key_base, moments earlier, each selector
//                                  // vector was given a unique last value from 1..num_selectors. So we
//                                  // avoid those values and continue the count, to ensure uniqueness.
//     poly_q_table_column_1[subgroup_size - 1] = unique_last_value;
//     poly_q_table_column_2[subgroup_size - 1] = ++unique_last_value;
//     poly_q_table_column_3[subgroup_size - 1] = ++unique_last_value;
//     poly_q_table_column_4[subgroup_size - 1] = ++unique_last_value;

//     add_table_column_selector_poly_to_proving_key(poly_q_table_column_1, "table_value_1");
//     add_table_column_selector_poly_to_proving_key(poly_q_table_column_2, "table_value_2");
//     add_table_column_selector_poly_to_proving_key(poly_q_table_column_3, "table_value_3");
//     add_table_column_selector_poly_to_proving_key(poly_q_table_column_4, "table_value_4");

//     // Instantiate z_lookup and s polynomials in the proving key (no values assigned yet).
//     // Note: might be better to add these polys to cache only after they've been computed, as is convention
//     // TODO(luke): Don't put empty polynomials in the store, just add these where they're computed
//     polynomial z_lookup_fft(subgroup_size * 4);
//     polynomial s_fft(subgroup_size * 4);
//     circuit_proving_key->polynomial_store.put("z_lookup_fft", std::move(z_lookup_fft));
//     circuit_proving_key->polynomial_store.put("s_fft", std::move(s_fft));

//     // // TODO: composer-level constant variable needed for the program width
//     // compute_sigma_permutations<4, true>(circuit_proving_key.get());

//     bonk::compute_plonk_generalized_sigma_permutations<CircuitConstructor::program_width>(circuit_constructor,
//                                                                                           circuit_proving_key.get());

//     // Copy memory read/write record data into proving key. Prover needs to know which gates contain a read/write
//     // 'record' witness on the 4th wire. This wire value can only be fully computed once the first 3 wire polynomials
//     // have been committed to. The 4th wire on these gates will be a random linear combination of the first 3 wires,
//     // using the plookup challenge `eta`
//     std::copy(circuit_constructor.memory_read_records.begin(),
//               circuit_constructor.memory_read_records.end(),
//               std::back_inserter(circuit_proving_key->memory_read_records));
//     std::copy(circuit_constructor.memory_write_records.begin(),
//               circuit_constructor.memory_write_records.end(),
//               std::back_inserter(circuit_proving_key->memory_write_records));

//     circuit_proving_key->recursive_proof_public_input_indices =
//         std::vector<uint32_t>(recursive_proof_public_input_indices.begin(),
//         recursive_proof_public_input_indices.end());

//     circuit_proving_key->contains_recursive_proof = contains_recursive_proof;

//     return circuit_proving_key;
// }

/**
 * Compute verification key consisting of selector precommitments.
 *
 * @return Pointer to created circuit verification key.
 * */
template <typename CircuitConstructor>
std::shared_ptr<bonk::verification_key> UltraPlonkComposerHelper<CircuitConstructor>::compute_verification_key(
    const CircuitConstructor& circuit_constructor)
{
    if (circuit_verification_key) {
        return circuit_verification_key;
    }

    if (!circuit_proving_key) {
        compute_proving_key(circuit_constructor);
    }
    circuit_verification_key = compute_verification_key_common(circuit_proving_key, crs_factory_->get_verifier_crs());

    circuit_verification_key->composer_type = type; // Invariably plookup for this class.

    // See `add_recusrive_proof()` for how this recursive data is assigned.
    circuit_verification_key->recursive_proof_public_input_indices =
        std::vector<uint32_t>(recursive_proof_public_input_indices.begin(), recursive_proof_public_input_indices.end());

    circuit_verification_key->contains_recursive_proof = contains_recursive_proof;

    return circuit_verification_key;
}

template <typename CircuitConstructor>
void UltraPlonkComposerHelper<CircuitConstructor>::add_table_column_selector_poly_to_proving_key(
    polynomial& selector_poly_lagrange_form, const std::string& tag)
{
    polynomial selector_poly_lagrange_form_copy(selector_poly_lagrange_form, circuit_proving_key->small_domain.size);

    selector_poly_lagrange_form.ifft(circuit_proving_key->small_domain);
    auto& selector_poly_coeff_form = selector_poly_lagrange_form;

    polynomial selector_poly_coset_form(selector_poly_coeff_form, circuit_proving_key->circuit_size * 4);
    selector_poly_coset_form.coset_fft(circuit_proving_key->large_domain);

    circuit_proving_key->polynomial_store.put(tag, std::move(selector_poly_coeff_form));
    circuit_proving_key->polynomial_store.put(tag + "_lagrange", std::move(selector_poly_lagrange_form_copy));
    circuit_proving_key->polynomial_store.put(tag + "_fft", std::move(selector_poly_coset_form));
}

template class UltraPlonkComposerHelper<UltraCircuitConstructor>;
} // namespace plonk
