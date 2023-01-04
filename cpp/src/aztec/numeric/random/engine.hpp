#pragma once
#include "../uint128/uint128.hpp"
#include "../uint256/uint256.hpp"
#include "../uintx/uintx.hpp"
#include "stdint.h"
#include "unistd.h"
#include <cstdint>

namespace numeric {
namespace random {

class Engine {
  public:
    virtual uint8_t get_random_uint8() = 0;

    virtual uint16_t get_random_uint16() = 0;

    virtual uint32_t get_random_uint32() = 0;

    virtual uint64_t get_random_uint64() = 0;

    virtual uint128_t get_random_uint128() = 0;

    virtual uint256_t get_random_uint256() = 0;

    uint512_t get_random_uint512()
    {
        // Do not inline in constructor call. Evaluation order is important for cross-compiler consistency.
        auto lo = get_random_uint256();
        auto hi = get_random_uint256();
        return uint512_t(lo, hi);
    }

    uint1024_t get_random_uint1024()
    {
        // Do not inline in constructor call. Evaluation order is important for cross-compiler consistency.
        auto lo = get_random_uint512();
        auto hi = get_random_uint512();
        return uint1024_t(lo, hi);
    }
};

/**
 * @brief Retruns a reference to the global debug engine.
 *
 * @details When seed=0 (by default), the engine is not reset. Otherwise, the engine weill be reset
 * and the new seed will be used.
 *
 * @warning Only a single instance of a DebugEngine exists at any given time. If the engine is reseeded,
 * all previous references to the debug engine will use the newly reset engine.
 *
 * @param seed
 * @return Engine&
 */
Engine& get_debug_engine(uint64_t seed = 0);
Engine& get_engine();

} // namespace random
} // namespace numeric