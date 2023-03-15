#pragma once

namespace honk::sumcheck {

template <typename FF> struct RelationParameters {
    FF beta;
    FF gamma;
    FF public_input_delta;
};
} // namespace honk::sumcheck
