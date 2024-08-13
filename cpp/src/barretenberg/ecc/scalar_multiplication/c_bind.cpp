#include "scalar_multiplication.hpp"
#include <barretenberg/common/log.hpp>
#include <barretenberg/common/streams.hpp>
#include <barretenberg/common/wasm_export.hpp>
#include <barretenberg/ecc/curves/bn254/fr.hpp>
#include <barretenberg/ecc/curves/bn254/g1.hpp>
#include <barretenberg/ecc/curves/grumpkin/grumpkin.hpp>

template <typename Field> inline Field bn254_field_decode(Field* f_)
{
    if (!f_->get_bit(255)) {
        f_->self_to_montgomery_form();
        f_->set_bit(255, true);
    }

    auto f = *f_;
    f.set_bit(255, false);
    return f;
}

using Point = struct {
    bb::fr x;
    bb::fr y;
    uint256_t is_infinity;
};

// Fq has to be split over 2 limbs as it doesn't fit in Fr.
using Scalar = struct {
    bb::fr lo;
    bb::fr hi;
};

WASM_EXPORT void blackbox_msm(Point* points_, size_t num_fields, Scalar* scalars_, Point* output)
{
    using namespace bb::scalar_multiplication;
    std::vector<bb::grumpkin::g1::affine_element> points;
    std::vector<bb::fq> scalars;
    auto num_points = num_fields / 3;
    points.reserve(num_points);
    scalars.reserve(num_points);

    for (size_t i = 0; i < num_points; ++i) {
        Point& p = points_[i];
        auto x = bn254_field_decode(&p.x);
        auto y = bn254_field_decode(&p.y);

        if (p.is_infinity) {
            points.emplace_back(bb::grumpkin::g1::point_at_infinity);
        } else {
            bb::grumpkin::g1::affine_element c(x, y);
            points.emplace_back(c);
        }

        auto shi = (uint256_t)bn254_field_decode(&scalars_[i].hi);
        auto slo = (uint256_t)bn254_field_decode(&scalars_[i].lo);
        auto s = slo | (shi << 128);
        scalars.emplace_back(s);
    }

    // Think this actually requires a weird point table layout.
    // pippenger_runtime_state<bb::curve::Grumpkin> run_state(num_points);
    // auto e = pippenger<bb::curve::Grumpkin>(scalars.data(), points.data(), num_points, run_state);

    auto e = bb::grumpkin::g1::point_at_infinity;
    for (size_t i = 0; i < num_points; ++i) {
        e += points[i] * scalars[i];
    }

    auto el = e.normalize();
    output->is_infinity = el.is_point_at_infinity();
    output->x = el.x;
    output->y = el.y;
    output->x.set_bit(255, true);
    output->y.set_bit(255, true);
}

WASM_EXPORT void blackbox_ecc_add(
    bb::fr* x1, bb::fr* y1, uint256_t* i1, bb::fr* x2, bb::fr* y2, uint256_t* i2, Point* output)
{
    auto input1 = *i1 ? bb::grumpkin::g1::affine_point_at_infinity
                      : bb::grumpkin::g1::affine_element(bn254_field_decode(x1), bn254_field_decode(y1));
    auto input2 = *i2 ? bb::grumpkin::g1::affine_point_at_infinity
                      : bb::grumpkin::g1::affine_element(bn254_field_decode(x2), bn254_field_decode(y2));
    auto r = input1 + input2;
    output->is_infinity = r.is_point_at_infinity();
    output->x = r.x;
    output->y = r.y;
    output->x.set_bit(255, true);
    output->y.set_bit(255, true);
}