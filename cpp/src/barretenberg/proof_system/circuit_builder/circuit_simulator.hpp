#pragma once
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/proof_system/arithmetization/gate_data.hpp"

namespace proof_system {

class CircuitSimulatorBN254 {
  public:
    using FF = barretenberg::fr; // IOU templating
    bool contains_recursive_proof = false;
    uint32_t zero_idx = 0;

    // uint32_t add_variable([[maybe_unused]]const FF& in){
    //   return 0; // WORKTODO: return part of `in` for debugging purposes?
    // }

    inline uint32_t add_variable([[maybe_unused]] const barretenberg::fr index) const { return 404; }
    inline barretenberg::fr get_variable([[maybe_unused]] const uint32_t index) const { return 404; }

    uint32_t put_constant_variable([[maybe_unused]] const barretenberg::fr& variable) { return 404; }
    void set_public_input([[maybe_unused]] const uint32_t witness_index)
    {
        // WORKTODO Public input logic?
    }

    void fix_witness([[maybe_unused]] const uint32_t witness_index,
                     [[maybe_unused]] const barretenberg::fr& witness_value){
        // WORKTODO: logic?
    };

    [[nodiscard]] size_t get_num_gates() const { return 404; }

    void create_add_gate([[maybe_unused]] const add_triple& in){};
    void create_mul_gate([[maybe_unused]] const mul_triple& in){};
    void create_bool_gate([[maybe_unused]] const uint32_t a){};
    void create_poly_gate([[maybe_unused]] const poly_triple& in){};
    void create_big_add_gate([[maybe_unused]] const add_quad& in){};
    void create_big_add_gate_with_bit_extraction([[maybe_unused]] const add_quad& in){};
    void create_big_mul_gate([[maybe_unused]] const mul_quad& in){};
    void create_balanced_add_gate([[maybe_unused]] const add_quad& in){};
    void create_fixed_group_add_gate([[maybe_unused]] const fixed_group_add_quad& in){};
    void create_fixed_group_add_gate_with_init([[maybe_unused]] const fixed_group_add_quad& in,
                                               [[maybe_unused]] const fixed_group_init_quad& init){};
    void create_fixed_group_add_gate_final([[maybe_unused]] const add_quad& in){};

    std::vector<uint32_t> decompose_into_base4_accumulators(
        [[maybe_unused]] const uint32_t witness_index,
        [[maybe_unused]] const size_t num_bits,
        [[maybe_unused]] std::string const& msg = "create_range_constraint")
    {
        return { 404 };
    };

    size_t get_num_constant_gates() { return 404; };
    // maybe this shouldn't be implemented?

    void assert_equal(FF left, FF right, std::string const& msg)
    {
        if (left != right) {
            failure(msg);
        }
    }

    void assert_equal_constant(FF left, FF right, std::string const& msg) { assert_equal(left, right, msg); }

    bool _failed = false;
    std::string _err;

    [[maybe_unused]] bool failed() const { return _failed; };
    [[nodiscard]] const std::string& err() const { return _err; };

    void set_err(std::string msg) { _err = std::move(msg); }
    void failure(std::string msg)
    {
        _failed = true;
        set_err(std::move(msg));
    }

    [[nodiscard]] bool check_circuit() const { return !_failed; }
};

} // namespace proof_system
