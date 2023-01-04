#include "stdlib/testing/testing.hpp"

namespace test_stdlib_byte_array {
using namespace barretenberg;
using namespace plonk;

template <typename Composer> class stdlib_byte_array : public plonk::stdlib::test::StdlibTest<Composer> {};

TYPED_TEST_SUITE(stdlib_byte_array, plonk::stdlib::test::ComposerTypes);

TYPED_STDLIB_TEST(stdlib_byte_array, test_reverse)
{
    Composer composer;
    std::vector<uint8_t> expected = { 0x04, 0x03, 0x02, 0x01 };
    byte_array_ct arr(&composer, std::vector<uint8_t>{ 0x01, 0x02, 0x03, 0x04 });

    EXPECT_EQ(arr.size(), 4UL);
    EXPECT_EQ(arr.reverse().get_value(), expected);
}

TYPED_STDLIB_TEST(stdlib_byte_array, test_string_constructor)
{
    Composer composer;
    std::string a = "ascii";
    byte_array_ct arr(&composer, a);
    EXPECT_EQ(arr.get_string(), a);
}

TYPED_STDLIB_TEST(stdlib_byte_array, test_ostream_operator)
{
    Composer composer;
    std::string a = "\1\2\3a";
    byte_array_ct arr(&composer, a);
    std::ostringstream os;
    os << arr;
    EXPECT_EQ(os.str(), "[ 01 02 03 61 ]");
}

TYPED_STDLIB_TEST(stdlib_byte_array, test_byte_array_input_output_consistency)
{
    Composer composer;
    fr a_expected = fr::random_element(this->engine);
    fr b_expected = fr::random_element(this->engine);

    field_ct a = witness_ct(&composer, a_expected);
    field_ct b = witness_ct(&composer, b_expected);

    byte_array_ct arr(&composer);

    arr.write(static_cast<byte_array_ct>(a));
    arr.write(static_cast<byte_array_ct>(b));

    EXPECT_EQ(arr.size(), 64UL);

    field_ct a_result(arr.slice(0, 32));
    field_ct b_result(arr.slice(32));

    EXPECT_EQ(a_result.get_value(), a_expected);
    EXPECT_EQ(b_result.get_value(), b_expected);

    EXPECT_TRUE(this->circuit_verifies(composer));
}

TYPED_STDLIB_TEST(stdlib_byte_array, get_bit)
{
    Composer composer;
    byte_array_ct arr(&composer, std::vector<uint8_t>{ 0x01, 0x02, 0x03, 0x04 });

    EXPECT_EQ(arr.get_bit(0).get_value(), false);
    EXPECT_EQ(arr.get_bit(1).get_value(), false);
    EXPECT_EQ(arr.get_bit(2).get_value(), true);
    EXPECT_EQ(arr.get_bit(3).get_value(), false);
    EXPECT_EQ(arr.get_bit(4).get_value(), false);
    EXPECT_EQ(arr.get_bit(5).get_value(), false);
    EXPECT_EQ(arr.get_bit(6).get_value(), false);
    EXPECT_EQ(arr.get_bit(7).get_value(), false);

    EXPECT_EQ(arr.get_bit(8).get_value(), true);
    EXPECT_EQ(arr.get_bit(9).get_value(), true);
    EXPECT_EQ(arr.get_bit(10).get_value(), false);
    EXPECT_EQ(arr.get_bit(11).get_value(), false);
    EXPECT_EQ(arr.get_bit(12).get_value(), false);
    EXPECT_EQ(arr.get_bit(13).get_value(), false);
    EXPECT_EQ(arr.get_bit(14).get_value(), false);
    EXPECT_EQ(arr.get_bit(15).get_value(), false);

    EXPECT_EQ(arr.size(), 4UL);

    EXPECT_TRUE(this->circuit_verifies(composer));
}

TYPED_STDLIB_TEST(stdlib_byte_array, set_bit)
{
    Composer composer;
    byte_array_ct arr(&composer, std::vector<uint8_t>{ 0x01, 0x02, 0x03, 0x04 });

    arr.set_bit(16, bool_ct(witness_ct(&composer, true)));
    arr.set_bit(18, bool_ct(witness_ct(&composer, true)));
    arr.set_bit(24, bool_ct(witness_ct(&composer, false)));
    arr.set_bit(0, bool_ct(witness_ct(&composer, true)));

    const auto out = arr.get_value();
    EXPECT_EQ(out[0], uint8_t(0));
    EXPECT_EQ(out[1], uint8_t(7));
    EXPECT_EQ(out[3], uint8_t(5));

    EXPECT_TRUE(this->circuit_verifies(composer));
}

TYPED_STDLIB_TEST(stdlib_byte_array, safe_uint_constructor)
{
    Composer composer;
    std::vector<std::pair<uint256_t, uint8_t>> expected_sizes = {
        { 0, 0 },
        { 1, 1 },
        { 2, 1 },
        { 255, 1 },
        { 256, 2 },
        { (1 << 16) - 1, 2 },
        { (1 << 16), 3 },
        { uint256_t{ UINT64_MAX, UINT64_MAX, UINT64_MAX, 0 }, 24 },
        { uint256_t{ UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX }, 32 },
    };

    for (const auto& [value, expected_bytes] : expected_sizes) {
        if (value < fr::modulus) {
            fr value_fr(value);
            // create a witness and not a constant
            witness_ct w_value_fr(&composer, value_fr);

            // we explicitly set the number of bytes required
            byte_array_ct b_value_fr(w_value_fr, expected_bytes);
            auto values = b_value_fr.get_value();
            EXPECT_EQ(values.size(), expected_bytes);

            // For the value 0 or 1, msb is 0.
            // for 0, we actually want the number of bits to be zero as well.
            // but for 1, we need one bit.
            // so we set num_bits in a way that handles this
            const auto value_msb = value.get_msb();
            const auto num_bits = (value == 0) ? 0 : value_msb + 1;

            // create a safe uint with the right number of bits
            suint_ct s_value_fr(w_value_fr, num_bits);
            byte_array_ct bs_value_fr(s_value_fr);
            values = bs_value_fr.get_value();
            EXPECT_EQ(values.size(), expected_bytes);
        }
        suint_ct s_value(value);
        byte_array_ct b_value(s_value);
        EXPECT_EQ(b_value.get_value().size(), expected_bytes);
    }

    // Test writing safe uint to the array
    field_ct elt(witness_ct(&composer, 0x7f6f5f4f00010203UL));
    suint_ct safe(elt, 63);
    // safe.value is a 63 bit uint256_t, so we serialize to a 8-byte array
    std::string expected = { 0x7f, 0x6f, 0x5f, 0x4f, 0x00, 0x01, 0x02, 0x03 };

    byte_array_ct arr(&composer);
    arr.write(static_cast<byte_array_ct>(safe));
    EXPECT_EQ(arr.get_string(), expected);

    EXPECT_TRUE(this->circuit_verifies(composer));
}

} // namespace test_stdlib_byte_array