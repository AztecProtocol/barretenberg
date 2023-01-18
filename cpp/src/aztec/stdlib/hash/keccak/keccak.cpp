#include "keccak.hpp"
#include <plonk/composer/ultra_composer.hpp>
#include <stdlib/primitives/uint/uint.hpp>
#include <common/constexpr_utils.hpp>
#include <numeric/bitop/sparse_form.hpp>
namespace plonk {
namespace stdlib {

/**
 * @brief Compute twisted representation of hash lane
 *
 * The THETA round requires computation of XOR(A, ROTL(B, 1))
 *
 * We do this via a 'twisted' base-11 representation.
 *
 * If the bit slices for a regular variable are arranged [b63, ..., b0],
 * the twisted representation is a 65-bit variable [b63, ..., b0, b63]
 *
 * The equivalent of XOR(A, ROTL(B, 1)) is A.twist + 2B.twist (in base-11 form)
 * The output is present in bit slices 1-64
 *
 * @tparam Composer
 * @param internal
 */
template <typename Composer> void keccak<Composer>::compute_twisted_state(keccak_state& internal)
{
    for (size_t i = 0; i < 25; ++i) {
        internal.twisted_state[i] = ((internal.state[i] * 11) + internal.state_msb[i]).normalize();
    }
}

/**
 * @brief THETA round
 *
 * @tparam Composer
 *
 * THETA consists of XOR operations as well as left rotations by 1 bit.
 *
 * We represent 64-bit integers in a base-11 representation where
 *  limb = \sum_{i=0}^63 b_i * 11^i
 *
 * At the start of THETA, all b_i values are either 0 or 1
 *
 * We can efficiently evaluate XOR operations via simple additions!
 * If b_i = even, this represents a bit value of 0
 * If b_i = odd, this represents a bit value of 1
 *
 * The KECCAK_THETA_OUTPUT lookup table is used to 'normalize' base-11 integers,
 * i.e. convert b_i values from [0, ..., 10] to [0, 1] where even == 0, odd == 1
 *
 * The choice of base for our representation effects the following:
 * 1. the number of normalization lookups required to avoid overflowing the base
 * 2. the cost of normalization lookups
 *
 * Bigger base reduces (1) but increases (2). For THETA, base-11 is optimal (I think...)
 *
 * ### HANDLING ROTATIONS
 *
 * We need to left-rotate the C[5] array by 1-bit to compute D[5]. Naive way is expensive so we cheat!
 * When converting integers into base-11 representation, we use a lookup table column to give us the
 * most significant bit of the integer.
 *
 * This enables us to create a 'twisted' representation of the integer in base-11:
 *
 * twisted_limb = (b_63) + \sum_{i=0}^63 b_i * 11^{i + 1}
 *
 * e.g. if limb's bit ordering is [0,   b63, ..., b1, b0 ]
 * twisted limb bit ordering is   [b63, b62, ..., b0, b63]
 *
 * We want to be able to compute XOR(A, B.rotate_left(1)) and can do this via twisted representations
 *
 * The equivalent in base-11 world is twisted_A * 2 + twisted_B.
 * The output of the XOR operation exists in bit-slices 1, ..., 63
 * (which can be extracted by removing the least and most significant slices of the output)
 * This is MUCH cheaper than the extra range constraints required for a naive left-rotation
 */
template <typename Composer> void keccak<Composer>::theta(keccak_state& internal)
{
    std::array<field_ct, 5> C;
    std::array<field_ct, 5> D;

    auto& state = internal.state;
    const auto& twisted_state = internal.twisted_state;
    for (size_t i = 0; i < 5; ++i) {

        /**
         * field_ct::accumulate can compute 5 addition operations in only 2 gates:
         * Gate 0 wires [a0, a1, a2, a3]
         * Gate 1 wires [b0, b1, b2, b3]
         * b3 = a0 + a1 + a2 + a3
         * b2 = b3 + b0 + b1
         * (b2 is the output wire)
         **/
        C[i] = field_ct::accumulate({ twisted_state[i],
                                      twisted_state[5 + i],
                                      twisted_state[10 + i],
                                      twisted_state[15 + i],
                                      twisted_state[20 + i] });
    }

    /**
     * Compute D by exploiting twisted representation
     * to get a cheap left-rotation by 1 bit
     */
    for (size_t i = 0; i < 5; ++i) {
        const auto non_shifted_equivalent = (C[(i + 4) % 5]);
        const auto shifted_equivalent = C[(i + 1) % 5] * 11;
        // TODO need normalize??
        D[i] = (non_shifted_equivalent + shifted_equivalent).normalize();
    }

    /**
     * D contains 66 base-11 slices.
     *
     * We need to remove the 2 most significant slices as they
     * are artifacts of our twist operation.
     *
     * We also need to 'normalize' D (i.e. convert each base value to be 0 or 1),
     * to prevent our base from overflowing when we XOR D into internal.state
     *
     * 1. create sliced_D witness, plus lo and hi slices
     * 2. validate D = lo + (sliced_D * 11) + (hi * 11^65)
     * 3. feed sliced_D into KECCAK_THETA_OUTPUT lookup table
     * 4. validate most significant lookup for sliced_D is < 11^4
     *
     * (point 4 is required because KECCAK_THETA_OUTPUT is a sequence of 13 5-bit lookups)
     * i.e. can support a maximum input value of 11^65 - 1
     *      and we need to ensure that `sliced_D < 11^64`
     *
     * N.B. we could improve efficiency by using more complex, larger lookup tables
     * i.e. special lookup tables that remove the least and most significant bits of D
     */
    static constexpr uint256_t divisor = BASE.pow(64);
    static constexpr uint256_t multiplicand = BASE.pow(65);
    for (size_t i = 0; i < 5; ++i) {
        uint256_t D_native = D[i].get_value();
        const auto [D_quotient, lo_native] = D_native.divmod(11);
        const uint256_t hi_native = D_quotient / divisor;
        const uint256_t mid_native = D_quotient - hi_native * divisor;

        field_ct hi(witness_ct(internal.context, hi_native));
        field_ct mid(witness_ct(internal.context, mid_native));
        field_ct lo(witness_ct(internal.context, lo_native));

        // assert equal should cost 1 gate (mulitpliers are all constants)
        D[i].assert_equal((hi * multiplicand).add_two(mid * 11, lo));
        internal.context->create_new_range_constraint(hi.get_witness_index(), 11);
        internal.context->create_new_range_constraint(lo.get_witness_index(), 11);
        D[i] = mid;
    }

    // Perform the lookup read from KECCAK_THETA_OUTPUT to normalize D
    for (size_t i = 0; i < 5; ++i) {
        const auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_THETA_OUTPUT, D[i]);
        D[i] = accumulators[ColumnIdx::C2][0];

        // Ensure input to lookup is < 11^64,
        // by validating most significant input slice is < 11^4
        const field_ct most_significant_slice = accumulators[ColumnIdx::C1][accumulators[ColumnIdx::C1].size() - 1];

        // N.B. cheaper to validate (11^4 - slice < 2^14) as this
        // prevents an extra range table from being created
        constexpr uint256_t maximum = BASE.pow(4);
        const field_ct target = -most_significant_slice + maximum;
        ASSERT(((uint256_t(1) << Composer::DEFAULT_PLOOKUP_RANGE_BITNUM) - 1) > maximum);
        target.create_range_constraint(Composer::DEFAULT_PLOOKUP_RANGE_BITNUM,
                                       "input to KECCAK_THETA_OUTPUT too large!");
    }

