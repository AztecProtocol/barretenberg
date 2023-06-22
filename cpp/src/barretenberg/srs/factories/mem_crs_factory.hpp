#pragma once
#include "barretenberg/ecc/curves/bn254/bn254.hpp"
#include "crs_factory.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/ecc/curves/bn254/g2.hpp"
#include "barretenberg/ecc/scalar_multiplication/point_table.hpp"
#include "barretenberg/ecc/scalar_multiplication/scalar_multiplication.hpp"
#include <utility>
#include <cstddef>

namespace barretenberg::srs::factories {

/**
 * Create reference strings given pointers to in memory buffers.
 */
template <typename Curve> class MemCrsFactory : public CrsFactory<Curve> {
  public:
    // MemCrsFactory(std::vector<g1::affine_element> const& points, g2::affine_element const g2_point);
    MemCrsFactory(MemCrsFactory&& other) = default;

    std::shared_ptr<ProverCrs<Curve>> get_prover_crs(size_t) { return prover_crs_; };

    std::shared_ptr<VerifierCrs<Curve>> get_verifier_crs(size_t) { return verifier_crs_; };

  private:
    std::shared_ptr<ProverCrs<Curve>> prover_crs_;
    std::shared_ptr<VerifierCrs<Curve>> verifier_crs_;
};

template <typename Curve> class MemProverCrs : public ProverCrs<Curve> {
  public:
    MemProverCrs(std::vector<typename Curve::AffineElement> const& points)
        : num_points(points.size())
    {
        monomials_ = scalar_multiplication::point_table_alloc<typename Curve::AffineElement>(num_points);
        std::copy(points.begin(), points.end(), monomials_.get());
        scalar_multiplication::generate_pippenger_point_table<Curve>(monomials_.get(), monomials_.get(), num_points);
    }

    typename Curve::AffineElement* get_monomial_points() override { return monomials_.get(); }

    size_t get_monomial_size() const override { return num_points; }

  private:
    size_t num_points;
    std::shared_ptr<typename Curve::AffineElement[]> monomials_;
};

template <typename Curve> class MemVerifierCrs : public VerifierCrs<Curve> {
  private:
    size_t num_points;
};

extern template class MemProverCrs<curve::BN254>;
extern template class MemProverCrs<curve::Grumpkin>;

} // namespace barretenberg::srs::factories
