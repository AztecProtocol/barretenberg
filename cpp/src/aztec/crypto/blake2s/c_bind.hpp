#include <stdint.h>
#include <stddef.h>

extern "C" {

void blake2s_to_field(uint8_t const* data, size_t length, uint8_t* r);
}