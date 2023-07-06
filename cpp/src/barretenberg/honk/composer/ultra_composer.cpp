#include "ultra_composer.hpp"
#include "barretenberg/honk/proof_system/ultra_prover.hpp"
#include "barretenberg/proof_system/circuit_builder/ultra_circuit_builder.hpp"
#include "barretenberg/proof_system/composer/composer_lib.hpp"
#include "barretenberg/proof_system/composer/permutation_lib.hpp"

namespace proof_system::honk {

/**
 * @brief Compute witness polynomials
 *
 */
template <UltraFlavor Flavor> void UltraComposer_<Flavor>::compute_witness(CircuitBuilder& circuit_constructor)
{
    if (computed_witness) {
        return;
    }

    const size_t filled_gates = circuit_constructor.num_gates + circuit_constructor.public_inputs.size();

    // Pad the wires (pointers to `witness_indices` of the `variables` vector).
    // Note: total_num_gates = filled_gates + tables_size
    for (size_t i = filled_gates; i < total_num_gates; ++i) {
        circuit_constructor.w_l.emplace_back(circuit_constructor.zero_idx);
        circuit_constructor.w_r.emplace_back(circuit_constructor.zero_idx);
        circuit_constructor.w_o.emplace_back(circuit_constructor.zero_idx);
        circuit_constructor.w_4.emplace_back(circuit_constructor.zero_idx);
    }

    auto wire_polynomials = construct_wire_polynomials_base<Flavor>(circuit_constructor, dyadic_circuit_size);

    proving_key->w_l = wire_polynomials[0];
    proving_key->w_r = wire_polynomials[1];
    proving_key->w_o = wire_polynomials[2];
    proving_key->w_4 = wire_polynomials[3];

    polynomial s_1(dyadic_circuit_size);
    polynomial s_2(dyadic_circuit_size);
    polynomial s_3(dyadic_circuit_size);
    polynomial s_4(dyadic_circuit_size);
    // TODO(luke): The +1 size for z_lookup is not necessary and can lead to confusion. Resolve.
    polynomial z_lookup(dyadic_circuit_size + 1); // Only instantiated in this function; nothing assigned.

    // TODO(kesha): Look at this once we figure out how we do ZK (previously we had roots cut out, so just added
    // randomness)
    // The size of empty space in sorted polynomials
    size_t count = dyadic_circuit_size - tables_size - lookups_size;
    ASSERT(count > 0); // We need at least 1 row of zeroes for the permutation argument
    for (size_t i = 0; i < count; ++i) {
        s_1[i] = 0;
        s_2[i] = 0;
        s_3[i] = 0;
        s_4[i] = 0;
    }

    for (auto& table : circuit_constructor.lookup_tables) {
        const fr table_index(table.table_index);
        auto& lookup_gates = table.lookup_gates;
        for (size_t i = 0; i < table.size; ++i) {
            if (table.use_twin_keys) {
                lookup_gates.push_back({
                    {
                        table.column_1[i].from_montgomery_form().data[0],
                        table.column_2[i].from_montgomery_form().data[0],
                    },
                    {
                        table.column_3[i],
                        0,
                    },
                });
            } else {
                lookup_gates.push_back({
                    {
                        table.column_1[i].from_montgomery_form().data[0],
                        0,
                    },
                    {
                        table.column_2[i],
                        table.column_3[i],
                    },
                });
            }
        }

#ifdef NO_TBB
        std::sort(lookup_gates.begin(), lookup_gates.end());
#else
        std::sort(std::execution::par_unseq, lookup_gates.begin(), lookup_gates.end());
#endif

        for (const auto& entry : lookup_gates) {
            const auto components = entry.to_sorted_list_components(table.use_twin_keys);
            s_1[count] = components[0];
            s_2[count] = components[1];
            s_3[count] = components[2];
            s_4[count] = table_index;
            ++count;
        }
    }

    // Polynomial memory is zeroed out when constructed with size hint, so we don't have to initialize trailing space
    proving_key->sorted_1 = s_1;
    proving_key->sorted_2 = s_2;
    proving_key->sorted_3 = s_3;
    proving_key->sorted_4 = s_4;

    // Copy memory read/write record data into proving key. Prover needs to know which gates contain a read/write
    // 'record' witness on the 4th wire. This wire value can only be fully computed once the first 3 wire polynomials
    // have been committed to. The 4th wire on these gates will be a random linear combination of the first 3 wires,
    // using the plookup challenge `eta`. Because we shift the gates by the number of public inputs, we need to update
    // the records with the public_inputs offset
    const uint32_t public_inputs_count = static_cast<uint32_t>(circuit_constructor.public_inputs.size());
    auto add_public_inputs_offset = [public_inputs_count](uint32_t gate_index) {
        return gate_index + public_inputs_count;
    };
    proving_key->memory_read_records = std::vector<uint32_t>();
    proving_key->memory_write_records = std::vector<uint32_t>();

    std::transform(circuit_constructor.memory_read_records.begin(),
                   circuit_constructor.memory_read_records.end(),
                   std::back_inserter(proving_key->memory_read_records),
                   add_public_inputs_offset);
    std::transform(circuit_constructor.memory_write_records.begin(),
                   circuit_constructor.memory_write_records.end(),
                   std::back_inserter(proving_key->memory_write_records),
                   add_public_inputs_offset);

    computed_witness = true;
}

template <UltraFlavor Flavor>
UltraProver_<Flavor> UltraComposer_<Flavor>::create_prover(CircuitBuilder& circuit_constructor)
{
    circuit_constructor.add_gates_to_ensure_all_polys_are_non_zero();
    circuit_constructor.finalize_circuit();

    for (const auto& table : circuit_constructor.lookup_tables) {
        tables_size += table.size;
        lookups_size += table.lookup_gates.size();
    }

    const size_t minimum_circuit_size = tables_size + lookups_size;

    num_public_inputs = circuit_constructor.public_inputs.size();
    const size_t num_constraints = circuit_constructor.num_gates + num_public_inputs;
    total_num_gates = std::max(minimum_circuit_size, num_constraints);
    dyadic_circuit_size = circuit_constructor.get_circuit_subgroup_size(total_num_gates);

    compute_proving_key(circuit_constructor);
    compute_witness(circuit_constructor);

    compute_commitment_key(proving_key->circuit_size, crs_factory_);

    UltraProver_<Flavor> output_state(proving_key, commitment_key);

    return output_state;
}

/**
 * Create verifier: compute verification key,
 * initialize verifier with it and an initial manifest and initialize commitment_scheme.
 *
 * @return The verifier.
 * */
template <UltraFlavor Flavor>
UltraVerifier_<Flavor> UltraComposer_<Flavor>::create_verifier(const CircuitBuilder& circuit_constructor)
{
    auto verification_key = compute_verification_key(circuit_constructor);

    UltraVerifier_<Flavor> output_state(verification_key);

    auto pcs_verification_key = std::make_unique<PCSVerificationKey>(verification_key->circuit_size, crs_factory_);

    output_state.pcs_verification_key = std::move(pcs_verification_key);

    return output_state;
}

template <UltraFlavor Flavor>
std::shared_ptr<typename Flavor::ProvingKey> UltraComposer_<Flavor>::compute_proving_key(
    const CircuitBuilder& circuit_constructor)
{
    if (proving_key) {
        return proving_key;
    }

    proving_key = std::make_shared<ProvingKey>(dyadic_circuit_size, num_public_inputs);

    construct_selector_polynomials<Flavor>(circuit_constructor, proving_key.get());

    compute_honk_generalized_sigma_permutations<Flavor>(circuit_constructor, proving_key.get());

    compute_first_and_last_lagrange_polynomials<Flavor>(proving_key.get());

    polynomial poly_q_table_column_1(dyadic_circuit_size);
    polynomial poly_q_table_column_2(dyadic_circuit_size);
    polynomial poly_q_table_column_3(dyadic_circuit_size);
    polynomial poly_q_table_column_4(dyadic_circuit_size);

    size_t offset = dyadic_circuit_size - tables_size;

    // Create lookup selector polynomials which interpolate each table column.
    // Our selector polys always need to interpolate the full subgroup size, so here we offset so as to
    // put the table column's values at the end. (The first gates are for non-lookup constraints).
    // [0, ..., 0, ...table, 0, 0, 0, x]
    //  ^^^^^^^^^  ^^^^^^^^  ^^^^^^^  ^nonzero to ensure uniqueness and to avoid infinity commitments
    //  |          table     randomness
    //  ignored, as used for regular constraints and padding to the next power of 2.

    for (size_t i = 0; i < offset; ++i) {
        poly_q_table_column_1[i] = 0;
        poly_q_table_column_2[i] = 0;
        poly_q_table_column_3[i] = 0;
        poly_q_table_column_4[i] = 0;
    }

    for (const auto& table : circuit_constructor.lookup_tables) {
        const fr table_index(table.table_index);

        for (size_t i = 0; i < table.size; ++i) {
            poly_q_table_column_1[offset] = table.column_1[i];
            poly_q_table_column_2[offset] = table.column_2[i];
            poly_q_table_column_3[offset] = table.column_3[i];
            poly_q_table_column_4[offset] = table_index;
            ++offset;
        }
    }

    // Polynomial memory is zeroed out when constructed with size hint, so we don't have to initialize trailing space

    proving_key->table_1 = poly_q_table_column_1;
    proving_key->table_2 = poly_q_table_column_2;
    proving_key->table_3 = poly_q_table_column_3;
    proving_key->table_4 = poly_q_table_column_4;

    proving_key->recursive_proof_public_input_indices =
        std::vector<uint32_t>(recursive_proof_public_input_indices.begin(), recursive_proof_public_input_indices.end());

    proving_key->contains_recursive_proof = contains_recursive_proof;

    return proving_key;
}

/**
 * Compute verification key consisting of selector precommitments.
 *
 * @return Pointer to created circuit verification key.
 * */
template <UltraFlavor Flavor>
std::shared_ptr<typename Flavor::VerificationKey> UltraComposer_<Flavor>::compute_verification_key(
    const CircuitBuilder& circuit_constructor)
{
    if (verification_key) {
        return verification_key;
    }

    if (!proving_key) {
        compute_proving_key(circuit_constructor);
    }

    verification_key =
        std::make_shared<typename Flavor::VerificationKey>(proving_key->circuit_size, proving_key->num_public_inputs);

    // Compute and store commitments to all precomputed polynomials
    verification_key->q_m = commitment_key->commit(proving_key->q_m);
    verification_key->q_l = commitment_key->commit(proving_key->q_l);
    verification_key->q_r = commitment_key->commit(proving_key->q_r);
    verification_key->q_o = commitment_key->commit(proving_key->q_o);
    verification_key->q_4 = commitment_key->commit(proving_key->q_4);
    verification_key->q_c = commitment_key->commit(proving_key->q_c);
    verification_key->q_arith = commitment_key->commit(proving_key->q_arith);
    verification_key->q_sort = commitment_key->commit(proving_key->q_sort);
    verification_key->q_elliptic = commitment_key->commit(proving_key->q_elliptic);
    verification_key->q_aux = commitment_key->commit(proving_key->q_aux);
    verification_key->q_lookup = commitment_key->commit(proving_key->q_lookup);
    verification_key->sigma_1 = commitment_key->commit(proving_key->sigma_1);
    verification_key->sigma_2 = commitment_key->commit(proving_key->sigma_2);
    verification_key->sigma_3 = commitment_key->commit(proving_key->sigma_3);
    verification_key->sigma_4 = commitment_key->commit(proving_key->sigma_4);
    verification_key->id_1 = commitment_key->commit(proving_key->id_1);
    verification_key->id_2 = commitment_key->commit(proving_key->id_2);
    verification_key->id_3 = commitment_key->commit(proving_key->id_3);
    verification_key->id_4 = commitment_key->commit(proving_key->id_4);
    verification_key->table_1 = commitment_key->commit(proving_key->table_1);
    verification_key->table_2 = commitment_key->commit(proving_key->table_2);
    verification_key->table_3 = commitment_key->commit(proving_key->table_3);
    verification_key->table_4 = commitment_key->commit(proving_key->table_4);
    verification_key->lagrange_first = commitment_key->commit(proving_key->lagrange_first);
    verification_key->lagrange_last = commitment_key->commit(proving_key->lagrange_last);

    // // See `add_recusrive_proof()` for how this recursive data is assigned.
    // verification_key->recursive_proof_public_input_indices =
    //     std::vector<uint32_t>(recursive_proof_public_input_indices.begin(),
    //     recursive_proof_public_input_indices.end());

    // verification_key->contains_recursive_proof = contains_recursive_proof;

    return verification_key;
}
template class UltraComposer_<honk::flavor::Ultra>;
template class UltraComposer_<honk::flavor::UltraGrumpkin>;

} // namespace proof_system::honk
