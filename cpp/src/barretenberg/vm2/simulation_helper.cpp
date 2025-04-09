#include "barretenberg/vm2/simulation_helper.hpp"

#include <cstdint>

#include "barretenberg/common/log.hpp"
#include "barretenberg/vm2/common/avm_inputs.hpp"
#include "barretenberg/vm2/common/aztec_types.hpp"
#include "barretenberg/vm2/common/field.hpp"
#include "barretenberg/vm2/simulation/addressing.hpp"
#include "barretenberg/vm2/simulation/alu.hpp"
#include "barretenberg/vm2/simulation/bytecode_manager.hpp"
#include "barretenberg/vm2/simulation/concrete_dbs.hpp"
#include "barretenberg/vm2/simulation/context.hpp"
#include "barretenberg/vm2/simulation/ecc.hpp"
#include "barretenberg/vm2/simulation/events/address_derivation_event.hpp"
#include "barretenberg/vm2/simulation/events/addressing_event.hpp"
#include "barretenberg/vm2/simulation/events/alu_event.hpp"
#include "barretenberg/vm2/simulation/events/bitwise_event.hpp"
#include "barretenberg/vm2/simulation/events/bytecode_events.hpp"
#include "barretenberg/vm2/simulation/events/class_id_derivation_event.hpp"
#include "barretenberg/vm2/simulation/events/ecc_events.hpp"
#include "barretenberg/vm2/simulation/events/event_emitter.hpp"
#include "barretenberg/vm2/simulation/events/execution_event.hpp"
#include "barretenberg/vm2/simulation/events/field_gt_event.hpp"
#include "barretenberg/vm2/simulation/events/memory_event.hpp"
#include "barretenberg/vm2/simulation/events/merkle_check_event.hpp"
#include "barretenberg/vm2/simulation/events/public_data_tree_read_event.hpp"
#include "barretenberg/vm2/simulation/events/range_check_event.hpp"
#include "barretenberg/vm2/simulation/events/sha256_event.hpp"
#include "barretenberg/vm2/simulation/events/siloing_event.hpp"
#include "barretenberg/vm2/simulation/events/to_radix_event.hpp"
#include "barretenberg/vm2/simulation/events/update_check.hpp"
#include "barretenberg/vm2/simulation/execution.hpp"
#include "barretenberg/vm2/simulation/execution_components.hpp"
#include "barretenberg/vm2/simulation/field_gt.hpp"
#include "barretenberg/vm2/simulation/lib/instruction_info.hpp"
#include "barretenberg/vm2/simulation/lib/raw_data_dbs.hpp"
#include "barretenberg/vm2/simulation/merkle_check.hpp"
#include "barretenberg/vm2/simulation/poseidon2.hpp"
#include "barretenberg/vm2/simulation/range_check.hpp"
#include "barretenberg/vm2/simulation/sha256.hpp"
#include "barretenberg/vm2/simulation/siloing.hpp"
#include "barretenberg/vm2/simulation/to_radix.hpp"
#include "barretenberg/vm2/simulation/tx_execution.hpp"
#include "barretenberg/vm2/simulation/update_check.hpp"

