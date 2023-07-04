#pragma once
#include "barretenberg/proof_system/op_queue/ecc_op_queue.hpp"
#include "barretenberg/proof_system/circuit_builder/ultra_circuit_builder.hpp"

namespace proof_system {

using namespace barretenberg;

class GoblinUltraCircuitBuilder : public UltraCircuitBuilder {
  public:
    static constexpr std::string_view NAME_STRING = "GoblinUltraArithmetization";
    static constexpr CircuitType CIRCUIT_TYPE = CircuitType::ULTRA;

    const size_t NUM_LIMB_BITS = 68;

    ECCOpQueue op_queue; // WORKTODO: TBD.

    // WORKTODO: same as WireVector, probably dont need
    using OpWitnessVector = std::vector<uint32_t, ContainerSlabAllocator<uint32_t>>;
    std::array<OpWitnessVector, NUM_WIRES> op_witness;

    OpWitnessVector& op_witness_1 = std::get<0>(op_witness);
    OpWitnessVector& op_witness_2 = std::get<1>(op_witness);
    OpWitnessVector& op_witness_3 = std::get<2>(op_witness);
    OpWitnessVector& op_witness_4 = std::get<3>(op_witness);

    /**
     * ECC operations
     **/

    void queue_ecc_add_accum(const g1::affine_element& point);
    void queue_ecc_mul_accum(const g1::affine_element& point, const fr& scalar);
    void queue_ecc_eq(const g1::affine_element& point);

  private:
    void queue_ecc_op(const ecc_op_tuple& in);
    void add_ecc_op_gates(uint32_t op, const g1::affine_element& point, const fr& scalar = fr::zero());
    ecc_op_tuple make_ecc_op_tuple(uint32_t op, const g1::affine_element& point, const fr& scalar = fr::zero());
};
} // namespace proof_system
