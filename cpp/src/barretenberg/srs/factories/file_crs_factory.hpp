#pragma once
#include "crs_factory.hpp"
#include <utility>
#include <cstddef>

namespace barretenberg::srs::factories {

class FileProverCrs : public ProverCrs {
  public:
    FileProverCrs(const size_t num_points, std::string const& path);

    g1::affine_element* get_monomial_points() override;

    size_t get_monomial_size() const override;

  private:
    size_t num_points;
    g1::affine_element* monomials_;
};

class FileVerifierCrs : public VerifierCrs {
  public:
    FileVerifierCrs(std::string const& path);

    ~FileVerifierCrs();

    g2::affine_element get_g2x() const override;

    pairing::miller_lines const* get_precomputed_g2_lines() const override;

  private:
    g2::affine_element g2_x;
    pairing::miller_lines* precomputed_g2_lines;
};

/**
 * Create reference strings given a path to a directory of transcript files.
 */
class FileCrsFactory : public CrsFactory {
  public:
    FileCrsFactory(std::string path, size_t initial_degree = 0);
    FileCrsFactory(FileCrsFactory&& other) = default;

    std::shared_ptr<barretenberg::srs::factories::ProverCrs> get_prover_crs(size_t degree) override;

    std::shared_ptr<barretenberg::srs::factories::VerifierCrs> get_verifier_crs() override;

  private:
    std::string path_;
    size_t degree_;
    std::shared_ptr<barretenberg::srs::factories::ProverCrs> prover_crs_;
    std::shared_ptr<barretenberg::srs::factories::VerifierCrs> verifier_crs_;
};

} // namespace barretenberg::srs::factories
