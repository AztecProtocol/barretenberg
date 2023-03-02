// To be provided by the environment.
// For barretenberg.wasm, this is provided by the JavaScript environment.
// For anything other than barretenberg.wasm, this is provided in this module.

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#else
#define EM_IMPORT(name)
#endif
extern "C" void logstr(char const*) EM_IMPORT(logstr);
