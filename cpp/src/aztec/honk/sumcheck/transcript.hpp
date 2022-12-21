#pragma once

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace honk {
template <class Fr> class Transcript {
  public:
    template <class... Frs> void add(Frs... field_elements){}; // TODO(Cody): implementation
    Fr get_challenge() { return Fr::random_element(); };
    Fr get_challenge_equals_one() { return Fr::one(); };
    // Univariate get_sumcheck_round_univariate(size_t) // NOLINT(readability-named-parameter)
    // {
    //     Univariate result;
    //     return result;
    // };
};
}; // namespace honk
