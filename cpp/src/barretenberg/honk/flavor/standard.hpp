#pragma once
#include <array>
#include <concepts>
#include <span>
#include <string>
#include <type_traits>
#include <vector>
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/honk/sumcheck/polynomials/univariate.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/honk/transcript/transcript.hpp"
#include "barretenberg/plonk/proof_system/proving_key/proving_key.hpp"
#include "barretenberg/polynomials/evaluation_domain.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/proof_system/circuit_constructors/standard_circuit_constructor.hpp"
#include "barretenberg/srs/reference_string/reference_string.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"

namespace proof_system::honk::flavor {

class Standard {
  public:
    using CircuitConstructor = StandardCircuitConstructor;
    using FF = barretenberg::fr;
    using Polynomial = barretenberg::Polynomial<FF>;
    using PolynomialHandle = std::span<FF>;
    using G1 = barretenberg::g1;
    using GroupElement = G1::element;
    using Commitment = G1::affine_element;
    using CommitmentHandle = G1::affine_element;
    using PCSParams = pcs::kzg::Params;

    static constexpr size_t num_wires = CircuitConstructor::num_wires;
    static constexpr size_t NUM_ALL_ENTITIES = 18;
    static constexpr size_t NUM_PRECOMPUTED_ENTITIES = 13;
    // The total number of witness entities not including shifts.
    static constexpr size_t NUM_WITNESS_ENTITIES = 4;

  private:
    // TODO(Cody): Made this public derivation so that I could populate selector
    // polys from circuit constructor.
    template <typename DataType, typename HandleType>
    class PrecomputedEntities : public PrecomputedEntities_<DataType, HandleType, NUM_PRECOMPUTED_ENTITIES> {
      public:
        DataType& q_m = std::get<0>(this->_data);
        DataType& q_l = std::get<1>(this->_data);
        DataType& q_r = std::get<2>(this->_data);
        DataType& q_o = std::get<3>(this->_data);
        DataType& q_c = std::get<4>(this->_data);
        DataType& sigma_1 = std::get<5>(this->_data);
        DataType& sigma_2 = std::get<6>(this->_data);
        DataType& sigma_3 = std::get<7>(this->_data);
        DataType& id_1 = std::get<8>(this->_data);
        DataType& id_2 = std::get<9>(this->_data);
        DataType& id_3 = std::get<10>(this->_data);
        DataType& lagrange_first = std::get<11>(this->_data);
        DataType& lagrange_last = std::get<12>(this->_data); // = LAGRANGE_N-1 whithout ZK, but can be less

        std::vector<HandleType> get_selectors() override { return { q_m, q_l, q_r, q_o, q_c }; };
        std::vector<HandleType> get_sigma_polynomials() override { return { sigma_1, sigma_2, sigma_3 }; };
        std::vector<HandleType> get_id_polynomials() override { return { id_1, id_2, id_3 }; };
    };

    /**
     * @brief Container for all witness polynomials used/constructed by the prover.
     * @details Shifts are not included here since they do not occupy their own memory
     *
     * @tparam T
     * @tparam HandleType
     */
    template <typename DataType, typename HandleType>
    class WitnessEntities : public Entities_<DataType, HandleType, NUM_WITNESS_ENTITIES> {
      public:
        DataType& w_l = std::get<0>(this->_data);
        DataType& w_r = std::get<1>(this->_data);
        DataType& w_o = std::get<2>(this->_data);
        DataType& z_perm = std::get<3>(this->_data);

        std::vector<HandleType> get_wires() { return { w_l, w_r, w_o }; };
    };

    template <typename DataType, typename HandleType>
    class AllEntities : public AllEntities_<DataType, HandleType, NUM_ALL_ENTITIES> {
      public:
        DataType& q_c = std::get<0>(this->_data);
        DataType& q_l = std::get<1>(this->_data);
        DataType& q_r = std::get<2>(this->_data);
        DataType& q_o = std::get<3>(this->_data);
        DataType& q_m = std::get<4>(this->_data);
        DataType& sigma_1 = std::get<5>(this->_data);
        DataType& sigma_2 = std::get<6>(this->_data);
        DataType& sigma_3 = std::get<7>(this->_data);
        DataType& id_1 = std::get<8>(this->_data);
        DataType& id_2 = std::get<9>(this->_data);
        DataType& id_3 = std::get<10>(this->_data);
        DataType& lagrange_first = std::get<11>(this->_data);
        DataType& lagrange_last = std::get<12>(this->_data);
        DataType& w_l = std::get<13>(this->_data);
        DataType& w_r = std::get<14>(this->_data);
        DataType& w_o = std::get<15>(this->_data);
        DataType& z_perm = std::get<16>(this->_data);
        DataType& z_perm_shift = std::get<17>(this->_data);

        std::vector<HandleType> get_wires() override { return { w_l, w_r, w_o }; };

        std::vector<HandleType> get_unshifted() override
        { // ...z_perm_shift is in here?
            return { q_c,           q_l, q_r, q_o, q_m,   sigma_1, sigma_2, sigma_3, id_1, id_2, id_3, lagrange_first,
                     lagrange_last, w_l, w_r, w_o, z_perm };
        };

        std::vector<HandleType> get_to_be_shifted() override { return { z_perm }; };

        std::vector<HandleType> get_shifted() override { return { z_perm_shift }; };

        AllEntities() = default;

        AllEntities(const AllEntities& other)
            : AllEntities_<DataType, HandleType, NUM_ALL_ENTITIES>(other){};

