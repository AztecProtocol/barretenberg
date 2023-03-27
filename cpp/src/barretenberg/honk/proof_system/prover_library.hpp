#pragma once
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/proof_system/proving_key/proving_key.hpp"
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/plonk/proof_system/types/program_settings.hpp"

namespace honk::prover_library {

using Fr = barretenberg::fr;
using Polynomial = barretenberg::Polynomial<Fr>;

template <size_t program_width>
Polynomial compute_permutation_grand_product(std::shared_ptr<bonk::proving_key>& key,
                                             std::vector<Polynomial>& wire_polynomials,
                                             Fr beta,
                                             Fr gamma);

template <size_t program_width>
Polynomial compute_lookup_grand_product_polynomial(auto& key, Fr eta, Fr beta, Fr gamma);

} // namespace honk::prover_library
