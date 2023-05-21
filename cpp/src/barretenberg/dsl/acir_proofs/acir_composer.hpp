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

    void init_proving_key(acir_format::acir_format&& constraint_system);

    std::vector<uint8_t> create_proof(std::vector<fr> const& witness);

    void init_verification_key();

    bool verify_proof(std::vector<uint8_t> const& proof);

    std::string get_solidity_verifier();
    size_t get_exact_circuit_size() { return exact_circuit_size_; };
    size_t get_total_circuit_size() { return total_circuit_size_; };

  private:
    std::shared_ptr<proof_system::ReferenceStringFactory> crs_factory_;
    acir_format::acir_format constraint_system_;
    size_t exact_circuit_size_;
    size_t total_circuit_size_;
    std::shared_ptr<proof_system::plonk::proving_key> proving_key_;
    std::shared_ptr<proof_system::plonk::verification_key> verification_key_;
};

} // namespace acir_proofs
