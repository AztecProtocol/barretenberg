#include "block_constraint.hpp"
//#include "round.hpp"
//#include "stdlib/types/types.hpp"
#include "stdlib/primitives/memory/rom_table.hpp"
//using namespace plonk::stdlib::types;

namespace acir_format {

void create_block_constraints(plonk::UltraComposer& composer, const BlockConstraint constraint)
{
   switch (constraint.type)
   {
     case BlockType::ROM:
     {
          stdlib::rom_table<plonk::UltraComposer> table(constraint.init);
          for (auto& op: constraint.trace)
          {
               ASSERT(op.access_type.get_value() == barretenberg::fr(0));
               op.value.assert_equal(table[op.index]);
          }
     }
     break;
     case BlockType::RAM:
          create_block_constraints_for_RAM(composer, constraint);
     break;
     default:
          ASSERT(false);
     break;
   }
   

}

     void create_block_constraints_for_RAM(plonk::UltraComposer& composer, const BlockConstraint constraint)
     {
     size_t block_id = composer.create_RAM_array(constraint.init.size());
     size_t len = constraint.init.size();
     for (size_t i = 0; i < len; ++i)
     {
          auto val = constraint.init[i];
          //quid isi constant ??  assert(val.is_constant());
          composer.init_RAM_element(block_id, i, val.normalize().get_witness_index());

     }

     for (auto& op: constraint.trace)
     {
          //!! op.index.normalize().get_witness_index() est INCORRECT
          // il faut creer un witness si pas constant => on peut utiliser la ram_table une fois qu'elle est mergee.
          if (op.access_type.get_value() == barretenberg::fr(0))
          {
               uint32_t load = composer.read_RAM_array(block_id, op.index.normalize().get_witness_index());
               op.value.assert_equal(load);
          } else
          {
               ASSERT(op.access_type.get_value() == barretenberg::fr(1));
               composer.write_RAM_array(block_id, op.index.normalize().get_witness_index(), op.value.normalize().get_witness_index());
          }
          
     }
     }


}