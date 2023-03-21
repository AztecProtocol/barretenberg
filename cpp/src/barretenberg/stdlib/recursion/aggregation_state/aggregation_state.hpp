#pragma once
#include "../../primitives/field/field.hpp"

namespace plonk {
namespace stdlib {
namespace recursion {

template <typename Curve> struct aggregation_state {
    typename Curve::g1_ct P0;
    typename Curve::g1_ct P1;

    // The public inputs of the inner ciruit are now private inputs of the outer circuit!
    std::vector<typename Curve::fr_ct> public_inputs;
    std::vector<uint32_t> proof_witness_indices;
    bool has_data = false;

    void add_proof_outputs_as_public_inputs()
    {
        ASSERT(proof_witness_indices.size() > 0);

        auto* context = P0.get_context();

        context->add_recursive_proof(proof_witness_indices);
    }
};

} // namespace recursion
} // namespace stdlib
} // namespace plonk