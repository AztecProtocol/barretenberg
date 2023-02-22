#include <srs/lagrange_base_transformation/lagrange_base.hpp>
#include <srs/reference_string/file_reference_string.hpp>
#include <ecc/groups/element.hpp>
#include <ecc/curves/bn254/g1.hpp>
#include <ecc/curves/bn254/g2.hpp>
#include <srs/io.hpp>
#include <iostream>
#include <algorithm>

int main(int argc, char** argv)
{
    /*
        This script generates lagrange base transcript from a monomial base srs transcript and
        for a given subgroup size. The subgroup size must be greater than 1 because for subgroup size
        equals 1, the corresponding monomial srs has only one term (g1::affine_one) and thus
        we encounter a `No input file found` error in io::read_transcript_g1().

        Sample usage: ./bin/lagrange_base_gen 8
        The bash script lagrange_base_gen.sh runs this script for a given set of subgroup sizes (only
        powers of two). To run the srs_tests successfully, you need to run the bash script once to
        generate relevant lagrange base transcripts.
    */
    std::vector<std::string> args(argv, argv + argc);
    if (args.size() <= 1) {
        info("usage: ", args[0], "<subgroup_size> [srs_path] [lagrange_srs_path]");
        return 1;
    }

    const size_t subgroup_size = (size_t)atoi(args[1].c_str());
    const std::string srs_path = (args.size() > 2) ? args[2] : "../srs_db/ignition";
    const std::string lagrange_srs_path = (args.size() > 3) ? args[3] : "../srs_db/lagrange";

    auto reference_string = std::make_shared<bonk::FileReferenceString>(subgroup_size, srs_path);
    std::vector<barretenberg::g1::affine_element> monomial_srs(subgroup_size);
    for (size_t i = 0; i < subgroup_size; ++i) {
        monomial_srs[i] = reference_string->get_monomial_points()[2 * i];
    }

    auto verifier_ref_string = std::make_shared<bonk::VerifierFileReferenceString>(srs_path);

    std::vector<barretenberg::g1::affine_element> lagrange_base_srs(subgroup_size);
    barretenberg::lagrange_base::transform_srs(&monomial_srs[0], &lagrange_base_srs[0], subgroup_size);

    std::vector<barretenberg::g2::affine_element> g2_elements;
    g2_elements.push_back(verifier_ref_string->get_g2x());
    g2_elements.push_back(barretenberg::g2::affine_one);

    const uint32_t subgroup_size_ = static_cast<uint32_t>(subgroup_size);
    const uint32_t max_lagrange_transcript_size = static_cast<uint32_t>((size_t(1) << 24));

    // If the transcript size is > 2^24, write the Lagrange srs into individual transcripts each containing 2^24
    // points. This means that the individual lagrange transcripts will have a max size of 1.1 GB.
    const uint32_t total_transcripts = std::max(uint32_t(1), subgroup_size_ / max_lagrange_transcript_size);
    const uint32_t num_g1_points_ = std::min(subgroup_size_, max_lagrange_transcript_size);
    size_t start_idx = 0;
    for (uint32_t i = 0; i < total_transcripts; i++) {
        barretenberg::io::Manifest manifest{ .transcript_number = i,
                                             .total_transcripts = total_transcripts,
                                             .total_g1_points = subgroup_size_,
                                             .total_g2_points = 2,
                                             .num_g1_points = num_g1_points_,
                                             .num_g2_points = 2,
                                             .start_from = 0 };
        barretenberg::io::write_transcript(
            &lagrange_base_srs[start_idx], &g2_elements[0], manifest, lagrange_srs_path, true);
        start_idx += max_lagrange_transcript_size;
    }

    return 0;
}
