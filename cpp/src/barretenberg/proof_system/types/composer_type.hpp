#pragma once
#include <concepts>

namespace proof_system {
// TODO(#392) This enum has been overloaded. Perhaps its different responsibilities have been split up.
enum ComposerType { STANDARD, TURBO, PLOOKUP, UNDEFINED };

template <typename T, typename... U> concept IsAnyOf = (std::same_as<T, U> || ...);
} // namespace proof_system