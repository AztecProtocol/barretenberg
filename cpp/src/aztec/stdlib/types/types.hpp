#pragma once
#include <plonk/proof_system/constants.hpp>
#include <plonk/composer/standard_composer.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include <plonk/composer/ultra_composer.hpp>

#include <stdlib/primitives/bigfield/bigfield.hpp>
#include <stdlib/primitives/biggroup/biggroup.hpp>
#include <stdlib/primitives/bit_array/bit_array.hpp>
#include <stdlib/primitives/bool/bool.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>
#include <stdlib/primitives/byte_array/byte_array.hpp>
#include <stdlib/primitives/uint/uint.hpp>
#include <stdlib/primitives/safe_uint/safe_uint.hpp>
#include <stdlib/primitives/witness/witness.hpp>
#include <stdlib/primitives/bigfield/bigfield.hpp>
#include <stdlib/primitives/biggroup/biggroup.hpp>
#include <stdlib/hash/pedersen/pedersen.hpp>
#include <stdlib/merkle_tree/hash_path.hpp>
#include <stdlib/encryption/schnorr/schnorr.hpp>
#include <stdlib/primitives/curves/bn254.hpp>
#include <stdlib/primitives/curves/secp256k1.hpp>
#include <stdlib/primitives/memory/rom_table.hpp>
#include <stdlib/recursion/verifier/program_settings.hpp>

namespace plonk::stdlib::types {

using namespace plonk;
static constexpr size_t SYSTEM_COMPOSER = waffle::SYSTEM_COMPOSER;

typedef std::conditional_t<
    SYSTEM_COMPOSER == waffle::STANDARD,
    waffle::StandardComposer,
    std::conditional_t<SYSTEM_COMPOSER == waffle::TURBO, waffle::TurboComposer, waffle::UltraComposer>>
    Composer;

typedef std::conditional_t<
    SYSTEM_COMPOSER == waffle::STANDARD,
    waffle::Prover,
    std::conditional_t<SYSTEM_COMPOSER == waffle::TURBO, waffle::TurboProver, waffle::UltraProver>>
    Prover;

typedef std::conditional_t<
    SYSTEM_COMPOSER == waffle::STANDARD,
    waffle::Verifier,
    std::conditional_t<SYSTEM_COMPOSER == waffle::TURBO, waffle::TurboVerifier, waffle::UltraVerifier>>
    Verifier;

typedef std::conditional_t<
    SYSTEM_COMPOSER == waffle::STANDARD,
    waffle::UnrolledProver,
    std::conditional_t<SYSTEM_COMPOSER == waffle::TURBO, waffle::UnrolledTurboProver, waffle::UnrolledUltraProver>>
    UnrolledProver;

typedef std::conditional_t<
    SYSTEM_COMPOSER == waffle::STANDARD,
    waffle::UnrolledVerifier,
    std::conditional_t<SYSTEM_COMPOSER == waffle::TURBO, waffle::UnrolledTurboVerifier, waffle::UnrolledUltraVerifier>>
    UnrolledVerifier;

#define STDLIB_TYPE_ALIASES                                                                                            \
    using witness_ct = stdlib::witness_t<Composer>;                                                                    \
    using public_witness_ct = stdlib::public_witness_t<Composer>;                                                      \
    using bool_ct = stdlib::bool_t<Composer>;                                                                          \
    using byte_array_ct = stdlib::byte_array<Composer>;                                                                \
    using packed_byte_array_ct = stdlib::packed_byte_array<Composer>;                                                  \
    using field_ct = stdlib::field_t<Composer>;                                                                        \
    using uint8_ct = stdlib::uint8<Composer>;                                                                          \
    using uint16_ct = stdlib::uint16<Composer>;                                                                        \
    using uint32_ct = stdlib::uint32<Composer>;                                                                        \
    using uint64_ct = stdlib::uint64<Composer>;                                                                        \
    using suint_ct = stdlib::safe_uint_t<Composer>;                                                                    \
    using bit_array_ct = stdlib::bit_array<Composer>;                                                                  \
    using fq_ct = stdlib::bigfield<Composer, barretenberg::Bn254FqParams>;                                             \
    using biggroup_ct = stdlib::element<Composer, fq_ct, field_ct, barretenberg::g1>;                                  \
    using point_ct = stdlib::point<Composer>;                                                                          \
    using pedersen = stdlib::pedersen<Composer>;                                                                       \
    using group_ct = stdlib::group<Composer>;                                                                          \
    using bn254 = stdlib::bn254<Composer>;                                                                             \
    using secp256k1_ct = stdlib::secp256k1<Composer>;

STDLIB_TYPE_ALIASES

namespace merkle_tree {
using namespace stdlib::merkle_tree;
typedef stdlib::merkle_tree::hash_path<Composer> hash_path;
} // namespace merkle_tree

namespace schnorr {
typedef stdlib::schnorr::signature_bits<Composer> signature_bits;
} // namespace schnorr

// Ultra-composer specific types
typedef stdlib::rom_table<waffle::UltraComposer> rom_table_ct;

typedef std::conditional_t<SYSTEM_COMPOSER == waffle::TURBO,
                           recursion::recursive_turbo_verifier_settings<bn254>,
                           recursion::recursive_ultra_verifier_settings<bn254>>
    recursive_inner_verifier_settings;

} // namespace plonk::stdlib::types