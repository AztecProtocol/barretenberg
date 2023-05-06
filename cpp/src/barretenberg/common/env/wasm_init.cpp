/**
 * WASI "reactors" expect an exported _initialize function, and for it to be called before any other exported
 * function. It triggers initialization of all globals and statics. If you don't do this, every function call will
 * trigger the initialization of globals as if they are "main". Good luck with that...
 */
#ifdef __wasm__
#define WASM_EXPORT(name) __attribute__((export_name(#name))) name

extern "C" {
extern void __wasm_call_ctors(void);

void WASM_EXPORT(_initialize)()
{
    __wasm_call_ctors();
}
}
#endif