#pragma once
#include <cstdint>
#include <vector>

namespace waffle {

struct plonk_proof {
    std::vector<uint8_t> proof_data;
};

} // namespace waffle