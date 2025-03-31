#include "barretenberg/vm2/simulation/execution.hpp"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <functional>

#include "barretenberg/vm2/common/memory_types.hpp"
#include "barretenberg/vm2/common/opcodes.hpp"
#include "barretenberg/vm2/simulation/addressing.hpp"
#include "barretenberg/vm2/simulation/context.hpp"
#include "barretenberg/vm2/simulation/events/execution_event.hpp"

namespace bb::avm2::simulation {

void Execution::add(ContextInterface& context, MemoryAddress a_addr, MemoryAddress b_addr, MemoryAddress dst_addr)
{
    auto& memory = context.get_memory();
    ValueRefAndTag a = memory.get(a_addr);
    ValueRefAndTag b = memory.get(b_addr);
    FF c = alu.add(a, b);
    memory.set(dst_addr, c, a.tag);
}

// TODO: My dispatch system makes me have a uint8_t tag. Rethink.
void Execution::set(ContextInterface& context, MemoryAddress dst_addr, uint8_t tag, MemoryValue value)
{
    context.get_memory().set(dst_addr, std::move(value), static_cast<MemoryTag>(tag));
}

void Execution::mov(ContextInterface& context, MemoryAddress src_addr, MemoryAddress dst_addr)
{
    auto& memory = context.get_memory();
    auto [value, tag] = memory.get(src_addr);
    memory.set(dst_addr, value, tag);
}

void Execution::call(ContextInterface& context, MemoryAddress addr, MemoryAddress cd_offset, MemoryAddress cd_size)
{
    // Emit Snapshot of current context
    emit_context_snapshot(context);

    auto& memory = context.get_memory();

    // TODO: Read more stuff from call operands (e.g., calldata, gas)
    // TODO(ilyas): How will we tag check these?
    const auto [contract_address, _] = memory.get(addr);

    // We could load cd_size here, but to keep symmetry with cd_offset - we will defer the loads to a (possible)
    // calldatacopy
    auto nested_context = execution_components.make_nested_context(contract_address,
                                                                   /*msg_sender=*/context.get_address(),
                                                                   /*parent_context=*/context,
                                                                   /*cd_offset_addr=*/cd_offset,
                                                                   /*cd_size_addr=*/cd_size,
                                                                   /*is_static=*/false);

    // We recurse. When we return, we'll continue with the current loop and emit the execution event.
    // That event will be out of order, but it will have the right order id. It should be sorted in tracegen.
    auto result = execute_internal(*nested_context);

    // TODO: do more things based on the result. This happens in the parent context
    // 1) Accept / Reject side effects (e.g. tree state, newly emitted nullifiers, notes, public writes)
    // 2) Set return data information
    context.set_child_context(std::move(nested_context));
    // TODO(ilyas): Look into just having a setter using ExecutionResult, but this gives us more flexibility
    context.set_last_rd_offset(result.rd_offset);
    context.set_last_rd_size(result.rd_size);
    context.set_last_success(result.success);
}

void Execution::ret(ContextInterface& context, MemoryAddress ret_offset, MemoryAddress ret_size_offset)
{
    set_execution_result({ .rd_offset = ret_offset, .rd_size = ret_size_offset, .success = true });
    context.halt();
}

void Execution::jump(ContextInterface& context, uint32_t loc)
{
    context.set_next_pc(loc);
}

void Execution::jumpi(ContextInterface& context, MemoryAddress cond_addr, uint32_t loc)
{
    auto& memory = context.get_memory();

    // TODO: in gadget.
    auto resolved_cond = memory.get(cond_addr);
    if (!resolved_cond.value.is_zero()) {
        context.set_next_pc(loc);
    }
}

// This context interface is an top-level enqueued one
ExecutionResult Execution::execute(ContextInterface& context)
{
    auto result = execute_internal(context);
    return result;
}

ExecutionResult Execution::execute_internal(ContextInterface& context)
{
    while (!context.halted()) {
        // This allocates an order id for the event.
        auto ex_event = ExecutionEvent::allocate();

        // We'll be filling in the event as we go. And we always emit at the end.
        try {
            // Basic pc and bytecode setup.
            auto pc = context.get_pc();
            ex_event.bytecode_id = context.get_bytecode_manager().get_bytecode_id();

            // We try to fetch an instruction.
            // WARNING: the bytecode has already been fetched in make_context. Maybe it is wrong and should be here.
            // But then we have no way to know the bytecode id when constructing the manager.
            Instruction instruction = context.get_bytecode_manager().read_instruction(pc);
            ex_event.wire_instruction = instruction;

            // Go from a wire instruction to an execution opcode.
            const WireInstructionSpec& wire_spec = instruction_info_db.get(instruction.opcode);
            context.set_next_pc(pc + wire_spec.size_in_bytes);
            info("@", pc, " ", instruction.to_string());
            ExecutionOpCode opcode = wire_spec.exec_opcode;
            ex_event.opcode = opcode;

            // Resolve the operands.
            auto addressing = execution_components.make_addressing(ex_event.addressing_event);
            std::vector<Operand> resolved_operands = addressing->resolve(instruction, context.get_memory());
            ex_event.resolved_operands = resolved_operands;

            // "Emit" the context event
            // TODO: think about whether we need to know the success at this point
            auto context_event = context.serialize_context_event();
            ex_event.context_event = context_event;

            // Execute the opcode.
            dispatch_opcode(opcode, context, resolved_operands);

            // Move on to the next pc.
            context.set_pc(context.get_next_pc());
        } catch (const std::exception& e) {
            info("Error: ", e.what());
            // Bah, we are done (for now).
            // TODO: we eventually want this to just set and handle exceptional halt.
            return {
                .success = true,
            };
        }

        events.emit(std::move(ex_event));
    }

    // FIXME: Should return an ExecutionResult.
    return get_execution_result();
}

void Execution::dispatch_opcode(ExecutionOpCode opcode,
                                ContextInterface& context,
                                const std::vector<Operand>& resolved_operands)
{
    switch (opcode) {
    case ExecutionOpCode::ADD:
        call_with_operands(&Execution::add, context, resolved_operands);
        break;
    case ExecutionOpCode::SET:
        call_with_operands(&Execution::set, context, resolved_operands);
        break;
    case ExecutionOpCode::MOV:
        call_with_operands(&Execution::mov, context, resolved_operands);
        break;
    case ExecutionOpCode::CALL:
        call_with_operands(&Execution::call, context, resolved_operands);
        break;
    case ExecutionOpCode::RETURN:
        call_with_operands(&Execution::ret, context, resolved_operands);
        break;
    case ExecutionOpCode::JUMP:
        call_with_operands(&Execution::jump, context, resolved_operands);
        break;
    case ExecutionOpCode::JUMPI:
        call_with_operands(&Execution::jumpi, context, resolved_operands);
        break;
    default:
        // TODO: should be caught by parsing.
        throw std::runtime_error("Unknown opcode");
    }
}

// Some template magic to dispatch the opcode by deducing the number of arguments and types,
// and making the appropriate checks and casts.
template <typename... Ts>
inline void Execution::call_with_operands(void (Execution::*f)(ContextInterface&, Ts...),
                                          ContextInterface& context,
                                          const std::vector<Operand>& resolved_operands)
{
    assert(resolved_operands.size() == sizeof...(Ts));
    auto operand_indices = std::make_index_sequence<sizeof...(Ts)>{};
    using types = std::tuple<Ts...>;
    [f, this, &context, &resolved_operands]<std::size_t... Is>(std::index_sequence<Is...>) {
        (this->*f)(context, static_cast<std::tuple_element_t<Is, types>>(resolved_operands[Is])...);
    }(operand_indices);
}

void Execution::emit_context_snapshot(ContextInterface& context)
{
    ctx_stack_events.emit({ .id = context.get_context_id(),
                            .next_pc = context.get_next_pc(),
                            .msg_sender = context.get_msg_sender(),
                            .contract_addr = context.get_address(),
                            .is_static = context.get_is_static() });
};

} // namespace bb::avm2::simulation
