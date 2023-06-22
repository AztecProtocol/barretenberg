#include "mem_crs_factory.hpp"
#include "barretenberg/ecc/curves/bn254/bn254.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/ecc/curves/bn254/pairing.hpp"
#include "barretenberg/ecc/scalar_multiplication/point_table.hpp"
#include "barretenberg/ecc/scalar_multiplication/scalar_multiplication.hpp"

namespace barretenberg::srs::factories {

template <> class MemVerifierCrs<curve::BN254> {
  public:
    MemVerifierCrs(g2::affine_element const& g2_point)
        : g2_x(g2_point)
    {
        precomputed_g2_lines =
            (pairing::miller_lines*)(aligned_alloc(64, sizeof(barretenberg::pairing::miller_lines) * 2));
        barretenberg::pairing::precompute_miller_lines(barretenberg::g2::one, precomputed_g2_lines[0]);
        barretenberg::pairing::precompute_miller_lines(g2_x, precomputed_g2_lines[1]);
    }

    ~MemVerifierCrs() { aligned_free(precomputed_g2_lines); }

    g2::affine_element get_g2x() const { return g2_x; }

    pairing::miller_lines const* get_precomputed_g2_lines() const { return precomputed_g2_lines; }

  private:
    g2::affine_element g2_x;
    pairing::miller_lines* precomputed_g2_lines;
};

template <> class MemVerifierCrs<curve::Grumpkin> {
    using Curve = curve::Grumpkin;

  public:
    MemVerifierCrs(std::vector<typename Curve::AffineElement> const& points)
        : num_points(points.size())
    {
        monomials_ = scalar_multiplication::point_table_alloc<typename Curve::AffineElement>(num_points);
        std::copy(points.begin(), points.end(), monomials_.get());
        scalar_multiplication::generate_pippenger_point_table<Curve>(monomials_.get(), monomials_.get(), num_points);
    }

    typename Curve::AffineElement* get_monomial_points() { return monomials_.get(); }

    size_t get_monomial_size() const { return num_points; }

  private:
    size_t num_points;
    std::shared_ptr<typename Curve::AffineElement[]> monomials_;
};

template <> class MemCrsFactory<curve::BN254> {
    using Curve = curve::BN254;

    MemCrsFactory(std::vector<Curve::AffineElement> const& points, Curve::G2AffineElement const g2_point)
        : prover_crs_(std::make_shared<MemProverCrs<Curve>>(points))
        , verifier_crs_(std::make_shared<MemVerifierCrs<Curve>>(g2_point))
    {}
};

template <> class MemCrsFactory<curve::Grumpkin> {
    using Curve = curve::Grumpkin;

    MemCrsFactory(std::vector<Curve::AffineElement> const& points)
        : prover_crs_(std::make_shared<MemProverCrs<Curve>>(points))
        , verifier_crs_(std::make_shared<MemVerifierCrs<Curve>>(points))
    {}
};

template class MemProverCrs<curve::BN254>;
template class MemProverCrs<curve::Grumpkin>;

} // namespace barretenberg::srs::factories