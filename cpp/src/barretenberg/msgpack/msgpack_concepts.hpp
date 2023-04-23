#pragma once

struct DoNothing {
    void operator()(auto...){}
};
namespace msgpack {
template<typename T>
concept HasMsgPack = requires(T t, DoNothing nop) { t.msgpack(nop); };
template<typename T>
concept HasMsgPackPack = requires(T t, DoNothing nop) { t.msgpack_pack(nop); };
template<typename T>
concept HasMsgPackFlat = requires(T t, DoNothing nop) { t.msgpack_flat(nop); };
template<typename T>
concept SharedPtr = std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
}