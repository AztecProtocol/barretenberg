#pragma once
#include <array>
#include <stdlib/primitives/uint/uint.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>
#include <stdlib/primitives/byte_array/byte_array.hpp>

namespace waffle {
class UltraComposer;
} // namespace waffle

namespace plonk {
namespace stdlib {
template <typename Composer> class bit_array;

template <typename Composer> class keccak {
  public:
    using witness_ct = stdlib::witness_t<Composer>;
    using field_ct = stdlib::field_t<Composer>;
    using byte_array_ct = stdlib::byte_array<Composer>;

    struct keccak_state {
        std::array<field_ct, 25> state;
        std::array<field_ct, 25> state_lsb;
        std::array<field_ct, 25> twisted_state;
        Composer* context;
    };

    static byte_array_ct hash(byte_array_ct& input);

    static void keccakf1600(keccak_state& state);

    static void compute_twisted_state(keccak_state& internal);

    static void keccak_block();

    static void normalize_chi_table(const field_ct& input, field_ct& output, field_ct& lsb_slice);

    static void theta(keccak_state& state, size_t round);
    static void rho(keccak_state& state, size_t round);
    static void pi(keccak_state& state);
    static void chi(keccak_state& state, bool is_last_round);
    static void iota(keccak_state& state);

    static void sponge_absorb(keccak_state& internal, std::vector<field_ct>& input_buffer);
    static byte_array_ct sponge_squeeze(keccak_state& internal);

    // static std::array<field_t, 8> keccak_block(const std::array<uint32<Composer>, 8>& h_init,
    //                                              const std::array<uint32<Composer>, 16>& input);
};

extern template class keccak<waffle::UltraComposer>;

} // namespace stdlib
} // namespace plonk
