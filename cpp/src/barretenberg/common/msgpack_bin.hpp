#include <cstdint>
#include <cstddef>

namespace msgpack::v2 { // Hack, have to forward v2 namespace
struct object;
}

// For field serialization
namespace msgpack {

void read_bin64(const msgpack::v2::object& obj, uint64_t* data, size_t size);

}
