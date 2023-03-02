#include <stddef.h>
#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#else
#define EM_IMPORT(name)
#endif

// To be provided by the environment.
// For barretenberg.wasm, this is provided by the JavaScript environment.
// For anything other than barretenberg.wasm, this is provided in this module.
extern "C" void* get_data(char const* key, size_t* length_out) EM_IMPORT(get_data);
extern "C" void set_data(char const* key, void* addr, size_t length) EM_IMPORT(set_data);
extern "C" void logstr(char const* str) EM_IMPORT(logstr);