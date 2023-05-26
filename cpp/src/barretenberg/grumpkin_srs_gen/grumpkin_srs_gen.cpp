#include <iostream>
#include <fstream>

#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"
#include "barretenberg/srs/io.hpp"

/* Generates a monomial basis Grumpkin SRS for testing purposes.
   We only provide functionality create a single transcript file.
   The SRS has the form [1]_1, [x]_1, [x^2]_1, ... where x = 2. */
int main(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);
    if (args.size() <= 1) {
        info("usage: ", args[0], " <subgroup_size> [output_srs_path]");
        return 1;
    }

    // Note: the number of points in one Ignition transcript file is 5'040'000; see
    // https://github.com/AztecProtocol/ignition-verification/blob/master/Transcript_spec.md
    const size_t subgroup_size = (size_t)atoi(args[1].c_str());
    const std::string srs_path = (args.size() > 2) ? args[2] : "../srs_db/grumpkin/";

    std::vector<grumpkin::g1::affine_element> srs(subgroup_size);

    auto point = grumpkin::g1::one;
    uint32_t x_secret = 2;
    for (size_t point_idx = 0; point_idx < subgroup_size; ++point_idx) {
        srs.at(point_idx) = static_cast<grumpkin::g1::affine_element>(point);
        point *= x_secret;
    }

    srs::Manifest manifest{ 0, 1, static_cast<uint32_t>(subgroup_size), 0, static_cast<uint32_t>(subgroup_size), 0, 0 };

    srs::IO<curve::Grumpkin>::write_transcript(&srs[0], manifest, srs_path);

    return 0;
}