#pragma once
#include <cstdint>
#include <vector>
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/streams.hpp"

namespace plonk {

struct proof {
    std::vector<uint8_t> proof_data;
};

inline void read(uint8_t const*& it, proof& data)
{
    using serialize::read;
    read(it, data.proof_data);
};

template <typename B> inline void write(B& buf, proof const& data)
{
    using serialize::write;
    write(buf, data.proof_data);
}

inline std::ostream& operator<<(std::ostream& os, proof const& data)
{
    os << data.proof_data << "\n";
    return os;
}

} // namespace plonk