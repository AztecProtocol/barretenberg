#include "./plookup.hpp"
#include <plonk/composer/plookup_composer.hpp>
#include <plonk/composer/plookup_tables/plookup_tables.hpp>

namespace waffle {
class PlookupComposer;
} // namespace waffle

namespace plonk {
namespace stdlib {

using namespace barretenberg;

template <typename Composer>
std::array<std::vector<plonk::stdlib::field_t<Composer>>, 3> plookup_base<Composer>::read_sequence_from_table(
    const waffle::PlookupMultiTableId id,
    const field_t<Composer>& key_a_in,
    const field_t<Composer>& key_b_in,
    const bool is_2_to_1_lookup)
{
    auto key_a = key_a_in.normalize();
    auto key_b = key_b_in.normalize();
    Composer* ctx = key_a.get_context() ? key_a.get_context() : key_b.get_context();
    const auto sequence_data =
        waffle::plookup::get_table_values(id, key_a.get_value(), key_b.get_value(), is_2_to_1_lookup);

    std::array<std::vector<plonk::stdlib::field_t<Composer>>, 3> sequence_values;
    if (key_a.witness_index == IS_CONSTANT && key_b.witness_index == IS_CONSTANT) {
        for (size_t i = 0; i < sequence_data.column_1_accumulator_values.size(); ++i) {
            sequence_values[0].emplace_back(field_t<Composer>(ctx, sequence_data.column_1_accumulator_values[i]));
            sequence_values[1].emplace_back(field_t<Composer>(ctx, sequence_data.column_2_accumulator_values[i]));
            sequence_values[2].emplace_back(field_t<Composer>(ctx, sequence_data.column_3_accumulator_values[i]));
        }
    } else {
        auto key_b_witness = std::make_optional(key_b.witness_index);
        if (key_b.witness_index == IS_CONSTANT) {
            key_b_witness = std::nullopt;
        }

        const auto sequence_indices =
            ctx->read_sequence_from_multi_table(id, sequence_data, key_a.witness_index, key_b_witness);
        for (size_t i = 0; i < sequence_data.column_1_accumulator_values.size(); ++i) {
            sequence_values[0].emplace_back(field_t<Composer>::from_witness_index(ctx, sequence_indices[0][i]));
            sequence_values[1].emplace_back(field_t<Composer>::from_witness_index(ctx, sequence_indices[1][i]));
            sequence_values[2].emplace_back(field_t<Composer>::from_witness_index(ctx, sequence_indices[2][i]));
        }
    }
    return sequence_values;
}

template <typename Composer>
std::pair<field_t<Composer>, field_t<Composer>> plookup_base<Composer>::read_pair_from_table(
    const waffle::PlookupMultiTableId id, const field_t<Composer>& key)
{
    const auto sequence_elements = read_sequence_from_table(id, key);

    return { sequence_elements[1][0], sequence_elements[2][0] };
}

template <typename Composer>
field_t<Composer> plookup_base<Composer>::read_from_2_to_1_table(const waffle::PlookupMultiTableId id,
                                                                 const field_t<Composer>& key_a,
                                                                 const field_t<Composer>& key_b)
{
    const auto sequence_elements = read_sequence_from_table(id, key_a, key_b, true);

    return sequence_elements[1][0];
}

template <typename Composer>
field_t<Composer> plookup_base<Composer>::read_from_1_to_2_table(const waffle::PlookupMultiTableId id,
                                                                 const field_t<Composer>& key_a)
{
    const auto sequence_elements = read_sequence_from_table(id, key_a);

    return sequence_elements[1][0];
}

template class plookup_base<waffle::PlookupComposer>;
} // namespace stdlib
} // namespace plonk
