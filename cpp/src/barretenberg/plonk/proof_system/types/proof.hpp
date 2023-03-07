#pragma once
#include <cstdint>
#include <vector>

namespace bonk {

struct proof {
    std::vector<uint8_t> proof_data;
};

} // namespace bonk