#ifndef __wasm__
#include <iostream>

extern "C" {

void logstr(char const* str)
{
    std::cerr << str << std::endl;
}
}
#endif