#pragma once
// Note: heavy header due to serialization logic, don't include if auto parameters will do
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#define MSGPACK_NO_BOOST
#include <msgpack.hpp>

// A simple name-value pair helper for msgpack serialization
#define NVP(x) #x, x

// User defined class template specialization
namespace msgpack::adaptor {
template <typename T>
concept AnyType = true;

struct DefineMapArchive {
    template <typename... Args> auto operator()(Args&&... args) const
    {
        return msgpack::type::define_map{ std::forward<Args>(args)... };
    }
};

struct DefineArchive {
    template <typename... Args> auto operator()(Args&&... args) const
    {
        return msgpack::type::define_array{ std::forward<Args>(args)... };
    }
};

template <typename T>
concept HasMsgPack = requires(T t, DefineMapArchive ar) { t.msgpack(ar); };
template <typename T>
concept HasMsgPackFlat = requires(T t, const DefineArchive& ar) { t.msgpack_flat(ar); };

template <HasMsgPack T> struct convert<T> {
    msgpack::object const& operator()(msgpack::object const& o, T& v) const
    {
        v.msgpack([&](auto&... args) { DefineMapArchive{}(args...).msgpack_unpack(o); });
        return o;
    }
};

template <HasMsgPackFlat T> struct convert<T> {
    msgpack::object const& operator()(msgpack::object const& o, T& v) const
    {
        v.msgpack_flat([&](auto&... args) { DefineArchive{}(args...).msgpack_unpack(o); });
        return o;
    }
};

template <HasMsgPack T> struct pack<T> {
    template <typename Stream> packer<Stream>& operator()(msgpack::packer<Stream>& o, T const& v) const
    {
        const_cast<T&>(v).msgpack([&](auto&... args) { DefineMapArchive{}(args...).msgpack_pack(o); });
        return o;
    }
};

template <HasMsgPackFlat T> struct pack<T> {
    template <typename Stream> packer<Stream>& operator()(msgpack::packer<Stream>& o, T const& v) const
    {
        const_cast<T&>(v).msgpack_flat([&](auto&... args) { DefineArchive{}(args...).msgpack_pack(o); });
        return o;
    }
};

template <typename... T> struct pack<std::variant<T...>> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, std::variant<T...> const& variant) const
    {
        std::visit([&o](auto&& arg) { msgpack::pack(o, arg); }, variant);
        return o;
    }
};

} // namespace msgpack::adaptor
