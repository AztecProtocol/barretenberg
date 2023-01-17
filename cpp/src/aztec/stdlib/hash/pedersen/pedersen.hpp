#pragma once
#include <crypto/pedersen_hash/pedersen.hpp>
#include "../../primitives/composers/composers_fwd.hpp"
#include "../../primitives/field/field.hpp"
#include "../../primitives/point/point.hpp"

namespace plonk {
namespace stdlib {

using namespace barretenberg;
using namespace crypto::pedersen_hash;

template <typename ComposerContext> class pedersen_hash {

  public:
    typedef plonk::stdlib::field_t<ComposerContext> field_t;
    typedef plonk::stdlib::point<ComposerContext> point;
    typedef plonk::stdlib::bool_t<ComposerContext> bool_t;

  private:
    static point add_points(const point& first, const point& second);

  public:
    static void validate_wnaf_is_in_field(ComposerContext* ctx, const std::vector<uint32_t>& accumulator);

    static point accumulate(const std::vector<point>& to_accumulate);

    static point hash_single(const field_t& in,
                             const crypto::generators::generator_index_t hash_index,
                             const bool validate_input_is_in_field = true);

    static field_t hash_multiple(const std::vector<field_t>& in,
                                 const size_t hash_index = 0,
                                 const bool validate_inputs_in_field = true);
};

EXTERN_STDLIB_TYPE(pedersen_hash);

} // namespace stdlib
} // namespace plonk