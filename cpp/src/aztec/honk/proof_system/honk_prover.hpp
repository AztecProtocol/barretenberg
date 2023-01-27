#pragma once
#include "../../proof_system/proving_key/proving_key.hpp"
#include "../../plonk/proof_system/types/plonk_proof.hpp"
#include "../../plonk/proof_system/types/program_settings.hpp"

namespace honk {

template <typename settings> class HonkProver {

  public:
    // TODO(luke): update this appropriately to work with Honk Manifest
    HonkProver(std::shared_ptr<waffle::proving_key> input_key = nullptr,
               const transcript::Manifest& manifest = transcript::Manifest({}));

    void execute_preamble_round();
    void execute_first_round();
    void execute_second_round();
    void execute_third_round();
    void execute_fourth_round();
    void execute_fifth_round();
    void execute_sixth_round();

    void compute_wire_commitments();

    void compute_grand_product_polynomial();

    waffle::plonk_proof& export_proof();
    waffle::plonk_proof& construct_proof();

    size_t get_circuit_size() const { return n; }

    void flush_queued_work_items() { queue.flush_queue(); }

    waffle::work_queue::work_item_info get_queued_work_item_info() const { return queue.get_queued_work_item_info(); }

    size_t get_scalar_multiplication_size(const size_t work_item_number) const
    {
        return queue.get_scalar_multiplication_size(work_item_number);
    }

    // TODO(luke): Eventually get rid of this but leave it for convenience for now
    const size_t n;

    // No more widgets. The new 'relations' may be owned by Sumcheck rather than Prover...
    // std::vector<std::unique_ptr<ProverRandomWidget>> random_widgets;
    // std::vector<std::unique_ptr<widget::TransitionWidgetBase<barretenberg::fr>>> transition_widgets;

    // TODO(luke): maybe pointer instead?
    transcript::StandardTranscript transcript;

    // TODO(luke): Honk PoC will have equivalent members
    std::shared_ptr<waffle::proving_key> key;
    // std::unique_ptr<CommitmentScheme> commitment_scheme;

    // TODO(luke): Honk only needs a small portion of the functionality but may be fine to use existing work_queue
    waffle::work_queue queue;

    // This makes 'settings' accesible from HonkProver
    typedef settings settings_;

  private:
    waffle::plonk_proof proof;
};

// TODO(luke): need equivalent notion of settings for Honk
extern template class HonkProver<waffle::standard_settings>;

typedef HonkProver<waffle::standard_settings> Prover;

} // namespace honk