namespace bb::avm2 {

using namespace bb::avm2::simulation;

namespace {

// Configuration for full simulation (for proving).
struct ProvingSettings {
    template <typename E> using DefaultEventEmitter = EventEmitter<E>;
    template <typename E> using DefaultDeduplicatingEventEmitter = DeduplicatingEventEmitter<E>;
};

// Configuration for fast simulation.
struct FastSettings {
    template <typename E> using DefaultEventEmitter = NoopEventEmitter<E>;
    template <typename E> using DefaultDeduplicatingEventEmitter = NoopEventEmitter<E>;
};

} // namespace

template <typename S> EventsContainer AvmSimulationHelper::simulate_with_settings()
{
    typename S::template DefaultEventEmitter<ExecutionEvent> execution_emitter;
    typename S::template DefaultDeduplicatingEventEmitter<AluEvent> alu_emitter;
    typename S::template DefaultEventEmitter<BitwiseEvent> bitwise_emitter;
    typename S::template DefaultEventEmitter<MemoryEvent> memory_emitter;
    typename S::template DefaultEventEmitter<BytecodeRetrievalEvent> bytecode_retrieval_emitter;
    typename S::template DefaultEventEmitter<BytecodeHashingEvent> bytecode_hashing_emitter;
    typename S::template DefaultEventEmitter<BytecodeDecompositionEvent> bytecode_decomposition_emitter;
    typename S::template DefaultDeduplicatingEventEmitter<InstructionFetchingEvent> instruction_fetching_emitter;
    typename S::template DefaultEventEmitter<AddressDerivationEvent> address_derivation_emitter;
    typename S::template DefaultEventEmitter<ClassIdDerivationEvent> class_id_derivation_emitter;
    typename S::template DefaultEventEmitter<SiloingEvent> siloing_emitter;
    typename S::template DefaultEventEmitter<Sha256CompressionEvent> sha256_compression_emitter;
    typename S::template DefaultEventEmitter<EccAddEvent> ecc_add_emitter;
    typename S::template DefaultEventEmitter<ScalarMulEvent> scalar_mul_emitter;
    typename S::template DefaultEventEmitter<Poseidon2HashEvent> poseidon2_hash_emitter;
    typename S::template DefaultEventEmitter<Poseidon2PermutationEvent> poseidon2_perm_emitter;
    typename S::template DefaultEventEmitter<ToRadixEvent> to_radix_emitter;
    typename S::template DefaultEventEmitter<FieldGreaterThanEvent> field_gt_emitter;
    typename S::template DefaultEventEmitter<MerkleCheckEvent> merkle_check_emitter;
    typename S::template DefaultDeduplicatingEventEmitter<RangeCheckEvent> range_check_emitter;
    typename S::template DefaultEventEmitter<ContextStackEvent> context_stack_emitter;
    typename S::template DefaultEventEmitter<PublicDataTreeReadEvent> public_data_read_emitter;
    typename S::template DefaultEventEmitter<UpdateCheckEvent> update_check_emitter;

    uint32_t current_block_number = static_cast<uint32_t>(hints.tx.globalVariables.blockNumber);

    Poseidon2 poseidon2(poseidon2_hash_emitter, poseidon2_perm_emitter);
    ToRadix to_radix(to_radix_emitter);
    Ecc ecc(to_radix, ecc_add_emitter, scalar_mul_emitter);
    MerkleCheck merkle_check(poseidon2, merkle_check_emitter);
    RangeCheck range_check(range_check_emitter);
    FieldGreaterThan field_gt(range_check, field_gt_emitter);
    PublicDataTreeCheck public_data_tree_check(poseidon2, merkle_check, field_gt, public_data_read_emitter);

    AddressDerivation address_derivation(poseidon2, ecc, address_derivation_emitter);
    ClassIdDerivation class_id_derivation(poseidon2, class_id_derivation_emitter);
    HintedRawContractDB raw_contract_db(hints);
    HintedRawMerkleDB raw_merkle_db(hints);
    ContractDB contract_db(raw_contract_db, address_derivation, class_id_derivation);
    MerkleDB merkle_db(raw_merkle_db, public_data_tree_check);
    UpdateCheck update_check(poseidon2, range_check, merkle_db, current_block_number, update_check_emitter);

    BytecodeHasher bytecode_hasher(poseidon2, bytecode_hashing_emitter);
    Siloing siloing(siloing_emitter);
    InstructionInfoDB instruction_info_db;
    TxBytecodeManager bytecode_manager(contract_db,
                                       merkle_db,
                                       siloing,
                                       bytecode_hasher,
                                       range_check,
                                       update_check,
                                       current_block_number,
                                       bytecode_retrieval_emitter,
                                       bytecode_decomposition_emitter,
                                       instruction_fetching_emitter);
    ExecutionComponentsProvider execution_components(bytecode_manager, memory_emitter, instruction_info_db);

    Alu alu(alu_emitter);
    Execution execution(alu, execution_components, instruction_info_db, execution_emitter, context_stack_emitter);
    TxExecution tx_execution(execution, merkle_db);
    Sha256 sha256(sha256_compression_emitter);

    tx_execution.simulate(hints.tx);

    return { execution_emitter.dump_events(),
             alu_emitter.dump_events(),
             bitwise_emitter.dump_events(),
             memory_emitter.dump_events(),
             bytecode_retrieval_emitter.dump_events(),
             bytecode_hashing_emitter.dump_events(),
             bytecode_decomposition_emitter.dump_events(),
             instruction_fetching_emitter.dump_events(),
             address_derivation_emitter.dump_events(),
             class_id_derivation_emitter.dump_events(),
             siloing_emitter.dump_events(),
             sha256_compression_emitter.dump_events(),
             ecc_add_emitter.dump_events(),
             scalar_mul_emitter.dump_events(),
             poseidon2_hash_emitter.dump_events(),
             poseidon2_perm_emitter.dump_events(),
             to_radix_emitter.dump_events(),
             field_gt_emitter.dump_events(),
             merkle_check_emitter.dump_events(),
             range_check_emitter.dump_events(),
             context_stack_emitter.dump_events(),
             public_data_read_emitter.dump_events(),
             update_check_emitter.dump_events() };
}

EventsContainer AvmSimulationHelper::simulate()
{
    return simulate_with_settings<ProvingSettings>();
}

void AvmSimulationHelper::simulate_fast()
{
    simulate_with_settings<FastSettings>();
}

} // namespace bb::avm2
