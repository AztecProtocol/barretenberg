#include "./polynomial_store_cache.hpp"

namespace proof_system {

PolynomialStoreCache::PolynomialStoreCache(size_t capacity_bytes)
    : capacity_bytes_(capacity_bytes)
{}

void PolynomialStoreCache::put(std::string const& key, Polynomial const& value)
{
    // info("cache put ", key);
    auto it = cache.find(key);
    if (it != cache.end()) {
        // info("cache put erase preexisting ", key);
        // Entry exists in cache already. Erase.
        lru.erase(it->second);
        cache.erase(it);
        internal_store.remove(key);
    }

    // Move polys to external until we have capacity.
    purge_until_free(value.size());

    // info("cache put to internal ", key);
    lru.push_front(key);
    cache[key] = lru.begin();
    internal_store.put(key, value);

    info("cache put: ",
         internal_store.get_size_in_bytes() / (1024 * 1024),
         "/",
         external_store.get_size_in_bytes() / (1024 * 1024));
};

PolynomialStoreCache::Polynomial PolynomialStoreCache::get(std::string const& key)
{
    auto it = cache.find(key);
    if (it != cache.end()) {
        // info("cache get hit ", key);
        // Already in internal store. Just bump to front of lru and return it.
        lru.splice(lru.begin(), lru, it->second);
        return internal_store.get(key);
    }

    // Move polys to external until we have capacity.
    size_t external_size = external_store.get_size_of(key);
    purge_until_free(external_size);

    // Move from external store to internal store.
    // info("cache get move from external to internal ", key);
    auto p = external_store.get(key);
    external_store.remove(key);
    lru.push_front(key);
    cache[key] = lru.begin();
    internal_store.put(key, p);

    info("cache get: ",
         internal_store.get_size_in_bytes() / (1024 * 1024),
         "/",
         external_store.get_size_in_bytes() / (1024 * 1024));
    return p;
};

void PolynomialStoreCache::purge_until_free(size_t bytes)
{
    while (internal_store.get_size_in_bytes() + bytes > capacity_bytes_) {
        auto least_key = lru.back();
        info("cache purging ", least_key);
        lru.pop_back();
        cache.erase(least_key);
        auto p = internal_store.get(least_key);
        internal_store.remove(least_key);
        external_store.put(least_key, p);
    }
}

void PolynomialStoreCache::remove(std::string const& key)
{
    auto it = cache.find(key);
    if (it != cache.end()) {
        lru.erase(it->second);
        cache.erase(it);
        internal_store.remove(key);
    } else {
        external_store.remove(key);
    }
};

size_t PolynomialStoreCache::get_size_in_bytes() const
{
    return internal_store.get_size_in_bytes() + external_store.get_size_in_bytes();
};

} // namespace proof_system