    // compute state[j * 5 + i] XOR D[i] in base-11 representation
    for (size_t i = 0; i < 5; ++i) {
        for (size_t j = 0; j < 5; ++j) {
            state[j * 5 + i] = state[j * 5 + i] + D[i];
        }
    }
}

/**
 * @brief RHO round
 *
 * @tparam Composer
 *
 * The limbs of internal.state are represented via base-11 integers
 *  limb = \sum_{i=0}^63 b_i * 11^i
 * The value of each b_i can be in the range [0, 1, 2] due to the THETA round XOR operations
 *
 * We need to do the following:
 *
 * 1. 'normalize' each limb so that each b_i value is 0 or 1
 * 2. left-rotate each limb as defined by the keccak `rotations` matrix
 *
 * The KECCAK_RHO_OUTPUT lookup table is used to normalize each limb.
 * Rotations are trickier.
 *
 * To efficiently rotate, we split each input limb into 'left' and 'right' components.
 * If input bits = [left, right], rotated bits = [right, left]
 * We then independently perform the KECCAK_RHO_OUTPUT lookup on the left and right input components.
 * This gives us implicit range checks for 'free' as part of the lookup protocol.
 *
 * Finally we stitch together the left, right lookup table outputs to produce our normalized rotated limb
 *
 * COST PER LIMB...
 *     (1 gate) Validate (left * left_shift + right) equals input limb
 *     (6-7 gates) 6-7 11-bit lookups (splitting into left/right can add an extra lookup)
 *     (2.5 gates) Range-constraining the most significant lookup slice of the right component (see comments)
 *     (1 gate) Stitching together normalized output limb from lookup table outputs
 *
 * Total costs are 10.5-11.5 gates per limb
 *
 * N.B. Future efficiency improvements that are possible but a pain to implement:
 *      1. can increase efficiency by changing lookup table interface,
 *         to accomodate accumulations across multiple MultiTables :o
 *         (i.e. remove explicitly validating `left * left_shift + right = input limb`)
 *      2. create multiple RHO lookup tables of varying bit slices
 *         to remove range constraint on right-component lookup slice.
 *         This range check is needed because the lookup table is 11 bits.
 *         if the 'right' slice is not an even multiple of 11 bits we need
 *         a range check to validate Prover has not sneaked part of the 'left' witness into 'right'.
 *         For example, for a rotate-left by 5 bits, we have a 5-bit RHO lookup table for LEFT
 *         and 5 * 11-bit lookups + a 5-bit lookup for RIGHT.
 *         (would likely need to reduce max lookup table bits to 10 to accomodate increased number of tables)
 *         (I think this would add 1 lookup gate on average vs current, but remove the 2.5 gates from range check)
 */
template <typename Composer> void keccak<Composer>::rho(keccak_state& internal)
{
    // 1st rotation of rho is 0, treat separately

    internal.state[0] = plookup_read::read_from_1_to_2_table(KECCAK_RHO_OUTPUT, internal.state[0]);

    constexpr_for<1, 25, 1>([&]<size_t i>() {
        // use a constexpr loop so that expensive uint256_t::pow method can be constexpr
        constexpr size_t left_bits = ROTATIONS[i];
        constexpr size_t right_bits = 64 - ROTATIONS[i];

        // TODO make max_bits_per_table a constant in class header
        constexpr size_t max_bits_per_table = 11;

        constexpr size_t num_left_tables =
            left_bits / max_bits_per_table + (left_bits % max_bits_per_table > 0 ? 1 : 0);
        constexpr size_t num_right_tables =
            right_bits / max_bits_per_table + (right_bits % max_bits_per_table > 0 ? 1 : 0);

        // voila, a compile time constant!
        constexpr uint256_t divisor = BASE.pow(right_bits);

        uint256_t input = internal.state[i].get_value();
        const auto [quotient, remainder] = input.divmod(divisor);
        const field_ct left(witness_ct(internal.context, quotient));
        const field_ct right(witness_ct(internal.context, remainder));

        internal.state[i].assert_equal(left.madd(divisor, right));

        field_ct rol_left = 0;
        field_ct rol_right = 0;

        if (num_left_tables > 0) {
            rol_left = plookup_read::read_from_1_to_2_table(KECCAK_RHO_OUTPUT, left, num_left_tables);
        }
        if (num_right_tables > 0) {
            const auto ror_accumulators =
                plookup_read::get_lookup_accumulators(KECCAK_RHO_OUTPUT, right, 0, false, num_right_tables);
            rol_right = ror_accumulators[ColumnIdx::C2][0];

            /**
             * validate the most significant slice < 11^{most significant slice bits}
             * N.B. If we do this for the right slice I don't think we need to do it for the left
             * as we can infer inductively it's correct...
             *
             * (the following reasoning is described in the binary basis for simplicity,
             * but is also valid in base 11)
             *
             * we know (left << right_bits + right) = input
             *
             * we also know that input < 2^64
             * We *want to validate that `left << right_bits` and `right` do not overlap
             * i.e. (left << right_bits).bit[i] == 1 && (right.bit[i] == 1) == FALSE
             *
             * If we validate that `right < (2 << left_bits)`...
             * The only way the two bitfields can overlap,
             * is iff `left << right_bits` wraps mod p
             *
             * But `left` has been fed into a lookup table sequence
             * which validates `left < (2 << num_left_tables)`
             *
             * i.e. left <<< p
             **/
            constexpr size_t most_significant_slice_bits = right_bits % 11;
            if (num_left_tables > 0 && most_significant_slice_bits > 0) {
                // if rotation == 0 we can implicitly rely on the fact that the input is already constraint to be < 2^64
                // if right_bits % 11 == 0 the RHO lookup table
                // will correctly range constrain the slice without additional constraints
                const field_ct ror_right_most_significant_slice = ror_accumulators[ColumnIdx::C1][num_right_tables - 1];

                constexpr uint256_t maximum = BASE.pow(most_significant_slice_bits);

                const field_ct should_be_greater_than_zero =
                    (field_ct(maximum) - ror_right_most_significant_slice).normalize();
                // check (maximum - slice) is < 2^(log2(maximum) + 1)
                // should be sufficient iff maximum < sqrt(p)
                constexpr size_t maximum_msb = maximum.get_msb();
                should_be_greater_than_zero.create_range_constraint(maximum_msb + 1,
                                                                    "keccak rho. rotated slice too large");
            }
        }
        const uint256_t multiplicand = BASE.pow(left_bits);

        // todo need normalize?
        internal.state[i] = rol_right.madd(multiplicand, rol_left).normalize();
    });
}

/**
 * @brief PI
 *
 * PI permutes the keccak lanes. Adds 0 constraints as this is simply a
 * re-ordering of witnesses
 *
 * @tparam Composer
 * @param internal
 */
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

/**
 * @brief CHI
 *
 * The CHI round applies the following logic to the hash lanes:
 *     A XOR (~B AND C)
 *
 * In base-11 representation we can create an equivalent linear operation:
 *     1 + 2A - B + C
 *
 * Output values will range from [0, 1, 2, 3, 4] and are mapped back into [0, 1]
 * via the KECCAK_CHI_OUTPUT lookup table
 *
 * N.B. the KECCAK_CHI_OUTPUT table also has a column for the most significant bit of each lookup.
 *      We use this to create a 'twisted representation of each hash lane (see THETA comments for more details)
 * @tparam Composer
 */
template <typename Composer> void keccak<Composer>::chi(keccak_state& internal)
{
    auto& state = internal.state;

    for (size_t y = 0; y < 5; ++y) {
        std::array<field_ct, 5> lane_outputs;
        for (size_t x = 0; x < 5; ++x) {
            const auto A = state[y * 5 + x];
            const auto B = state[y * 5 + ((x + 1) % 5)];
            const auto C = state[y * 5 + ((x + 2) % 5)];

            // vv should cost 1 gate
            lane_outputs[x] = (A + A + get_chi_offset()).add_two(-B, C);
        }
        for (size_t x = 0; x < 5; ++x) {
            // Normalize lane outputs and assign to internal.state
            auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_CHI_OUTPUT, lane_outputs[x]);
            internal.state[y * 5 + x] = accumulators[ColumnIdx::C2][0];
            internal.state_msb[y * 5 + x] = accumulators[ColumnIdx::C3][accumulators[ColumnIdx::C3].size() - 1];
        }
    }
}

