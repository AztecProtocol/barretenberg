#pragma once
#include <cstdint>
#include <vector>
#include "plonk/composer/ultra_composer.hpp"
#include "stdlib/primitives/field/field.hpp"

namespace acir_format {

struct MemOp {
    field_pt access_type;
    field_pt index;
    field_pt value;
};

enum BlockType {
    ROM = 0,
    RAM = 1,
};

typedef stdlib::field_t<plonk::UltraComposer> field_pt;

struct BlockConstraint {
    std::vector<field_pt> init;
    std::vector<MemOp> trace;
    BlockType type;
   // friend bool operator==(Sha256Constraint const& lhs, Sha256Constraint const& rhs) = default;
};


}