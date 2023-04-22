#pragma once
namespace msgpack {
template<typename T>
concept HasMsgPack = requires(T t) { t.msgpack([](auto...){}); };
template<typename T>
concept HasMsgPackFlat = requires(T t) { t.msgpack_flat([](auto...){}); };
template<typename T>
concept SharedPtr = std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
}