#include "block_constraint.hpp"
#include "barretenberg/stdlib/primitives/memory/rom_table.hpp"
#include "barretenberg/stdlib/primitives/memory/ram_table.hpp"

using namespace proof_system::plonk;

namespace acir_format {

// Returns a field_ct representing the poly_triple
// This function only works for constant or single witness poly_triple
// Supporting more general poly_triple for block constraints yield to complications when we have no witness assignment
// (e.g during verification or getting the circuit size)
field_ct poly_to_field_ct(const poly_triple poly, Composer& composer)
{
    ASSERT(poly.q_m == 0);
    ASSERT(poly.q_r == 0);
    ASSERT(poly.q_o == 0);
    if (poly.q_l == 0) {
        return field_ct(poly.q_c);
    }
    field_ct x = field_ct::from_witness_index(&composer, poly.a);
    ASSERT(poly.q_c == 0);
    ASSERT(poly.q_l == 1);
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
        rom_table_ct table(init);
        for (auto& op : constraint.trace) {
            ASSERT(op.access_type == 0);
            field_ct value = poly_to_field_ct(op.value, composer);
            field_ct index = poly_to_field_ct(op.index, composer);
            // For a ROM table, constant read should be optimised out:
            // The rom_table won't work with a constant read because the table may not be initialised
            ASSERT(op.index.q_l != 0);
            value.assert_equal(table[index]);
        }
    } break;
    case BlockType::RAM: {
        ram_table_ct table(init);
        for (auto& op : constraint.trace) {
            field_ct value = poly_to_field_ct(op.value, composer);
            field_ct index = poly_to_field_ct(op.index, composer);
            if (op.access_type == 0) {
                value.assert_equal(table.read(index));
            } else {
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