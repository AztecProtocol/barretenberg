#pragma once
#include <string>
#include "cbind.hpp"

/***
 * Do a roundtrip test encode/decode of an object.
 * @tparam T The object type.
 * @param object The object. Can be a default-initialized object.
 */
template<typename T>
std::pair<T, T> msgpack_roundtrip(const T &object) {
    T result;
    msgpack::sbuffer buffer;
    msgpack::pack(buffer, object);
    msgpack::unpack(buffer.data(), buffer.size()).get().convert(result);
    return {object, result};
}

inline std::pair<std::string, std::string> cbind_test_impl(auto cbind_func, auto... test_args) {
    auto expected_ret = func(test_args...);
    auto [input, input_len] = msgpack::encode_buffer(std::make_tuple(test_args...));
    uint8_t *output;
    size_t output_len;
    cbind_func(input, input_len, &output, &output_len);
    decltype(expected_ret) actual_ret;
    msgpack::decode(&actual_ret, output, output_len);
    aligned_free(output);
    return {msgpack::string_encode(actual_ret), msgpack::string_encode(expected_ret)};
}
