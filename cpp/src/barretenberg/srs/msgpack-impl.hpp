#pragma once
// Meant to be included by cbind compilation units.
// Avoid including in headers as it msgpack.hpp includes a good deal of headers.
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
concept Serializable = requires(T t, DefineMapArchive ar) { t.msgpack(ar); };
template <typename T>
concept FlatSerializable = requires(T t, const DefineArchive& ar) { t.nsgpack_flat(ar); };

template <Serializable T>
// TODO constrain with concept for types that have a msgpack_map() method
struct convert<T> {
    msgpack::object const& operator()(msgpack::object const& o, T& v) const
    {
        return v.msgpack(DefineMapArchive{}).unpack(o);
    }
};

template <FlatSerializable T>
// TODO constrain with concept for types that have a msgpack() method
struct convert<T> {
    msgpack::object const& operator()(msgpack::object const& o, T& v) const
    {
        return v.msgpack_flat(DefineArchive{}).msgpack_unpack(o);
    }
};

template <Serializable T> struct pack<T> {
    template <typename Stream> packer<Stream>& operator()(msgpack::packer<Stream>& o, T const& v) const
    {
        const_cast<T&>(v).msgpack(DefineMapArchive{}).msgpack_pack(o);
        return o;
    }
};

template <FlatSerializable T> struct pack<T> {
    template <typename Stream> packer<Stream>& operator()(msgpack::packer<Stream>& o, T const& v) const
    {
        const_cast<T&>(v).msgpack_flat(DefineArchive{}).msgpack_pack(o);
        return o;
    }
};

template <Serializable T> struct object_with_zone<T> {
    void operator()(msgpack::object::with_zone& o, T const& v) const
    {
        const_cast<T&>(v).msgpack(DefineMapArchive{}).msgpack_object(&o, o.zone);
    }
};
template <FlatSerializable T> struct object_with_zone<T> {
    void operator()(msgpack::object::with_zone& o, T const& v) const
    {
        const_cast<T&>(v).msgpack_flat(DefineArchive{}).msgpack_object(o);
    }
};
} // namespace msgpack::adaptor
