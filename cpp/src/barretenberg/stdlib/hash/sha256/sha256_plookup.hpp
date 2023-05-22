#pragma once
#include <array>
#include "barretenberg/proof_system/plookup_tables/plookup_tables.hpp"
#include "barretenberg/stdlib/primitives/uint/uint.hpp"

#include "barretenberg/numeric/bitop/sparse_form.hpp"

#include "../../primitives/field/field.hpp"
#include "../../primitives/packed_byte_array/packed_byte_array.hpp"

namespace proof_system::plonk {
class UltraPlonkComposer;
} // namespace proof_system::plonk

namespace proof_system::plonk {
namespace stdlib {
namespace sha256_plookup {

struct sparse_ch_value {
    field_t<plonk::UltraPlonkComposer> normal;
    field_t<plonk::UltraPlonkComposer> sparse;
    field_t<plonk::UltraPlonkComposer> rot6;
    field_t<plonk::UltraPlonkComposer> rot11;
    field_t<plonk::UltraPlonkComposer> rot25;
};

struct sparse_maj_value {
    field_t<plonk::UltraPlonkComposer> normal;
    field_t<plonk::UltraPlonkComposer> sparse;
    field_t<plonk::UltraPlonkComposer> rot2;
    field_t<plonk::UltraPlonkComposer> rot13;
    field_t<plonk::UltraPlonkComposer> rot22;
};

struct sparse_witness_limbs {
    sparse_witness_limbs(const field_t<plonk::UltraPlonkComposer>& in = 0)
    {
        normal = in;
        has_sparse_limbs = false;
    }
    sparse_witness_limbs(const sparse_witness_limbs& other) = default;
    sparse_witness_limbs(sparse_witness_limbs&& other) = default;

    sparse_witness_limbs& operator=(const sparse_witness_limbs& other) = default;
    sparse_witness_limbs& operator=(sparse_witness_limbs&& other) = default;

    field_t<plonk::UltraPlonkComposer> normal;

    std::array<field_t<plonk::UltraPlonkComposer>, 4> sparse_limbs;

    std::array<field_t<plonk::UltraPlonkComposer>, 4> rotated_limbs;

    bool has_sparse_limbs = false;
};

struct sparse_value {
    sparse_value(const field_t<plonk::UltraPlonkComposer>& in = 0)
    {
        normal = in;
        if (normal.witness_index == IS_CONSTANT) {
            sparse = field_t<plonk::UltraPlonkComposer>(
                in.get_context(),
                barretenberg::fr(numeric::map_into_sparse_form<16>(uint256_t(in.get_value()).data[0])));
        }
    }

    sparse_value(const sparse_value& other) = default;
    sparse_value(sparse_value&& other) = default;

    sparse_value& operator=(const sparse_value& other) = default;
    sparse_value& operator=(sparse_value&& other) = default;

    field_t<plonk::UltraPlonkComposer> normal;
    field_t<plonk::UltraPlonkComposer> sparse;
};

sparse_witness_limbs convert_witness(const field_t<plonk::UltraPlonkComposer>& w);

std::array<field_t<plonk::UltraPlonkComposer>, 64> extend_witness(
    const std::array<field_t<plonk::UltraPlonkComposer>, 16>& w_in);

field_t<plonk::UltraPlonkComposer> choose(sparse_value& e, const sparse_value& f, const sparse_value& g);
field_t<plonk::UltraPlonkComposer> majority(sparse_value& a, const sparse_value& b, const sparse_value& c);

std::array<field_t<plonk::UltraPlonkComposer>, 8> sha256_block(
    const std::array<field_t<plonk::UltraPlonkComposer>, 8>& h_init,
    const std::array<field_t<plonk::UltraPlonkComposer>, 16>& input);

packed_byte_array<plonk::UltraPlonkComposer> sha256(const packed_byte_array<plonk::UltraPlonkComposer>& input);
} // namespace sha256_plookup
} // namespace stdlib
} // namespace proof_system::plonk
