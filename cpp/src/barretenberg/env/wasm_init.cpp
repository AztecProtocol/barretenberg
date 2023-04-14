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