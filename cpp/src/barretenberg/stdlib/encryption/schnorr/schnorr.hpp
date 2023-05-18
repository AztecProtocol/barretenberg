#pragma once
#include "barretenberg/crypto/schnorr/schnorr.hpp"
#include "../../primitives/field/field.hpp"
#include "../../primitives/bool/bool.hpp"
#include "../../primitives/witness/witness.hpp"
#include "../../primitives/byte_array/byte_array.hpp"
#include "../../primitives/point/point.hpp"
#include "../../primitives/group/group.hpp"

namespace proof_system::plonk {
namespace stdlib {
namespace schnorr {

template <typename C> struct signature_bits {
    field_t<C> s_lo;
    field_t<C> s_hi;
    field_t<C> e_lo;
    field_t<C> e_hi;
};

template <typename C> struct wnaf_record {
    std::vector<bool_t<C>> bits;
    bool_t<C> skew;
};

template <typename C> wnaf_record<C> convert_field_into_wnaf(C* context, const field_t<C>& limb);

template <typename C>
point<C> variable_base_mul(const point<C>& pub_key, const point<C>& current_accumulator, const wnaf_record<C>& scalar);
template <typename C>
point<C> variable_base_mul(const point<C>& pub_key, const field_t<C>& low_bits, const field_t<C>& high_bits);

template <typename C> signature_bits<C> convert_signature(C* context, const crypto::schnorr::signature& sig);

template <typename C>
std::array<field_t<C>, 2> verify_signature_internal(const byte_array<C>& message,
                                                    const point<C>& pub_key,
                                                    const signature_bits<C>& sig);

template <typename C>
void verify_signature(const byte_array<C>& message, const point<C>& pub_key, const signature_bits<C>& sig);

template <typename C>
bool_t<C> signature_verification_result(const byte_array<C>& message,
                                        const point<C>& pub_key,
                                        const signature_bits<C>& sig);

extern template point<plonk::TurboPlonkComposer> variable_base_mul<plonk::TurboPlonkComposer>(
    const point<plonk::TurboPlonkComposer>&,
    const point<plonk::TurboPlonkComposer>&,
    const wnaf_record<plonk::TurboPlonkComposer>&);

extern template point<plonk::TurboPlonkComposer> variable_base_mul(const point<plonk::TurboPlonkComposer>& pub_key,
                                                                   const field_t<plonk::TurboPlonkComposer>& low_bits,
                                                                   const field_t<plonk::TurboPlonkComposer>& high_bits);

extern template wnaf_record<plonk::TurboPlonkComposer> convert_field_into_wnaf<plonk::TurboPlonkComposer>(
    plonk::TurboPlonkComposer* context, const field_t<plonk::TurboPlonkComposer>& limb);

extern template wnaf_record<plonk::UltraComposer> convert_field_into_wnaf<plonk::UltraComposer>(
    plonk::UltraComposer* context, const field_t<plonk::UltraComposer>& limb);

extern template std::array<field_t<plonk::TurboPlonkComposer>, 2> verify_signature_internal<plonk::TurboPlonkComposer>(
    const byte_array<plonk::TurboPlonkComposer>&,
    const point<plonk::TurboPlonkComposer>&,
    const signature_bits<plonk::TurboPlonkComposer>&);
extern template std::array<field_t<plonk::UltraComposer>, 2> verify_signature_internal<plonk::UltraComposer>(
    const byte_array<plonk::UltraComposer>&,
    const point<plonk::UltraComposer>&,
    const signature_bits<plonk::UltraComposer>&);

extern template void verify_signature<plonk::TurboPlonkComposer>(const byte_array<plonk::TurboPlonkComposer>&,
                                                                 const point<plonk::TurboPlonkComposer>&,
                                                                 const signature_bits<plonk::TurboPlonkComposer>&);
extern template void verify_signature<plonk::UltraComposer>(const byte_array<plonk::UltraComposer>&,
                                                            const point<plonk::UltraComposer>&,
                                                            const signature_bits<plonk::UltraComposer>&);

extern template bool_t<plonk::TurboPlonkComposer> signature_verification_result<plonk::TurboPlonkComposer>(
    const byte_array<plonk::TurboPlonkComposer>&,
    const point<plonk::TurboPlonkComposer>&,
    const signature_bits<plonk::TurboPlonkComposer>&);
extern template bool_t<plonk::UltraComposer> signature_verification_result<plonk::UltraComposer>(
    const byte_array<plonk::UltraComposer>&,
    const point<plonk::UltraComposer>&,
    const signature_bits<plonk::UltraComposer>&);

extern template signature_bits<plonk::TurboPlonkComposer> convert_signature<plonk::TurboPlonkComposer>(
    plonk::TurboPlonkComposer*, const crypto::schnorr::signature&);
extern template signature_bits<plonk::UltraComposer> convert_signature<plonk::UltraComposer>(
    plonk::UltraComposer*, const crypto::schnorr::signature&);

} // namespace schnorr
} // namespace stdlib
} // namespace proof_system::plonk
