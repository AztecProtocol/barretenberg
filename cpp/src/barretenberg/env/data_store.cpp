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
    info("set data: ", key, " length: ", length);
    store[k] = std::vector<uint8_t>(addr, addr + length);
}

uint8_t* get_data(char const* key, size_t* length_out)
{
    std::string k = key;
    if (store.contains(key)) {
        *length_out = store[k].size();
        // Aligning to 32 bytes, because at least at present this is just used by the polynomial store.
        // The poly store holds fields aligned to 32 bytes.
        // Also however, it has some capacity nonsense that means right now we're taking a copy of this memory
        // in the polynomial rather than taking ownership of it, and then releasing this memory. A bit inefficient
        // but if we stick with this we should just remove the alignment here and generalise it fully and make the
        // caller always deal with copying the data to whatever alignment boundary it requires.
        auto* ptr = (uint8_t*)aligned_alloc(32, *length_out);
        info("get data hit: ", key, " length: ", *length_out, " ptr ", (void*)ptr);
        std::memcpy(ptr, store[k].data(), *length_out);
        return ptr;
    }
    info("get data miss: ", key);
    *length_out = 0;
    return nullptr;
}
}

#endif