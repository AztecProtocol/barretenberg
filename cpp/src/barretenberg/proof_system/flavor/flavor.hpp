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

// WORKTODO: Names of these classes
// WORKTODO: Selectors should come from arithmetization.
// WORKTODO: Define special member functions in reasonable way and untangle the bad consequences elsewhere (e.g., in
// SumcheckOutput)
/**
 * WORKTODO: Outline what's going on here, explain the data model and how to think about these classes.
 *
 */
namespace proof_system::honk::flavor {

/**
 * @brief Base data class, a wrapper for std::array, from which every flavor class ultimately derives.
 * @details Ideally we could derive from std::array, but that is unsafe because std::array's destructor is non-virtual.
 *
 * @tparam T The underlying data type stored in the array
 * @tparam HandleType The type that will be used to
 * @tparam NUM_ENTITIES The size of the underlying array.
 */
template <typename DataType, typename HandleType, size_t NUM_ENTITIES> class Data {
  public:
    using ArrayType = std::array<DataType, NUM_ENTITIES>;
    ArrayType _data;

    // TODO(Cody): now it's safe to inherit from this... right?
    // virtual ~Data() = default;
    // Data() = default;
    // // TODO(Cody): are these needed?
    // Data(const Data&) = default;
    // Data(Data&&) = default;
    // Data& operator=(const Data&) = default;
    // Data& operator=(Data&&) = default;

    DataType& operator[](size_t idx) { return _data[idx]; };
    typename ArrayType::iterator begin() { return _data.begin(); };
    typename ArrayType::iterator end() { return _data.end(); };

    size_t size() { return _data.size(); }; // WORKTODO: constexpr
};

template <typename DataType, typename HandleType, size_t NUM_PRECOMPUTED_ENTITIES>
class BasePrecomputedData : public Data<DataType, HandleType, NUM_PRECOMPUTED_ENTITIES> {
  public:
    size_t circuit_size;
    size_t log_circuit_size;
    size_t num_public_inputs;
    ComposerType composer_type; // TODO(#392)

    // virtual ~BasePrecomputedData() = default;

    virtual std::vector<HandleType> get_selectors() = 0;
    virtual std::vector<HandleType> get_sigma_polynomials() = 0;
    virtual std::vector<HandleType> get_id_polynomials() = 0;
};

// TODO(Cody): Made this public derivation so that I could iterate through
// the selectors
template <typename PrecomputedData, typename FF> class BaseProvingKey : public PrecomputedData {
  public:
    bool contains_recursive_proof;
    std::vector<uint32_t> recursive_proof_public_input_indices;
    std::shared_ptr<ProverReferenceString> crs;
    EvaluationDomain<FF> evaluation_domain;
};

/**
 * @brief Collect all entities (really, views of these) from the protocol in one place.
 * @details No need for a distinction between storage and view classes here.
 *
 * @tparam PrecomputedData
 */
template <typename PrecomputedData> class BaseVerificationKey : public PrecomputedData {
  public:
    std::shared_ptr<VerifierReferenceString> vrs;
};

template <typename DataType, size_t NUM_ALL_ENTITIES>
class BaseAllData : public Data<DataType, DataType, NUM_ALL_ENTITIES> {
  public:
    virtual std::vector<DataType> get_unshifted() = 0;
    virtual std::vector<DataType> get_to_be_shifted() = 0;
    virtual std::vector<DataType> get_shifted() = 0;

    // TODO(Cody): Look for a better solution?
    std::vector<DataType> get_unshifted_then_shifted()
    {
        std::vector<DataType> result{ get_unshifted() };
        std::vector<DataType> shifted{ get_shifted() };
        result.insert(result.end(), shifted.begin(), shifted.end());
        return result;
    };
};

class Standard {
  public:
    using CircuitConstructor = StandardCircuitConstructor;
    using FF = barretenberg::fr;
    using Polynomial = barretenberg::Polynomial<FF>;
    using PolynomialView = std::span<FF>;
    using G1 = barretenberg::g1;
    using GroupElement = G1::element;
    using Commitment = G1::affine_element;
    using CommitmentView = G1::affine_element; // TODO(Cody): make a pointer?
    using PCSParams = pcs::kzg::Params;

