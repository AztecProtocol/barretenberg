#include "c_bind.hpp"
#include "./curves/bn254//scalar_multiplication/scalar_multiplication.hpp"
#include "./curves/bn254//scalar_multiplication/pippenger.hpp"

using namespace barretenberg;

extern "C" {

WASM_EXPORT void ecc_new_pippenger(void const* points, uint32_t const* num_points, void** out_ptr)
{
    *out_ptr = new scalar_multiplication::Pippenger((g1::affine_element*)points, ntohl(*num_points));
}

WASM_EXPORT void ecc_delete_pippenger(void const* pippenger)
{
    delete (scalar_multiplication::Pippenger*)(pippenger);
}

WASM_EXPORT void ecc_pippenger_unsafe(void const* pippenger_ptr,
                                      void const* scalars_ptr,
                                      uint32_t const* from_ptr,
                                      uint32_t const* range_ptr,
                                      affine_element::out_buf result_ptr)
{
    uint32_t from = ntohl(*from_ptr);
    uint32_t range = ntohl(*range_ptr);
    scalar_multiplication::pippenger_runtime_state state(range);
    auto pippenger = (scalar_multiplication::Pippenger*)(pippenger_ptr);
    auto result = pippenger->pippenger_unsafe((fr*)scalars_ptr, from, range);
    write(result_ptr, static_cast<g1::affine_element>(result));
}

WASM_EXPORT void ecc_g1_sum(void const* points_ptr, uint32_t const* num_points_ptr, affine_element::out_buf result_ptr)
{
    auto points = (g1::element*)(points_ptr);
    uint32_t num_points = ntohl(*num_points_ptr);
    g1::element initial;
    initial.self_set_infinity();
    auto result = std::accumulate(points, points + num_points, initial);
    write(result_ptr, static_cast<g1::affine_element>(result));
}

// TODO
// WASM_EXPORT void ecc_grumpkin__mul(uint8_t const* point_buf, uint8_t const* scalar_buf, uint8_t* result)
// {
//     auto point = from_buffer<grumpkin::g1::affine_element>(point_buf);
//     auto scalar = from_buffer<grumpkin::fr>(scalar_buf);
//     grumpkin::g1::affine_element r = point * scalar;
//     write(result, r);
// }

// //  multiplies a vector of points by a single scalar. Returns a vector of points (this is NOT a multi-exponentiation)
// WASM_EXPORT void ecc_grumpkin__batch_mul(uint8_t const* point_buf,
//                                          uint8_t const* scalar_buf,
//                                          uint32_t num_points,
//                                          uint8_t* result)
// {
//     std::vector<grumpkin::g1::affine_element> points;
//     points.reserve(num_points);
//     for (size_t i = 0; i < num_points; ++i) {
//         points.emplace_back(from_buffer<grumpkin::g1::affine_element>(point_buf + (i * 64)));
//     }
//     auto scalar = from_buffer<grumpkin::fr>(scalar_buf);
//     auto output = grumpkin::g1::element::batch_mul_with_endomorphism(points, scalar);
//     for (size_t i = 0; i < num_points; ++i) {
//         grumpkin::g1::affine_element r = output[i];
//         uint8_t* result_ptr = result + (i * 64);
//         write(result_ptr, r);
//     }
// }
}
