#include "../bn254/fr.hpp"
#include "barretenberg/common/wasm_export.hpp"
#include <ostream>
#include <string>

using namespace bb;

// #pragma clang diagnostic ignored "-Wunused-parameter"

// WASM_EXPORT void foreign_call_print_sa(bool const* newline, uint256_t const* input, size_t num)
// {
//     std::string str;
//     str.reserve(num);
//     for (size_t i = 0; i < num; ++i) {
//         str += (char)input[i].data[0];
//     }
//     std::cerr << str;
//     if (*newline) {
//         std::cerr << std::endl;
//     } else {
//         std::cerr << std::flush;
//     }
// }

// WASM_EXPORT void foreign_call_print_ss(bool const* newline, uint256_t const* input)
// {
//     std::cerr << *input;
//     if (*newline) {
//         std::cerr << std::endl;
//     } else {
//         std::cerr << std::flush;
//     }
// }

WASM_EXPORT void print_u256(uint256_t const* input, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        if (input[i].get_bit(255)) {
            auto v = input[i];
            v.set_bit(255, false);
            info(bb::fr{ v.data[0], v.data[1], v.data[2], v.data[3] });
        } else {
            info(input[i]);
        }
    }
}

// WASM_EXPORT void bb_printf(char const* ptr, ...)
// {
//     std::string str;
//     for (int i = 0; ptr[i] != 0; i += 32) {
//         str += ptr[i];
//     }
//     info(str);

//     // va_list args;
//     // va_start(args, ptr);
//     // vprintf(format, args);
//     // va_end(args);
// }

// First encodes the memory slot to montgomery form if it's not already.
// Then returns the decoded field in montgomery form.
inline bb::fr bn254_fr_decode(bb::fr* f_)
{
    if (!f_->get_bit(255)) {
        f_->self_to_montgomery_form();
        f_->set_bit(255, true);
    }

    auto f = *f_;
    f.set_bit(255, false);
    return f;
}

WASM_EXPORT void bn254_fr_normalize(bb::fr* f)
{
    if (f->get_bit(255)) {
        f->set_bit(255, false);
        f->self_from_montgomery_form();
    }
}

WASM_EXPORT void bn254_fr_add(bb::fr* lhs_, bb::fr* rhs_, bb::fr* result)
{
    auto lhs = bn254_fr_decode(lhs_);
    auto rhs = bn254_fr_decode(rhs_);
    *result = lhs + rhs;
    result->set_bit(255, true);
}

WASM_EXPORT void bn254_fr_sub(bb::fr* lhs_, bb::fr* rhs_, bb::fr* result)
{
    auto lhs = bn254_fr_decode(lhs_);
    auto rhs = bn254_fr_decode(rhs_);
    *result = lhs - rhs;
    result->set_bit(255, true);
}

WASM_EXPORT void bn254_fr_mul(bb::fr* lhs_, bb::fr* rhs_, bb::fr* result)
{
    auto lhs = bn254_fr_decode(lhs_);
    auto rhs = bn254_fr_decode(rhs_);
    *result = lhs * rhs;
    result->set_bit(255, true);
}

WASM_EXPORT void bn254_fr_div(bb::fr* lhs_, bb::fr* rhs_, bb::fr* result)
{
    auto lhs = bn254_fr_decode(lhs_);
    auto rhs = bn254_fr_decode(rhs_);
    *result = lhs / rhs;
    result->set_bit(255, true);
}

WASM_EXPORT void bn254_fr_eq(bb::fr* lhs_, bb::fr* rhs_, uint256_t* result)
{
    auto lhs = bn254_fr_decode(lhs_);
    auto rhs = bn254_fr_decode(rhs_);
    *result = lhs == rhs;
}

WASM_EXPORT void bn254_fr_lt(bb::fr* lhs_, bb::fr* rhs_, uint256_t* result)
{
    // Not sure why comparing field elements gives wrong result, convert to uint256.
    uint256_t lhs = bn254_fr_decode(lhs_);
    uint256_t rhs = bn254_fr_decode(rhs_);
    *result = lhs < rhs;
}

WASM_EXPORT void bn254_fr_leq(bb::fr* lhs_, bb::fr* rhs_, uint256_t* result)
{
    uint256_t lhs = bn254_fr_decode(lhs_);
    uint256_t rhs = bn254_fr_decode(rhs_);
    *result = (lhs < rhs) || (lhs == rhs);
}

WASM_EXPORT void bn254_fr_sqrt(uint8_t const* input, uint8_t* result)
{
    using serialize::write;
    auto input_fr = from_buffer<bb::fr>(input);
    auto [is_sqr, root] = input_fr.sqrt();

    uint8_t* is_sqrt_result_ptr = result;
    uint8_t* root_result_ptr = result + 1;

    write(is_sqrt_result_ptr, is_sqr);
    write(root_result_ptr, root);
}

WASM_EXPORT void to_radix(uint256_t const* input_, uint256_t* output, uint64_t size, uint64_t radix_)
{
    uint256_t input = *input_;
    if (input.get_bit(255)) {
        input.set_bit(255, false);
        input = bb::fr(input.data[0], input.data[1], input.data[2], input.data[3]);
    }
    uint256_t radix = radix_;
    for (size_t i = 0; i < size; ++i) {
        output[i] = input % radix;
        input /= radix;
    }
}
// NOLINTEND(cert-dcl37-c, cert-dcl51-cpp, bugprone-reserved-identifier)