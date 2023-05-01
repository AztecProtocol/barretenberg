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
#include "barretenberg/proof_system/circuit_constructors/turbo_circuit_constructor.hpp"
#include "barretenberg/proof_system/circuit_constructors/ultra_circuit_constructor.hpp"
#include "barretenberg/srs/reference_string/reference_string.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"

namespace proof_system::honk::flavor {

class Ultra {
  public:
    using CircuitConstructor = UltraCircuitConstructor;
    using FF = barretenberg::fr;
    using Polynomial = barretenberg::Polynomial<FF>;
    using PolynomialHandle = std::span<FF>;
    using G1 = barretenberg::g1;
    using GroupElement = G1::element;
    using Commitment = G1::affine_element;
    using CommitmentHandle = G1::affine_element; // TODO(Cody): make a pointer?
    using PCSParams = pcs::kzg::Params;

    static constexpr size_t num_wires = CircuitConstructor::num_wires;
    // TODO(luke): sure would be nice if this was computed programtically
    // WORKTODO: this is 43 since we're not including individual sorted_i; (UP has 41 since no L_first, L_last)
    static constexpr size_t NUM_ALL_ENTITIES = 43;
    // TODO(luke): what does this need to reflect? e.g. are shifts of precomputed polys counted here?
    static constexpr size_t NUM_PRECOMPUTED_ENTITIES = 25;
    static constexpr size_t NUM_WITNESS_ENTITIES = 11;

    template <typename DataType, typename HandleType>
    class PrecomputedData : public PrecomputedData_<DataType, HandleType, NUM_PRECOMPUTED_ENTITIES> {
      public:
        DataType& q_m = std::get<0>(this->_data);
        DataType& q_c = std::get<1>(this->_data);
        DataType& q_l = std::get<2>(this->_data);
        DataType& q_r = std::get<3>(this->_data);
        DataType& q_o = std::get<4>(this->_data);
        DataType& q_4 = std::get<5>(this->_data);
        DataType& q_arith = std::get<6>(this->_data);
        DataType& q_sort = std::get<7>(this->_data);
        DataType& q_elliptic = std::get<8>(this->_data);
        DataType& q_aux = std::get<9>(this->_data);
        DataType& q_lookup = std::get<10>(this->_data);
        DataType& sigma_1 = std::get<11>(this->_data);
        DataType& sigma_2 = std::get<12>(this->_data);
        DataType& sigma_3 = std::get<13>(this->_data);
        DataType& sigma_4 = std::get<14>(this->_data);
        DataType& id_1 = std::get<15>(this->_data);
        DataType& id_2 = std::get<16>(this->_data);
        DataType& id_3 = std::get<17>(this->_data);
        DataType& id_4 = std::get<18>(this->_data);
        DataType& table_1 = std::get<19>(this->_data);
        DataType& table_2 = std::get<20>(this->_data);
        DataType& table_3 = std::get<21>(this->_data);
        DataType& table_4 = std::get<22>(this->_data);
        DataType& lagrange_first = std::get<23>(this->_data);
        DataType& lagrange_last = std::get<24>(this->_data);

        std::vector<HandleType> get_selectors() // WORKTODO: make these arrays?
        {
            // return { q_c, q_l, q_r, q_o, q_4, q_m, q_arith, q_sort, q_elliptic, q_aux, q_lookup };
            return { q_m, q_c, q_l, q_r, q_o, q_4, q_arith, q_sort, q_elliptic, q_aux, q_lookup };
        };
        std::vector<HandleType> get_sigma_polynomials() { return { sigma_1, sigma_2, sigma_3, sigma_4 }; };
        std::vector<HandleType> get_table_polynomials() { return { table_1, table_2, table_3, table_4 }; };
        std::vector<HandleType> get_id_polynomials() { return { id_1, id_2, id_3, id_4 }; };
    };

    // Container for all witness polys
    template <typename DataType, typename HandleType>
    class WitnessData : public Data_<DataType, HandleType, NUM_WITNESS_ENTITIES> {
      public:
        DataType& w_l = std::get<0>(this->_data);
        DataType& w_r = std::get<1>(this->_data);
        DataType& w_o = std::get<2>(this->_data);
        DataType& w_4 = std::get<3>(this->_data);
        DataType& sorted_1 = std::get<4>(this->_data);
        DataType& sorted_2 = std::get<5>(this->_data);
        DataType& sorted_3 = std::get<6>(this->_data);
        DataType& sorted_4 = std::get<7>(this->_data);
        DataType& sorted_accum = std::get<8>(this->_data);
        DataType& z_perm = std::get<9>(this->_data);
        DataType& z_lookup = std::get<10>(this->_data);

        std::vector<HandleType> get_wires() { return { w_l, w_r, w_o, w_4 }; };
        std::vector<HandleType> get_sorted_polynomials() { return { sorted_1, sorted_2, sorted_3, sorted_4 }; };
    };

    class ProvingKey : public ProvingKey_<PrecomputedData<Polynomial, PolynomialHandle>, FF> {
      public:
        WitnessData<Polynomial, PolynomialHandle> _witness_data;

