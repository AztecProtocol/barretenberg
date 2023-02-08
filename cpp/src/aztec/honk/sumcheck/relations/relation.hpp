#pragma once

namespace honk::sumcheck {

template <typename Fr> class Relation {}; // TODO(Cody): Use or eventually remove.

template <typename FF> struct RelationParameters {
    FF zeta;
    FF alpha;
    FF beta;
    FF gamma;
    FF public_input_delta;
    FF subgroup_size;
};
} // namespace honk::sumcheck