    static constexpr size_t num_wires = CircuitConstructor::num_wires;
    static constexpr size_t NUM_ALL_ENTITIES = 18;
    static constexpr size_t NUM_PRECOMPUTED_ENTITIES = 13;
    // WORKTODO(luke): This number needs to be total witness polys not including shifts.
    static constexpr size_t NUM_WITNESS_ENTITIES = 4;

    // TODO(Cody): Made this public derivation so that I could populate selector
    // polys from circuit constructor.
    template <typename DataType, typename HandleType>
    class PrecomputedData : public BasePrecomputedData<DataType, HandleType, NUM_PRECOMPUTED_ENTITIES> {
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

        virtual ~PrecomputedData() = default;
    };

    /**
     * @brief Container for all witness polynomials used/constructed by the prover.
     * @details Shifts are not included here since they do not occupy their own memory
     *
     * @tparam T
     * @tparam HandleType
     */
    template <typename DataType, typename HandleType>
    class WitnessData : public Data<DataType, HandleType, NUM_WITNESS_ENTITIES> {
      public:
        DataType& w_l = std::get<0>(this->_data);
        DataType& w_r = std::get<1>(this->_data);
        DataType& w_o = std::get<2>(this->_data);
        DataType& z_perm = std::get<3>(this->_data);

        std::vector<HandleType> get_wires() { return { w_l, w_r, w_o }; };

        virtual ~WitnessData() = default;
    };

    // WORKTODO(luke): Proving key now stores all multivariate polynomials used by the prover. Is Pkey still the right
    // name?
    class ProvingKey : public BaseProvingKey<PrecomputedData<Polynomial, PolynomialView>, FF> {
      public:
        WitnessData<Polynomial, PolynomialView> _witness_data; // WORKTODO: name?

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

        std::vector<PolynomialView> get_wires() { return _witness_data.get_wires(); };
    };

    class VerificationKey : public BaseVerificationKey<PrecomputedData<Commitment, CommitmentView>> {
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

