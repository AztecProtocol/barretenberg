#pragma once
#include "barretenberg/honk/proof_system/work_queue.hpp"
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/honk/transcript/transcript.hpp"
#include "barretenberg/honk/flavor/ultra.hpp"
#include "barretenberg/honk/sumcheck/relations/relation.hpp"
#include "barretenberg/honk/sumcheck/sumcheck_output.hpp"

namespace proof_system::honk {

// We won't compile this class with honk::flavor::Ultra, but we will like want to compile it (at least for testing)
// with a flavor that uses the curve Grumpkin, or a flavor that does/does not have zk, etc.
template <typename T> concept UltraFlavor = IsAnyOf<T, honk::flavor::Ultra>;
template <UltraFlavor Flavor> class UltraProver_ {

    using FF = typename Flavor::FF;
    using PCSParams = typename Flavor::PCSParams;
    using ProvingKey = typename Flavor::ProvingKey;
    using Polynomial = typename Flavor::Polynomial;
    using ProverPolynomials = typename Flavor::ProverPolynomials;
    using CommitmentLabels = typename Flavor::CommitmentLabels;

  public:
    explicit UltraProver_(std::shared_ptr<ProvingKey> input_key = nullptr);

    void execute_preamble_round();
    void execute_wire_commitments_round();
    void execute_sorted_list_accumulator_round();
    void execute_grand_product_computation_round();
    void execute_relation_check_rounds();
    void execute_univariatization_round();
    void execute_pcs_evaluation_round();
    void execute_shplonk_batched_quotient_round();
    void execute_shplonk_partial_evaluation_round();
    void execute_kzg_round();

    void compute_wire_commitments();

    plonk::proof& export_proof();
    plonk::proof& construct_proof();

    ProverTranscript<FF> transcript;

    std::vector<FF> public_inputs;

    sumcheck::RelationParameters<FF> relation_parameters;

    std::shared_ptr<ProvingKey> key;

    // Container for spans of all polynomials required by the prover (i.e. all multivariates evaluated by Sumcheck).
    ProverPolynomials prover_polynomials;

    CommitmentLabels commitment_labels;

    work_queue<pcs::kzg::Params> queue;

    sumcheck::SumcheckOutput<Flavor> sumcheck_output;

  private:
    plonk::proof proof;
};

extern template class UltraProver_<honk::flavor::Ultra>;

using UltraProver = UltraProver_<honk::flavor::Ultra>;

} // namespace proof_system::honk
