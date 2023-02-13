#include "pedersen_plookup.hpp"
#include <crypto/pedersen_commitment/pedersen.hpp>
#include <ecc/curves/grumpkin/grumpkin.hpp>
#include "../../hash/pedersen/pedersen_plookup.hpp"

#include <plonk/composer/plookup_tables/types.hpp>
#include "../../primitives/composers/composers.hpp"
#include "../../primitives/plookup/plookup.hpp"

namespace plonk {
namespace stdlib {

using namespace barretenberg;

template <typename C>
point<C> pedersen_plookup_commitment<C>::compress_to_point(const field_t& left, const field_t& right)
{
    auto p2 = pedersen_plookup_hash<C>::hash_single(left, false);
    auto p1 = pedersen_plookup_hash<C>::hash_single(right, true);

    return pedersen_plookup_hash<C>::add_points(p1, p2);
}

template <typename C> field_t<C> pedersen_plookup_commitment<C>::compress(const field_t& left, const field_t& right)
{
    return compress_to_point(left, right).x;
}

template <typename C>
point<C> pedersen_plookup_commitment<C>::merkle_damgard_compress(const std::vector<field_t>& inputs, const field_t& iv)
{
    if (inputs.size() == 0) {
        return point{ 0, 0 };
    }

    auto result = plookup_read::get_lookup_accumulators(MultiTableId::PEDERSEN_IV, iv)[ColumnIdx::C2][0];
    auto num_inputs = inputs.size();
    for (size_t i = 0; i < num_inputs; i++) {
        result = compress(result, inputs[i]);
    }

    return compress_to_point(result, field_t(num_inputs));
}

template <typename C>
point<C> pedersen_plookup_commitment<C>::commit(const std::vector<field_t>& inputs, const size_t hash_index)
{
    return merkle_damgard_compress(inputs, field_t(hash_index));
}

template <typename C>
field_t<C> pedersen_plookup_commitment<C>::compress(const std::vector<field_t>& inputs, const size_t hash_index)
{
    return commit(inputs, hash_index).x;
}

template class pedersen_plookup_commitment<waffle::UltraComposer>;

} // namespace stdlib
} // namespace plonk