        Polynomial& w_l = _witness_data.w_l;
        Polynomial& w_r = _witness_data.w_r;
        Polynomial& w_o = _witness_data.w_o;
        Polynomial& w_4 = _witness_data.w_4;
        // WORKTODO: these are somewhat unique; can be deleted after construction of s_accum it seems.
        Polynomial& sorted_1 = _witness_data.sorted_1;
        Polynomial& sorted_2 = _witness_data.sorted_2;
        Polynomial& sorted_3 = _witness_data.sorted_3;
        Polynomial& sorted_4 = _witness_data.sorted_4;
        Polynomial& sorted_accum = _witness_data.sorted_accum;
        Polynomial& z_perm = _witness_data.z_perm;
        Polynomial& z_lookup = _witness_data.z_lookup;

        std::vector<uint32_t> memory_read_records;
        std::vector<uint32_t> memory_write_records;

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
        // The plookup wires that store plookup read data.
        std::array<PolynomialHandle, 3> get_plookup_read_wires() { return { w_l, w_r, w_o }; };
        // The sorted concatenations of table and witness data needed for plookup.
        std::vector<PolynomialHandle> get_sorted_polynomials() { return _witness_data.get_sorted_polynomials(); };
    };

    // using VerificationKey = VerificationKey_<PrecomputedData<Commitment, CommitmentHandle>>;
    class VerificationKey : public VerificationKey_<PrecomputedData<Commitment, CommitmentHandle>> {
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

    template <typename DataType, typename HandleType>
    class AllData : public AllData_<DataType, HandleType, NUM_ALL_ENTITIES> {
      public:
        DataType& q_c = std::get<0>(this->_data);
        DataType& q_l = std::get<1>(this->_data);
        DataType& q_r = std::get<2>(this->_data);
        DataType& q_o = std::get<3>(this->_data);
        DataType& q_4 = std::get<4>(this->_data);
        DataType& q_m = std::get<5>(this->_data);
        DataType& q_arith = std::get<6>(this->_data);
        DataType& q_sort = std::get<7>(this->_data);
        DataType& q_elliptic = std::get<8>(this->_data);
        DataType& q_aux = std::get<9>(this->_data);
        DataType& q_lookup = std::get<10>(this->_data);
        DataType& sigma_1 = std::get<11>(this->_data);
        DataType& sigma_2 = std::get<12>(this->_data);
        DataType& sigma_3 = std::get<13>(this->_data);
        DataType& sigma_4 = std::get<14>(this->_data);
        DataType& id_1 = std::get<15>(this->_data);
        DataType& id_2 = std::get<16>(this->_data);
        DataType& id_3 = std::get<17>(this->_data);
        DataType& id_4 = std::get<18>(this->_data);
        DataType& table_1 = std::get<19>(this->_data);
        DataType& table_2 = std::get<20>(this->_data);
        DataType& table_3 = std::get<21>(this->_data);
        DataType& table_4 = std::get<22>(this->_data);
        DataType& lagrange_first = std::get<23>(this->_data);
        DataType& lagrange_last = std::get<24>(this->_data);
        DataType& w_l = std::get<25>(this->_data);
        DataType& w_r = std::get<26>(this->_data);
        DataType& w_o = std::get<27>(this->_data);
        DataType& w_4 = std::get<28>(this->_data);
        DataType& sorted_accum = std::get<29>(this->_data);
        DataType& z_perm = std::get<30>(this->_data);
        DataType& z_lookup = std::get<31>(this->_data);
        DataType& table_1_shift = std::get<32>(this->_data);
        DataType& table_2_shift = std::get<33>(this->_data);
        DataType& table_3_shift = std::get<34>(this->_data);
        DataType& table_4_shift = std::get<35>(this->_data);
        DataType& w_l_shift = std::get<36>(this->_data);
        DataType& w_r_shift = std::get<37>(this->_data);
        DataType& w_o_shift = std::get<38>(this->_data);
        DataType& w_4_shift = std::get<39>(this->_data);
        DataType& sorted_accum_shift = std::get<40>(this->_data);
        DataType& z_perm_shift = std::get<41>(this->_data);
        DataType& z_lookup_shift = std::get<42>(this->_data);

        std::vector<HandleType> get_wires() { return { w_l, w_r, w_o, w_4 }; };
        std::vector<HandleType> get_unshifted() override
        {
            return { q_c,           q_l,   q_r,      q_o,     q_4,     q_m,          q_arith, q_sort,
                     q_elliptic,    q_aux, q_lookup, sigma_1, sigma_2, sigma_3,      sigma_4, id_1,
                     id_2,          id_3,  id_4,     table_1, table_2, table_3,      table_4, lagrange_first,
                     lagrange_last, w_l,   w_r,      w_o,     w_4,     sorted_accum, z_perm,  z_lookup

            };
        };
        // WORKTODO: table polys? sorted polys?
        std::vector<HandleType> get_to_be_shifted() override
        {
            return { table_1, table_2, table_3, table_4, w_l, w_r, w_o, w_4, sorted_accum, z_perm, z_lookup };
        };
        std::vector<HandleType> get_shifted() override
        {
            return { table_1_shift, table_2_shift, table_3_shift,      table_4_shift, w_l_shift,     w_r_shift,
                     w_o_shift,     w_4_shift,     sorted_accum_shift, z_perm_shift,  z_lookup_shift };
        };

