#pragma once
#include "barretenberg/proof_system/circuit_builder/ultra_circuit_builder.hpp"
#include "barretenberg/proof_system/op_queue/ecc_op_queue.hpp"

namespace proof_system {

using namespace barretenberg;

class GoblinUltraCircuitBuilder : public UltraCircuitBuilder {
  public:
    static constexpr std::string_view NAME_STRING = "GoblinUltraArithmetization";
    static constexpr CircuitType CIRCUIT_TYPE = CircuitType::ULTRA;

    // Used for simulating big field; Equal to NUM_LIMB_BITS_IN_FIELD_SIMULATION
    const size_t NUM_LIMB_BITS = 68; // WORKTODO: Set via NUM_LIMB_BITS_IN_FIELD_SIMULATION?

    ECCOpQueue op_queue; // WORKTODO: TBD.

    // number of ecc op "gates"; these are placed at the start of the circuit
    int num_ecc_op_gates = 0;

    // WORKTODO: same as WireVector, probably dont need
    using OpWitnessVector = std::vector<uint32_t, ContainerSlabAllocator<uint32_t>>;
    std::array<OpWitnessVector, NUM_WIRES> op_wires;

    OpWitnessVector& op_wire_1 = std::get<0>(op_wires);
    OpWitnessVector& op_wire_2 = std::get<1>(op_wires);
    OpWitnessVector& op_wire_3 = std::get<2>(op_wires);
    OpWitnessVector& op_wire_4 = std::get<3>(op_wires);

    /**
     * ECC operations
     **/

    void queue_ecc_add_accum(const g1::affine_element& point);
    void queue_ecc_mul_accum(const g1::affine_element& point, const fr& scalar);
    void queue_ecc_eq(const g1::affine_element& point);
    g1::affine_element batch_mul(const std::vector<g1::affine_element>& points, const std::vector<fr>& scalars);

  private:
    void queue_ecc_op(const ecc_op_tuple& in);
    void add_ecc_op_gates(uint32_t op, const g1::affine_element& point, const fr& scalar = fr::zero());
    ecc_op_tuple make_ecc_op_tuple(uint32_t op, const g1::affine_element& point, const fr& scalar = fr::zero());
};
} // namespace proof_system
