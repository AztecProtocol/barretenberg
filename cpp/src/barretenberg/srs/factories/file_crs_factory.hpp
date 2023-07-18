#pragma once
#include "barretenberg/ecc/curves/bn254/bn254.hpp"
#include "crs_factory.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"
#include "barretenberg/ecc/scalar_multiplication/point_table.hpp"
#include "barretenberg/ecc/scalar_multiplication/scalar_multiplication.hpp"
#include "../io.hpp"
#include <utility>
#include <cstddef>

namespace barretenberg::srs::factories {

/**
 * Create reference strings given a path to a directory of transcript files.
 */
template <typename Curve> class FileCrsFactory : public CrsFactory<Curve> {
  public:
    FileCrsFactory(std::string path, size_t initial_degree = 0);
    FileCrsFactory(FileCrsFactory&& other) = default;

    std::shared_ptr<barretenberg::srs::factories::ProverCrs<Curve>> get_prover_crs(size_t degree) override;

    std::shared_ptr<barretenberg::srs::factories::VerifierCrs<Curve>> get_verifier_crs(size_t degree) override;

  private:
    std::string path_;
    size_t degree_;
    std::shared_ptr<barretenberg::srs::factories::ProverCrs<Curve>> prover_crs_;
    std::shared_ptr<barretenberg::srs::factories::VerifierCrs<Curve>> verifier_crs_;
};

template <typename Curve> class FileProverCrs : public ProverCrs<Curve> {
  public:
    // this could go in the source file but let's see
    FileProverCrs(const size_t num_points, std::string const& path)
        : num_points(num_points)
    {
        monomials_ = scalar_multiplication::point_table_alloc<typename Curve::AffineElement>(num_points);

        srs::IO<Curve>::read_transcript_g1(monomials_.get(), num_points, path);
        scalar_multiplication::generate_pippenger_point_table<Curve>(monomials_.get(), monomials_.get(), num_points);
    };

    typename Curve::AffineElement* get_monomial_points() { return monomials_.get(); }

    size_t get_monomial_size() const { return num_points; }

  private:
    size_t num_points;
    std::shared_ptr<typename Curve::AffineElement[]> monomials_;
};

template <typename Curve> class FileVerifierCrs : public VerifierCrs<Curve> {
  public:
    FileVerifierCrs(const size_t num_points, std::string const& path);

  private:
    size_t num_points;
};

extern template class FileProverCrs<curve::BN254>;
extern template class FileProverCrs<curve::Grumpkin>;

} // namespace barretenberg::srs::factories
