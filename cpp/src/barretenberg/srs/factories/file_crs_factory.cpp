#include "file_crs_factory.hpp"
#include "../io.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/ecc/curves/bn254/pairing.hpp"
#include "barretenberg/ecc/curves/bn254/scalar_multiplication/pippenger.hpp"

namespace barretenberg::srs::factories {

FileProverCrs::FileProverCrs(const size_t num_points, std::string const& path)
    : num_points(num_points)
{
    monomials_ = scalar_multiplication::point_table_alloc<g1::affine_element>(num_points);

    io::read_transcript_g1(monomials_, num_points, path);
    scalar_multiplication::generate_pippenger_point_table(monomials_, monomials_, num_points);
}

g1::affine_element* FileProverCrs::get_monomial_points()
{
    return monomials_;
}

size_t FileProverCrs::get_monomial_size() const
{
    return num_points;
}

FileVerifierCrs::FileVerifierCrs(std::string const& path)
    : precomputed_g2_lines(
          (barretenberg::pairing::miller_lines*)(aligned_alloc(64, sizeof(barretenberg::pairing::miller_lines) * 2)))
{

    barretenberg::io::read_transcript_g2(g2_x, path);
    barretenberg::pairing::precompute_miller_lines(barretenberg::g2::one, precomputed_g2_lines[0]);
    barretenberg::pairing::precompute_miller_lines(g2_x, precomputed_g2_lines[1]);
}

FileVerifierCrs::~FileVerifierCrs()
{
    aligned_free(precomputed_g2_lines);
}

g2::affine_element FileVerifierCrs::get_g2x() const
{
    return g2_x;
}

pairing::miller_lines const* FileVerifierCrs::get_precomputed_g2_lines() const
{
    return precomputed_g2_lines;
}

FileCrsFactory::FileCrsFactory(std::string path, size_t initial_degree)
    : path_(std::move(path))
    , degree_(initial_degree)
    , verifier_crs_(std::make_shared<FileVerifierCrs>(path_))
{}

std::shared_ptr<barretenberg::srs::factories::ProverCrs> FileCrsFactory::get_prover_crs(size_t degree)
{
    if (degree != degree_ || !prover_crs_) {
        prover_crs_ = std::make_shared<FileProverCrs>(degree, path_);
        degree_ = degree;
    }
    return prover_crs_;
}

std::shared_ptr<barretenberg::srs::factories::VerifierCrs> FileCrsFactory::get_verifier_crs()
{
    return verifier_crs_;
}

} // namespace barretenberg::srs::factories