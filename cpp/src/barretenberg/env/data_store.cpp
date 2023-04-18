// Native implementation. Wasm implementation must be implemented by host environment.
#ifndef __wasm__
#include "data_store.hpp"
#include <string>
#include <map>
#include <vector>
#include <cstring>

namespace {
std::map<std::string, std::vector<uint8_t>> store;
}

void set_data(char const* key, uint8_t* addr, size_t length)
{
    std::string k = key;
    store[k] = std::vector<uint8_t>(addr, addr + length);
}

uint8_t* get_data(char const* key, size_t* length_out)
{
    std::string k = key;
    if (store.contains(key)) {
        *length_out = store[k].size();
        auto* ptr = aligned_alloc(64, *length_out);
        std::memcpy(ptr, store[k].data(), *length_out);
    }
    *length_out = 0;
    return nullptr;
}

#endif