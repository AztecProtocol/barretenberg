#include "standard_plonk_composer_helper.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/numeric/bitop/get_msb.hpp"
#include "barretenberg/plonk/proof_system/widgets/transition_widgets/arithmetic_widget.hpp"
#include "barretenberg/plonk/proof_system/widgets/random_widgets/permutation_widget.hpp"
#include "barretenberg/plonk/proof_system/commitment_scheme/kate_commitment_scheme.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace bonk {

/**
 * Compute proving key base.
 *
 * 1. Load crs.
 * 2. Initialize this.circuit_proving_key.
 * 3. Create constraint selector polynomials from each of this composer's `selectors` vectors and add them to the
 * proving key.
 *
 * N.B. Need to add the fix for coefficients
 *
 * @param minimum_circuit_size Used as the total number of gates when larger than n + count of public inputs.
 * @param num_reserved_gates The number of reserved gates.
 * @return Pointer to the initialized proving key updated with selector polynomials.
 * */
template <typename CircuitConstructor>
std::shared_ptr<bonk::proving_key> StandardPlonkComposerHelper<CircuitConstructor>::compute_proving_key_base(
    const CircuitConstructor& constructor, const size_t minimum_circuit_size, const size_t num_randomized_gates)
{

    // Initialize circuit_proving_key
    // TODO(#229)(Kesha): replace composer types.
    circuit_proving_key = initialize_proving_key(
        constructor, crs_factory_.get(), minimum_circuit_size, num_randomized_gates, plonk::ComposerType::STANDARD);
    // Compute lagrange selectors
    construct_lagrange_selector_forms(constructor, circuit_proving_key.get());
    // Compute selectors in monomial form
    compute_monomial_and_coset_selector_forms(circuit_proving_key.get(), standard_selector_properties());

    return circuit_proving_key;
}

/**
 * @brief Computes the verification key by computing the:
 * (1) commitments to the selector, permutation, and lagrange (first/last) polynomials,
 * (2) sets the polynomial manifest using the data from proving key.
 */

template <typename CircuitConstructor>
std::shared_ptr<bonk::verification_key> StandardPlonkComposerHelper<CircuitConstructor>::compute_verification_key_base(
    std::shared_ptr<bonk::proving_key> const& proving_key, std::shared_ptr<bonk::VerifierReferenceString> const& vrs)
{

    return compute_verification_key_base_common(proving_key, vrs);
}

/**
 * Compute witness polynomials (w_1, w_2, w_3, w_4).
 *
 * @details Fills 3 or 4 witness polynomials w_1, w_2, w_3, w_4 with the values of in-circuit variables. The beginning
 * of w_1, w_2 polynomials is filled with public_input values.
 * @return Witness with computed witness polynomials.
 *
 * @tparam Program settings needed to establish if w_4 is being used.
 * */
template <typename CircuitConstructor>
void StandardPlonkComposerHelper<CircuitConstructor>::compute_witness(const CircuitConstructor& circuit_constructor,
                                                                      const size_t minimum_circuit_size)
{

    if (computed_witness) {
        return;
    }
    auto wire_polynomial_evaluations =
        compute_witness_base(circuit_constructor, minimum_circuit_size, NUM_RANDOMIZED_GATES);

    for (size_t j = 0; j < program_width; ++j) {
        std::string index = std::to_string(j + 1);
        circuit_proving_key->polynomial_store.put("w_" + index + "_lagrange",
                                                  std::move(wire_polynomial_evaluations[j]));
    }
    computed_witness = true;
}

/**
 * Compute proving key.
 * Compute the polynomials q_l, q_r, etc. and sigma polynomial.
 *
 * @return Proving key with saved computed polynomials.
 * */

template <typename CircuitConstructor>
std::shared_ptr<bonk::proving_key> StandardPlonkComposerHelper<CircuitConstructor>::compute_proving_key(
    const CircuitConstructor& circuit_constructor)
{
    if (circuit_proving_key) {
        return circuit_proving_key;
    }
    // Compute q_l, q_r, q_o, etc polynomials
    StandardPlonkComposerHelper::compute_proving_key_base(circuit_constructor, plonk::ComposerType::STANDARD_HONK);

    // Compute sigma polynomials (we should update that late)
    bonk::compute_standard_plonk_sigma_permutations<CircuitConstructor::program_width>(circuit_constructor,
                                                                                       circuit_proving_key.get());

    circuit_proving_key->recursive_proof_public_input_indices =
        std::vector<uint32_t>(recursive_proof_public_input_indices.begin(), recursive_proof_public_input_indices.end());
    // What does this line do exactly?
    circuit_proving_key->contains_recursive_proof = contains_recursive_proof;
    return circuit_proving_key;
}

