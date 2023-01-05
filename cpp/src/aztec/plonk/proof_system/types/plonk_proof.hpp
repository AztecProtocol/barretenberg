#pragma once
#include <cstdint>
#include <vector>

// Uncomment the following line to run the prover/verifier in a print-debug mode.
// #define DEBUG_PROVER

namespace waffle {

struct plonk_proof {
    std::vector<uint8_t> proof_data;
};

} // namespace waffle