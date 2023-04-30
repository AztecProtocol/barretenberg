#pragma once
#include "barretenberg/polynomials/polynomial.hpp"
#include "./polynomial_store.hpp"
#include "./polynomial_store_wasm.hpp"
#include <cstddef>
#include <map>
#include <list>
#include <string>
#include <unordered_map>
#include <limits>

namespace proof_system {

class PolynomialStoreCache {
  private:
    using Polynomial = barretenberg::Polynomial<barretenberg::fr>;
    std::unordered_map<std::string, std::list<std::string>::iterator> cache;
    std::list<std::string> lru;
    PolynomialStore<barretenberg::fr> internal_store;
    PolynomialStoreWasm<barretenberg::fr> external_store;
    size_t capacity_bytes_;

  public:
    explicit PolynomialStoreCache(size_t capacity_bytes = std::numeric_limits<size_t>::max());

    void put(std::string const& key, Polynomial const& value);

    Polynomial get(std::string const& key);

    void remove(std::string const& key);

    size_t get_size_in_bytes() const;

  private:
    void purge_until_free(size_t bytes);
};

} // namespace proof_system