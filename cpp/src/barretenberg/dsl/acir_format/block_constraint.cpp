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
        return field_ct(poly.q_c);
    }
    field_ct x = field_ct::from_witness_index(&composer, poly.a);
    // info("x: ", x);
    x.additive_constant = poly.q_c;
    x.multiplicative_constant = poly.q_l;
    return x;
}

void create_block_constraints(Composer& composer, const BlockConstraint constraint, bool has_valid_witness_assignments)
{
    std::vector<field_ct> init;
    for (auto i : constraint.init) {
        field_ct value = poly_to_field_ct(i, composer);
        init.push_back(value);
    }

    switch (constraint.type) {
    case BlockType::ROM: {
        info("rom len:", init.size());
        if (has_valid_witness_assignments == false) {
            info("no assignement");
        }
        rom_table_ct table(init);
        for (auto& op : constraint.trace) {
            ASSERT(op.access_type == 0);
            field_ct value = poly_to_field_ct(op.value, composer);
            field_ct index = poly_to_field_ct(op.index, composer);
            // For a ROM table, constant read should be optimised out:
            // The rom_table won't work with a constant read because the table may not be initialised
            ASSERT(op.index.q_l != 0);
            // We create a new witness w to avoid issues with non-valid witness assignements:
            // if witness are not assigned, then w will be zero and table[w] will work
            fr w_value = 0;
            if (has_valid_witness_assignments) {
                // If witness are assigned, we use the correct value for w
                w_value = index.get_value();
            }
            field_ct w = field_ct::from_witness(&composer, w_value);
            value.assert_equal(table[w]);
            w.assert_equal(index);
        }
    } break;
    case BlockType::RAM: {
        info("ram len:", init.size());
        ram_table_ct table(init);
        for (auto& op : constraint.trace) {
            field_ct value = poly_to_field_ct(op.value, composer);
            field_ct index = poly_to_field_ct(op.index, composer);

            fr w_value = 0;
            if (has_valid_witness_assignments == false) {
                info("no assignement");
                // Without this line we get a failure on `assert_equal`
                index = field_ct(0);
            } else {
                info("assignement");
                w_value = index.get_value();
                info("assignement:", w_value);
            }
            field_ct w = field_ct::from_witness(&composer, w_value);
            if (op.access_type == 0) {
                field_ct v1 = table.read(w);
                info(v1);
                info(value);
                value.assert_equal(v1);
            } else {
                ASSERT(op.access_type == 1);
                table.write(w, value);
            }
            info(w);
            info("index:", index);
            w.assert_equal(index);
        }
    } break;
    default:
        ASSERT(false);
        break;
    }
}

} // namespace acir_format