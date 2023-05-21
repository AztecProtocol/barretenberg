#pragma once

namespace proof_system::plonk {
class StandardPlonkComposer;
class TurboPlonkComposer;
class UltraPlonkComposer;

class StandardPlonkComposer;
} // namespace proof_system::plonk

namespace proof_system::honk {
class StandardHonkComposer;
} // namespace proof_system::honk

#define EXTERN_STDLIB_TYPE(stdlib_type)                                                                                \
    extern template class stdlib_type<plonk::StandardPlonkComposer>;                                                   \
    extern template class stdlib_type<honk::StandardHonkComposer>;                                                     \
    extern template class stdlib_type<plonk::StandardPlonkComposer>;                                                   \
    extern template class stdlib_type<plonk::TurboPlonkComposer>;                                                      \
    extern template class stdlib_type<plonk::UltraPlonkComposer>;

#define EXTERN_STDLIB_TYPE_VA(stdlib_type, ...)                                                                        \
    extern template class stdlib_type<plonk::StandardPlonkComposer, __VA_ARGS__>;                                      \
    extern template class stdlib_type<plonk::StandardPlonkComposer, __VA_ARGS__>;                                      \
    extern template class stdlib_type<plonk::TurboPlonkComposer, __VA_ARGS__>;                                         \
    extern template class stdlib_type<plonk::UltraPlonkComposer, __VA_ARGS__>;

#define EXTERN_STDLIB_BASIC_TYPE(stdlib_type)                                                                          \
    extern template class stdlib_type<plonk::StandardPlonkComposer>;                                                   \
    extern template class stdlib_type<plonk::StandardPlonkComposer>;                                                   \
    extern template class stdlib_type<plonk::TurboPlonkComposer>;

#define EXTERN_STDLIB_BASIC_TYPE_VA(stdlib_type, ...)                                                                  \
    extern template class stdlib_type<honk::StandardHonkComposer, __VA_ARGS__>;                                        \
    extern template class stdlib_type<plonk::StandardPlonkComposer, __VA_ARGS__>;                                      \
    extern template class stdlib_type<plonk::StandardPlonkComposer, __VA_ARGS__>;                                      \
    extern template class stdlib_type<plonk::TurboPlonkComposer, __VA_ARGS__>;

#define EXTERN_STDLIB_ULTRA_TYPE(stdlib_type) extern template class stdlib_type<plonk::UltraPlonkComposer>;

#define EXTERN_STDLIB_ULTRA_TYPE_VA(stdlib_type, ...)                                                                  \
    extern template class stdlib_type<plonk::UltraPlonkComposer, __VA_ARGS__>;
