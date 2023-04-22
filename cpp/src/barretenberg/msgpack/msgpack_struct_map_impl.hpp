#pragma once
// Note: heavy header due to serialization logic, don't include if auto parameters will do
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#define MSGPACK_NO_BOOST
#include <msgpack.hpp>
#include "msgpack_concepts.hpp"

namespace msgpack::adaptor {
// reads structs with msgpack() method from a JSON-like dictionary
template <HasMsgPack T> struct convert<T> {
    msgpack::object const& operator()(msgpack::object const& o, T& v) const
    {
        v.msgpack([&](auto&... args) { msgpack::type::define_map{args...}.msgpack_unpack(o); });
        return o;
    }
};

// converts structs with msgpack() method from a JSON-like dictionary
template <HasMsgPack T> struct pack<T> {
    template <typename Stream> packer<Stream>& operator()(msgpack::packer<Stream>& o, T const& v) const
    {
        const_cast<T&>(v).msgpack([&](auto&... args) { msgpack::type::define_map{args...}.msgpack_pack(o); });
        return o;
    }
};

} // namespace msgpack::adaptor
