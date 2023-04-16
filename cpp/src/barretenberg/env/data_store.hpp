// To be provided by the environment.
// For a WASM build, this is provided by the JavaScript environment.
// For a native build, this is provided in this module.
#include <cstddef>
#include <cstdint>

// Takes a copy of buf and saves it associated with key.
extern "C" void set_data(char const* key, uint8_t const* buf, size_t length);

// Returns a copy of buf associated with key. Caller takes ownership.
extern "C" uint8_t* get_data(char const* key, size_t* length_out);
