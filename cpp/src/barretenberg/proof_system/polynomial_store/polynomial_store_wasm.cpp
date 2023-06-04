#include "polynomial_store_wasm.hpp"
#include "barretenberg/common/assert.hpp"
#include "barretenberg/env/data_store.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <unordered_map>

namespace proof_system {

template <typename Fr>
PolynomialStoreWasm<Fr>::PolynomialStoreWasm()
    : max_cache_size_(0)
{}

template <typename Fr> void PolynomialStoreWasm<Fr>::put(std::string const& key, Polynomial&& value)
{
    // info("put ", key, ": ", value.hash());
    set_data(key.c_str(), (uint8_t*)value.data().get(), value.size() * sizeof(barretenberg::fr));
    size_map[key] = value.size();

    if (max_cache_size_ == 0) {
        return;
    }

    // Find the key in the cache.
    auto it = std::find_if(cache_.rbegin(), cache_.rend(), [&key](const auto& p) { return p.first == key; });

    if (it != cache_.rend()) {
        // Key found. Update the value.
        it->second = std::move(value);

        // Move the found element to the end of the vector.
        std::rotate(it.base() - 1, it.base(), cache_.end());
    } else {
        // Key not found. Add new value.
        if (cache_.size() >= max_cache_size_) {
            // Cache is at max capacity. Remove the least recently used Polynomial.
            cache_.erase(cache_.begin());
        }

        // Add the new key-value pair to the end of the cache.
        cache_.push_back({ key, std::move(value) });
    }
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
    // Find the key in the cache.
    auto it = std::find_if(cache_.rbegin(), cache_.rend(), [&key](const auto& p) { return p.first == key; });

    if (it != cache_.rend()) {
        // Move the found element to the end of the vector.
        // std::rotate(it.base() - 1, it.base(), cache_.end());
        // it = cache_.rbegin();
        auto p = it->second.clone();
        // info("got (hit): ", key);
        return p;
    }

    size_t size = size_map.at(key);
    auto fetched = Polynomial(size);
    get_data(key.c_str(), (uint8_t*)fetched.data().get());
    // info("got (miss): ", key);

    if (max_cache_size_ == 0) {
        return fetched;
    }

    // Ensure the cache doesn't exceed MAX_LENGTH
    if (cache_.size() >= max_cache_size_) {
        cache_.erase(cache_.begin());
    }

    // Move it to the back of the cache.
    cache_.push_back({ key, std::move(fetched) });
    auto p = cache_.back().second.clone();
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