    template <typename DataType> class AllData : public BaseAllData<DataType, NUM_ALL_ENTITIES> {
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

        std::vector<DataType> get_wires() { return { w_l, w_r, w_o }; };

        std::vector<DataType> get_unshifted() override
        { // ...z_perm_shift is in here?
            return { q_c,           q_l, q_r, q_o, q_m,   sigma_1, sigma_2, sigma_3, id_1, id_2, id_3, lagrange_first,
                     lagrange_last, w_l, w_r, w_o, z_perm };
        };

        std::vector<DataType> get_to_be_shifted() override { return { z_perm }; };

        std::vector<DataType> get_shifted() override { return { z_perm_shift }; };

        AllData() = default;

        AllData(const AllData& other)
            : BaseAllData<DataType, NUM_ALL_ENTITIES>(other){};

        AllData(AllData&& other)
            : BaseAllData<DataType, NUM_ALL_ENTITIES>(other){};

        // WORKTODO(luke): avoiding self assignment (clang warning) here is a bit gross. Is there a better way?
        AllData& operator=(const AllData& other)
        {
            BaseAllData<DataType, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        AllData& operator=(AllData&& other)
        {
            BaseAllData<DataType, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        AllData(std::array<FF, NUM_ALL_ENTITIES> data_in) { this->_data = data_in; }
    };

    // These are classes are views of data living in different entities. They
    // provide the utility of grouping these and ranged `for` loops over
    // subsets.
    using ProverPolynomials = AllData<PolynomialView>;

    // WORKTODO: Handle univariates right
    using FoldedPolynomials = AllData<std::vector<FF>>; // WORKTODO add view class type.
    template <size_t MAX_RELATION_LENGTH> using ExtendedEdges = AllData<sumcheck::Univariate<FF, MAX_RELATION_LENGTH>>;

    using PurportedEvaluations = AllData<FF>;

    class CommitmentLabels : public AllData<std::string> {
      public:
        // this does away with the ENUM_TO_COMM array while preserving the
        // transcript interface, which takes a string
        // note: we could consider "enriching" the transcript interface to not use
        // strings in the future, but I leave it this way for simplicity

        CommitmentLabels()
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

    class VerifierCommitments : public AllData<CommitmentView> {
      public:
        VerifierCommitments(std::shared_ptr<VerificationKey> verification_key, VerifierTranscript<FF> transcript)
        {
            static_cast<void>(transcript);
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

class Ultra {
  public:
    using CircuitConstructor = UltraCircuitConstructor;
    using FF = barretenberg::fr;
    using Polynomial = barretenberg::Polynomial<FF>;
    using PolynomialView = std::span<FF>;
    using G1 = barretenberg::g1;
    using GroupElement = G1::element;
    using Commitment = G1::affine_element;
    using CommitmentView = G1::affine_element; // TODO(Cody): make a pointer?
    using PCSParams = pcs::kzg::Params;

    static constexpr size_t num_wires = CircuitConstructor::num_wires;
    // TODO(luke): sure would be nice if this was computed programtically
    static constexpr size_t NUM_ALL_ENTITIES = 47;
    // TODO(luke): what does this need to reflect? e.g. are shifts of precomputed polys counted here?
    static constexpr size_t NUM_PRECOMPUTED_ENTITIES = 25;
    static constexpr size_t NUM_WITNESS_ENTITIES = 11;

    template <typename DataType, typename HandleType>
    class PrecomputedData : public BasePrecomputedData<DataType, HandleType, NUM_PRECOMPUTED_ENTITIES> {
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

        virtual ~PrecomputedData() = default;
    };

    // Container for all witness polys
    template <typename DataType, typename HandleType>
    class WitnessData : public Data<DataType, HandleType, NUM_WITNESS_ENTITIES> {
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

        virtual ~WitnessData() = default;
    };

    class ProvingKey : public BaseProvingKey<PrecomputedData<Polynomial, PolynomialView>, FF> {
      public:
        WitnessData<Polynomial, PolynomialView> _witness_data;

        Polynomial& w_l = _witness_data.w_l;
        Polynomial& w_r = _witness_data.w_r;
        Polynomial& w_o = _witness_data.w_o;
        Polynomial& w_4 = _witness_data.w_4;
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

        std::vector<PolynomialView> get_wires() { return _witness_data.get_wires(); };
        // The plookup wires that store plookup read data.
        std::array<PolynomialView, 3> get_plookup_read_wires() { return { w_l, w_r, w_o }; };
        // The sorted concatenations of table and witness data needed for plookup.
        std::vector<PolynomialView> get_sorted_polynomials() { return _witness_data.get_sorted_polynomials(); };
    };

    using VerificationKey = BaseVerificationKey<PrecomputedData<Commitment, CommitmentView>>;

    template <typename DataType> class AllData : public BaseAllData<DataType, NUM_ALL_ENTITIES> {
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
        DataType& sorted_1 = std::get<29>(this->_data);
        DataType& sorted_2 = std::get<30>(this->_data);
        DataType& sorted_3 = std::get<31>(this->_data);
        DataType& sorted_4 = std::get<32>(this->_data);
        DataType& sorted_accum = std::get<33>(this->_data);
        DataType& z_perm = std::get<34>(this->_data);
        DataType& z_lookup = std::get<35>(this->_data);
        DataType& table_1_shift = std::get<36>(this->_data);
        DataType& table_2_shift = std::get<37>(this->_data);
        DataType& table_3_shift = std::get<38>(this->_data);
        DataType& table_4_shift = std::get<39>(this->_data);
        DataType& w_l_shift = std::get<40>(this->_data);
        DataType& w_r_shift = std::get<41>(this->_data);
        DataType& w_o_shift = std::get<42>(this->_data);
        DataType& w_4_shift = std::get<43>(this->_data);
        DataType& sorted_accum_shift = std::get<44>(this->_data);
        DataType& z_perm_shift = std::get<45>(this->_data);
        DataType& z_lookup_shift = std::get<46>(this->_data);

        std::vector<DataType> get_wires() { return { w_l, w_r, w_o, w_4 }; };
        std::vector<DataType> get_unshifted() override
        {
            return { q_c,           q_l,    q_r,      q_o,     q_4,     q_m,      q_arith,  q_sort,
                     q_elliptic,    q_aux,  q_lookup, sigma_1, sigma_2, sigma_3,  sigma_4,  id_1,
                     id_2,          id_3,   id_4,     table_1, table_2, table_3,  table_4,  lagrange_first,
                     lagrange_last, w_l,    w_r,      w_o,     w_4,     sorted_1, sorted_2, sorted_3,
                     sorted_4,      z_perm, z_lookup

            };
        };
        std::vector<DataType> get_to_be_shifted() override { return { w_l, w_4, z_perm, z_lookup }; };
        std::vector<DataType> get_shifted() override { return { w_l_shift, w_4_shift, z_perm_shift, z_lookup_shift }; };

        AllData() = default;

        AllData(const AllData& other)
            : BaseAllData<DataType, NUM_ALL_ENTITIES>(other){};

        AllData(AllData&& other)
            : BaseAllData<DataType, NUM_ALL_ENTITIES>(other){};

        // TODO(luke): avoiding self assignment (clang warning) here is a bit gross. Is there a better way?
        AllData& operator=(const AllData& other)
        {
            BaseAllData<DataType, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        AllData& operator=(AllData&& other)
        {
            BaseAllData<DataType, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        AllData(std::array<FF, NUM_ALL_ENTITIES> data_in) { this->_data = data_in; }
    };

    // These are classes are views of data living in different entities. They
    // provide the utility of grouping these and ranged `for` loops over
    // subsets.
    using ProverPolynomials = AllData<PolynomialView>;
    using VerifierCommitments = AllData<CommitmentView>;

    // WORKTODO: Handle univariates right
    using FoldedPolynomials = AllData<std::vector<FF>>; // WORKTODO add view class type.
    template <size_t MAX_RELATION_LENGTH> using ExtendedEdges = AllData<sumcheck::Univariate<FF, MAX_RELATION_LENGTH>>;

    class PurportedEvaluations : public AllData<FF> {
      public:
        PurportedEvaluations() { this->_data = {}; }
        PurportedEvaluations(std::array<FF, NUM_ALL_ENTITIES> read_evals) { this->_data = read_evals; }
    };

    class CommitmentLabels : public AllData<std::string> {
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
            sorted_1 = "__SORTED_1";
            sorted_2 = "__SORTED_2";
            sorted_3 = "__SORTED_3";
            sorted_4 = "__SORTED_4";
            sorted_accum = "__SORTED_ACCUM";
            table_1_shift = "__TABLE_1_SHIFT";
            table_2_shift = "__TABLE_2_SHIFT";
            table_3_shift = "__TABLE_3_SHIFT";
            table_4_shift = "__TABLE_4_SHIFT";
            w_l_shift = "__W_L_SHIFT";
            w_r_shift = "__W_R_SHIFT";
            w_o_shift = "__W_O_SHIFT";
            w_4_shift = "__W_4_SHIFT";
            sorted_accum_shift = "__SORTED_ACCUM_SHIFT";
            z_perm_shift = "__Z_PERM_SHIFT";
            z_lookup_shift = "__Z_LOOKUP_SHIFT";
        };
    };
};

} // namespace proof_system::honk::flavor

namespace proof_system::plonk::flavor {
struct Standard {
    using CircuitConstructor = proof_system::StandardCircuitConstructor;
    using ProvingKey = plonk::proving_key;
    static constexpr size_t num_wires = CircuitConstructor::num_wires;
};

struct Turbo {
    using CircuitConstructor = proof_system::TurboCircuitConstructor;
    using ProvingKey = plonk::proving_key;
    static constexpr size_t num_wires = CircuitConstructor::num_wires;
};

struct Ultra {
    using CircuitConstructor = proof_system::UltraCircuitConstructor;
    using ProvingKey = plonk::proving_key;
    static constexpr size_t num_wires = CircuitConstructor::num_wires;
};
} // namespace proof_system::plonk::flavor

namespace proof_system {

template <typename T, typename... U> concept IsAnyOf = (std::same_as<T, U> || ...);

template <typename T>
concept IsPlonkFlavor = IsAnyOf<T, plonk::flavor::Standard, plonk::flavor::Turbo, plonk::flavor::Ultra>;

template <typename T> concept IsHonkFlavor = IsAnyOf<T, honk::flavor::Standard, honk::flavor::Ultra>;

} // namespace proof_system
