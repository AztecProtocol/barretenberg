#pragma once

#include "barretenberg/common/assert.hpp"
#include "barretenberg/common/timer.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/env/data_store.hpp"
#include <cstddef>
#include <initializer_list>
#include <map>
#include <stdexcept>
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
  private:
    using Polynomial = barretenberg::Polynomial<Fr>;
    std::unordered_map<std::string, size_t> polynomial_map;

  public:
    // PolynomialStore() = default;
    // PolynomialStore(PolynomialStore& other) noexcept = default;
    // PolynomialStore(PolynomialStore&& other) noexcept = default;
    // PolynomialStore& operator=(const PolynomialStore& other) = default;
    // PolynomialStore& operator=(PolynomialStore&& other) noexcept = default;
    // ~PolynomialStore() = default;

    inline void put(std::string const& key, Polynomial const& value)
    {
        Timer t;
        size_t size = value.size();
        polynomial_map[key] = size;
        set_data(key.c_str(), (uint8_t*)value.data().get(), size * sizeof(barretenberg::fr));
        // info("set_data: ", key, " ", t.seconds(), "s ", get_size_in_bytes() / (1024 * 1024), "MB");
    };

    inline Polynomial get(std::string const& key)
    {
        ASSERT(polynomial_map.contains(key));
        size_t size = polynomial_map[key];
        Timer t;
        Polynomial p(size);
        get_data(key.c_str(), (uint8_t*)p.data().get());
        // info("get_data: ", key, " ", t.seconds(), "s");
        return p;
    };

    // inline std::unordered_map<std::string, Polynomial> get_many(std::vector<std::string> const& keys)
    // {
    //     std::unordered_map<std::string, Polynomial> result;
    //     for (auto& k : keys) {
    //         result[k] = get(k);
    //     }
    //     return result;
    // }

    /**
     * @brief Erase the polynomial with the given key from the map if it exists. (ASSERT that it does)
     *
     * @param key
     */
    inline void remove(std::string const& key)
    {
        ASSERT(polynomial_map.contains(key));
        set_data(key.c_str(), (uint8_t*)0, 0);
        polynomial_map.erase(key);
    };

    /**
     * @brief Get the current size (bytes) of all polynomials in the PolynomialStore
     *
     * @return size_t
     */
    inline size_t get_size_in_bytes() const
    {
        size_t size_in_bytes = 0;
        for (auto& entry : polynomial_map) {
            size_in_bytes += sizeof(Fr) * entry.second;
        }
        return size_in_bytes;
    };

    inline size_t get_size_of(std::string const& key)
    {
        ASSERT(polynomial_map.contains(key));
        return polynomial_map[key];
    }

    /**
     * @brief Print a summary of the PolynomialStore contents
     *
     */
    // inline void print()
    // {
    //     double size_in_mb = static_cast<double>(get_size_in_bytes()) / 1e6;
    //     info("\n PolynomialStore contents (total size ", size_in_mb, " MB):");
    // }

    // Basic map methods
    // bool contains(std::string const& key) { return polynomial_map.contains(key); };
    // size_t size() { return polynomial_map.size(); };

    // // Allow for const range based for loop
    // typename std::unordered_map<std::string, size_t>::const_iterator begin() const { return polynomial_map.begin(); }
    // typename std::unordered_map<std::string, size_t>::const_iterator end() const { return polynomial_map.end(); }
};

} // namespace proof_system