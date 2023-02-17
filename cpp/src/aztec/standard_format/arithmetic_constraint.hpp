#pragma once
#include <common/serialize.hpp>

namespace waffle {

template <typename B> inline void read(B& buf, poly_triple& constraint)
{
    using serialize::read;
    read(buf, constraint.a);
    read(buf, constraint.b);
    read(buf, constraint.c);
    read(buf, constraint.q_m);
    read(buf, constraint.q_l);
    read(buf, constraint.q_r);
    read(buf, constraint.q_o);
    read(buf, constraint.q_c);
}

template <typename B> inline void write(B& buf, poly_triple const& constraint)
{
    using serialize::write;
    write(buf, constraint.a);
    write(buf, constraint.b);
    write(buf, constraint.c);
    write(buf, constraint.q_m);
    write(buf, constraint.q_l);
    write(buf, constraint.q_r);
    write(buf, constraint.q_o);
    write(buf, constraint.q_c);
}

} // namespace waffle
