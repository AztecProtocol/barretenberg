#pragma once

#include "../../ecc/curves/bn254/fr.hpp"

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace honk {
class Transcript {
    using Fr =
        barretenberg::fr; // TODO(luke): de-templatizing this class for now since StandardTranscript is not templatized
  public:
    template <class... Frs> void add(Frs... field_elements){}; // TODO(Cody): implementation
    Fr get_mock_challenge() { return Fr::random_element(); };
};
}; // namespace honk
