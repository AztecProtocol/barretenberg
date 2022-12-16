#pragma once
#include <crypto/pedersen/pedersen.hpp>
#include "../../primitives/composers/composers_fwd.hpp"
#include "../../primitives/field/field.hpp"
#include "../../primitives/point/point.hpp"
#include "../../primitives/byte_array/byte_array.hpp"

namespace plonk {
namespace stdlib {

constexpr uint64_t WNAF_MASK = crypto::pedersen::WNAF_MASK;

template <typename ComposerContext> class pedersen {
  private:
    typedef plonk::stdlib::field_t<ComposerContext> field_t;
    typedef plonk::stdlib::point<ComposerContext> point;
    typedef plonk::stdlib::byte_array<ComposerContext> byte_array;
    typedef plonk::stdlib::bool_t<ComposerContext> bool_t;

    static point hash_single(const field_t& in,
                             const crypto::pedersen::generator_index_t hash_index,
                             const bool validate_input_is_in_field = true);
    static point accumulate(const std::vector<point>& to_accumulate);

  public:
    // called unsafe because allowing the option of not validating the input elements are unique, i.e. <r
    static field_t compress_unsafe(const field_t& left,
                                   const field_t& right,
                                   const size_t hash_index,
                                   const bool validate_input_is_in_field);

    static field_t compress(const field_t& left, const field_t& right, const size_t hash_index = 0)
    {
        return compress_unsafe(left, right, hash_index, true);
    }

    static field_t compress(const std::vector<field_t>& inputs, const size_t hash_index = 0);

    template <size_t T> static field_t compress(const std::array<field_t, T>& inputs)
    {
        std::vector<field_t> in(inputs.begin(), inputs.end());
        return compress(in);
    }

    static field_t compress(const byte_array& inputs);

    static void validate_wnaf_is_in_field(ComposerContext* ctx, const std::vector<uint32_t>& accumulator);

  private:
    static point commit(const std::vector<field_t>& inputs, const size_t hash_index = 0);
};

extern template class pedersen<waffle::StandardComposer>;
extern template class pedersen<waffle::TurboComposer>;
extern template class pedersen<waffle::UltraComposer>;

} // namespace stdlib
} // namespace plonk