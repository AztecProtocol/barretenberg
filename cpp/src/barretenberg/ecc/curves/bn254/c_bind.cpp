#include "../bn254/fr.hpp"
#include "barretenberg/common/wasm_export.hpp"
#include <string>

using namespace bb;

#pragma clang diagnostic ignored "-Wunused-parameter"
WASM_EXPORT void to_radix(uint256_t const* input_, uint256_t* output, uint64_t size, uint64_t radix_)
{
    uint256_t input = *input_;
    uint256_t radix = radix_;
    for (size_t i = 0; i < size; ++i) {
        output[i] = input % radix;
        input /= 2;
    }
}

WASM_EXPORT void bb_printf(char const* ptr, ...)
{
    std::string str;
    for (int i = 0; ptr[i] != 0; i += 32) {
        str += ptr[i];
    }
    info(str);

    // va_list args;
    // va_start(args, ptr);
    // vprintf(format, args);
    // va_end(args);
}

WASM_EXPORT void bn254_fr_add(uint64_t const* lhs, uint64_t const* rhs, uint64_t* result)
{
    auto lhs_fr = bb::fr(lhs[0], lhs[1], lhs[2], lhs[3]);
    lhs_fr.self_to_montgomery_form();
    auto rhs_fr = bb::fr(rhs[0], rhs[1], rhs[2], rhs[3]);
    rhs_fr.self_to_montgomery_form();
    auto r_fr = lhs_fr + rhs_fr;
    r_fr.self_from_montgomery_form();
    result[0] = r_fr.data[0];
    result[1] = r_fr.data[1];
    result[2] = r_fr.data[2];
    result[3] = r_fr.data[3];
}

WASM_EXPORT void bn254_fr_sub(uint64_t const* lhs, uint64_t const* rhs, uint64_t* result)
{
    auto lhs_fr = bb::fr(lhs[0], lhs[1], lhs[2], lhs[3]);
    lhs_fr.self_to_montgomery_form();
    auto rhs_fr = bb::fr(rhs[0], rhs[1], rhs[2], rhs[3]);
    rhs_fr.self_to_montgomery_form();
    auto r_fr = lhs_fr - rhs_fr;
    r_fr.self_from_montgomery_form();
    result[0] = r_fr.data[0];
    result[1] = r_fr.data[1];
    result[2] = r_fr.data[2];
    result[3] = r_fr.data[3];
}

WASM_EXPORT void bn254_fr_mul(uint64_t const* lhs, uint64_t const* rhs, uint64_t* result)
{
    auto lhs_fr = bb::fr(lhs[0], lhs[1], lhs[2], lhs[3]);
    lhs_fr.self_to_montgomery_form();
    // info("lhs_fr: ", lhs_fr);
    auto rhs_fr = bb::fr(rhs[0], rhs[1], rhs[2], rhs[3]);
    rhs_fr.self_to_montgomery_form();
    // info("rhs_fr: ", rhs_fr);
    auto r_fr = lhs_fr * rhs_fr;
    // info("r_fr: ", r_fr);
    r_fr.self_from_montgomery_form();
    result[0] = r_fr.data[0];
    result[1] = r_fr.data[1];
    result[2] = r_fr.data[2];
    result[3] = r_fr.data[3];
}

WASM_EXPORT bool bn254_fr_eql(uint64_t const* lhs, uint64_t const* rhs)
{
    // Prob don't need montgomery here.
    // Assuming inputs are already normalised, prob don't even need to make this call?
    auto lhs_fr = bb::fr(lhs[0], lhs[1], lhs[2], lhs[3]);
    lhs_fr.to_montgomery_form();
    auto rhs_fr = bb::fr(rhs[0], rhs[1], rhs[2], rhs[3]);
    rhs_fr.to_montgomery_form();
    return lhs_fr == rhs_fr;
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

// NOLINTEND(cert-dcl37-c, cert-dcl51-cpp, bugprone-reserved-identifier)