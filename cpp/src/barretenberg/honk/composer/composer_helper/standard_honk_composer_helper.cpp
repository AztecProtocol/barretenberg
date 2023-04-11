#include "standard_honk_composer_helper.hpp"
#include "barretenberg/plonk/proof_system/proving_key/proving_key.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/honk/flavor/flavor.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/numeric/bitop/get_msb.hpp"
#include "barretenberg/srs/reference_string/reference_string.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace proof_system::honk {

/**
 * Compute proving key base.
 *
 * 1. Load crs.
 * 2. Initialize this.proving_key.
 * 3. Create constraint selector polynomials from each of this composer's `selectors` vectors and add them to the
 * proving key.
 *
 * @param minimum_circuit_size Used as the total number of gates when larger than n + count of public inputs.
 * @param num_reserved_gates The number of reserved gates.
 * @return Pointer to the initialized proving key updated with selector polynomials.
 * */
std::shared_ptr<StandardHonkComposerHelper::ProvingKey> StandardHonkComposerHelper::compute_proving_key_base(
    const CircuitConstructor& constructor, const size_t minimum_circuit_size, const size_t num_randomized_gates)
{
    // Initialize proving_key
    // TODO(#229)(Kesha): replace composer types.
    proving_key = initialize_proving_key<Flavor>(
        constructor, crs_factory_.get(), minimum_circuit_size, num_randomized_gates, ComposerType::STANDARD_HONK);
    // Compute lagrange selectors
    construct_selector_polynomials<Flavor>(constructor, proving_key.get());

    return proving_key;
}

/**
 * @brief Computes the verification key by computing the:
 * (1) commitments to the selector, permutation, and lagrange (first/last) polynomials,
 * (2) sets the polynomial manifest using the data from proving key.
 */

std::shared_ptr<plonk::verification_key> StandardHonkComposerHelper::compute_verification_key_base(
    std::shared_ptr<StandardHonkComposerHelper::ProvingKey> const& proving_key,
    std::shared_ptr<VerifierReferenceString> const& vrs)
{
    auto key = std::make_shared<plonk::verification_key>(
        proving_key->circuit_size, proving_key->num_public_inputs, vrs, proving_key->composer_type);
    // TODO(kesha): Dirty hack for now. Need to actually make commitment-agnositc
    auto commitment_key = pcs::kzg::CommitmentKey(proving_key->circuit_size, "../srs_db/ignition");

    // // Compute and store commitments to all precomputed polynomials
    // key->commitments["Q_M"] = commitment_key.commit(proving_key->polynomial_store.get("q_m_lagrange"));
    // key->commitments["Q_1"] = commitment_key.commit(proving_key->polynomial_store.get("q_1_lagrange"));
    // key->commitments["Q_2"] = commitment_key.commit(proving_key->polynomial_store.get("q_2_lagrange"));
    // key->commitments["Q_3"] = commitment_key.commit(proving_key->polynomial_store.get("q_3_lagrange"));
    // key->commitments["Q_C"] = commitment_key.commit(proving_key->polynomial_store.get("q_c_lagrange"));
    // key->commitments["SIGMA_1"] = commitment_key.commit(proving_key->polynomial_store.get("sigma_1_lagrange"));
    // key->commitments["SIGMA_2"] = commitment_key.commit(proving_key->polynomial_store.get("sigma_2_lagrange"));
    // key->commitments["SIGMA_3"] = commitment_key.commit(proving_key->polynomial_store.get("sigma_3_lagrange"));
    // key->commitments["ID_1"] = commitment_key.commit(proving_key->polynomial_store.get("id_1_lagrange"));
    // key->commitments["ID_2"] = commitment_key.commit(proving_key->polynomial_store.get("id_2_lagrange"));
    // key->commitments["ID_3"] = commitment_key.commit(proving_key->polynomial_store.get("id_3_lagrange"));
    // key->commitments["LAGRANGE_FIRST"] =
    // commitment_key.commit(proving_key->polynomial_store.get("L_first_lagrange")); key->commitments["LAGRANGE_LAST"] =
    // commitment_key.commit(proving_key->polynomial_store.get("L_last_lagrange"));

    return key;
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
void StandardHonkComposerHelper::compute_witness(const CircuitConstructor& circuit_constructor,
                                                 const size_t minimum_circuit_size)
{
    if (computed_witness) {
        return;
    }
    wire_polynomials =
        construct_wire_polynomials_base<Flavor>(circuit_constructor, minimum_circuit_size, NUM_RANDOMIZED_GATES);

    computed_witness = true;
}

/**
 * Compute proving key.
 * Compute the polynomials q_l, q_r, etc. and sigma polynomial.
 *
 * @return Proving key with saved computed polynomials.
 * */

std::shared_ptr<StandardHonkComposerHelper::ProvingKey> StandardHonkComposerHelper::compute_proving_key(
    const CircuitConstructor& circuit_constructor)
{
    if (proving_key) {
        return proving_key;
    }
    // Compute q_l, q_r, q_o, etc polynomials
    // WORKTODO(Cody): meaningless constructor here
    StandardHonkComposerHelper::compute_proving_key_base(circuit_constructor, ComposerType::STANDARD_HONK);

    // Compute sigma polynomials (we should update that late)
    compute_standard_honk_sigma_permutations<Flavor>(circuit_constructor, proving_key.get());
    compute_standard_honk_id_polynomials<Flavor>(proving_key.get());

    compute_first_and_last_lagrange_polynomials(proving_key.get());

    return proving_key;
}

/**
 * Compute verification key consisting of selector precommitments.
 *
 * @return Pointer to created circuit verification key.
 * */
std::shared_ptr<plonk::verification_key> StandardHonkComposerHelper::compute_verification_key(
    const CircuitConstructor& circuit_constructor)
{
    if (circuit_verification_key) {
        return circuit_verification_key;
    }
    if (!proving_key) {
        compute_proving_key(circuit_constructor);
    }

    circuit_verification_key =
        StandardHonkComposerHelper::compute_verification_key_base(proving_key, crs_factory_->get_verifier_crs());
    circuit_verification_key->composer_type = proving_key->composer_type;

    return circuit_verification_key;
}

StandardVerifier StandardHonkComposerHelper::create_verifier(const CircuitConstructor& circuit_constructor)
{
    compute_verification_key(circuit_constructor);
    StandardVerifier output_state(circuit_verification_key);

    // TODO(Cody): This should be more generic
    auto kate_verification_key = std::make_unique<pcs::kzg::VerificationKey>("../srs_db/ignition");

    output_state.kate_verification_key = std::move(kate_verification_key);

    return output_state;
}

StandardProver StandardHonkComposerHelper::create_prover(const CircuitConstructor& circuit_constructor)
{
    compute_proving_key(circuit_constructor);
    compute_witness(circuit_constructor);

    StandardProver output_state(std::move(wire_polynomials), proving_key);

    return output_state;
}
} // namespace proof_system::honk
