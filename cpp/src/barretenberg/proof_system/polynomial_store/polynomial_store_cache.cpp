#include "./polynomial_store_cache.hpp"

namespace proof_system {

PolynomialStoreCache::PolynomialStoreCache()
    : max_cache_size_(30)
{}

PolynomialStoreCache::PolynomialStoreCache(size_t max_cache_size)
    : max_cache_size_(max_cache_size)
{}

void PolynomialStoreCache::put(std::string const& key, Polynomial&& value)
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
    purge_until_free();

    // info("cache put to internal ", key);
    lru.push_front(key);
    cache[key] = lru.begin();
    internal_store.put(key, std::move(value));

    info("cache put: ",
         key,
         " ",
         value.size() * 32,
         " ",
         internal_store.get_size_in_bytes() / (1024ULL * 1024),
         "/",
         external_store.get_size_in_bytes() / (1024ULL * 1024));
};

PolynomialStoreCache::Polynomial PolynomialStoreCache::get(std::string const& key)
{
    auto it = cache.find(key);
    if (it != cache.end()) {
        info("cache get hit ", key);
        // Already in internal store. Just bump to front of lru and return it.
        // lru.splice(lru.begin(), lru, it->second);
        return internal_store.get(key);
    }

    // Move polys to external until we have capacity.
    purge_until_free();

    // Move from external store to internal store.
    // info("cache get move from external to internal ", key);
    auto p = external_store.get(key);
    external_store.remove(key);
    lru.push_front(key);
    cache[key] = lru.begin();
    internal_store.put(key, std::move(p));

    info("cache get miss: ",
         key,
         " ",
         internal_store.get_size_in_bytes() / (1024ULL * 1024),
         "/",
         external_store.get_size_in_bytes() / (1024ULL * 1024));
    return internal_store.get(key);
};

void PolynomialStoreCache::purge_until_free()
{
    while (lru.size() >= max_cache_size_) {
        auto least_key = lru.back();
        info("cache purging ", least_key);
        lru.pop_back();
        cache.erase(least_key);
        auto p = internal_store.get(least_key);
        internal_store.remove(least_key);
        external_store.put(least_key, std::move(p));
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