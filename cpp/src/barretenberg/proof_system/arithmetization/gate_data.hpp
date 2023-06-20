#pragma once
#include <cstdint>
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/common/serialize.hpp"

namespace proof_system {
template <typename FF> struct add_triple_ {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    FF a_scaling;
    FF b_scaling;
    FF c_scaling;
    FF const_scaling;
};
using add_triple = add_triple_<barretenberg::fr>;

template <typename FF> struct add_quad_ {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    FF a_scaling;
    FF b_scaling;
    FF c_scaling;
    FF d_scaling;
    FF const_scaling;
};
using add_quad = add_quad_<barretenberg::fr>;

template <typename FF> struct mul_quad_ {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    FF mul_scaling;
    FF a_scaling;
    FF b_scaling;
    FF c_scaling;
    FF d_scaling;
    FF const_scaling;
};
using mul_quad = mul_quad_<barretenberg::fr>;

template <typename FF> struct mul_triple_ {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    FF mul_scaling;
    FF c_scaling;
    FF const_scaling;
};
using mul_triple = mul_triple_<barretenberg::fr>;

template <typename FF> struct poly_triple_ {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    FF q_m;
    FF q_l;
    FF q_r;
    FF q_o;
    FF q_c;

    friend bool operator==(poly_triple_<FF> const& lhs, poly_triple_<FF> const& rhs) = default;
};

using poly_triple = poly_triple_<barretenberg::fr>;

// TODO: figure out what to do with this...
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
// TODO: and this..
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

template <typename FF> struct fixed_group_add_quad_ {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    FF q_x_1;
    FF q_x_2;
    FF q_y_1;
    FF q_y_2;
};
using fixed_group_add_quad = fixed_group_add_quad_<barretenberg::fr>;

template <typename FF> struct fixed_group_init_quad_ {
    FF q_x_1;
    FF q_x_2;
    FF q_y_1;
    FF q_y_2;
};
using fixed_group_init_quad = fixed_group_init_quad_<barretenberg::fr>;

template <typename FF> struct accumulator_triple_ {
    std::vector<uint32_t> left;
    std::vector<uint32_t> right;
    std::vector<uint32_t> out;
};
using accumulator_triple = accumulator_triple_<barretenberg::fr>;

template <typename FF> struct ecc_add_gate_ {
    uint32_t x1;
    uint32_t y1;
    uint32_t x2;
    uint32_t y2;
    uint32_t x3;
    uint32_t y3;
    FF endomorphism_coefficient;
    FF sign_coefficient;
};
using ecc_add_gate = ecc_add_gate_<barretenberg::fr>;
} // namespace proof_system
