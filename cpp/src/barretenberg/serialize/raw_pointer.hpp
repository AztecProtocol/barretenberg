#include <cstdint>
#include "msgpack_impl/schema_name.hpp"

template <typename T>
struct RawPointer {
    T* prover {};
    void msgpack_pack(auto& packer) const {
        packer.pack(reinterpret_cast<uintptr_t>(prover));
    }
    void msgpack_unpack(auto object) {
        prover = reinterpret_cast<T*>((uintptr_t)object);
    }
    T* operator->() {
        return prover;
    }
};

namespace msgpack {
// help our msgpack schema compiler with this struct
template <typename T>
inline void schema_pack(auto& packer, RawPointer<T> const&)
{
    packer.pack_alias(schema_name(T {}) + "Ptr", "int");
}
} // namespace msgpack