/**
 * Compute verification key consisting of selector precommitments.
 *
 * @return Pointer to created circuit verification key.
 * */
template <typename CircuitConstructor>
std::shared_ptr<bonk::verification_key> StandardPlonkComposerHelper<CircuitConstructor>::compute_verification_key(
    const CircuitConstructor& circuit_constructor)
{
    if (circuit_verification_key) {
        return circuit_verification_key;
    }
    if (!circuit_proving_key) {
        compute_proving_key(circuit_constructor);
    }

    circuit_verification_key = StandardPlonkComposerHelper::compute_verification_key_base(
        circuit_proving_key, crs_factory_->get_verifier_crs());
    circuit_verification_key->composer_type = circuit_proving_key->composer_type;
    circuit_verification_key->recursive_proof_public_input_indices =
        std::vector<uint32_t>(recursive_proof_public_input_indices.begin(), recursive_proof_public_input_indices.end());
    circuit_verification_key->contains_recursive_proof = contains_recursive_proof;

    return circuit_verification_key;
}

/**
 * Create verifier: compute verification key,
 * initialize verifier with it and an initial manifest and initialize commitment_scheme.
 *
 * @return The verifier.
 * */
// TODO(Cody): This should go away altogether.
template <typename CircuitConstructor>
plonk::Verifier StandardPlonkComposerHelper<CircuitConstructor>::create_verifier(
    const CircuitConstructor& circuit_constructor)
{
    auto verification_key = compute_verification_key(circuit_constructor);

    plonk::Verifier output_state(circuit_verification_key, create_manifest(circuit_constructor.public_inputs.size()));

    std::unique_ptr<plonk::KateCommitmentScheme<plonk::standard_settings>> kate_commitment_scheme =
        std::make_unique<plonk::KateCommitmentScheme<plonk::standard_settings>>();

    output_state.commitment_scheme = std::move(kate_commitment_scheme);

    return output_state;
}

/**
 * Create prover.
 *  1. Compute the starting polynomials (q_l, etc, sigma, witness polynomials).
 *  2. Initialize StandardProver with them.
 *  3. Add Permutation and arithmetic widgets to the prover.
 *  4. Add KateCommitmentScheme to the prover.
 *
 * @return Initialized prover.
 * */
template <typename CircuitConstructor>
plonk::Prover StandardPlonkComposerHelper<CircuitConstructor>::create_prover(
    const CircuitConstructor& circuit_constructor)
{
    // Compute q_l, etc. and sigma polynomials.
    compute_proving_key(circuit_constructor);

    // Compute witness polynomials.
    compute_witness(circuit_constructor);

    plonk::Prover output_state(circuit_proving_key, create_manifest(circuit_constructor.public_inputs.size()));

    std::unique_ptr<plonk::ProverPermutationWidget<3, false>> permutation_widget =
        std::make_unique<plonk::ProverPermutationWidget<3, false>>(circuit_proving_key.get());

    std::unique_ptr<plonk::ProverArithmeticWidget<plonk::standard_settings>> arithmetic_widget =
        std::make_unique<plonk::ProverArithmeticWidget<plonk::standard_settings>>(circuit_proving_key.get());

    output_state.random_widgets.emplace_back(std::move(permutation_widget));
    output_state.transition_widgets.emplace_back(std::move(arithmetic_widget));

    std::unique_ptr<plonk::KateCommitmentScheme<plonk::standard_settings>> kate_commitment_scheme =
        std::make_unique<plonk::KateCommitmentScheme<plonk::standard_settings>>();

    output_state.commitment_scheme = std::move(kate_commitment_scheme);

    return output_state;
}

template class StandardPlonkComposerHelper<StandardCircuitConstructor>;
} // namespace bonk
