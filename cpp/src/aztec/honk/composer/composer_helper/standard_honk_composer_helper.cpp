
#include "standard_honk_composer_helper.hpp"
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
 * @param minimum_circuit_size Used as the total number of gates when larger than n + count of public inputs.
 * @param num_reserved_gates The number of reserved gates.
 * @return Pointer to the initialized proving key updated with selector polynomials.
 * */
template <typename CircuitConstructor>
std::shared_ptr<bonk::proving_key> StandardHonkComposerHelper<CircuitConstructor>::compute_proving_key_base(
    const CircuitConstructor& constructor, const size_t minimum_circuit_size, const size_t num_randomized_gates)
{
    // Initialize circuit_proving_key
    // TODO: replace composer types.
    circuit_proving_key = initialize_proving_key(constructor,
                                                 crs_factory_.get(),
                                                 minimum_circuit_size,
                                                 num_randomized_gates,
                                                 plonk::ComposerType::STANDARD_HONK);
    // Compute lagrange selectors
    put_selectors_in_polynomial_cache(constructor, circuit_proving_key.get());

    return circuit_proving_key;
}

/**
 * @brief Computes the verification key by computing the:
 * (1) commitments to the selector, permutation, and lagrange (first/last) polynomials,
 * (2) sets the polynomial manifest using the data from proving key.
 */

template <typename CircuitConstructor>
std::shared_ptr<bonk::verification_key> StandardHonkComposerHelper<CircuitConstructor>::compute_verification_key_base(
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
void StandardHonkComposerHelper<CircuitConstructor>::compute_witness_base(const CircuitConstructor& circuit_constructor,
                                                                          const size_t minimum_circuit_size)
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
std::shared_ptr<bonk::proving_key> StandardHonkComposerHelper<CircuitConstructor>::compute_proving_key(
    const CircuitConstructor& circuit_constructor)
{
    if (circuit_proving_key) {
        return circuit_proving_key;
    }
    // Compute q_l, q_r, q_o, etc polynomials
    StandardHonkComposerHelper::compute_proving_key_base(circuit_constructor, plonk::ComposerType::STANDARD_HONK);

    // Compute sigma polynomials (we should update that late)
    compute_standard_honk_sigma_permutations<CircuitConstructor::program_width>(circuit_constructor,
                                                                                circuit_proving_key.get());
    compute_standard_honk_id_polynomials<CircuitConstructor::program_width>(circuit_proving_key.get());

    compute_first_and_last_lagrange_polynomials(circuit_proving_key.get());

    return circuit_proving_key;
}

/**
 * Compute verification key consisting of selector precommitments.
 *
 * @return Pointer to created circuit verification key.
 * */
template <typename CircuitConstructor>
std::shared_ptr<bonk::verification_key> StandardHonkComposerHelper<CircuitConstructor>::compute_verification_key(
    const CircuitConstructor& circuit_constructor)
{
    if (circuit_verification_key) {
        return circuit_verification_key;
    }
    if (!circuit_proving_key) {
        compute_proving_key(circuit_constructor);
    }

    circuit_verification_key = StandardHonkComposerHelper::compute_verification_key_base(
        circuit_proving_key, crs_factory_->get_verifier_crs());
    circuit_verification_key->composer_type = circuit_proving_key->composer_type;

    return circuit_verification_key;
}

template <typename CircuitConstructor>
StandardVerifier ComposerHelper<CircuitConstructor>::create_verifier(const CircuitConstructor& circuit_constructor)
{
    compute_verification_key(circuit_constructor);
    StandardVerifier output_state(
        circuit_verification_key,
        honk::StandardHonk::create_manifest(circuit_constructor.public_inputs.size(),
                                            numeric::get_msb(circuit_verification_key->circuit_size)));

    // TODO(Cody): This should be more generic
    auto kate_verification_key = std::make_unique<pcs::kzg::VerificationKey>("../srs_db/ignition");

    output_state.kate_verification_key = std::move(kate_verification_key);

    return output_state;
}

template <typename CircuitConstructor>
template <typename Flavor>
// TODO(Cody): this file should be generic with regard to flavor/arithmetization/whatever.
StandardProver ComposerHelper<CircuitConstructor>::create_prover(const CircuitConstructor& circuit_constructor)
{
    compute_proving_key(circuit_constructor);
    compute_witness(circuit_constructor);

    size_t num_sumcheck_rounds(circuit_proving_key->log_circuit_size);
    auto manifest = Flavor::create_manifest(circuit_constructor.public_inputs.size(), num_sumcheck_rounds);
    StandardProver output_state(std::move(wire_polynomials), circuit_proving_key, manifest);

    // TODO(Cody): This should be more generic
    std::unique_ptr<pcs::kzg::CommitmentKey> kate_commitment_key =
        std::make_unique<pcs::kzg::CommitmentKey>(circuit_proving_key->circuit_size, "../srs_db/ignition");

    output_state.commitment_key = std::move(kate_commitment_key);

    return output_state;
}
template class ComposerHelper<StandardCircuitConstructor>;
template StandardProver ComposerHelper<StandardCircuitConstructor>::create_prover<StandardHonk>(
    const StandardCircuitConstructor& circuit_constructor);
} // namespace honk
