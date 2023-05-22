#pragma once
#include "barretenberg/plonk/composer/standard_plonk_composer.hpp"
#include "barretenberg/honk/composer/standard_honk_composer.hpp"
#include "barretenberg/plonk/composer/standard_plonk_composer.hpp"
#include "barretenberg/plonk/composer/turbo_plonk_composer.hpp"
#include "barretenberg/plonk/composer/ultra_plonk_composer.hpp"

#define INSTANTIATE_STDLIB_TYPE(stdlib_type)                                                                           \
    template class stdlib_type<plonk::StandardPlonkComposer>;                                                          \
    template class stdlib_type<honk::StandardHonkComposer>;                                                            \
    template class stdlib_type<plonk::TurboPlonkComposer>;                                                             \
    template class stdlib_type<plonk::UltraPlonkComposer>;

#define INSTANTIATE_STDLIB_TYPE_VA(stdlib_type, ...)                                                                   \
    template class stdlib_type<plonk::StandardPlonkComposer, __VA_ARGS__>;                                             \
    template class stdlib_type<honk::StandardPlonkComposer, __VA_ARGS__>;                                              \
    template class stdlib_type<plonk::TurboPlonkComposer, __VA_ARGS__>;                                                \
    template class stdlib_type<plonk::UltraPlonkComposer, __VA_ARGS__>;

#define INSTANTIATE_STDLIB_BASIC_TYPE(stdlib_type)                                                                     \
    template class stdlib_type<plonk::StandardPlonkComposer>;                                                          \
    template class stdlib_type<honk::StandardPlonkComposer>;                                                           \
    template class stdlib_type<plonk::TurboPlonkComposer>;

#define INSTANTIATE_STDLIB_BASIC_TYPE_VA(stdlib_type, ...)                                                             \
    template class stdlib_type<honk::StandardHonkComposer, __VA_ARGS__>;                                               \
    template class stdlib_type<plonk::StandardPlonkComposer, __VA_ARGS__>;                                             \
    template class stdlib_type<plonk::TurboPlonkComposer, __VA_ARGS__>;

#define INSTANTIATE_STDLIB_ULTRA_TYPE(stdlib_type) template class stdlib_type<plonk::UltraPlonkComposer>;

#define INSTANTIATE_STDLIB_ULTRA_TYPE_VA(stdlib_type, ...)                                                             \
    template class stdlib_type<plonk::UltraPlonkComposer, __VA_ARGS__>;
