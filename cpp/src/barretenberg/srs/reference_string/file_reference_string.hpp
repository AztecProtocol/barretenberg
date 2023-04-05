/**
 * Create reference strings given a path to a directory of transcript files.
 */
#pragma once
#include "reference_string.hpp"

#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/ecc/curves/bn254/g2.hpp"
#include "barretenberg/ecc/curves/bn254/scalar_multiplication/pippenger.hpp"

#include <utility>
#include <cstddef>

namespace barretenberg::pairing {
struct miller_lines;
} // namespace barretenberg::pairing

namespace proof_system {

using namespace barretenberg;

class VerifierFileReferenceString : public VerifierReferenceString {
  public:
    VerifierFileReferenceString(std::string const& path);
    ~VerifierFileReferenceString();

    g2::affine_element get_g2x() const override { return g2_x; }

    pairing::miller_lines const* get_precomputed_g2_lines() const override { return precomputed_g2_lines; }

  private:
    g2::affine_element g2_x;
    pairing::miller_lines* precomputed_g2_lines;
};

class FileReferenceString : public ProverReferenceString {
  public:
    FileReferenceString(const size_t num_points, std::string const& path)
        : num_points(num_points)
        , pippenger_(path, num_points)
    {}

    g1::affine_element* get_monomial_points() override { return pippenger_.get_point_table(); }

    size_t get_monomial_size() const override { return num_points; }

  private:
    size_t num_points;
    scalar_multiplication::Pippenger pippenger_;
};

class FileReferenceStringFactory : public ReferenceStringFactory {
  public:
    FileReferenceStringFactory(std::string path)
        : path_(std::move(path))
    {}

    FileReferenceStringFactory(FileReferenceStringFactory&& other) = default;

    std::shared_ptr<ProverReferenceString> get_prover_crs(size_t degree) override
    {
        return std::make_shared<FileReferenceString>(degree, path_);
    }

    std::shared_ptr<VerifierReferenceString> get_verifier_crs() override
    {
        return std::make_shared<VerifierFileReferenceString>(path_);
    }

  private:
    std::string path_;
};

class DynamicFileReferenceStringFactory : public ReferenceStringFactory {
  public:
    DynamicFileReferenceStringFactory(std::string path, size_t initial_degree = 0)
        : path_(std::move(path))
        , degree_(initial_degree)
        , verifier_crs_(std::make_shared<VerifierFileReferenceString>(path_))
    {}

    DynamicFileReferenceStringFactory(DynamicFileReferenceStringFactory&& other) = default;

    std::shared_ptr<ProverReferenceString> get_prover_crs(size_t degree) override
    {
        if (degree != degree_) {
            prover_crs_ = std::make_shared<FileReferenceString>(degree, path_);
            degree_ = degree;
        }
        return prover_crs_;
    }

    std::shared_ptr<VerifierReferenceString> get_verifier_crs() override { return verifier_crs_; }

  private:
    std::string path_;
    size_t degree_;
    std::shared_ptr<FileReferenceString> prover_crs_;
    std::shared_ptr<VerifierFileReferenceString> verifier_crs_;
};

} // namespace proof_system
