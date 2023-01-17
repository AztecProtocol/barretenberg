#include <ecc/curves/bn254/fq12.hpp>
#include <ecc/curves/bn254/pairing.hpp>
#include <ecc/curves/grumpkin/grumpkin.hpp>
#include <crypto/blake2s/blake2s.hpp>
#include "io.hpp"
#include <common/mem.hpp>
#include <common/test.hpp>
#include <gtest/gtest.h>
#include <stdio.h>

using namespace barretenberg;

TEST(io, read_transcript_loads_well_formed_srs)
{
    size_t degree = 100000;
    g1::affine_element* monomials = (g1::affine_element*)(aligned_alloc(32, sizeof(g1::affine_element) * (degree + 2)));
    g2::affine_element g2_x;
    io::read_transcript(monomials, g2_x, degree, "../srs_db/ignition");

    EXPECT_EQ(g1::affine_one, monomials[0]);

    g1::affine_element P[2];
    g2::affine_element Q[2];
    P[0] = monomials[1];
    P[1] = g1::affine_one;
    P[0].y.self_neg();
    Q[0] = g2::affine_one;
    Q[1] = g2_x;
    fq12 res = pairing::reduced_ate_pairing_batch(P, Q, 2);

    EXPECT_EQ(res, fq12::one());
    for (size_t i = 0; i < degree; ++i) {
        EXPECT_EQ(monomials[i].on_curve(), true);
    }
    aligned_free(monomials);
}

TEST(io, read_transcript_ipa_srs)
{
    size_t degree = 100000;
    grumpkin::g1::affine_element* monomials =
        (grumpkin::g1::affine_element*)(aligned_alloc(32, sizeof(grumpkin::g1::affine_element) * (degree + 2)));
    io::read_transcript_ipa(monomials, degree, "mnt/usr/suyash/trustless/grumpkin");
    info("monomials[1].x = ", monomials[1].x, "\n");
    info("read from file successfully");
    aligned_free(monomials);
}

HEAVY_TEST(io, generate_and_write_ipa_srs)
{
    constexpr size_t points_per_transcript = 5040000;
    constexpr size_t num_transcripts = 20;
    constexpr size_t subgroup_size = points_per_transcript * num_transcripts;

    for (size_t i = 0; i < num_transcripts; i++) {
        // Generate a 64-bit seed
        const std::string seed_str = "AZTEC_IPA_GENERATORS" + std::to_string(i);
        std::vector<uint8_t> seed_vec(seed_str.begin(), seed_str.end());
        const auto seed_hash_vec = (blake2::blake2s(seed_vec));
        uint64_t seed_hash = 0, multiplicand = 1;
        for (size_t i = 0; i < (seed_hash_vec.size() / 4); i++) {
            seed_hash += multiplicand * seed_hash_vec[i];
            multiplicand *= (1UL << 8);
        }

        // Derive generators
        auto generators = grumpkin::g1::derive_generator_vector<subgroup_size>(seed_hash);

        // Write generators to a transcript file
        barretenberg::io::Manifest manifest{ .transcript_number = static_cast<uint32_t>(i),
                                             .total_transcripts = 20,
                                             .total_g1_points = static_cast<uint32_t>(subgroup_size),
                                             .total_g2_points = 0,
                                             .num_g1_points = points_per_transcript,
                                             .num_g2_points = 0,
                                             .start_from = 0 };
        barretenberg::io::write_transcript(&generators[0], manifest, "../srs_db/trustless/grumpkin");
    }
}