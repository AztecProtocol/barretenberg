#pragma once

struct DoNothing {
    void operator()(auto...){}
};
namespace msgpack {
template<typename T>
concept HasMsgPack = requires(T t, DoNothing nop) { t.msgpack(nop); };
template<typename T>
concept HasMsgPackPack = requires(T t, DoNothing nop) { t.msgpack_pack(nop); };
}