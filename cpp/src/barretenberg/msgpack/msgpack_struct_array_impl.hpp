#pragma once
// Note: Meant to only be included in compilation units that need msgpack
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include "barretenberg/common/throw_or_abort.hpp"
#define MSGPACK_NO_BOOST
#include <msgpack.hpp>
#include "msgpack_concepts.hpp"

namespace msgpack::adaptor {
// converts structs with msgpack_flat() method from a JSON-like dictionary
template <HasMsgPackFlat T> struct convert<T> {
    msgpack::object const& operator()(msgpack::object const& o, T& v) const
    {
        v.msgpack_flat([&](auto&... args) {  msgpack::type::define_array{args...}.msgpack_unpack(o); });
        return o;
    }
};

// converts structs with msgpack_flat() method to a JSON-like dictionary
template <HasMsgPackFlat T> struct pack<T> {
    template <typename Stream> packer<Stream>& operator()(msgpack::packer<Stream>& o, T const& v) const
    {
        const_cast<T&>(v).msgpack_flat([&](auto&... args) { msgpack::type::define_array{args...}.msgpack_pack(o); });
        return o;
    }
};

} // namespace msgpack::adaptor