        AllEntities(AllEntities&& other)
            : AllEntities_<DataType, HandleType, NUM_ALL_ENTITIES>(other){};

        AllEntities& operator=(const AllEntities& other)
        {
            if (this == &other) {
                return *this;
            }
            AllEntities_<DataType, HandleType, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        AllEntities& operator=(AllEntities&& other)
        {
            AllEntities_<DataType, HandleType, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        ~AllEntities() = default;
    };

  public:
    class ProvingKey : public ProvingKey_<PrecomputedEntities<Polynomial, PolynomialHandle>, FF> {
      public:
        WitnessEntities<Polynomial, PolynomialHandle> _witness_data;

        Polynomial& w_l = _witness_data.w_l;
        Polynomial& w_r = _witness_data.w_r;
        Polynomial& w_o = _witness_data.w_o;
        Polynomial& z_perm = _witness_data.z_perm;

        ProvingKey() = default;
        ProvingKey(const size_t circuit_size,
                   const size_t num_public_inputs,
                   std::shared_ptr<ProverReferenceString> const& crs,
                   ComposerType composer_type)
        {
            this->crs = crs;
            this->evaluation_domain = EvaluationDomain<FF>(circuit_size, circuit_size);

            this->circuit_size = circuit_size;
            this->log_circuit_size = numeric::get_msb(circuit_size);
            this->num_public_inputs = num_public_inputs;
            this->composer_type = composer_type;
            // Allocate memory for precomputed polynomials
            for (auto& poly : this->_data) {
                poly = Polynomial(circuit_size);
            }
            // Allocate memory for witness polynomials
            for (auto& poly : this->_witness_data._data) {
                poly = Polynomial(circuit_size);
            }
        };

        std::vector<PolynomialHandle> get_wires() { return _witness_data.get_wires(); };
    };

    // WORKTODO: verifiercircuitdata
    class VerificationKey : public VerificationKey_<PrecomputedEntities<Commitment, CommitmentHandle>> {
      public:
        VerificationKey() = default;
        VerificationKey(const size_t circuit_size,
                        const size_t num_public_inputs,
                        std::shared_ptr<VerifierReferenceString> const& vrs,
                        ComposerType composer_type)
        {
            this->circuit_size = circuit_size;
            this->log_circuit_size = numeric::get_msb(circuit_size);
            this->num_public_inputs = num_public_inputs;
            this->vrs = vrs;
            this->composer_type = composer_type;
        };
    };

    // These collect lightweight handles of data living in different entities. They
    // provide the utility of grouping these and ranged `for` loops over
    // subsets.
    using ProverPolynomials = AllEntities<PolynomialHandle, PolynomialHandle>;

    using FoldedPolynomials = AllEntities<std::vector<FF>, PolynomialHandle>;
    // TODO(#390): Simplify this?
    template <size_t MAX_RELATION_LENGTH>
    using ExtendedEdges =
        AllEntities<sumcheck::Univariate<FF, MAX_RELATION_LENGTH>, sumcheck::Univariate<FF, MAX_RELATION_LENGTH>>;

    class PurportedEvaluations : public AllEntities<FF, FF> {
      public:
        using Base = AllEntities<FF, FF>;
        using Base::Base;
        PurportedEvaluations(std::array<FF, NUM_ALL_ENTITIES> _data_in) { this->_data = _data_in; }
    };

    class CommitmentLabels : public AllEntities<std::string, std::string> {
      public:
        // this does away with the ENUM_TO_COMM array while preserving the
        // transcript interface, which takes a string
        // note: we could consider "enriching" the transcript interface to not use
        // strings in the future, but I leave it this way for simplicity

        CommitmentLabels()
            : AllEntities<std::string, std::string>()
        {
            w_l = "W_1";
            w_r = "W_2";
            w_o = "W_3";
            z_perm = "Z_PERM";
            // The ones beginning with "__" are only used for debugging
            z_perm_shift = "__Z_PERM_SHIFT";
            q_m = "__Q_M";
            q_l = "__Q_L";
            q_r = "__Q_R";
            q_o = "__Q_O";
            q_c = "__Q_C";
            sigma_1 = "__SIGMA_1";
            sigma_2 = "__SIGMA_2";
            sigma_3 = "__SIGMA_3";
            id_1 = "__ID_1";
            id_2 = "__ID_2";
            id_3 = "__ID_3";
            lagrange_first = "__LAGRANGE_FIRST";
            lagrange_last = "__LAGRANGE_LAST";
        };
    };

    // WORKTODO: this should not be an allEntities
    class VerifierCommitments : public AllEntities<Commitment, CommitmentHandle> {
      public:
        VerifierCommitments(std::shared_ptr<VerificationKey> verification_key, VerifierTranscript<FF> transcript)
        {
            static_cast<void>(transcript); // WORKTODO
            q_m = verification_key->q_m;
            q_l = verification_key->q_l;
            q_r = verification_key->q_r;
            q_o = verification_key->q_o;
            q_c = verification_key->q_c;
            sigma_1 = verification_key->sigma_1;
            sigma_2 = verification_key->sigma_2;
            sigma_3 = verification_key->sigma_3;
            id_1 = verification_key->id_1;
            id_2 = verification_key->id_2;
            id_3 = verification_key->id_3;
            lagrange_first = verification_key->lagrange_first;
            lagrange_last = verification_key->lagrange_last;
        }
    };
};

} // namespace proof_system::honk::flavor
