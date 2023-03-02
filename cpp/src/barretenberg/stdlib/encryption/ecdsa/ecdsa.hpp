#pragma once

#include <crypto/ecdsa/ecdsa.hpp>
#include "../../primitives/byte_array/byte_array.hpp"
#include "../../primitives/composers/composers_fwd.hpp"

namespace proof_system::plonk {
namespace stdlib {
namespace ecdsa {

template <typename Composer> struct signature {
    stdlib::byte_array<Composer> r;
    stdlib::byte_array<Composer> s;
};

template <typename Composer, typename Curve, typename Fq, typename Fr, typename G1>
bool_t<Composer> verify_signature(const stdlib::byte_array<Composer>& message,
                                  const G1& public_key,
                                  const signature<Composer>& sig);

template <typename Composer>
static signature<Composer> from_witness(Composer* ctx, const crypto::ecdsa::signature& input)
{
    byte_array x(ctx, input.r);
    byte_array y(ctx, input.s);
    signature<Composer> out(x, y);
    return out;
}

} // namespace ecdsa
} // namespace stdlib
} // namespace proof_system::plonk

#include "./ecdsa_impl.hpp"