#pragma once
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include <string>
#include <cxxabi.h>

namespace msgpack {
/**
 * Converts C++ type information into a human-readable format.
 * This function leverages __cxa_demangle to demangle the name generated by typeid.
 * Special cases are handled for "basic_string" (returns "string") and "i" (returns "int").
 * Template specializations are truncated and only the base type name is returned
 * @tparam T the type.
 * @return the readable schema name.
 */
template <typename T> std::string schema_name(T const&)
{
    std::string result = abi::__cxa_demangle(typeid(T).name(), NULL, NULL, NULL);
    if (result.find("basic_string") != std::string::npos) {
        return "string";
    }
    if (result == "i") {
        return "int";
    }

    if (result.find('<') != size_t(-1)) {
        result = result.substr(0, result.find('<'));
    }
    if (result.rfind(':') != size_t(-1)) {
        result = result.substr(result.rfind(':') + 1, result.size());
    }
    return result;
}
} // namespace msgpack