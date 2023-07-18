#pragma once
#include "barretenberg/proof_system/circuit_builder/ultra_circuit_builder.hpp"
#include "barretenberg/proof_system/op_queue/ecc_op_queue.hpp"

namespace proof_system {

using namespace barretenberg;

// TODO(luke): The Goblin builder requires all Ultra functionality plus a bit more. For now I've implented it as a
// standalone class that inherits from UltraBuilder. This is nice because it clearly separates out the "Goblin"
// functionlity, however, it is a break from our usual pattern. I may eventually simply incorporate the new
// functionality directly into the UltraBuilder.
class GoblinUltraCircuitBuilder : public UltraCircuitBuilder {
  public:
    static constexpr std::string_view NAME_STRING = "GoblinUltraArithmetization";
    static constexpr CircuitType CIRCUIT_TYPE = CircuitType::ULTRA;

    // Used for simulating big field; Equal to NUM_LIMB_BITS_IN_FIELD_SIMULATION
    const size_t NUM_LIMB_BITS = 68; // TODO(luke): Set via NUM_LIMB_BITS_IN_FIELD_SIMULATION?

    // Stores record of ecc operations and performs corresponding native operations internally
    ECCOpQueue op_queue;

    // number of ecc op "gates" (rows); these are placed at the start of the circuit
    int num_ecc_op_gates = 0;

    // Wires storing ecc op queue data; values are indices into the variables array
    std::array<WireVector, NUM_WIRES> op_wires;

    WireVector& op_wire_1 = std::get<0>(op_wires);
    WireVector& op_wire_2 = std::get<1>(op_wires);
    WireVector& op_wire_3 = std::get<2>(op_wires);
    WireVector& op_wire_4 = std::get<3>(op_wires);

    /**
     * ECC operations
     **/

    void queue_ecc_add_accum(const g1::affine_element& point);
    void queue_ecc_mul_accum(const g1::affine_element& point, const fr& scalar);
    void queue_ecc_eq(const g1::affine_element& point);
    g1::affine_element batch_mul(const std::vector<g1::affine_element>& points, const std::vector<fr>& scalars);

  private:
    void record_ecc_op(const ecc_op_tuple& in);
    void add_ecc_op_gates(uint32_t op, const g1::affine_element& point, const fr& scalar = fr::zero());
    ecc_op_tuple make_ecc_op_tuple(uint32_t op, const g1::affine_element& point, const fr& scalar = fr::zero());
};
} // namespace proof_system
