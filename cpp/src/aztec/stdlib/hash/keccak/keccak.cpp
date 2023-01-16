#include "keccak.hpp"
#include <plonk/composer/ultra_composer.hpp>
#include <stdlib/primitives/uint/uint.hpp>

namespace plonk {
namespace stdlib {

/* Round constants */
static constexpr uint64_t RC[24] = { 0x0000000000000001, 0x0000000000008082, 0x800000000000808a, 0x8000000080008000,
                                     0x000000000000808b, 0x0000000080000001, 0x8000000080008081, 0x8000000000008009,
                                     0x000000000000008a, 0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
                                     0x000000008000808b, 0x800000000000008b, 0x8000000000008089, 0x8000000000008003,
                                     0x8000000000008002, 0x8000000000000080, 0x000000000000800a, 0x800000008000000a,
                                     0x8000000080008081, 0x8000000000008080, 0x0000000080000001, 0x8000000080008008 };

template <typename Composer> void keccak<Composer>::keccak_block() {}

template <typename Composer> void keccak<Composer>::compute_twisted_state(keccak_state& internal)
{
    for (size_t i = 0; i < 25; ++i) {
        const field_ct shift = (uint256_t(1) << 64) - uint256_t(1);
        internal.twisted_state[i] = internal.state[i] + shift * internal.state_lsb[i];
    }
}
/*
825059588639499075
539089842888977227
1114420279991805767
6203257851319037775
698499540697168667
1975720462846735187
1686359771495218011
5045772449123612503*/
template <typename Composer> void keccak<Composer>::theta(keccak_state& internal, size_t /*round*/)
{
    // std::cout << "THETA" << std::endl;
    // if (round == 0) {
    //     std::cout << "computed THETA INPUT " << std::endl;
    //     for (size_t i = 0; i < 25; ++i) {
    //         std::cout << std::hex << convert_to_binary(internal.state[i]) << std::dec << std::endl;
    //     }
    // }
    std::array<field_ct, 5> C;
    std::array<field_ct, 5> D;

    for (size_t i = 0; i < 25; ++i) {
        // std::cout << "theta lookup at i = " << i << std::endl;
        // check_limb_values(internal.state[i]);
        // uint256_t v = internal.state[i].get_value();
        // constexpr uint256_t max = uint256_t(11).pow(64);
        // std::cout << "PRE RHO state " << i << " bits = " << v.get_msb() << "vs " << max.get_msb() << std::endl;
    }
    auto& state = internal.state;
    const auto& twisted_state = internal.twisted_state;
    for (size_t i = 0; i < 5; ++i) {
        // TODO use optimized big add gates
        C[i] = twisted_state[i] + twisted_state[5 + i] + twisted_state[10 + i] + twisted_state[15 + i] +
               twisted_state[20 + i];
    }

    for (size_t i = 0; i < 5; ++i) {
        const auto rotl_equivalent = C[(i + 1) % 5] + C[(i + 1) % 5];
        D[i] = C[(i + 4) % 5] + rotl_equivalent;
    }

    // D contains 66 base-11 slices. Need to remove the 2 most significant slices
    for (size_t i = 0; i < 5; ++i) {
        // TODO: much more efficient to do this as part of the KECCAK_THETA_OUTPUT table
        // 1. create witness `D_hi` for component of D that is > 11^64 and range constraint to be < 11^2
        // 2. D[i] = D[i] - (D_hi * 11^64);
        // 3. ensure that KECCAK_THETA_OUTPUT multitable can only accomodate a maximum input value of 11^64 - 1
        uint256_t D_native = D[i].get_value();
        constexpr uint256_t divisor = uint256_t(11).pow(64);
        const uint256_t hi_native = D_native / divisor;
        const uint256_t lo_native = D_native - (hi_native * divisor);
        field_ct hi(witness_ct(internal.context, hi_native));
        field_ct lo(witness_ct(internal.context, lo_native));

        D[i].assert_equal(hi.madd(divisor, lo));
        internal.context->create_new_range_constraint(hi.get_witness_index(), 11 * 11);
        // todo range constrain `lo` if we're going to keep this logic
        D[i] = lo;
    }
    // std::cout << "A" << std::endl;
    for (size_t i = 0; i < 5; ++i) {
        D[i] = plookup_read::read_from_1_to_2_table(KECCAK_THETA_OUTPUT, D[i]);
    }
    // std::cout << "B" << std::endl;
    for (size_t i = 0; i < 5; ++i) {
        for (size_t j = 0; j < 5; ++j) {
            state[j * 5 + i] = state[j * 5 + i] + D[i];
        }
    }
}

template <typename Composer> void keccak<Composer>::rho(keccak_state& internal, size_t round)
{
    // std::cout << "RHO" << std::endl;
    for (size_t i = 0; i < 25; ++i) {
        // std::cout << "rho output i = " << i << std::endl;
        // uint256_t v = internal.state[i].get_value();
        // constexpr uint256_t max = uint256_t(11).pow(64);
        // std::cout << "input bits = " << v.get_msb() << "vs " << max.get_msb() << std::endl;
        internal.state[i] = plookup_read::read_from_1_to_2_table(KECCAK_RHO_OUTPUT, internal.state[i]);

        // rotate :/
    }
    if (round == 0) {
        std::cout << "computed THETA OUTPUT " << std::endl;
        for (size_t i = 0; i < 25; ++i) {
            std::cout << std::hex << convert_to_binary(internal.state[i]) << std::dec << std::endl;
        }
    }
    for (size_t i = 0; i < 25; ++i) {
        // check_limb_values(internal.state[i]);
        // uint256_t v = internal.state[i].get_value();
        // constexpr uint256_t max = uint256_t(11).pow(64);
        // std::cout << "PRE RHO state " << i << " bits = " << v.get_msb() << "vs " << max.get_msb() << std::endl;
    }
    // std::cout << "mark" << std::endl;
}

template <typename Composer> void keccak<Composer>::pi(keccak_state& internal)
{
    std::array<field_ct, 25> B;

    for (size_t j = 0; j < 5; ++j) {
        for (size_t i = 0; i < 5; ++i) {
            B[j * 5 + i] = internal.state[j * 5 + i];
        }
    }

    for (size_t y = 0; y < 5; ++y) {
        for (size_t x = 0; x < 5; ++x) {
            size_t u = (0 * x + 1 * y) % 5;
            size_t v = (2 * x + 3 * y) % 5;

            internal.state[v * 5 + u] = B[5 * y + x];
        }
    }
}

template <typename Composer> uint256_t convert_to_binary(stdlib::field_t<Composer> limb)
{
    uint256_t input = limb.get_value();
    uint256_t output = 0;
    uint256_t scale_factor = 1;
    while (input > 0) {
        uint256_t slice = input % 11;
        output += (static_cast<uint256_t>(slice) * scale_factor);
        scale_factor += scale_factor;
        input /= 11;
    }
    return output;
}

template <typename Composer> void check_limb_values(stdlib::field_t<Composer> limb, uint64_t max_allowed_value = 1)
{
    uint256_t value = limb.get_value();

    size_t count = 0;
    while (value > 0) {
        uint256_t slice = value % 11;
        if (slice > max_allowed_value) {
            std::cout << "slice value of " << slice << " at bit index " << count << std::endl;
        }
        value -= slice;
        value = value / 11;
        count++;
    }
}

constexpr uint256_t get_chi_offset()
{
    uint256_t result = 0;
    for (size_t i = 0; i < 64; ++i) {
        result *= 11;
        result += 1;
    }
    return result;
}
template <typename Composer> void keccak<Composer>::chi(keccak_state& internal, bool is_last_round)
{
    // std::cout << "CHI" << std::endl;
    auto& state = internal.state;

    for (size_t i = 0; i < 25; ++i) {
        // check_limb_values(internal.state[i]);
    }
    const auto normalize_state_limb = [&](const size_t i) {
        // uint256_t v = internal.state[i].get_value();
        // constexpr uint256_t max = uint256_t(11).pow(64);
        // std::cout << "chi state " << i << " bits = " << v.get_msb() << "vs " << max.get_msb() << std::endl;
        // check_limb_values(internal.state[i], 4);

        auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_CHI_OUTPUT, internal.state[i]);
        internal.state[i] = accumulators[ColumnIdx::C2][0];
        internal.state_lsb[i] = accumulators[ColumnIdx::C3][0];
    };

