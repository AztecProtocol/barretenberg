#include "polynomial_store_wasm.hpp"
#include "barretenberg/common/assert.hpp"
#include "barretenberg/env/data_store.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <unordered_map>

namespace proof_system {

template <typename Fr> void PolynomialStoreWasm<Fr>::put(std::string const& key, Polynomial&& value)
{
    // info("put ", key, ": ", value.hash());
    set_data(key.c_str(), (uint8_t*)value.data().get(), value.size() * sizeof(barretenberg::fr));
    size_map[key] = value.size();
};

/**
 * @brief Get a reference to a polynomial in the PolynomialStoreWasm; will throw exception if the
 * key does not exist in the map
 *
 * @param key string ID of the polynomial
 * @return Polynomial&; a reference to the polynomial associated with the given key
 */
template <typename Fr> barretenberg::Polynomial<Fr> PolynomialStoreWasm<Fr>::get(std::string const& key)
{
    size_t size = size_map.at(key);
    auto p = Polynomial(size);
    get_data(key.c_str(), (uint8_t*)p.data().get());
    // info("got ", key, ": ", p.hash());
    return p;
};

/**
 * @brief Erase the polynomial with the given key from the map if it exists. (ASSERT that it does)
 *
 * @param key
 */
template <typename Fr> void PolynomialStoreWasm<Fr>::remove(std::string const& key)
{
    ASSERT(size_map.contains(key));
    size_map.erase(key);
    set_data(key.c_str(), (uint8_t*)0, 0);
};

/**
 * @brief Get the current size (bytes) of all polynomials in the PolynomialStoreWasm
 *
 * @return size_t
 */
template <typename Fr> size_t PolynomialStoreWasm<Fr>::get_size_in_bytes() const
{
    size_t size_in_bytes = 0;
    for (auto& entry : size_map) {
        size_in_bytes += sizeof(Fr) * entry.second;
    }
    return size_in_bytes;
};

/**
 * @brief Print a summary of the PolynomialStoreWasm contents
 *
 */
template <typename Fr> void PolynomialStoreWasm<Fr>::print()
{
    double size_in_mb = static_cast<double>(get_size_in_bytes()) / 1e6;
    info("\n PolynomialStoreWasm contents (total size ", size_in_mb, " MB):");
    for (auto& entry : size_map) {
        size_t entry_bytes = entry.second * sizeof(Fr);
        info(entry.first, " (", entry_bytes, " bytes): \t", entry.second);
    }
    info();
}

template class PolynomialStoreWasm<barretenberg::fr>;

} // namespace proof_system