#pragma once
#include "barretenberg/honk/pcs/shplonk/shplonk.hpp"
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/honk/pcs/gemini/gemini.hpp"
#include "barretenberg/honk/pcs/shplonk/shplonk_single.hpp"
#include "barretenberg/honk/pcs/kzg/kzg.hpp"
#include "barretenberg/honk/transcript/transcript.hpp"
#include "barretenberg/honk/sumcheck/sumcheck.hpp"
#include "barretenberg/honk/sumcheck/sumcheck_output.hpp"
#include "barretenberg/honk/proof_system/prover_library.hpp"
#include "barretenberg/honk/proof_system/work_queue.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"

namespace proof_system::honk {

// We won't compile this class with honk::flavor::Ultra, but we will like want to compile it (at least for testing)
// with a flavor that uses the curve Grumpkin, or a flavor that does/does not have zk, etc.
template <typename T> concept StandardFlavor = IsAnyOf<T, honk::flavor::Standard>;

template <StandardFlavor Flavor> class StandardProver_ {

    using FF = typename Flavor::FF;
    using PCSParams = typename Flavor::PCSParams;
    using ProvingKey = typename Flavor::ProvingKey;
    using Polynomial = typename Flavor::Polynomial;
    using ProverPolynomials = typename Flavor::ProverPolynomials;
    using CommitmentLabels = typename Flavor::CommitmentLabels;

  public:
    explicit StandardProver_(std::shared_ptr<ProvingKey> input_key = nullptr);

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

    std::shared_ptr<ProvingKey> key;

    // Container for spans of all polynomials required by the prover (i.e. all multivariates evaluated by Sumcheck).
    ProverPolynomials prover_polynomials;

    // TODO(Cody): Improve this, or at least make the lables static constexpr?
    // WORKTODO: Maybe this doesn't really need to be one of the flavor components? There are commitments not
    // corresponding to ProverPolynomials/entities/whatever.
    CommitmentLabels commitment_labels;

    // Container for d + 1 Fold polynomials produced by Gemini
    std::vector<Polynomial> fold_polynomials;

    Polynomial batched_quotient_Q; // batched quotient poly computed by Shplonk
    FF nu_challenge;               // needed in both Shplonk rounds

    Polynomial quotient_W;

    work_queue<PCSParams> queue;

    sumcheck::SumcheckOutput<Flavor> sumcheck_output;
    pcs::gemini::ProverOutput<PCSParams> gemini_output;
    pcs::shplonk::ProverOutput<PCSParams> shplonk_output;

    using Gemini = pcs::gemini::MultilinearReductionScheme<PCSParams>;
    using Shplonk = pcs::shplonk::SingleBatchOpeningScheme<PCSParams>;
    using KZG = pcs::kzg::UnivariateOpeningScheme<PCSParams>;

  private:
    plonk::proof proof;
};

extern template class StandardProver_<honk::flavor::Standard>;

using StandardProver = StandardProver_<honk::flavor::Standard>;
// using GrumpkinStandardProver = StandardProver_<honk::flavor::StandardGrumpkin>; // e.g.

} // namespace proof_system::honk
