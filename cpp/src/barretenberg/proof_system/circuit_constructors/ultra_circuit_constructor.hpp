#pragma once
#include "barretenberg/proof_system/arithmetization/arithmetization.hpp"
#include "barretenberg/plonk/proof_system/types/polynomial_manifest.hpp"
#include "circuit_constructor_base.hpp"
#include "barretenberg/plonk/proof_system/constants.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"
#include "barretenberg/proof_system/types/merkle_hash_type.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/proof_system/plookup_tables/types.hpp"
#include "barretenberg/proof_system/plookup_tables/plookup_tables.hpp"
#include "barretenberg/plonk/proof_system/types/prover_settings.hpp"
#include <optional>

namespace proof_system {
static constexpr ComposerType type = ComposerType::PLOOKUP;
static constexpr merkle::HashType merkle_hash_type = merkle::HashType::LOOKUP_PEDERSEN;
static constexpr size_t NUM_RESERVED_GATES = 4; // This must be >= num_roots_cut_out_of_vanishing_polynomial
                                                // See the comment in plonk/proof_system/prover/prover.cpp
                                                // ProverBase::compute_quotient_commitments() for why 4 exactly.
static constexpr size_t UINT_LOG2_BASE = 6;     // DOCTODO: explain what this is, or rename.
// The plookup range proof requires work linear in range size, thus cannot be used directly for
// large ranges such as 2^64. For such ranges the element will be decomposed into smaller
// chuncks according to the parameter below
static constexpr size_t DEFAULT_PLOOKUP_RANGE_BITNUM = 14;
static constexpr size_t DEFAULT_PLOOKUP_RANGE_STEP_SIZE = 3;
static constexpr size_t DEFAULT_PLOOKUP_RANGE_SIZE = (1 << DEFAULT_PLOOKUP_RANGE_BITNUM) - 1;
static constexpr size_t DEFAULT_NON_NATIVE_FIELD_LIMB_BITS = 68;
static constexpr uint32_t UNINITIALIZED_MEMORY_RECORD = UINT32_MAX;
static constexpr size_t NUMBER_OF_GATES_PER_RAM_ACCESS = 2;
static constexpr size_t NUMBER_OF_ARITHMETIC_GATES_PER_RAM_ARRAY = 1;
// number of gates created per non-native field operation in process_non_native_field_multiplications
static constexpr size_t GATES_PER_NON_NATIVE_FIELD_MULTIPLICATION_ARITHMETIC = 7;

struct non_native_field_witnesses {
    // first 4 array elements = limbs
    // 5th element = prime basis limb
    std::array<uint32_t, 5> a;
    std::array<uint32_t, 5> b;
    std::array<uint32_t, 5> q;
    std::array<uint32_t, 5> r;
    std::array<barretenberg::fr, 5> neg_modulus;
    barretenberg::fr modulus;
};

struct non_native_field_multiplication_cross_terms {
    uint32_t lo_0_idx;
    uint32_t lo_1_idx;
    uint32_t hi_0_idx;
    uint32_t hi_1_idx;
    uint32_t hi_2_idx;
    uint32_t hi_3_idx;
};

/**
 * @brief Used to store instructions to create non_native_field_multiplication gates.
 *        We want to cache these (and remove duplicates) as the stdlib code can end up multiplying the same inputs
 * repeatedly.
 */
struct cached_non_native_field_multiplication {
    std::array<uint32_t, 5> a;
    std::array<uint32_t, 5> b;
    std::array<uint32_t, 5> q;
    std::array<uint32_t, 5> r;
    non_native_field_multiplication_cross_terms cross_terms;
    std::array<barretenberg::fr, 5> neg_modulus;

