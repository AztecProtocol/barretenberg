#pragma once

#include "barretenberg/common/assert.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <unordered_map>

namespace proof_system {
/**
 * @brief Basic storage class for Polynomials
 *
 * @tparam Fr
 */
// TODO(Cody): Move into plonk namespace.
template <typename Fr> class PolynomialStoreWasm {

    using Polynomial = barretenberg::Polynomial<Fr>;

  private:
    std::unordered_map<std::string, size_t> size_map;

  public:
    PolynomialStoreWasm() = default;
    PolynomialStoreWasm(PolynomialStoreWasm& other) noexcept = default;
    PolynomialStoreWasm(PolynomialStoreWasm&& other) noexcept = default;
    PolynomialStoreWasm& operator=(const PolynomialStoreWasm& other) = default;
    PolynomialStoreWasm& operator=(PolynomialStoreWasm&& other) noexcept = default;
    ~PolynomialStoreWasm() = default;

    /**
     * @brief Transfer ownership of a polynomial to the PolynomialStore
     *
     * @param key string ID of the polynomial
     * @param value a Polynomial
     */
    void put(std::string const& key, Polynomial&& value);

    /**
     * @brief Get a reference to a polynomial in the PolynomialStore; will throw exception if the
     * key does not exist in the map
     *
     * @param key string ID of the polynomial
     * @return Polynomial&; a reference to the polynomial associated with the given key
     */
    Polynomial get(std::string const& key);

    /**
     * @brief Erase the polynomial with the given key from the map if it exists. (ASSERT that it does)
     *
     * @param key
     */
    void remove(std::string const& key);

    /**
     * @brief Get the current size (bytes) of all polynomials in the PolynomialStore
     *
     * @return size_t
     */
    size_t get_size_in_bytes() const;

    /**
     * @brief Print a summary of the PolynomialStore contents
     *
     */
    void print();

    // Basic map methods
    bool contains(std::string const& key) { return size_map.contains(key); };
    size_t size() { return size_map.size(); };

    // Allow for const range based for loop
    // typename std::unordered_map<std::string, Polynomial>::const_iterator begin() const { return size_map.begin(); }
    // typename std::unordered_map<std::string, Polynomial>::const_iterator end() const { return size_map.end(); }
};

extern template class PolynomialStoreWasm<barretenberg::fr>;

} // namespace proof_system