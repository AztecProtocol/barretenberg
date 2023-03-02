#include <stddef.h>
#ifdef EMSCRIPTEN
#include <iostream>
// TODO
extern "C" void* get_data(char const* key, size_t* length_out)
{
    // Hack to stop unused warnings
    (void)key;
    (void)length_out;
    std::cout << "ALL THE TIME2!" << std::endl;
    return nullptr;
}
// TODO
extern "C" void set_data(char const* key, void* addr, size_t length)
{
    // Hack to stop unused warnings
    (void)key;
    (void)addr;
    (void)length;
    std::cout << "ALL THE TIME!" << std::endl;
}
extern "C" void logstr(char const* str)
{
    std::cout << str << std::endl;
}
#endif