        AllData() = default;

        AllData(const AllData& other)
            : AllData_<DataType, HandleType, NUM_ALL_ENTITIES>(other){};

        AllData(AllData&& other)
            : AllData_<DataType, HandleType, NUM_ALL_ENTITIES>(other){};

        // TODO(luke): avoiding self assignment (clang warning) here is a bit gross. Is there a better way?
        AllData& operator=(const AllData& other)
        {
            AllData_<DataType, HandleType, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        AllData& operator=(AllData&& other)
        {
            AllData_<DataType, HandleType, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        AllData(std::array<FF, NUM_ALL_ENTITIES> data_in) { this->_data = data_in; }
    };

    // These collect lightweight handles of data living in different entities. They
    // provide the utility of grouping these and ranged `for` loops over
    // subsets.
    using ProverPolynomials = AllData<PolynomialHandle, PolynomialHandle>;
    // using VerifierCommitments = AllData<Commitment, CommitmentHandle>;

    using FoldedPolynomials = AllData<std::vector<FF>, PolynomialHandle>; // TODO(Cody): Just reuse ProverPolynomials?
    template <size_t MAX_RELATION_LENGTH>
    using ExtendedEdges =
        AllData<sumcheck::Univariate<FF, MAX_RELATION_LENGTH>, sumcheck::Univariate<FF, MAX_RELATION_LENGTH>>;

    class PurportedEvaluations : public AllData<FF, FF> {
      public:
        PurportedEvaluations() { this->_data = {}; }
        PurportedEvaluations(std::array<FF, NUM_ALL_ENTITIES> read_evals) { this->_data = read_evals; }
    };

    class CommitmentLabels : public AllData<std::string, std::string> {
      public:
        // this does away with the ENUM_TO_COMM array while preserving the
        // transcript interface, which takes a string
        // note: we could consider "enriching" the transcript interface to not use
        // strings in the future, but I leave it this way for simplicity

        CommitmentLabels()
        {
            w_l = "W_L";
            w_r = "W_R";
            w_o = "W_O";
            w_4 = "W_4";
            z_perm = "Z_PERM";
            z_lookup = "Z_LOOKUP";
            sorted_accum = "SORTED_ACCUM";

            // The ones beginning with "__" are only used for debugging
            q_c = "__Q_C";
            q_l = "__Q_L";
            q_r = "__Q_R";
            q_o = "__Q_O";
            q_4 = "__Q_4";
            q_m = "__Q_M";
            q_arith = "__Q_ARITH";
            q_sort = "__Q_SORT";
            q_elliptic = "__Q_ELLIPTIC";
            q_aux = "__Q_AUX";
            q_lookup = "__Q_LOOKUP";
            sigma_1 = "__SIGMA_1";
            sigma_2 = "__SIGMA_2";
            sigma_3 = "__SIGMA_3";
            sigma_4 = "__SIGMA_4";
            id_1 = "__ID_1";
            id_2 = "__ID_2";
            id_3 = "__ID_3";
            id_4 = "__ID_4";
            table_1 = "__TABLE_1";
            table_2 = "__TABLE_2";
            table_3 = "__TABLE_3";
            table_4 = "__TABLE_4";
            lagrange_first = "__LAGRANGE_FIRST";
            lagrange_last = "__LAGRANGE_LAST";
        };
    };

    class VerifierCommitments : public AllData<Commitment, CommitmentHandle> {
      public:
        VerifierCommitments(std::shared_ptr<VerificationKey> verification_key, VerifierTranscript<FF> transcript)
        {
            static_cast<void>(transcript); // WORKTODO
            q_m = verification_key->q_m;
            q_l = verification_key->q_l;
            q_r = verification_key->q_r;
            q_o = verification_key->q_o;
            q_4 = verification_key->q_4;
            q_c = verification_key->q_c;
            q_arith = verification_key->q_arith;
            q_sort = verification_key->q_sort;
            q_elliptic = verification_key->q_elliptic;
            q_aux = verification_key->q_aux;
            q_lookup = verification_key->q_lookup;
            sigma_1 = verification_key->sigma_1;
            sigma_2 = verification_key->sigma_2;
            sigma_3 = verification_key->sigma_3;
            sigma_4 = verification_key->sigma_4;
            id_1 = verification_key->id_1;
            id_2 = verification_key->id_2;
            id_3 = verification_key->id_3;
            id_4 = verification_key->id_4;
            table_1 = verification_key->table_1;
            table_2 = verification_key->table_2;
            table_3 = verification_key->table_3;
            table_4 = verification_key->table_4;
            lagrange_first = verification_key->lagrange_first;
            lagrange_last = verification_key->lagrange_last;
        }
    };
};

} // namespace proof_system::honk::flavor
