#pragma once
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/honk/pcs/shplonk/shplonk.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/honk/flavor/flavor.hpp"
#include <array>
#include "barretenberg/plonk/proof_system/proving_key/proving_key.hpp"
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/plonk/proof_system/types/program_settings.hpp"
#include "barretenberg/honk/pcs/gemini/gemini.hpp"
#include "barretenberg/honk/pcs/shplonk/shplonk_single.hpp"
#include "barretenberg/honk/pcs/kzg/kzg.hpp"
#include "barretenberg/honk/transcript/transcript.hpp"
#include "barretenberg/honk/sumcheck/sumcheck.hpp"
#include "barretenberg/honk/sumcheck/sumcheck_output.hpp"
#include <span>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>
#include <string>
#include "barretenberg/honk/pcs/claim.hpp"
#include "barretenberg/honk/proof_system/prover_library.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"

namespace proof_system::honk {

// TODO(luke): The naming here is awkward. The Standard Honk prover is called "Prover" and aliased as StandardProver. To
// be consistent with that convention outside of the prover class itself, I've called this class UltraHonkProver and use
// the alias UltraProver externally. Resolve.
template <typename Flavor> class UltraHonkProver {

    using FF = typename Flavor::FF;
    using PCSParams = typename Flavor::PCSParams;
    using ProvingKey = typename Flavor::ProvingKey;
    using Polynomial = typename Flavor::Polynomial;

  public:
    UltraHonkProver(std::vector<barretenberg::polynomial>&& wire_polys,
                    std::shared_ptr<ProvingKey> input_key = nullptr);

    plonk::proof& export_proof();
    plonk::proof& construct_proof();

    ProverTranscript<FF> transcript;

    std::vector<barretenberg::polynomial> wire_polynomials;

    std::shared_ptr<ProvingKey> key;

    work_queue<pcs::kzg::Params> queue;

  private:
    plonk::proof proof;
};

extern template class UltraHonkProver<honk::flavor::Ultra>;

using UltraProver = UltraHonkProver<honk::flavor::Ultra>;

} // namespace proof_system::honk
