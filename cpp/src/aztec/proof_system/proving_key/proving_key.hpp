#pragma once
#include <ecc/curves/bn254/scalar_multiplication/runtime_states.hpp>
#include <map>
#include <polynomials/evaluation_domain.hpp>
#include <polynomials/polynomial.hpp>

#include <srs/reference_string/reference_string.hpp>
#include <plonk/proof_system/constants.hpp>
#include <proof_system/types/polynomial_manifest.hpp>
#include <unordered_map>

#include "../polynomial_cache/polynomial_cache.hpp"

namespace bonk {

struct proving_key_data {
    uint32_t composer_type;
    uint32_t circuit_size;
    uint32_t num_public_inputs;
    bool contains_recursive_proof;
    std::vector<uint32_t> recursive_proof_public_input_indices;
    std::vector<uint32_t> memory_read_records;
    std::vector<uint32_t> memory_write_records;
    PolynomialCache polynomial_cache;
};

struct proving_key {
  public:
    enum LookupType {
        NONE,
        ABSOLUTE_LOOKUP,
        RELATIVE_LOOKUP,
    };

    proving_key(proving_key_data&& data, std::shared_ptr<ProverReferenceString> const& crs);

    proving_key(const size_t num_gates,
                const size_t num_inputs,
                std::shared_ptr<ProverReferenceString> const& crs,
                plonk::ComposerType type);

    proving_key(std::ostream& is, std::string const& crs_path);

    void init();

    uint32_t composer_type;
    size_t circuit_size;
    size_t log_circuit_size;
    size_t num_public_inputs;
    bool contains_recursive_proof = false;
    std::vector<uint32_t> recursive_proof_public_input_indices;
    std::vector<uint32_t> memory_read_records;  // Used by UltraComposer only; for ROM, RAM reads.
    std::vector<uint32_t> memory_write_records; // Used by UltraComposer only, for RAM writes.

    // Note: low-memory prover functionality can be achieved by uncommenting the lines below
    // which allow the polynomial cache to write polynomials to file as necessary. Similar
    // lines must also be uncommented in constructor.
    // #ifdef __wasm__
    //     PolynomialStoreWasm underlying_store;
    // #endif
    PolynomialCache polynomial_cache;

    barretenberg::evaluation_domain small_domain;
    barretenberg::evaluation_domain large_domain;

    // The reference_string object contains monomial as well as lagrange SRS. We can access them using:
    // Monomial SRS: reference_string->get_monomial_points()
    // Lagrange SRS: reference_string->get_lagrange_points()
    std::shared_ptr<ProverReferenceString> reference_string;

    barretenberg::polynomial quotient_polynomial_parts[plonk::NUM_QUOTIENT_PARTS];

    barretenberg::scalar_multiplication::pippenger_runtime_state pippenger_runtime_state;

    PolynomialManifest polynomial_manifest;

    static constexpr size_t min_thread_block = 4UL;
};

} // namespace bonk