    for (size_t y = 0; y < 5; ++y) {
        for (size_t x = 0; x < 5; ++x) {

            // 0 -> 0 1 2
            // 1 -> 1 2 3
            // 2 -> 2 3 4
            // 3 -> 3 4 0
            // 4 -> 4 0 1 <-- ooooh
            const auto A = state[y * 5 + x];
            const auto B = state[y * 5 + ((x + 1) % 5)];
            const auto C = state[y * 5 + ((x + 2) % 5)];

            const auto AA = A + A + get_chi_offset();
            state[y * 5 + x] = AA.add_two(-B, C);

            // if (!is_last_round) {
            normalize_state_limb(y * 5 + x);
            // }
        }
    }

    // handled in sponge construction iff this is last round
    if (!is_last_round) {
        // for (size_t i = 0; i < 25; ++i) {
        //     std::cout << "chi lookup at i = " << i << std::endl;
        //     uint256_t v = internal.state[i].get_value();
        //     constexpr uint256_t max = uint256_t(11).pow(64);
        //     std::cout << "input bits = " << v.get_msb() << "vs " << max.get_msb() << std::endl;

        //     auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_CHI_OUTPUT, internal.state[i]);
        //     internal.state[i] = accumulators[ColumnIdx::C2][0];
        //     internal.state_lsb[i] = accumulators[ColumnIdx::C3][0];
        //     compute_twisted_state(internal);
        // }
        compute_twisted_state(internal);
    }
}

template <typename Composer> void keccak<Composer>::iota(keccak_state& internal)
{
    // TODO FIX, this doesn't generate constraints
    const auto convert_to_binary = [](uint256_t input) {
        uint256_t output = 0;
        uint256_t scale_factor = 1;
        while (input > 0) {
            uint256_t slice = input % 11;
            output += (slice * scale_factor);
            scale_factor += scale_factor;
            input /= 11;
        }
        return output;
    };

    const auto convert_to_sparse = [](uint256_t input) {
        uint256_t output = 0;
        uint256_t scale_factor = 1;
        while (input > 0) {
            uint256_t slice = input & 1;
            output += (slice * scale_factor);
            scale_factor *= 11;
            input = input >> 1;
        }
        return output;
    };
    //  state[0] = state[0] ^ RC[i];
    uint256_t native = convert_to_binary(internal.state[0].get_value());
    uint256_t result = native ^ uint256_t(RC[0]);
    uint256_t lsb = result >> 63;

    const field_ct shift = (uint256_t(1) << 64) - uint256_t(1);

    internal.state[0] = field_ct(witness_ct(internal.context, convert_to_sparse(result)));
    internal.state_lsb[0] = field_ct(witness_ct(internal.context, lsb));
    internal.twisted_state[0] = (internal.state[0] + shift * internal.state_lsb[0]).normalize();
}

template <typename Composer> void keccak<Composer>::keccakf1600(keccak_state& internal)
{
    for (size_t i = 0; i < 24; ++i) {
        // std::cout << "KECCAK ROUND " << i << std::endl;
        theta(internal, i);
        rho(internal, i);
        pi(internal);
        chi(internal, i == 23);
        iota(internal);
    }
}

template <typename Composer>
void keccak<Composer>::sponge_absorb(keccak_state& internal, std::vector<field_ct>& input_buffer)
{
    const size_t l = input_buffer.size();

    // todo make these static class members
    constexpr size_t bits = 256;
    constexpr size_t word_size = 8;
    constexpr size_t block_size = (1600 - bits * 2) / word_size;
    constexpr size_t limbs_per_block = block_size / 8;
    const size_t num_blocks = l / (block_size / 8);

    std::cout << "BLOCK SIZE = " << block_size << " LENGTH = " << l << std::endl;
    for (size_t i = 0; i < num_blocks; ++i) {
        if (i == 0) {
            for (size_t j = 0; j < limbs_per_block; ++j) {
                internal.state[j] = input_buffer[j];
                // TODO FORMAT_INPUT table should add lsb into column C3
                uint256_t native = uint256_t(internal.state[j].get_value()) & uint256_t(1);
                internal.state_lsb[j] = field_ct(witness_ct(internal.context, native));
            }
            for (size_t j = limbs_per_block; j < 25; ++j) {
                internal.state[j] = witness_ct::create_constant_witness(internal.context, 0);
                internal.state_lsb[j] = witness_ct::create_constant_witness(internal.context, 0);
            }
        } else {
            for (size_t j = 0; j < limbs_per_block; ++j) {
                internal.state[j] += input_buffer[i * block_size + j];

                // we can re-use the RHO table to normalize the above XOR
                // TODO add lsb into 2nd RHO column
                auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_RHO_OUTPUT, internal.state[j]);
                internal.state[j] = accumulators[ColumnIdx::C2][0];
                internal.state_lsb[j] = accumulators[ColumnIdx::C3][0];
            }
        }

        std::cout << "KEEEECAAAAK" << std::endl;
        compute_twisted_state(internal);
        keccakf1600(internal);
    }
}

