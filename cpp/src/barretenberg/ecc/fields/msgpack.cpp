#define MSGPACK_NO_BOOST
#include <msgpack.hpp>
#include <cstddef>
#include <barretenberg/common/net.hpp>
#include <barretenberg/common/throw_or_abort.hpp>

void msgpack_read_bin64(const msgpack::object& obj, unsigned long long* data, size_t size)
{
    if (obj.type != msgpack::type::BIN) {
        throw_or_abort("Wrong data type when unpacking bin64");
    }
    if (obj.via.bin.size != size * sizeof(unsigned long long)) {
        throw_or_abort("Wrong size data while unpacking bin64");
    }

    unsigned long long* bin_data = (uint64_t*)obj.via.bin.ptr;
    for (size_t i = 0; i < size; i++) {
        data[i] = ntohll(bin_data[i]);
    }
}
