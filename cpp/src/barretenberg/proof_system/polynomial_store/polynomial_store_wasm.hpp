#pragma once

#include "barretenberg/common/assert.hpp"
#include "barretenberg/common/timer.hpp"
#include "barretenberg/env/data_store.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include <cstddef>
#include <initializer_list>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace proof_system {

/**
 * Stores polynomials outside of the wasm memory space, in the host environment.
 * Enables us to overcome the 4GB wasm memory limit.
 */
template <typename Fr> class PolynomialStoreWasm {
  private:
    using Polynomial = barretenberg::Polynomial<Fr>;
    std::unordered_map<std::string, size_t> polynomial_map;

  public:
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

    inline void remove(std::string const& key)
    {
        if (!polynomial_map.contains(key)) {
            return;
        }
        set_data(key.c_str(), (uint8_t*)0, 0);
        polynomial_map.erase(key);
    };

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
};

} // namespace proof_system