#pragma once
#include <stdlib/primitives/address/address.hpp>
#include <stdlib/encryption/schnorr/schnorr.hpp>
#include <stdlib/primitives/bigfield/bigfield.hpp>
#include <stdlib/primitives/biggroup/biggroup.hpp>
#include <stdlib/primitives/bit_array/bit_array.hpp>
#include <stdlib/primitives/bool/bool.hpp>
#include <stdlib/primitives/byte_array/byte_array.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>
#include <stdlib/primitives/uint/uint.hpp>
#include <stdlib/primitives/point/point.hpp>
#include <stdlib/primitives/group/group.hpp>
#include <stdlib/primitives/curves/bn254.hpp>
#include <stdlib/recursion/verifier/verifier.hpp>
#include <stdlib/recursion/verification_key/verification_key.hpp>
#include <stdlib/commitment/pedersen/pedersen.hpp>
#include <stdlib/hash/blake2s/blake2s.hpp>
#include "native_types.hpp"

namespace plonk::stdlib::types {

using plonk::stdlib::address_t;
// using NT = plonk::stdlib::types::NativeTypes;
// using plonk::stdlib::pedersen;

template <typename Composer> struct CircuitTypes {
    typedef bool_t<Composer> boolean;

    // typedef uint8<Composer> uint8;
    // typedef uint16<Composer> uint16;
    typedef stdlib::uint32<Composer> uint32;
    // typedef uint64<Composer> uint64;

    typedef field_t<Composer> fr; // of altbn
    typedef safe_uint_t<Composer> safe_fr;
    // TODO: make address_ct be the same as all other types. (And define it in the stdlib, instead of aztec3 dir).
    typedef address_t<Composer> address;

    typedef stdlib::bigfield<Composer, barretenberg::Bn254FqParams> fq; // of altbn

    // typedef fq grumpkin_fr;
    // typedef fr grumpkin_fq;
    typedef point<Composer> grumpkin_point; // affine
    typedef group<Composer> grumpkin_group;

    typedef stdlib::bn254<Composer> bn254;
    // typedef bn254::g1_ct bn254_point;
    typedef element<Composer, fq, fr, barretenberg::g1>
        bn254_point; // looks affine (despite the confusing name 'element').

    // typedef bit_array<Composer> bit_array;
    typedef stdlib::byte_array<Composer> byte_array;
    // typedef packed_byte_array<Composer> packed_byte_array;

    // typedef stdlib::schnorr::signature_bits<Composer> signature;

    typedef stdlib::recursion::recursion_output<bn254> AggregationObject;
    typedef stdlib::recursion::verification_key<bn254> VK;
    // Notice: no CircuitType for a Proof: we only ever handle native; the verify_proof() function swallows the
    // 'circuit-type-ness' of the proof.

    /// TODO: lots of these compress / commit functions aren't actually used: remove them.

    // Define the 'circuit' version of the function `compress`, with the name `compress`:
    static fr compress(std::vector<fr> const& inputs, const size_t hash_index = 0)
    {
        return plonk::stdlib::pedersen_commitment<Composer>::compress(inputs, hash_index);
    }

    static fr compress(std::vector<fr> const& inputs,
                       std::vector<size_t> const& hash_sub_indices,
                       const size_t hash_index = 0)
    {
        return plonk::stdlib::pedersen_commitment<Composer>::compress(inputs, hash_sub_indices, hash_index);
    }

    static fr compress(const std::vector<std::pair<fr, crypto::pedersen::generator_index_t>>& input_pairs)
    {
        return plonk::stdlib::pedersen_commitment<Composer>::compress(input_pairs);
    };

    static grumpkin_point commit(const std::vector<fr>& inputs, const size_t hash_index = 0)
    {
        return plonk::stdlib::pedersen_commitment<Composer>::commit(inputs, hash_index);
    };

    static grumpkin_point commit(const std::vector<std::pair<fr, crypto::pedersen::generator_index_t>>& input_pairs)
    {
        return plonk::stdlib::pedersen_commitment<Composer>::commit(input_pairs);
    };

    static byte_array blake2s(const byte_array& input) { return plonk::stdlib::blake2s(input); }
};

} // namespace plonk::stdlib::types