template <typename Composer> byte_array<Composer> keccak<Composer>::sponge_squeeze(keccak_state& internal)
{
    byte_array_ct result(internal.context);
    for (size_t i = 0; i < 4; ++i) {
        field_ct output_limb = plookup_read::read_from_1_to_2_table(KECCAK_FORMAT_OUTPUT, internal.state[i]);
        result.write(byte_array_ct(output_limb, 8));
    }
    return result;
}

template <typename Composer> stdlib::byte_array<Composer> keccak<Composer>::hash(byte_array_ct& input)
{
    auto ctx = input.get_context();
    ASSERT(ctx != nullptr);
    // TODO HANDLE CONSTANT INPUTS

    constexpr size_t bits = 256;
    constexpr size_t word_size = 8;
    const size_t input_size = input.size();

    const size_t block_size = (1600 - bits * 2) / word_size;

    // copy input into buffer and pad
    const size_t blocks = input_size / block_size;
    const size_t blocks_length = (block_size * (blocks + 1));

    byte_array_ct block_bytes(input);

    const size_t byte_difference = blocks_length - input_size;
    byte_array_ct padding_bytes(ctx, byte_difference);
    for (size_t i = 0; i < byte_difference; ++i) {
        padding_bytes.set_byte(i, witness_ct::create_constant_witness(ctx, 0));
    }

    block_bytes.write(padding_bytes);
    block_bytes.set_byte(input_size, witness_ct::create_constant_witness(ctx, 0x1));
    block_bytes.set_byte(block_bytes.size() - 1, witness_ct::create_constant_witness(ctx, 0x80));

    // keccak lanes interpret memory as little-endian integers,
    // means we need to swap our byte ordering...
    std::cout << "bb size = " << block_bytes.size() << std::endl;
    for (size_t i = 0; i < block_bytes.size(); i += 8) {
        std::array<field_ct, 8> temp;
        for (size_t j = 0; j < 8; ++j) {
            temp[j] = block_bytes[i + j];
        }
        if (i == 0) {
            std::cout << "TEMP0 = " << temp[0] << std::endl;
        }
        block_bytes.set_byte(i, temp[7]);
        block_bytes.set_byte(i + 1, temp[6]);
        block_bytes.set_byte(i + 2, temp[5]);
        block_bytes.set_byte(i + 3, temp[4]);
        block_bytes.set_byte(i + 4, temp[3]);
        block_bytes.set_byte(i + 5, temp[2]);
        block_bytes.set_byte(i + 6, temp[1]);
        block_bytes.set_byte(i + 7, temp[0]);
    }
    const size_t byte_size = block_bytes.size();
    keccak_state internal;
    internal.context = ctx;

    const size_t num_limbs = byte_size / word_size;
    std::vector<field_ct> converted_buffer(num_limbs);

    std::cout << "num limbs = " << num_limbs << std::endl;
    for (size_t i = 0; i < num_limbs; ++i) {
        field_ct sliced;
        if (i * word_size + word_size > byte_size) {
            const size_t slice_size = byte_size - (i * word_size);
            const size_t byte_shift = (word_size - slice_size) * 8;
            sliced = field_ct(block_bytes.slice(i * word_size, slice_size));
            sliced = (sliced * (uint256_t(1) << byte_shift)).normalize();
        } else {
            sliced = field_ct(block_bytes.slice(i * word_size, word_size));
            uint256_t debug = ((uint256_t(sliced.get_value())));
            std::cout << "sliced = " << std::hex << debug << std::dec << std::endl;
        }
        converted_buffer[i] = plookup_read::read_from_1_to_2_table(KECCAK_FORMAT_INPUT, sliced);
        // std::cout << "CONVERTED BUFFER VALUES " << std::endl;
        check_limb_values(converted_buffer[i]);
    }

    std::cout << "buffer after conversion: " << std::endl;
    for (size_t i = 0; i < 17; ++i) {
        std::cout << convert_to_binary(converted_buffer[i]) << std::endl;
    }
    sponge_absorb(internal, converted_buffer);
    return sponge_squeeze(internal);
    // //   std::cout << "NUM BYTES VS BLOCK SIZE = " << num_bytes << " : " << block_size << std::endl;
    // while (input_size >= block_size) {
    //     for (size_t i = 0; i < (block_size / word_size); ++i) {
    //         if (init == false) {
    //             internal.state[i] = converted_buffer[i + byte_offset];

    //             // get twisted form
    //             // TODO FORMAT_INPUT table should add lsb into column C3
    //             uint256_t native = uint256_t(internal.state[i].get_value()) & uint256_t(1);
    //             internal.state_lsb[i] = field_ct(witness_ct(ctx, native));
    //             // auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_CHI_OUTPUT, internal.state[i]);
    //             // internal.state[i] = accumulators[ColumnIdx::C2][0];
    //             // internal.state_lsb[i] = accumulators[ColumnIdx::C3][0];
    //             //  check_limb_values(internal.state[i]);

    //         } else {
    //             // TODO CHI table needs to be over base 6 not 5
    //             internal.state[i] += converted_buffer[i + byte_offset];
    //             auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_CHI_OUTPUT, internal.state[i]);
    //             internal.state[i] = accumulators[ColumnIdx::C2][0];
    //             internal.state_lsb[i] = accumulators[ColumnIdx::C3][0];
    //         }
    //     }
    //     compute_twisted_state(internal);
    //     init = true;
    //     byte_offset += block_size;
    //     keccakf1600(internal);
    //     num_bytes -= block_size;
    // }
    // std::cout << "NUM BYTES = " << num_bytes << std::endl;
    // size_t i = 0;
    // while (num_bytes >= word_size) {
    //     // xor current buffer data into state_iter
    //     // todo remove code duplication
    //     if (init == false) {
    //         internal.state[i] = converted_buffer[i + byte_offset];

    //         // TODO add lsb into FORMAT_INPUT table
    //         uint256_t native = uint256_t(internal.state[i].get_value()) & uint256_t(1);
    //         internal.state_lsb[i] = field_ct(witness_ct(ctx, native));
    //         // // get twisted form
    //         // // TODO this is v. v. inefficient! FORMAT_INPUT table should add lsb into column C3
    //         // auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_CHI_OUTPUT, internal.state[i]);
    //         // internal.state[i] = accumulators[ColumnIdx::C2][0];
    //         // internal.state_lsb[i] = accumulators[ColumnIdx::C3][0];
    //     } else {
    //         // TODO CHI table needs to be over base 6 not 5
    //         internal.state[i] += converted_buffer[i + byte_offset];
    //         auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_CHI_OUTPUT, internal.state[i]);
    //         internal.state[i] = accumulators[ColumnIdx::C2][0];
    //         internal.state_lsb[i] = accumulators[ColumnIdx::C3][0];
    //     }
    //     compute_twisted_state(internal);
    //     num_bytes -= word_size;
    //     i += 1;
    //     byte_offset += word_size;
    // }

    // std::cout << "CALLING KECCAK" << std::endl;
    // keccakf1600(internal);

    // byte_array_ct result(ctx);
    // for (size_t i = 0; i < 4; ++i) {
    //     field_ct output_limb = plookup_read::read_from_1_to_2_table(KECCAK_FORMAT_OUTPUT, internal.state[i]);
    //     result.write(byte_array_ct(output_limb, 8));
    // }
    // return result;
}

template class keccak<waffle::UltraComposer>;

} // namespace stdlib
} // namespace plonk