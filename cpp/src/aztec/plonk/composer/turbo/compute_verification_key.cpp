#include "compute_verification_key.hpp"
#include <ecc/curves/bn254/scalar_multiplication/scalar_multiplication.hpp>
#include <plonk/proof_system/proving_key/proving_key.hpp>
#include <plonk/proof_system/verification_key/verification_key.hpp>
#include <plonk/proof_system/types/polynomial_manifest.hpp>
#include <plonk/reference_string/reference_string.hpp>

namespace waffle {
namespace turbo_composer {

std::shared_ptr<verification_key> compute_verification_key(std::shared_ptr<proving_key> const& circuit_proving_key,
                                                           std::shared_ptr<VerifierReferenceString> const& vrs)
{
    std::array<barretenberg::fr*, 15> poly_coefficients;
    poly_coefficients[0] = circuit_proving_key->polynomial_cache.get("q_1").get_coefficients();
    poly_coefficients[1] = circuit_proving_key->polynomial_cache.get("q_2").get_coefficients();
    poly_coefficients[2] = circuit_proving_key->polynomial_cache.get("q_3").get_coefficients();
    poly_coefficients[3] = circuit_proving_key->polynomial_cache.get("q_4").get_coefficients();
    poly_coefficients[4] = circuit_proving_key->polynomial_cache.get("q_5").get_coefficients();
    poly_coefficients[5] = circuit_proving_key->polynomial_cache.get("q_m").get_coefficients();
    poly_coefficients[6] = circuit_proving_key->polynomial_cache.get("q_c").get_coefficients();
    poly_coefficients[7] = circuit_proving_key->polynomial_cache.get("q_arith").get_coefficients();
    poly_coefficients[8] = circuit_proving_key->polynomial_cache.get("q_ecc_1").get_coefficients();
    poly_coefficients[9] = circuit_proving_key->polynomial_cache.get("q_range").get_coefficients();
    poly_coefficients[10] = circuit_proving_key->polynomial_cache.get("q_logic").get_coefficients();
    poly_coefficients[11] = circuit_proving_key->polynomial_cache.get("sigma_1").get_coefficients();
    poly_coefficients[12] = circuit_proving_key->polynomial_cache.get("sigma_2").get_coefficients();
    poly_coefficients[13] = circuit_proving_key->polynomial_cache.get("sigma_3").get_coefficients();
    poly_coefficients[14] = circuit_proving_key->polynomial_cache.get("sigma_4").get_coefficients();

    std::vector<barretenberg::g1::affine_element> commitments;
    commitments.resize(15);

    for (size_t i = 0; i < 15; ++i) {
        commitments[i] =
            barretenberg::scalar_multiplication::pippenger(poly_coefficients[i],
                                                           circuit_proving_key->reference_string->get_monomials(),
                                                           circuit_proving_key->n,
                                                           circuit_proving_key->pippenger_runtime_state);
    }

    auto circuit_verification_key = std::make_shared<verification_key>(
        circuit_proving_key->n, circuit_proving_key->num_public_inputs, vrs, circuit_proving_key->composer_type);

    circuit_verification_key->constraint_selectors.insert({ "Q_1", commitments[0] });
    circuit_verification_key->constraint_selectors.insert({ "Q_2", commitments[1] });
    circuit_verification_key->constraint_selectors.insert({ "Q_3", commitments[2] });
    circuit_verification_key->constraint_selectors.insert({ "Q_4", commitments[3] });
    circuit_verification_key->constraint_selectors.insert({ "Q_5", commitments[4] });
    circuit_verification_key->constraint_selectors.insert({ "Q_M", commitments[5] });
    circuit_verification_key->constraint_selectors.insert({ "Q_C", commitments[6] });
    circuit_verification_key->constraint_selectors.insert({ "Q_ARITHMETIC_SELECTOR", commitments[7] });
    circuit_verification_key->constraint_selectors.insert({ "Q_FIXED_BASE_SELECTOR", commitments[8] });
    circuit_verification_key->constraint_selectors.insert({ "Q_RANGE_SELECTOR", commitments[9] });
    circuit_verification_key->constraint_selectors.insert({ "Q_LOGIC_SELECTOR", commitments[10] });

    circuit_verification_key->permutation_selectors.insert({ "SIGMA_1", commitments[11] });
    circuit_verification_key->permutation_selectors.insert({ "SIGMA_2", commitments[12] });
    circuit_verification_key->permutation_selectors.insert({ "SIGMA_3", commitments[13] });
    circuit_verification_key->permutation_selectors.insert({ "SIGMA_4", commitments[14] });

    return circuit_verification_key;
}

} // namespace turbo_composer
} // namespace waffle