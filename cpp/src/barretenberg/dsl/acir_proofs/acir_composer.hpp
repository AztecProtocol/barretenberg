#include <cstdint>
#include <cstddef>
#include <memory>
#include <barretenberg/plonk/proof_system/proving_key/proving_key.hpp>
#include <barretenberg/plonk/proof_system/verification_key/verification_key.hpp>
#include <barretenberg/dsl/acir_format/acir_format.hpp>

namespace acir_proofs {

class AcirComposer {
  public:
    AcirComposer(std::shared_ptr<proof_system::ReferenceStringFactory> const& crs_factory);

    void init_proving_key(acir_format::acir_format& constraint_system, size_t size_hint = 0);

    std::vector<uint8_t> create_proof(acir_format::acir_format& constraint_system,
                                      acir_format::WitnessVector& witness,
                                      bool is_recursive);

    std::shared_ptr<proof_system::plonk::verification_key> init_verification_key();

    bool verify_proof(std::vector<uint8_t> const& proof, bool is_recursive);

    std::string get_solidity_verifier();
    size_t get_exact_circuit_size() { return exact_circuit_size_; };
    size_t get_total_circuit_size() { return total_circuit_size_; };

    std::vector<barretenberg::fr> serialize_proof_into_fields(std::vector<uint8_t> const& proof,
                                                              size_t num_inner_public_inputs);

    std::vector<barretenberg::fr> serialize_verification_key_into_fields();

    // std::vector<barretenberg::fr> verify_recursive_proof(std::vector<barretenberg::fr> const& proof,
    //                                                  std::vector<barretenberg::fr> const& verification_key,
    //                                                  uint32_t num_public_inputs,
    //                                                  std::vector<barretenberg::fr> input_aggregation_object)

  private:
    std::shared_ptr<proof_system::ReferenceStringFactory> crs_factory_;
    acir_format::Composer composer_;
    size_t exact_circuit_size_;
    size_t total_circuit_size_;
    size_t circuit_subgroup_size_;
    std::shared_ptr<proof_system::plonk::proving_key> proving_key_;
    std::shared_ptr<proof_system::plonk::verification_key> verification_key_;
};

} // namespace acir_proofs
