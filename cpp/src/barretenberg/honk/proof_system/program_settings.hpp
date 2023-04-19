#pragma once

#include <cstdint>

#include "../../transcript/transcript_wrappers.hpp"
#include "../../plonk/proof_system/types/prover_settings.hpp"
#include "barretenberg/honk/flavor/flavor.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"

namespace proof_system::honk {

// TODO(#221)(Luke/Cody): Shouldn't subclass plonk settings here. Also, define standard_settings for Honk prover.
class standard_verifier_settings : public plonk::standard_settings {
  public:
    using fr = barretenberg::fr;
    using Transcript = transcript::StandardTranscript;
    static constexpr size_t num_challenge_bytes = 16;
    static constexpr transcript::HashType hash_type = transcript::HashType::PedersenBlake3s;
    static constexpr size_t num_wires = proof_system::honk::flavor::Standard::num_wires;
    static constexpr size_t num_polys = proof_system::honk::flavor::Standard::NUM_ALL_ENTITIES;
};

} // namespace proof_system::honk
