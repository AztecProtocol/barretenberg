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
#include "barretenberg/honk/proof_system/work_queue.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"

namespace proof_system::honk {

template <typename Flavor> class Prover {

    using FF = typename Flavor::FF;
    using PCSParams = typename Flavor::PCSParams;
    using ProvingKey = typename Flavor::ProvingKey;
    using Polynomial = typename Flavor::Polynomial;

  public:
    Prover(std::vector<Polynomial>&& wire_polys, std::shared_ptr<ProvingKey> input_key = nullptr);

    void execute_preamble_round();
    void execute_wire_commitments_round();
    void execute_tables_round();
    void execute_grand_product_computation_round();
    void execute_relation_check_rounds();
    void execute_univariatization_round();
    void execute_pcs_evaluation_round();
    void execute_shplonk_batched_quotient_round();
    void execute_shplonk_partial_evaluation_round();
    void execute_kzg_round();

    void compute_wire_commitments();

    void construct_prover_polynomials();

    plonk::proof& export_proof();
    plonk::proof& construct_proof();

    ProverTranscript<FF> transcript;

    std::vector<FF> public_inputs;

    sumcheck::RelationParameters<FF> relation_parameters;

    std::vector<Polynomial> wire_polynomials;
    Polynomial z_permutation;

    std::shared_ptr<ProvingKey> key;

    // Container for spans of all polynomials required by the prover (i.e. all multivariates evaluated by Sumcheck).
    std::array<std::span<FF>, Flavor::NUM_ALL_ENTITIES> prover_polynomials; // WORKTODO: to be replaced

    // Container for d + 1 Fold polynomials produced by Gemini
    std::vector<Polynomial> fold_polynomials;

    Polynomial batched_quotient_Q; // batched quotient poly computed by Shplonk
    FF nu_challenge;               // needed in both Shplonk rounds

    Polynomial quotient_W;

    work_queue<PCSParams> queue;

    sumcheck::SumcheckOutput<FF> sumcheck_output;
    pcs::gemini::ProverOutput<PCSParams> gemini_output;
    pcs::shplonk::ProverOutput<PCSParams> shplonk_output;

    using Gemini = pcs::gemini::MultilinearReductionScheme<PCSParams>;
    using Shplonk = pcs::shplonk::SingleBatchOpeningScheme<PCSParams>;
    using KZG = pcs::kzg::UnivariateOpeningScheme<PCSParams>;

  private:
    plonk::proof proof;
};

extern template class Prover<honk::flavor::Standard>;

using StandardProver = Prover<honk::flavor::Standard>;

} // namespace proof_system::honk
