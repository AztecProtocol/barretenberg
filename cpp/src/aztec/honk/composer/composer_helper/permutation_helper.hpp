#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <proof_system/proving_key/proving_key.hpp>
namespace waffle {
// Enum values spaced in increments of 30-bits (multiples of 2 ** 30).
enum WireType { LEFT = 0U, RIGHT = (1U << 30U), OUTPUT = (1U << 31U), FOURTH = 0xc0000000 };

/**
 * @brief cycle_node represents a particular witness at a particular gate. Used to collect permutation sets
 *
 */
struct cycle_node {
    uint32_t gate_index;
    WireType wire_type;

    cycle_node(const uint32_t a, const WireType b)
        : gate_index(a)
        , wire_type(b)
    {}
    cycle_node(const cycle_node& other)
        : gate_index(other.gate_index)
        , wire_type(other.wire_type)
    {}
    cycle_node(cycle_node&& other)
        : gate_index(other.gate_index)
        , wire_type(other.wire_type)
    {}
    cycle_node& operator=(const cycle_node& other)
    {
        gate_index = other.gate_index;
        wire_type = other.wire_type;
        return *this;
    }
    bool operator==(const cycle_node& other) const
    {
        return ((gate_index == other.gate_index) && (wire_type == other.wire_type));
    }
};
typedef std::vector<std::vector<cycle_node>> CycleCollector;

/**
 * Compute wire copy cycles
 *
 * First set all wire_copy_cycles corresponding to public_inputs to point to themselves.
 * Then go through all witnesses in w_l, w_r, w_o and w_4 (if program width is > 3) and
 * add them to cycles of their real indexes.
 *
 * @tparam program_width Program width
 * */
template <size_t program_width, typename CircuitConstructor>
void compute_wire_copy_cycles(CircuitConstructor& circuit_constructor, CycleCollector& wire_copy_cycles)
{
    auto& real_variable_index = circuit_constructor.real_variable_index;
    auto& public_inputs = circuit_constructor.public_inputs;
    auto& w_l = circuit_constructor.w_l;
    auto& w_r = circuit_constructor.w_r;
    auto& w_o = circuit_constructor.w_o;
    auto& n = circuit_constructor.n;
    auto& w_4 = circuit_constructor.w_4;

    size_t number_of_cycles = 0;
    // Initialize wire_copy_cycles of public input variables to point to themselves
    for (size_t i = 0; i < public_inputs.size(); ++i) {
        cycle_node left{ static_cast<uint32_t>(i), WireType::LEFT };
        cycle_node right{ static_cast<uint32_t>(i), WireType::RIGHT };

        const auto public_input_index = real_variable_index[public_inputs[i]];
        if (public_input_index >= number_of_cycles) {
            wire_copy_cycles.resize(public_input_index + 1);
        }
        std::vector<cycle_node>& cycle = wire_copy_cycles[static_cast<size_t>(public_input_index)];
        // These two nodes must be in adjacent locations in the cycle for correct handling of public inputs
        cycle.emplace_back(left);
        cycle.emplace_back(right);
    }

    const uint32_t num_public_inputs = static_cast<uint32_t>(public_inputs.size());

    // Go through all witnesses and add them to the wire_copy_cycles
    for (size_t i = 0; i < n; ++i) {
        const auto w_1_index = real_variable_index[w_l[i]];
        const auto w_2_index = real_variable_index[w_r[i]];
        const auto w_3_index = real_variable_index[w_o[i]];
        auto max_index = std::max({ w_1_index, w_2_index, w_3_index });
        if (max_index >= number_of_cycles) {
            wire_copy_cycles.resize(max_index + 1);
        }
        wire_copy_cycles[static_cast<size_t>(w_1_index)].emplace_back(static_cast<uint32_t>(i + num_public_inputs),
                                                                      WireType::LEFT);
        wire_copy_cycles[static_cast<size_t>(w_2_index)].emplace_back(static_cast<uint32_t>(i + num_public_inputs),
                                                                      WireType::RIGHT);
        wire_copy_cycles[static_cast<size_t>(w_3_index)].emplace_back(static_cast<uint32_t>(i + num_public_inputs),
                                                                      WireType::OUTPUT);

        if constexpr (program_width > 3) {
            const auto w_4_index = real_variable_index[w_4[i]];
            if (w_4_index >= number_of_cycles) {
                wire_copy_cycles.resize(w_4_index + 1);
            }
            wire_copy_cycles[static_cast<size_t>(w_4_index)].emplace_back(static_cast<uint32_t>(i + num_public_inputs),
                                                                          WireType::FOURTH);
        }
    }
}

template <size_t program_width, typename CircuitConstructor>
void compute_standard_honk_sigma_permutations(CircuitConstructor& circuit_constructor, proving_key* key)
{
    // Compute wire copy cycles for public and private variables
    CycleCollector wire_copy_cycles;
    compute_wire_copy_cycles<program_width>(circuit_constructor, wire_copy_cycles);
    auto n = key->n;
    // Fill sigma polynomials with default values
    barretenberg::polynomial sigma_polynomials_lagrange[program_width];
    for (uint64_t i = 0; i < program_width; ++i) {
        // Construct permutation polynomials in lagrange base
        std::string index = std::to_string(i + 1);
        sigma_polynomials_lagrange[i] = barretenberg::polynomial(key->n);
        auto& sigma_polynomial_lagrange = sigma_polynomials_lagrange[i];
        for (uint64_t j = 0; j < key->n; j++) {
            sigma_polynomial_lagrange.coefficients[j] = (i * n + j);
        }
    }
    // Go through each cycle
    for (size_t i = 0; i < wire_copy_cycles.size(); i++) {
        ASSERT(wire_copy_cycles[i].size() > 0);
        size_t cycle_size = wire_copy_cycles[i].size();
        cycle_node current_element = wire_copy_cycles[i][cycle_size - 1];
        // Get the index value of the last element
        auto last_index =
            sigma_polynomials_lagrange[current_element.wire_type >> 30].data()[current_element.gate_index];

        // Propagate indices through the cycle
        for (size_t j = 0; j < cycle_size; j++) {

            current_element = wire_copy_cycles[i][j];
            auto temp_index =
                sigma_polynomials_lagrange[current_element.wire_type >> 30].data()[current_element.gate_index];
            sigma_polynomials_lagrange[current_element.wire_type >> 30].data()[current_element.gate_index] = last_index;
            last_index = temp_index;
        }
    }
    // Save to polynomial cache
    for (size_t i = 0; i < program_width; i++) {

        std::string index = std::to_string(i + 1);
        key->polynomial_cache.put("sigma_" + index + "_lagrange", std::move(sigma_polynomials_lagrange[i]));
    }
}

} // namespace waffle
