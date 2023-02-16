#pragma once

namespace plonk {
class StandardComposer;
class TurboComposer;
class UltraComposer;
} // namespace plonk

namespace honk {
class StandardHonkComposer;
class StandardPlonkComposer;
} // namespace honk

#define EXTERN_STDLIB_TYPE(stdlib_type)                                                                                \
    extern template class stdlib_type<waffle::StandardComposer>;                                                       \
    extern template class stdlib_type<honk::StandardHonkComposer>;                                                     \
    extern template class stdlib_type<honk::StandardPlonkComposer>;                                                    \
    extern template class stdlib_type<waffle::TurboComposer>;                                                          \
    extern template class stdlib_type<waffle::UltraComposer>;

#define EXTERN_STDLIB_TYPE_VA(stdlib_type, ...)                                                                        \
    extern template class stdlib_type<waffle::StandardComposer, __VA_ARGS__>;                                          \
    extern template class stdlib_type<honk::StandardPlonkComposer, __VA_ARGS__>;                                       \
    extern template class stdlib_type<waffle::TurboComposer, __VA_ARGS__>;                                             \
    extern template class stdlib_type<waffle::UltraComposer, __VA_ARGS__>;

#define EXTERN_STDLIB_BASIC_TYPE(stdlib_type)                                                                          \
    extern template class stdlib_type<waffle::StandardComposer>;                                                       \
    extern template class stdlib_type<honk::StandardPlonkComposer>;                                                    \
    extern template class stdlib_type<waffle::TurboComposer>;

#define EXTERN_STDLIB_BASIC_TYPE_VA(stdlib_type, ...)                                                                  \
    extern template class stdlib_type<honk::StandardHonkComposer, __VA_ARGS__>;                                        \
    extern template class stdlib_type<honk::StandardPlonkComposer, __VA_ARGS__>;                                       \
    extern template class stdlib_type<waffle::StandardComposer, __VA_ARGS__>;                                          \
    extern template class stdlib_type<waffle::TurboComposer, __VA_ARGS__>;

#define EXTERN_STDLIB_ULTRA_TYPE(stdlib_type) extern template class stdlib_type<plonk::UltraComposer>;

#define EXTERN_STDLIB_ULTRA_TYPE_VA(stdlib_type, ...)                                                                  \
    extern template class stdlib_type<plonk::UltraComposer, __VA_ARGS__>;