/**
 * @brief IOTA
 *
 * XOR first hash limb with a precomputed constant.
 * We re-use the RHO_OUTPUT table to normalize after this operation
 * @tparam Composer
 * @param internal
 * @param round
 */
template <typename Composer> void keccak<Composer>::iota(keccak_state& internal, size_t round)
{
    const uint256_t base11_operand = SPARSE_RC[round];
    const field_ct xor_result = internal.state[0] + base11_operand;

    auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_RHO_OUTPUT, xor_result);
    internal.state[0] = accumulators[ColumnIdx::C2][0];
    internal.state_msb[0] = accumulators[ColumnIdx::C3][accumulators[ColumnIdx::C3].size() - 1];

    // No need to add constraints to compute twisted repr if this is the last round
    if (round != 23) {
        compute_twisted_state(internal);
    }
}

template <typename Composer> void keccak<Composer>::keccakf1600(keccak_state& internal)
{
    for (size_t i = 0; i < 24; ++i) {
        theta(internal);
        rho(internal);
        pi(internal);
        chi(internal);
        iota(internal, i);
    }
}

template <typename Composer>
void keccak<Composer>::sponge_absorb(keccak_state& internal,
                                     const std::vector<field_ct>& input_buffer,
                                     const std::vector<field_ct>& msb_buffer)
{
    const size_t l = input_buffer.size();

    // todo make these static class members
    const size_t num_blocks = l / (BLOCK_SIZE / 8);

    for (size_t i = 0; i < num_blocks; ++i) {
        if (i == 0) {
            for (size_t j = 0; j < LIMBS_PER_BLOCK; ++j) {
                internal.state[j] = input_buffer[j];
                internal.state_msb[j] = msb_buffer[j];
                // // TODO FORMAT_INPUT table should add lsb into column C3
                // constexpr uint256_t divisor = BASE.pow(63);
                // uint256_t native = uint256_t(internal.state[j].get_value()) / divisor;
                // internal.state_msb[j] = field_ct(witness_ct(internal.context, native));
            }
            for (size_t j = LIMBS_PER_BLOCK; j < 25; ++j) {
                internal.state[j] = witness_ct::create_constant_witness(internal.context, 0);
                internal.state_msb[j] = witness_ct::create_constant_witness(internal.context, 0);
            }
        } else {
            for (size_t j = 0; j < LIMBS_PER_BLOCK; ++j) {
                internal.state[j] += input_buffer[i * BLOCK_SIZE + j];
                auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_RHO_OUTPUT, internal.state[j]);
                internal.state[j] = accumulators[ColumnIdx::C2][0];
                internal.state_msb[j] = accumulators[ColumnIdx::C3][accumulators[ColumnIdx::C3].size() - 1];
            }
        }

        compute_twisted_state(internal);
        keccakf1600(internal);
    }
}

