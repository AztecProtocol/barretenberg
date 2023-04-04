#include "verification_key.hpp"
#include <gtest/gtest.h>

#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/proof_system/verification_key/verification_key.hpp"
#include "barretenberg/plonk/proof_system/constants.hpp"
#include "barretenberg/stdlib/types/types.hpp"

using namespace plonk;

namespace {
auto& engine = numeric::random::get_debug_engine();
}

verification_key_data rand_vk_data(plonk::ComposerType composer_type)
{
    verification_key_data key_data;
    key_data.composer_type = static_cast<uint32_t>(composer_type);
    key_data.circuit_size = 1024; // not random - must be power of 2
    key_data.num_public_inputs = engine.get_random_uint16();
    key_data.commitments["test1"] = g1::element::random_element();
    key_data.commitments["test2"] = g1::element::random_element();
    key_data.commitments["foo1"] = g1::element::random_element();
    key_data.commitments["foo2"] = g1::element::random_element();
    return key_data;
}

TEST(stdlib_verification_key, native_compress_comparison)
{
    // Compute compression of native verification key (i.e. vk_data)
    auto crs = std::make_unique<bonk::FileReferenceStringFactory>("../srs_db/ignition");
    verification_key_data vk_data = rand_vk_data(stdlib::types::Composer::type);
    const size_t hash_idx = 10;
    auto native_vk_compression = vk_data.compress_native(hash_idx);

    // Compute compression of recursive verification key
    auto verification_key = std::make_shared<bonk::verification_key>(std::move(vk_data), crs->get_verifier_crs());
    auto recursive_vk_compression =
        stdlib::recursion::verification_key<stdlib::types::bn254>::compress_native(verification_key, hash_idx);
    EXPECT_EQ(native_vk_compression, recursive_vk_compression);
}