#pragma once

#include <cstddef>
namespace proof_system::honk::sumcheck {

template <typename FF> struct RelationParameters {
    FF eta;
    FF beta;
    FF gamma;
    FF public_input_delta;
    FF lookup_grand_product_delta;
};
} // namespace proof_system::honk::sumcheck