    bool operator==(const cached_non_native_field_multiplication& other) const
    {
        bool valid = true;
        for (size_t i = 0; i < 5; ++i) {
            valid = valid && (a[i] == other.a[i]);
            valid = valid && (b[i] == other.b[i]);
            valid = valid && (q[i] == other.q[i]);
            valid = valid && (r[i] == other.r[i]);
        }
        return valid;
    }
    bool operator<(const cached_non_native_field_multiplication& other) const
    {
        if (a < other.a) {
            return true;
        }
        if (a == other.a) {
            if (b < other.b) {
                return true;
            }
            if (b == other.b) {
                if (q < other.q) {
                    return true;
                }
                if (q == other.q) {
                    if (r < other.r) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

enum AUX_SELECTORS {
    NONE,
    LIMB_ACCUMULATE_1,
    LIMB_ACCUMULATE_2,
    NON_NATIVE_FIELD_1,
    NON_NATIVE_FIELD_2,
    NON_NATIVE_FIELD_3,
    RAM_CONSISTENCY_CHECK,
    ROM_CONSISTENCY_CHECK,
    RAM_TIMESTAMP_CHECK,
    ROM_READ,
    RAM_READ,
    RAM_WRITE,
};

struct RangeList {
    uint64_t target_range;
    uint32_t range_tag;
    uint32_t tau_tag;
    std::vector<uint32_t> variable_indices;
    bool operator==(const RangeList& other) const noexcept
    {
        return target_range == other.target_range && range_tag == other.range_tag && tau_tag == other.tau_tag &&
               variable_indices == other.variable_indices;
    }
};

/**
 * @brief A ROM memory record that can be ordered
 *
 *
 */
struct RomRecord {
    uint32_t index_witness = 0;
    uint32_t value_column1_witness = 0;
    uint32_t value_column2_witness = 0;
    uint32_t index = 0;
    uint32_t record_witness = 0;
    size_t gate_index = 0;
    bool operator<(const RomRecord& other) const { return index < other.index; }
    bool operator==(const RomRecord& other) const noexcept
    {
        return index_witness == other.index_witness && value_column1_witness == other.value_column1_witness &&
               value_column2_witness == other.value_column2_witness && index == other.index &&
               record_witness == other.record_witness && gate_index == other.gate_index;
    }
};

/**
 * @brief A RAM memory record that can be ordered
 *
 *
 */
struct RamRecord {
    enum AccessType {
        READ,
        WRITE,
    };
    uint32_t index_witness = 0;
    uint32_t timestamp_witness = 0;
    uint32_t value_witness = 0;
    uint32_t index = 0;
    uint32_t timestamp = 0;
    AccessType access_type = AccessType::READ; // read or write?
    uint32_t record_witness = 0;
    size_t gate_index = 0;
    bool operator<(const RamRecord& other) const
    {
        bool index_test = (index) < (other.index);
        return index_test || (index == other.index && timestamp < other.timestamp);
    }
    bool operator==(const RamRecord& other) const noexcept
    {
        return index_witness == other.index_witness && timestamp_witness == other.timestamp_witness &&
               value_witness == other.value_witness && index == other.index && timestamp == other.timestamp &&
               access_type == other.access_type && record_witness == other.record_witness &&
               gate_index == other.gate_index;
    }
};

/**
 * @brief Each ram array is an instance of memory transcript. It saves values and indexes for a particular memory
 * array
 *
 *
 */
struct RamTranscript {
    // Contains the value of each index of the array
    std::vector<uint32_t> state;

    // A vector of records, each of which contains:
    // + The constant witness with the index
    // + The value in the memory slot
    // + The actual index value
    std::vector<RamRecord> records;

    // used for RAM records, to compute the timestamp when performing a read/write
    size_t access_count = 0;
    // Used to check that the state hasn't changed in tests
    bool operator==(const RamTranscript& other) const noexcept
    {
        return (state == other.state && records == other.records && access_count == other.access_count);
    }
};

/**
 * @brief Each rom array is an instance of memory transcript. It saves values and indexes for a particular memory
 * array
 *
 *
 */
struct RomTranscript {
    // Contains the value of each index of the array
    std::vector<std::array<uint32_t, 2>> state;

    // A vector of records, each of which contains:
    // + The constant witness with the index
    // + The value in the memory slot
    // + The actual index value
    std::vector<RomRecord> records;

    // Used to check that the state hasn't changed in tests
    bool operator==(const RomTranscript& other) const noexcept
    {
        return (state == other.state && records == other.records);
    }
};

inline std::vector<std::string> ultra_selector_names()
{
    std::vector<std::string> result{ "q_m",     "q_c",    "q_1",        "q_2",   "q_3",       "q_4",
                                     "q_arith", "q_sort", "q_elliptic", "q_aux", "table_type" };
    return result;
}

/**
 * @brief Circuit-in-the-head is a structure we use to store all the information about the circuit which should be used
 * during check_circuit method call, but needs to be discarded later
 *
 * @details In check_circuit method in UltraCircuitConstructor we want to check that the whole circuit works, but ultra
 * circuits need to have ram, rom and range gates added in the end for the check to be complete as well as the set
 * permutation check.
 */
struct CircuitInTheHead {
    std::vector<uint32_t> public_inputs;
    std::vector<barretenberg::fr> variables;
    // index of next variable in equivalence class (=REAL_VARIABLE if you're last)
    std::vector<uint32_t> next_var_index;
    // index of  previous variable in equivalence class (=FIRST if you're in a cycle alone)
    std::vector<uint32_t> prev_var_index;
    // indices of corresponding real variables
    std::vector<uint32_t> real_variable_index;
    std::vector<uint32_t> real_variable_tags;
    std::map<barretenberg::fr, uint32_t> constant_variable_indices;
    std::vector<uint32_t> w_l;
    std::vector<uint32_t> w_r;
    std::vector<uint32_t> w_o;
    std::vector<uint32_t> w_4;
    std::vector<barretenberg::fr> q_m;
    std::vector<barretenberg::fr> q_c;
    std::vector<barretenberg::fr> q_1;
    std::vector<barretenberg::fr> q_2;
    std::vector<barretenberg::fr> q_3;
    std::vector<barretenberg::fr> q_4;
    std::vector<barretenberg::fr> q_arith;
    std::vector<barretenberg::fr> q_sort;
    std::vector<barretenberg::fr> q_elliptic;
    std::vector<barretenberg::fr> q_aux;
    std::vector<barretenberg::fr> q_lookup_type;
    uint32_t current_tag = DUMMY_TAG;
    std::map<uint32_t, uint32_t> tau;

    std::vector<RamTranscript> ram_arrays;
    std::vector<RomTranscript> rom_arrays;

    std::vector<uint32_t> memory_read_records;
    std::vector<uint32_t> memory_write_records;
    std::map<uint64_t, RangeList> range_lists;
    size_t num_gates;
    bool circuit_finalised = false;
    /**
     * @brief Stores the state of everything logic-related in the constructor.
     *
     * @details We need this function for tests. Specifically, to ensure that we are not changing anything in
     * check_circuit
     *
     * @param circuit_constructor
     * @return CircuitInTheHead
     */
    template <typename CircuitConstructor>
    static CircuitInTheHead store_state(const CircuitConstructor& circuit_constructor)
    {
        CircuitInTheHead stored_state;
        stored_state.public_inputs = circuit_constructor.public_inputs;
        stored_state.variables = circuit_constructor.variables;

        stored_state.next_var_index = circuit_constructor.next_var_index;

        stored_state.prev_var_index = circuit_constructor.prev_var_index;

        stored_state.real_variable_index = circuit_constructor.real_variable_index;
        stored_state.real_variable_tags = circuit_constructor.real_variable_tags;
        stored_state.constant_variable_indices = circuit_constructor.constant_variable_indices;
        stored_state.w_l = circuit_constructor.w_l;
        stored_state.w_r = circuit_constructor.w_r;
        stored_state.w_o = circuit_constructor.w_o;
        stored_state.w_4 = circuit_constructor.w_4;
        stored_state.q_m = circuit_constructor.q_m;
        stored_state.q_c = circuit_constructor.q_c;
        stored_state.q_1 = circuit_constructor.q_1;
        stored_state.q_2 = circuit_constructor.q_2;
        stored_state.q_3 = circuit_constructor.q_3;
        stored_state.q_4 = circuit_constructor.q_4;
        stored_state.q_arith = circuit_constructor.q_arith;
        stored_state.q_sort = circuit_constructor.q_sort;
        stored_state.q_elliptic = circuit_constructor.q_elliptic;
        stored_state.q_aux = circuit_constructor.q_aux;
        stored_state.q_lookup_type = circuit_constructor.q_lookup_type;
        stored_state.current_tag = circuit_constructor.current_tag;
        stored_state.tau = circuit_constructor.tau;

        stored_state.ram_arrays = circuit_constructor.ram_arrays;
        stored_state.rom_arrays = circuit_constructor.rom_arrays;

        stored_state.memory_read_records = circuit_constructor.memory_read_records;
        stored_state.memory_write_records = circuit_constructor.memory_write_records;
        stored_state.range_lists = circuit_constructor.range_lists;
        stored_state.circuit_finalised = circuit_constructor.circuit_finalised;
        stored_state.num_gates = circuit_constructor.num_gates;
        return stored_state;
    }
    /**
     * @brief CHecks that the circuit states is the same as the stored circuit's one
     *
     * @param circuit_constructor
     * @return true
     * @return false
     */
    template <typename CircuitConstructor> bool is_same_state(const CircuitConstructor& circuit_constructor)
    {
        if (!(public_inputs == circuit_constructor.public_inputs)) {
            return false;
        }
        if (!(variables == circuit_constructor.variables)) {
            return false;
        }
        if (!(next_var_index == circuit_constructor.next_var_index)) {
            return false;
        }
        if (!(prev_var_index == circuit_constructor.prev_var_index)) {
            return false;
        }
        if (!(real_variable_index == circuit_constructor.real_variable_index)) {
            return false;
        }
        if (!(real_variable_tags == circuit_constructor.real_variable_tags)) {
            return false;
        }
        if (!(constant_variable_indices == circuit_constructor.constant_variable_indices)) {
            return false;
        }
        if (!(w_l == circuit_constructor.w_l)) {
            return false;
        }
        if (!(w_r == circuit_constructor.w_r)) {
            return false;
        }
        if (!(w_o == circuit_constructor.w_o)) {
            return false;
        }
        if (!(w_4 == circuit_constructor.w_4)) {
            return false;
        }
        if (!(q_m == circuit_constructor.q_m)) {
            return false;
        }
        if (!(q_c == circuit_constructor.q_c)) {
            return false;
        }
        if (!(q_1 == circuit_constructor.q_1)) {
            return false;
        }
        if (!(q_2 == circuit_constructor.q_2)) {
            return false;
        }
        if (!(q_3 == circuit_constructor.q_3)) {
            return false;
        }
        if (!(q_4 == circuit_constructor.q_4)) {
            return false;
        }
        if (!(q_arith == circuit_constructor.q_arith)) {
            return false;
        }
        if (!(q_sort == circuit_constructor.q_sort)) {
            return false;
        }
        if (!(q_elliptic == circuit_constructor.q_elliptic)) {
            return false;
        }
        if (!(q_aux == circuit_constructor.q_aux)) {
            return false;
        }
        if (!(q_lookup_type == circuit_constructor.q_lookup_type)) {
            return false;
        }
        if (!(current_tag == circuit_constructor.current_tag)) {
            return false;
        }
        if (!(tau == circuit_constructor.tau)) {
            return false;
        }
        if (!(ram_arrays == circuit_constructor.ram_arrays)) {
            return false;
        }
        if (!(rom_arrays == circuit_constructor.rom_arrays)) {
            return false;
        }
        if (!(memory_read_records == circuit_constructor.memory_read_records)) {
            return false;
        }
        if (!(memory_write_records == circuit_constructor.memory_write_records)) {
            return false;
        }
        if (!(range_lists == circuit_constructor.range_lists)) {
            return false;
        }
        if (!(num_gates == circuit_constructor.num_gates)) {
            return false;
        }
        if (!(circuit_finalised == circuit_constructor.circuit_finalised)) {
            return false;
        }
        return true;
    }
};
class UltraCircuitConstructor : public CircuitConstructorBase<arithmetization::Ultra> {
  public:
    // We use the concept of "Circuit-in-the-head" for the check_circuit method. We have to finalize the circuit to
    // check it, so we put all the updates in this structure, instead of messing with the circuit itself
    CircuitInTheHead circuit_in_the_head;
    // Switch, forcing gates to interact with circuit_in_the_head instead of the regular members
    bool in_the_head = false;
    std::vector<uint32_t>& w_l = std::get<0>(wires);
    std::vector<uint32_t>& w_r = std::get<1>(wires);
    std::vector<uint32_t>& w_o = std::get<2>(wires);
    std::vector<uint32_t>& w_4 = std::get<3>(wires);

    std::vector<barretenberg::fr>& q_m = std::get<0>(selectors);
    std::vector<barretenberg::fr>& q_c = std::get<1>(selectors);
    std::vector<barretenberg::fr>& q_1 = std::get<2>(selectors);
    std::vector<barretenberg::fr>& q_2 = std::get<3>(selectors);
    std::vector<barretenberg::fr>& q_3 = std::get<4>(selectors);
    std::vector<barretenberg::fr>& q_4 = std::get<5>(selectors);
    std::vector<barretenberg::fr>& q_arith = std::get<6>(selectors);
    std::vector<barretenberg::fr>& q_sort = std::get<7>(selectors);
    std::vector<barretenberg::fr>& q_elliptic = std::get<8>(selectors);
    std::vector<barretenberg::fr>& q_aux = std::get<9>(selectors);
    std::vector<barretenberg::fr>& q_lookup_type = std::get<10>(selectors);

    // TODO(#216)(Kesha): replace this with Honk enums after we have a verifier and no longer depend on plonk
    // prover/verifier
    static constexpr ComposerType type = ComposerType::STANDARD_HONK;
    static constexpr size_t UINT_LOG2_BASE = 2;

    // These are variables that we have used a gate on, to enforce that they are
    // equal to a defined value.
    // TODO(#216)(Adrian): Why is this not in CircuitConstructorBase
    std::map<barretenberg::fr, uint32_t> constant_variable_indices;

    std::vector<plookup::BasicTable> lookup_tables;
    std::vector<plookup::MultiTable> lookup_multi_tables;
    std::map<uint64_t, RangeList> range_lists; // DOCTODO: explain this.

    /**
     * @brief Each entry in ram_arrays represents an independent RAM table.
     * RamTranscript tracks the current table state,
     * as well as the 'records' produced by each read and write operation.
     * Used in `compute_proving_key` to generate consistency check gates required to validate the RAM read/write history
     */
    std::vector<RamTranscript> ram_arrays;

    /**
     * @brief Each entry in ram_arrays represents an independent ROM table.
     * RomTranscript tracks the current table state,
     * as well as the 'records' produced by each read operation.
     * Used in `compute_proving_key` to generate consistency check gates required to validate the ROM read history
     */
    std::vector<RomTranscript> rom_arrays;

    // Stores gate index of ROM and RAM reads (required by proving key)
    std::vector<uint32_t> memory_read_records;
    // Stores gate index of RAM writes (required by proving key)
    std::vector<uint32_t> memory_write_records;

    std::vector<cached_non_native_field_multiplication> cached_non_native_field_multiplications;

    void process_non_native_field_multiplications();

    bool circuit_finalised = false;

    UltraCircuitConstructor(const size_t size_hint = 0)
        : CircuitConstructorBase(ultra_selector_names(), size_hint)
    {
        w_l.reserve(size_hint);
        w_r.reserve(size_hint);
        w_o.reserve(size_hint);
        w_4.reserve(size_hint);
        zero_idx = put_constant_variable(0);
        tau.insert({ DUMMY_TAG, DUMMY_TAG });
    };

    UltraCircuitConstructor(const UltraCircuitConstructor& other) = delete;
    UltraCircuitConstructor(UltraCircuitConstructor&& other) = default;
    UltraCircuitConstructor& operator=(const UltraCircuitConstructor& other) = delete;
    UltraCircuitConstructor& operator=(UltraCircuitConstructor&& other) = delete;
    ~UltraCircuitConstructor() override = default;

    /**
     * Add a variable to variables
     *
     * @details This method proxies to the standard one or add a variable in-the-head
     *
     * @param in The value of the variable
     * @return The index of the new variable in the variables vector
     */
    uint32_t add_variable(const barretenberg::fr& in) override
    {
        if (!in_the_head) {
            return CircuitConstructorBase<arithmetization::Ultra>::add_variable(in);
        }

        circuit_in_the_head.variables.emplace_back(in);

        // By default, we assume each new variable belongs in its own copy-cycle. These defaults can be modified later
        // by `assert_equal`.
        const uint32_t index = static_cast<uint32_t>(circuit_in_the_head.variables.size()) - 1U;
        circuit_in_the_head.real_variable_index.emplace_back(index);
        circuit_in_the_head.next_var_index.emplace_back(REAL_VARIABLE);
        circuit_in_the_head.prev_var_index.emplace_back(FIRST_VARIABLE_IN_CLASS);
        circuit_in_the_head.real_variable_tags.emplace_back(DUMMY_TAG);
        return index;
    }
    /**
     * @brief Get the variable value from the actual of in-the-head circuit
     *
     * @param index
     * @return barretenberg::fr
     */
    inline barretenberg::fr get_variable(const uint32_t index) const
    {
        if (!in_the_head) {
            return CircuitConstructorBase<arithmetization::Ultra>::get_variable(index);
        }
        ASSERT(circuit_in_the_head.variables.size() > index);
        return circuit_in_the_head.variables[circuit_in_the_head.real_variable_index[index]];
    }

    /**
     * @brief Check that variable indices are valid
     *
     * @param variable_indices
     */
    void assert_valid_variables(const std::vector<uint32_t>& variable_indices)
    {
        for (const auto& variable_index : variable_indices) {
            ASSERT(is_valid_variable(variable_index));
        }
    }
    /**
     * @brief Checks that the variable index is valid (in the circuit or in the in-the-head circuit)
     *
     * @param variable_index
     * @return true
     * @return false
     */
    bool is_valid_variable(uint32_t variable_index)
    {
        if (in_the_head) {
            return variable_index < circuit_in_the_head.variables.size();
        } else {
            return variable_index < variables.size();
        }
    };

    void finalize_circuit();

    void create_add_gate(const add_triple& in) override;

    void create_big_add_gate(const add_quad& in, const bool use_next_gate_w_4 = false);
    void create_big_add_gate_with_bit_extraction(const add_quad& in);
    void create_big_mul_gate(const mul_quad& in);
    void create_balanced_add_gate(const add_quad& in);

    void create_mul_gate(const mul_triple& in) override;
    void create_bool_gate(const uint32_t a) override;
    void create_poly_gate(const poly_triple& in) override;
    void create_ecc_add_gate(const ecc_add_gate& in);

    void fix_witness(const uint32_t witness_index, const barretenberg::fr& witness_value);

    // void add_recursive_proof(const std::vector<uint32_t>& proof_output_witness_indices)
    // {
    //     if (contains_recursive_proof) {
    //         failure("added recursive proof when one already exists");
    //     }
    //     contains_recursive_proof = true;

    //     for (const auto& idx : proof_output_witness_indices) {
    //         set_public_input(idx);
    //         recursive_proof_public_input_indices.push_back((uint32_t)(public_inputs.size() - 1));
    //     }
    // }

    void create_new_range_constraint(const uint32_t variable_index,
                                     const uint64_t target_range,
                                     std::string const msg = "create_new_range_constraint");
    void create_range_constraint(const uint32_t variable_index, const size_t num_bits, std::string const& msg)
    {
        if (num_bits <= DEFAULT_PLOOKUP_RANGE_BITNUM) {
            /**
             * N.B. if `variable_index` is not used in any arithmetic constraints, this will create an unsatisfiable
             *      circuit!
             *      this range constraint will increase the size of the 'sorted set' of range-constrained integers by 1.
             *      The 'non-sorted set' of range-constrained integers is a subset of the wire indices of all arithmetic
             *      gates. No arithemtic gate => size imbalance between sorted and non-sorted sets. Checking for this
             *      and throwing an error would require a refactor of the Composer to catelog all 'orphan' variables not
             *      assigned to gates.
             **/
            create_new_range_constraint(variable_index, 1ULL << num_bits, msg);
        } else {
            decompose_into_default_range(variable_index, num_bits, DEFAULT_PLOOKUP_RANGE_BITNUM, msg);
        }
    }

    accumulator_triple create_logic_constraint(const uint32_t a,
                                               const uint32_t b,
                                               const size_t num_bits,
                                               bool is_xor_gate);
    accumulator_triple create_and_constraint(const uint32_t a, const uint32_t b, const size_t num_bits);
    accumulator_triple create_xor_constraint(const uint32_t a, const uint32_t b, const size_t num_bits);

    uint32_t put_constant_variable(const barretenberg::fr& variable);

    size_t get_num_constant_gates() const override { return 0; }

    // /**
    //  * @brief Get the final number of gates in a circuit, which consists of the sum of:
    //  * 1) Current number number of actual gates
    //  * 2) Number of public inputs, as we'll need to add a gate for each of them
    //  * 3) Number of Rom array-associated gates
    //  * 4) Number of range-list associated gates
    //  * 5) Number of non-native field multiplication gates.
    //  *
    //  *
    //  * @param count return arument, number of existing gates
    //  * @param rangecount return argument, extra gates due to range checks
    //  * @param romcount return argument, extra gates due to rom reads
    //  * @param ramcount return argument, extra gates due to ram read/writes
    //  * @param nnfcount return argument, extra gates due to queued non native field gates
    //  */
    // void get_num_gates_split_into_components(
    //     size_t& count, size_t& rangecount, size_t& romcount, size_t& ramcount, size_t& nnfcount) const
    // {
    //     count = num_gates;
    //     // each ROM gate adds +1 extra gate due to the rom reads being copied to a sorted list set
    //     for (size_t i = 0; i < rom_arrays.size(); ++i) {
    //         for (size_t j = 0; j < rom_arrays[i].state.size(); ++j) {
    //             if (rom_arrays[i].state[j][0] == UNINITIALIZED_MEMORY_RECORD) {
    //                 romcount += 2;
    //             }
    //         }
    //         romcount += (rom_arrays[i].records.size());
    //         romcount += 1; // we add an addition gate after procesing a rom array
    //     }

    //     constexpr size_t gate_width = ultra_settings::program_width;
    //     // each RAM gate adds +2 extra gates due to the ram reads being copied to a sorted list set,
    //     // as well as an extra gate to validate timestamps
    //     std::vector<size_t> ram_timestamps;
    //     std::vector<size_t> ram_range_sizes;
    //     std::vector<size_t> ram_range_exists;
    //     for (size_t i = 0; i < ram_arrays.size(); ++i) {
    //         for (size_t j = 0; j < ram_arrays[i].state.size(); ++j) {
    //             if (ram_arrays[i].state[j] == UNINITIALIZED_MEMORY_RECORD) {
    //                 ramcount += NUMBER_OF_GATES_PER_RAM_ACCESS;
    //             }
    //         }
    //         ramcount += (ram_arrays[i].records.size() * NUMBER_OF_GATES_PER_RAM_ACCESS);
    //         ramcount += NUMBER_OF_ARITHMETIC_GATES_PER_RAM_ARRAY; // we add an addition gate after procesing a ram
    //         array

    //         // there will be 'max_timestamp' number of range checks, need to calculate.
    //         const auto max_timestamp = ram_arrays[i].access_count - 1;

    //         // if a range check of length `max_timestamp` already exists, we are double counting.
    //         // We record `ram_timestamps` to detect and correct for this error when we process range lists.
    //         ram_timestamps.push_back(max_timestamp);
    //         size_t padding = (gate_width - (max_timestamp % gate_width)) % gate_width;
    //         if (max_timestamp == gate_width)
    //             padding += gate_width;
    //         const size_t ram_range_check_list_size = max_timestamp + padding;

    //         size_t ram_range_check_gate_count = (ram_range_check_list_size / gate_width);
    //         ram_range_check_gate_count += 1; // we need to add 1 extra addition gates for every distinct range list

    //         ram_range_sizes.push_back(ram_range_check_gate_count);
    //         ram_range_exists.push_back(false);
    //         // rangecount += ram_range_check_gate_count;
    //     }
    //     for (const auto& list : range_lists) {
    //         auto list_size = list.second.variable_indices.size();
    //         size_t padding = (gate_width - (list.second.variable_indices.size() % gate_width)) % gate_width;
    //         if (list.second.variable_indices.size() == gate_width)
    //             padding += gate_width;
    //         list_size += padding;

    //         for (size_t i = 0; i < ram_timestamps.size(); ++i) {
    //             if (list.second.target_range == ram_timestamps[i]) {
    //                 ram_range_exists[i] = true;
    //             }
    //         }
    //         rangecount += (list_size / gate_width);
    //         rangecount += 1; // we need to add 1 extra addition gates for every distinct range list
    //     }
    //     // update rangecount to include the ram range checks the composer will eventually be creating
    //     for (size_t i = 0; i < ram_range_sizes.size(); ++i) {
    //         if (!ram_range_exists[i]) {
    //             rangecount += ram_range_sizes[i];
    //         }
    //     }
    //     std::vector<cached_non_native_field_multiplication> nnf_copy(cached_non_native_field_multiplications);
    //     // update nnfcount
    //     std::sort(nnf_copy.begin(), nnf_copy.end());

    //     auto last = std::unique(nnf_copy.begin(), nnf_copy.end());
    //     const size_t num_nnf_ops = static_cast<size_t>(std::distance(nnf_copy.begin(), last));
    //     nnfcount = num_nnf_ops * GATES_PER_NON_NATIVE_FIELD_MULTIPLICATION_ARITHMETIC;
    // }

    // /**
    //  * @brief Get the final number of gates in a circuit, which consists of the sum of:
    //  * 1) Current number number of actual gates
    //  * 2) Number of public inputs, as we'll need to add a gate for each of them
    //  * 3) Number of Rom array-associated gates
    //  * 4) Number of range-list associated gates
    //  * 5) Number of non-native field multiplication gates.
    //  *
    //  * @return size_t
    //  */
    // virtual size_t get_num_gates() const override
    // {
    //     // if circuit finalised already added extra gates
    //     if (circuit_finalised) {
    //         return num_gates;
    //     }
    //     size_t count = 0;
    //     size_t rangecount = 0;
    //     size_t romcount = 0;
    //     size_t ramcount = 0;
    //     size_t nnfcount = 0;
    //     get_num_gates_split_into_components(count, rangecount, romcount, ramcount, nnfcount);
    //     return count + romcount + ramcount + rangecount + nnfcount;
    // }

    // virtual void print_num_gates() const override
    // {
    //     size_t count = 0;
    //     size_t rangecount = 0;
    //     size_t romcount = 0;
    //     size_t ramcount = 0;
    //     size_t nnfcount = 0;
    //     get_num_gates_split_into_components(count, rangecount, romcount, ramcount, nnfcount);

    //     size_t total = count + romcount + ramcount + rangecount;
    //     std::cout << "gates = " << total << " (arith " << count << ", rom " << romcount << ", ram " << ramcount
    //               << ", range " << rangecount << ", non native field gates " << nnfcount
    //               << "), pubinp = " << public_inputs.size() << std::endl;
    // }

    void assert_equal_constant(const uint32_t a_idx,
                               const barretenberg::fr& b,
                               std::string const& msg = "assert equal constant")
    {
        if (variables[a_idx] != b && !failed()) {
            failure(msg);
        }
        auto b_idx = put_constant_variable(b);
        assert_equal(a_idx, b_idx, msg);
    }

    /**
     * Plookup Methods
     **/
    void add_table_column_selector_poly_to_proving_key(barretenberg::polynomial& small, const std::string& tag);
    void initialize_precomputed_table(
        const plookup::BasicTableId id,
        bool (*generator)(std::vector<barretenberg::fr>&,
                          std::vector<barretenberg::fr>&,
                          std::vector<barretenberg::fr>&),
        std::array<barretenberg::fr, 2> (*get_values_from_key)(const std::array<uint64_t, 2>));

    plookup::BasicTable& get_table(const plookup::BasicTableId id);
    plookup::MultiTable& create_table(const plookup::MultiTableId id);

    plookup::ReadData<uint32_t> create_gates_from_plookup_accumulators(
        const plookup::MultiTableId& id,
        const plookup::ReadData<barretenberg::fr>& read_values,
        const uint32_t key_a_index,
        std::optional<uint32_t> key_b_index = std::nullopt);

    /**
     * Generalized Permutation Methods
     **/
    std::vector<uint32_t> decompose_into_default_range(
        const uint32_t variable_index,
        const uint64_t num_bits,
        const uint64_t target_range_bitnum = DEFAULT_PLOOKUP_RANGE_BITNUM,
        std::string const& msg = "decompose_into_default_range");
    std::vector<uint32_t> decompose_into_default_range_better_for_oddlimbnum(
        const uint32_t variable_index,
        const size_t num_bits,
        std::string const& msg = "decompose_into_default_range_better_for_oddlimbnum");
    void create_dummy_constraints(const std::vector<uint32_t>& variable_index);
    void create_sort_constraint(const std::vector<uint32_t>& variable_index);
    void create_sort_constraint_with_edges(const std::vector<uint32_t>& variable_index,
                                           const barretenberg::fr&,
                                           const barretenberg::fr&);
    void assign_tag(const uint32_t variable_index, const uint32_t tag)
    {
        if (in_the_head) {
            ASSERT(tag <= circuit_in_the_head.current_tag);

            auto& current_real_variable_tags = circuit_in_the_head.real_variable_tags;
            auto& current_real_variable_index = circuit_in_the_head.real_variable_index;
            // If we've already assigned this tag to this variable, return (can happen due to copy constraints)
            if (current_real_variable_tags[current_real_variable_index[variable_index]] == tag) {
                return;
            }
            ASSERT(current_real_variable_tags[current_real_variable_index[variable_index]] == DUMMY_TAG);
            current_real_variable_tags[current_real_variable_index[variable_index]] = tag;
        } else {
            ASSERT(tag <= current_tag);
            // If we've already assigned this tag to this variable, return (can happen due to copy constraints)
            if (real_variable_tags[real_variable_index[variable_index]] == tag) {
                return;
            }
            ASSERT(real_variable_tags[real_variable_index[variable_index]] == DUMMY_TAG);
            real_variable_tags[real_variable_index[variable_index]] = tag;
        }
    }

    uint32_t create_tag(const uint32_t tag_index, const uint32_t tau_index)
    {
        if (in_the_head) {

            ASSERT(circuit_in_the_head.current_tag >= current_tag);
            circuit_in_the_head.tau.insert({ tag_index, tau_index });
            circuit_in_the_head.current_tag++;
            return circuit_in_the_head.current_tag;
        }
        tau.insert({ tag_index, tau_index });
        current_tag++; // Why exactly?
        return current_tag;
    }

    uint32_t get_new_tag()
    {
        if (in_the_head) {
            // Check that we've reset the circuit in the head before proceeding with the virtual circuit construction
            ASSERT(circuit_in_the_head.current_tag >= current_tag);
            circuit_in_the_head.current_tag++;
            return circuit_in_the_head.current_tag;
        }
        current_tag++;
        return current_tag;
    }

    RangeList create_range_list(const uint64_t target_range);
    void process_range_list(const RangeList& list);
    void process_range_lists();

    /**
     * Custom Gate Selectors
     **/
    void apply_aux_selectors(const AUX_SELECTORS type);

    /**
     * Non Native Field Arithmetic
     **/
    void range_constrain_two_limbs(const uint32_t lo_idx,
                                   const uint32_t hi_idx,
                                   const size_t lo_limb_bits = DEFAULT_NON_NATIVE_FIELD_LIMB_BITS,
                                   const size_t hi_limb_bits = DEFAULT_NON_NATIVE_FIELD_LIMB_BITS);
    std::array<uint32_t, 2> decompose_non_native_field_double_width_limb(
        const uint32_t limb_idx, const size_t num_limb_bits = (2 * DEFAULT_NON_NATIVE_FIELD_LIMB_BITS));
    std::array<uint32_t, 2> queue_non_native_field_multiplication(
        const non_native_field_witnesses& input, const bool range_constrain_quotient_and_remainder = true);
    std::array<uint32_t, 2> evaluate_partial_non_native_field_multiplication(const non_native_field_witnesses& input);
    typedef std::pair<uint32_t, barretenberg::fr> scaled_witness;
    typedef std::tuple<scaled_witness, scaled_witness, barretenberg::fr> add_simple;
    std::array<uint32_t, 5> evaluate_non_native_field_subtraction(
        add_simple limb0,
        add_simple limb1,
        add_simple limb2,
        add_simple limb3,
        std::tuple<uint32_t, uint32_t, barretenberg::fr> limbp);
    std::array<uint32_t, 5> evaluate_non_native_field_addition(add_simple limb0,
                                                               add_simple limb1,
                                                               add_simple limb2,
                                                               add_simple limb3,
                                                               std::tuple<uint32_t, uint32_t, barretenberg::fr> limbp);

    /**
     * Memory
     **/

    // size_t create_RAM_array(const size_t array_size);
    size_t create_ROM_array(const size_t array_size);

    void set_ROM_element(const size_t rom_id, const size_t index_value, const uint32_t value_witness);
    void set_ROM_element_pair(const size_t rom_id,
                              const size_t index_value,
                              const std::array<uint32_t, 2>& value_witnesses);
    uint32_t read_ROM_array(const size_t rom_id, const uint32_t index_witness);
    std::array<uint32_t, 2> read_ROM_array_pair(const size_t rom_id, const uint32_t index_witness);
    void create_ROM_gate(RomRecord& record);
    void create_sorted_ROM_gate(RomRecord& record);
    void process_ROM_array(const size_t rom_id, const size_t gate_offset_from_public_inputs);
    void process_ROM_arrays(const size_t gate_offset_from_public_inputs);

    void create_RAM_gate(RamRecord& record);
    void create_sorted_RAM_gate(RamRecord& record);
    void create_final_sorted_RAM_gate(RamRecord& record, const size_t ram_array_size);

    size_t create_RAM_array(const size_t array_size);
    void init_RAM_element(const size_t ram_id, const size_t index_value, const uint32_t value_witness);
    uint32_t read_RAM_array(const size_t ram_id, const uint32_t index_witness);
    void write_RAM_array(const size_t ram_id, const uint32_t index_witness, const uint32_t value_witness);
    void process_RAM_array(const size_t ram_id, const size_t gate_offset_from_public_inputs);
    void process_RAM_arrays(const size_t gate_offset_from_public_inputs);

    /**
     * @brief Choose whether to use a virual or a real selector/witness/member
     *
     * @details For check_circuit we need to instantiate some finalizing gates, but we want to be able to reset the
     * state back to what it was before. So we create a "circuit-in-the-head". So we need to be able to switch between
     * an actual implementation and an in-the-head one
     *
     * @tparam T The type of the member that is being switched
     * @param virtual_member The in-the-head instance
     * @param real_member The real instance
     * @param in_the_head The switch
     * @return T& A reference to the in-the-head instance if in_the_head is true, otherwise a reference to the real
     * instance
     */
    template <typename T> inline T& choose_virtual_or_real(T& virtual_member, T& real_member, bool in_the_head)
    {
        if (in_the_head) {
            return virtual_member;
        } else {
            return real_member;
        }
    }

#define PARENS ()
// Rescan macro tokens 256 times
#define EXPAND(arg) EXPAND1(EXPAND1(EXPAND1(EXPAND1(arg))))
#define EXPAND1(arg) EXPAND2(EXPAND2(EXPAND2(EXPAND2(arg))))
#define EXPAND2(arg) EXPAND3(EXPAND3(EXPAND3(EXPAND3(arg))))
#define EXPAND3(arg) EXPAND4(EXPAND4(EXPAND4(EXPAND4(arg))))
#define EXPAND4(arg) arg

#define FOR_EACH(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, a2, a3, ...)                                                                        \
    macro(a1, a2, a3) __VA_OPT__(FOR_EACH_AGAIN PARENS(macro, a1, a2, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define HEAD(x, ...) x
#define TAIL(x, ...) __VA_ARGS__
#define IN_THE_HEAD_MEMBER_NAME(member) circuit_in_the_head.member
#define ASSIGN_VARIABLE_TO_VIRTUAL_OR_REAL(variable_prefix, switch_name, member)                                       \
    auto& variable_prefix##member = choose_virtual_or_real(IN_THE_HEAD_MEMBER_NAME(member), member, switch_name);      \
    (void)variable_prefix##member;
#define CHOOSE_VIRTUAL_OR_REAL_MULTIPLE(variable_prefix, switch_name, member_name, ...)                                \
    FOR_EACH(ASSIGN_VARIABLE_TO_VIRTUAL_OR_REAL, variable_prefix, switch_name, member_name, __VA_ARGS__)
#define ENABLE_ALL_IN_THE_HEAD_SWITCHES                                                                                \
    CHOOSE_VIRTUAL_OR_REAL_MULTIPLE(switched_,                                                                         \
                                    in_the_head,                                                                       \
                                    w_l,                                                                               \
                                    w_r,                                                                               \
                                    w_o,                                                                               \
                                    w_4,                                                                               \
                                    q_m,                                                                               \
                                    q_1,                                                                               \
                                    q_2,                                                                               \
                                    q_3,                                                                               \
                                    q_c,                                                                               \
                                    q_arith,                                                                           \
                                    q_4,                                                                               \
                                    q_sort,                                                                            \
                                    q_lookup_type,                                                                     \
                                    q_elliptic,                                                                        \
                                    q_aux,                                                                             \
                                    num_gates,                                                                         \
                                    ram_arrays,                                                                        \
                                    rom_arrays,                                                                        \
                                    memory_read_records,                                                               \
                                    memory_write_records,                                                              \
                                    range_lists,                                                                       \
                                    real_variable_tags,                                                                \
                                    real_variable_index)
    // Circuit evaluation methods

    fr compute_arithmetic_identity(fr q_arith_value,
                                   fr q_1_value,
                                   fr q_2_value,
                                   fr q_3_value,
                                   fr q_4_value,
                                   fr q_m_value,
                                   fr q_c_value,
                                   fr w_1_value,
                                   fr w_2_value,
                                   fr w_3_value,
                                   fr w_4_value,
                                   fr w_1_shifted_value,
                                   fr w_4_shifted_value,
                                   const fr alpha_base,
                                   const fr alpha) const;
    fr compute_auxilary_identity(fr q_aux_value,
                                 fr q_arith_value,
                                 fr q_1_value,
                                 fr q_2_value,
                                 fr q_3_value,
                                 fr q_4_value,
                                 fr q_m_value,
                                 fr q_c_value,
                                 fr w_1_value,
                                 fr w_2_value,
                                 fr w_3_value,
                                 fr w_4_value,
                                 fr w_1_shifted_value,
                                 fr w_2_shifted_value,
                                 fr w_3_shifted_value,
                                 fr w_4_shifted_value,
                                 fr alpha_base,
                                 fr alpha,
                                 fr eta) const;
    fr compute_elliptic_identity(fr q_elliptic_value,
                                 fr q_1_value,
                                 fr q_3_value,
                                 fr q_4_value,
                                 fr w_2_value,
                                 fr w_3_value,
                                 fr w_1_shifted_value,
                                 fr w_2_shifted_value,
                                 fr w_3_shifted_value,
                                 fr w_4_shifted_value,
                                 fr alpha_base,
                                 fr alpha) const;
    fr compute_genperm_sort_identity(fr q_sort_value,
                                     fr w_1_value,
                                     fr w_2_value,
                                     fr w_3_value,
                                     fr w_4_value,
                                     fr w_1_shifted_value,
                                     fr alpha_base,
                                     fr alpha) const;

    void update_circuit_in_the_head();
    bool check_circuit();
};
} // namespace proof_system
