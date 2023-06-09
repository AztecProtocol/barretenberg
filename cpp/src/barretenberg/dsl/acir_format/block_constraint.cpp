#include "block_constraint.hpp"
#include "barretenberg/stdlib/primitives/memory/rom_table.hpp"
#include "barretenberg/stdlib/primitives/memory/ram_table.hpp"

using namespace proof_system::plonk;

namespace acir_format {
field_ct poly_to_field_ct(const poly_triple poly, Composer& composer)
{
    ASSERT(poly.q_m == 0);
    ASSERT(poly.q_r == 0);
    ASSERT(poly.q_o == 0);
    if (poly.q_l == 0) {
        // info("poly.q_c: ", poly.q_c);
        return field_ct(poly.q_c);
    }
    // info("poly.a: ", poly.a);
    field_ct x = field_ct::from_witness_index(&composer, poly.a);
    // info("x: ", x.get_value());
    x.additive_constant = uint256_t(poly.q_c);
    x.multiplicative_constant = uint256_t(poly.q_l);
    return x;
}

barretenberg::fr poly_to_field(const poly_triple poly, Composer& composer, bool has_valid_witness_assignments)
{
    if (!has_valid_witness_assignments) {
        return 0;
    }
    ASSERT(poly.q_m == 0);
    ASSERT(poly.q_r == 0);
    ASSERT(poly.q_o == 0);
    if (poly.q_l == 0) {
        // info("poly.q_c: ", poly.q_c);
        return poly.q_c;
    }
    // info("poly.a: ", poly.a);
    barretenberg::fr dummy_field;
    if (has_valid_witness_assignments) {
        barretenberg::fr x = composer.get_variable(poly.a);
        x = (x * poly.q_l) + poly.q_c;
        dummy_field = x;
    }
    return dummy_field;
}

void create_block_constraints(Composer& composer, const BlockConstraint constraint, bool has_valid_witness_assignments)
{
    std::vector<field_ct> init;
    for (auto i : constraint.init) {
        auto value2 = poly_to_field(i, composer, has_valid_witness_assignments);
        auto v = field_ct::from_witness(&composer, value2);
        info("v.witness_index: ", v.witness_index);
        init.push_back(v);
    }

    switch (constraint.type) {
    case BlockType::ROM: {
        info("rom len:", init.size());
        if (has_valid_witness_assignments == false) {
            info("no assignement");
        }
        rom_table_ct table(init);
        // table.initialize_table();
        for (auto& op : constraint.trace) {
            ASSERT(op.access_type == 0);
            // field_ct value = poly_to_field_ct(op.value, composer);
            // field_ct index = poly_to_field_ct(op.index, composer);
            // info("value: ", value);
            // info("index: ", index);
            // maxim's
            // auto value2 = poly_to_field(op.value, composer, has_valid_witness_assignments);
            // auto index2 = poly_to_field(op.index, composer, has_valid_witness_assignments);
            // auto value = field_ct::from_witness(&composer, value2);
            // composer.assert_equal(composer.add_variable(value2), value.get_witness_index());

            // auto index = field_ct::from_witness(&composer, index2);
            // composer.assert_equal(composer.add_variable(index2), index.get_witness_index());
            auto value = poly_to_field_ct(op.value, composer);
            auto index = poly_to_field_ct(op.index, composer);

            info("value: ", value);
            info("index: ", index);
            auto v1 = table[index];
            info("value: ", value);
            info("v1: ", v1);
            value.assert_equal(v1);
        }
    } break;
    case BlockType::RAM: {
        // auto table_size = constraint.init.size();
        // std::vector<fr> table_values(table_size);
        // ram_table_ct table(&composer, table_size);
        // for (size_t i = 0; i < table_size; ++i) {
        //     auto value2 = poly_to_field(constraint.init[i], composer, has_valid_witness_assignments);
        //     info("value2: ", value2);
        //     auto v = field_ct::from_witness(&composer, value2);
        //     info("v: ", v);
        //     table.write(i, v);
        // }
        info("ram len:", init.size());
        ram_table_ct table(init);
        for (auto& op : constraint.trace) {
            // maxim's...
            // auto value2 = poly_to_field(op.value, composer, has_valid_witness_assignments);
            // auto index2 = poly_to_field(op.index, composer, has_valid_witness_assignments);

            // auto value = field_ct::from_witness(&composer, value2);//new input with value = value2
            // composer.assert_equal(composer.add_variable(value2), value.get_witness_index());

            // auto index = field_ct::from_witness(&composer, index2);
            // composer.assert_equal(composer.add_variable(index2), index.get_witness_index());

            auto value = poly_to_field_ct(op.value, composer);
            auto index = poly_to_field_ct(op.index, composer);
            info("value: ", value);
            info("value.wit: ", value.witness_index);
            info("index: ", index);
            info("index.wit: ", index.witness_index);
            if (op.access_type == 0) {
                field_ct v1 = table.read(index);
                info("v1: ", v1);
                info("value: ", value);
                value.assert_equal(v1);
            } else {
                info("about to write inside block constraint");
                ASSERT(op.access_type == 1);
                table.write(index, value);
            }
        }
    } break;
    default:
        ASSERT(false);
        break;
    }
}

} // namespace acir_format