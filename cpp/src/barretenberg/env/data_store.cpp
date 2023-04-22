// Native implementation. Wasm implementation must be implemented by host environment.
#ifndef __wasm__
#include "data_store.hpp"
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <barretenberg/common/log.hpp>

namespace {
std::map<std::string, std::vector<uint8_t>> store;
}

extern "C" {

void set_data(char const* key, uint8_t const* addr, size_t length)
{
    std::string k = key;
    // info("set data: ", key, " length: ", length);
    store[k] = std::vector<uint8_t>(addr, addr + length);
}

void get_data(char const* key, uint8_t* out_buf)
{
    std::string k = key;
    if (store.contains(key)) {
        // info("get data hit: ", key, " length: ", *length_out, " ptr ", (void*)ptr);
        std::memcpy(out_buf, store[k].data(), store[k].size());
    }
    // info("get data miss: ", key);
    // return nullptr;
}
}

#endif