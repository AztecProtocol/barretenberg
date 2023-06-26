#pragma once
#include <cstdint>
#include <concepts>

namespace proof_system {
// TODO(#392) This enum has been overloaded. Perhaps its different responsibilities have been split up.
enum ComposerType { STANDARD, TURBO, PLOOKUP };
enum class CircuitType : uint32_t { UNDEFINED, STANDARD, TURBO, ULTRA };

template <typename T, typename... U> concept IsAnyOf = (std::same_as<T, U> || ...);
} // namespace proof_system