template <typename Composer> byte_array<Composer> keccak<Composer>::sponge_squeeze(keccak_state& internal)
{
    byte_array_ct result(internal.context);

    // Each hash limb represents a little-endian integer. Need to reverse bytes before we write into the output array
    for (size_t i = 0; i < 4; ++i) {
        field_ct output_limb = plookup_read::read_from_1_to_2_table(KECCAK_FORMAT_OUTPUT, internal.state[i]);
        byte_array_ct limb_bytes(output_limb, 8);
        byte_array_ct little_endian_limb_bytes(internal.context, 8);
        little_endian_limb_bytes.set_byte(0, limb_bytes[7]);
        little_endian_limb_bytes.set_byte(1, limb_bytes[6]);
        little_endian_limb_bytes.set_byte(2, limb_bytes[5]);
        little_endian_limb_bytes.set_byte(3, limb_bytes[4]);
        little_endian_limb_bytes.set_byte(4, limb_bytes[3]);
        little_endian_limb_bytes.set_byte(5, limb_bytes[2]);
        little_endian_limb_bytes.set_byte(6, limb_bytes[1]);
        little_endian_limb_bytes.set_byte(7, limb_bytes[0]);
        result.write(little_endian_limb_bytes);
    }
    return result;
}

template <typename Composer> stdlib::byte_array<Composer> keccak<Composer>::hash(byte_array_ct& input)
{
    auto ctx = input.get_context();

    ASSERT(ctx != nullptr);
    // TODO HANDLE CONSTANT INPUTS

    const size_t input_size = input.size();

    // copy input into buffer and pad
    const size_t blocks = input_size / BLOCK_SIZE;
    const size_t blocks_length = (BLOCK_SIZE * (blocks + 1));

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
    for (size_t i = 0; i < block_bytes.size(); i += 8) {
        std::array<field_ct, 8> temp;
        for (size_t j = 0; j < 8; ++j) {
            temp[j] = block_bytes[i + j];
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

    const size_t num_limbs = byte_size / WORD_SIZE;
    std::vector<field_ct> converted_buffer(num_limbs);
    std::vector<field_ct> msb_buffer(num_limbs);

    for (size_t i = 0; i < num_limbs; ++i) {
        field_ct sliced;
        if (i * WORD_SIZE + WORD_SIZE > byte_size) {
            const size_t slice_size = byte_size - (i * WORD_SIZE);
            const size_t byte_shift = (WORD_SIZE - slice_size) * 8;
            sliced = field_ct(block_bytes.slice(i * WORD_SIZE, slice_size));
            sliced = (sliced * (uint256_t(1) << byte_shift)).normalize();
        } else {
            sliced = field_ct(block_bytes.slice(i * WORD_SIZE, WORD_SIZE));
        }
        const auto accumulators = plookup_read::get_lookup_accumulators(KECCAK_FORMAT_INPUT, sliced);
        converted_buffer[i] = accumulators[ColumnIdx::C2][0];
        msb_buffer[i] = accumulators[ColumnIdx::C3][accumulators[ColumnIdx::C3].size() - 1];
    }

    sponge_absorb(internal, converted_buffer, msb_buffer);

    auto result = sponge_squeeze(internal);

    return result;
}

template class keccak<waffle::UltraComposer>;

} // namespace stdlib
} // namespace plonk