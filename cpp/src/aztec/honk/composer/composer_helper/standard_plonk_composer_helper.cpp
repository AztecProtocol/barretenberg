#include "standard_plonk_composer_helper.hpp"
#include <polynomials/polynomial.hpp>
#include <proof_system/flavor/flavor.hpp>
#include <honk/pcs/commitment_key.hpp>
#include <numeric/bitop/get_msb.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

namespace honk {

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
std::shared_ptr<waffle::proving_key> StandardPlonkComposerHelper<CircuitConstructor>::compute_proving_key_base(
    const CircuitConstructor& constructor, const size_t minimum_circuit_size, const size_t num_randomized_gates)
{

    // Initialize circuit_proving_key
    // TODO: replace composer types.
    circuit_proving_key = initialize_proving_key(
        constructor, crs_factory_.get(), minimum_circuit_size, num_randomized_gates, waffle::ComposerType::STANDARD);
    // Compute lagrange selectors
    put_selectors_in_polynomial_cache(constructor, circuit_proving_key.get());
    // Compute selectors in monomial form
    compute_monomial_selector_forms_and_put_into_cache(circuit_proving_key.get(), standard_selector_properties());

    return circuit_proving_key;
}

/**
 * @brief Computes the verification key by computing the:
 * (1) commitments to the selector, permutation, and lagrange (first/last) polynomials,
 * (2) sets the polynomial manifest using the data from proving key.
 */

template <typename CircuitConstructor>
std::shared_ptr<waffle::verification_key> StandardPlonkComposerHelper<
    CircuitConstructor>::compute_verification_key_base(std::shared_ptr<waffle::proving_key> const& proving_key,
                                                       std::shared_ptr<waffle::VerifierReferenceString> const& vrs)
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
template <size_t program_width>
void StandardPlonkComposerHelper<CircuitConstructor>::compute_witness_base(
    const CircuitConstructor& circuit_constructor, const size_t minimum_circuit_size)
{

    if (computed_witness) {
        return;
    }
    compute_witness_base_common(
        circuit_constructor, minimum_circuit_size, NUM_RANDOMIZED_GATES, circuit_proving_key.get());

    computed_witness = true;
}

/**
 * Compute proving key.
 * Compute the polynomials q_l, q_r, etc. and sigma polynomial.
 *
 * @return Proving key with saved computed polynomials.
 * */

template <typename CircuitConstructor>
std::shared_ptr<waffle::proving_key> StandardPlonkComposerHelper<CircuitConstructor>::compute_proving_key(
    const CircuitConstructor& circuit_constructor)
{
    if (circuit_proving_key) {
        return circuit_proving_key;
    }
    // Compute q_l, q_r, q_o, etc polynomials
    StandardPlonkComposerHelper::compute_proving_key_base(circuit_constructor, waffle::ComposerType::STANDARD_HONK);

    // Compute sigma polynomials (we should update that late)
    compute_standard_plonk_sigma_permutations<CircuitConstructor::program_width>(circuit_constructor,
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
std::shared_ptr<waffle::verification_key> StandardPlonkComposerHelper<CircuitConstructor>::compute_verification_key(
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
StandardVerifier StandardPlonkComposerHelper<CircuitConstructor>::create_verifier(
    const CircuitConstructor& circuit_constructor)
{
    auto verification_key = compute_verification_key(circuit_constructor);
    // TODO figure out types, actually
    // circuit_verification_key->composer_type = type;

    // TODO: initialize verifier according to manifest and key
    // Verifier output_state(circuit_verification_key, create_manifest(public_inputs.size()));
    StandardVerifier output_state;
    // TODO: Do we need a commitment scheme defined here?
    // std::unique_ptr<KateCommitmentScheme<standard_settings>> kate_commitment_scheme =
    //    std::make_unique<KateCommitmentScheme<standard_settings>>();

    // output_state.commitment_scheme = std::move(kate_commitment_scheme);

    return output_state;
}

template <typename CircuitConstructor>
StandardUnrolledVerifier StandardPlonkComposerHelper<CircuitConstructor>::create_unrolled_verifier(
    const CircuitConstructor& circuit_constructor)
{
    compute_verification_key(circuit_constructor);
    StandardUnrolledVerifier output_state(
        circuit_verification_key,
        honk::StandardHonk::create_unrolled_manifest(circuit_constructor.public_inputs.size(),
                                                     numeric::get_msb(circuit_verification_key->circuit_size)));

    // TODO(Cody): This should be more generic
    auto kate_verification_key = std::make_unique<pcs::kzg::VerificationKey>("../srs_db/ignition");

    output_state.kate_verification_key = std::move(kate_verification_key);

    return output_state;
}

template <typename CircuitConstructor>
template <typename Flavor>
// TODO(Cody): this file should be generic with regard to flavor/arithmetization/whatever.
StandardUnrolledProver StandardPlonkComposerHelper<CircuitConstructor>::create_unrolled_prover(
    const CircuitConstructor& circuit_constructor)
{
    compute_proving_key(circuit_constructor);
    compute_witness(circuit_constructor);

    size_t num_sumcheck_rounds(circuit_proving_key->log_circuit_size);
    auto manifest = Flavor::create_unrolled_manifest(circuit_constructor.public_inputs.size(), num_sumcheck_rounds);
    StandardUnrolledProver output_state(circuit_proving_key, manifest);

    // TODO(Cody): This should be more generic
    std::unique_ptr<pcs::kzg::CommitmentKey> kate_commitment_key =
        std::make_unique<pcs::kzg::CommitmentKey>(circuit_proving_key->circuit_size, "../srs_db/ignition");

    output_state.commitment_key = std::move(kate_commitment_key);

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
StandardProver StandardPlonkComposerHelper<CircuitConstructor>::create_prover(
    const CircuitConstructor& circuit_constructor)
{
    // Compute q_l, etc. and sigma polynomials.
    compute_proving_key(circuit_constructor);

    // Compute witness polynomials.
    compute_witness(circuit_constructor);
    // TODO: Initialize prover properly
    // Prover output_state(circuit_proving_key, create_manifest(public_inputs.size()));
    StandardProver output_state(circuit_proving_key);
    // Initialize constraints

    // std::unique_ptr<ProverPermutationWidget<3, false>> permutation_widget =
    //     std::make_unique<ProverPermutationWidget<3, false>>(circuit_proving_key.get());

    // std::unique_ptr<ProverArithmeticWidget<standard_settings>> arithmetic_widget =
    //     std::make_unique<ProverArithmeticWidget<standard_settings>>(circuit_proving_key.get());

    // output_state.random_widgets.emplace_back(std::move(permutation_widget));
    // output_state.transition_widgets.emplace_back(std::move(arithmetic_widget));

    // Is commitment scheme going to stay a part of the prover? Why is it here?
    // std::unique_ptr<KateCommitmentScheme<standard_settings>> kate_commitment_scheme =
    //    std::make_unique<KateCommitmentScheme<standard_settings>>();

    // output_state.commitment_scheme = std::move(kate_commitment_scheme);

    return output_state;
}

template class StandardPlonkComposerHelper<StandardCircuitConstructor>;
template StandardUnrolledProver StandardPlonkComposerHelper<StandardCircuitConstructor>::create_unrolled_prover<
    StandardHonk>(const StandardCircuitConstructor& circuit_constructor);
} // namespace honk
