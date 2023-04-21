#include <cstdint>
#include <cstddef>

// Implementation detail for field serialization
// We are very careful to include msgpack.hpp only in cpp units

namespace msgpack::v2 { // Hack, have to forward v2 namespace
struct object;
}

namespace msgpack {

void read_bin64(const msgpack::v2::object& obj, uint64_t* data, size_t size);

}
