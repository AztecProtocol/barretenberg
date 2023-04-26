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
#include "barretenberg/proof_system/polynomial_store/polynomial_store.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"

// WORKTODO: Names of these classes
// WORKTODO: Define special member functions in reasonable way and untangle the bad consequences elsewhere (e.g., in
// SumcheckOutput)
namespace proof_system::honk::flavor {
template <typename T, typename TView, size_t NUM_ENTITIES> class Data {
  public:
    using DataType = std::array<T, NUM_ENTITIES>;
    DataType _data;

    // TODO(Cody): now it's safe to inherit from this... right?
    // virtual ~Data() = default;
    // Data() = default;
    // // TODO(Cody): are these needed?
    // Data(const Data&) = default;
    // Data(Data&&) = default;
    // Data& operator=(const Data&) = default;
    // Data& operator=(Data&&) = default;

    T& operator[](size_t idx) { return _data[idx]; };
    typename DataType::iterator begin() { return _data.begin(); };
    typename DataType::iterator end() { return _data.end(); };

    size_t size() { return _data.size(); }; // WORKTODO: constexpr
};

template <typename T, typename TView, size_t NUM_PRECOMPUTED_ENTITIES>
class BasePrecomputedData : public Data<T, TView, NUM_PRECOMPUTED_ENTITIES> {
  public:
    size_t circuit_size;
    size_t log_circuit_size;
    size_t num_public_inputs;
    ComposerType composer_type; // WORKTODO: Get rid of this

    // virtual ~BasePrecomputedData() = default;

    virtual std::vector<TView> get_selectors() = 0;
    virtual std::vector<TView> get_sigma_polynomials() = 0;
    virtual std::vector<TView> get_id_polynomials() = 0;
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

template <typename T, size_t NUM_ALL_ENTITIES> class BaseAllData : public Data<T, T, NUM_ALL_ENTITIES> {
  public:
    virtual std::vector<T> get_unshifted() = 0;
    virtual std::vector<T> get_to_be_shifted() = 0;
    virtual std::vector<T> get_shifted() = 0;

    // TODO(Cody): Look for a better solution?
    std::vector<T> get_unshifted_then_shifted()
    {
        std::vector<T> result{ get_unshifted() };
        std::vector<T> shifted{ get_shifted() };
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

    // TODO(Cody): Made this public derivation so that I could populate selector
    // polys from circuit constructor.
    template <typename T, typename TView>
    class PrecomputedData : public BasePrecomputedData<T, TView, NUM_PRECOMPUTED_ENTITIES> {
      public:
        T& q_m = std::get<0>(this->_data);
        T& q_l = std::get<1>(this->_data);
        T& q_r = std::get<2>(this->_data);
        T& q_o = std::get<3>(this->_data);
        T& q_c = std::get<4>(this->_data);
        T& sigma_1 = std::get<5>(this->_data);
        T& sigma_2 = std::get<6>(this->_data);
        T& sigma_3 = std::get<7>(this->_data);
        T& id_1 = std::get<8>(this->_data);
        T& id_2 = std::get<9>(this->_data);
        T& id_3 = std::get<10>(this->_data);
        T& lagrange_first = std::get<11>(this->_data);
        T& lagrange_last = std::get<12>(this->_data); // = LAGRANGE_N-1 whithout ZK, but can be less

        std::vector<TView> get_selectors() override { return { q_m, q_l, q_r, q_o, q_c }; };
        std::vector<TView> get_sigma_polynomials() override { return { sigma_1, sigma_2, sigma_3 }; };
        std::vector<TView> get_id_polynomials() override { return { id_1, id_2, id_3 }; };

        virtual ~PrecomputedData() = default;
    };

    class ProvingKey : public BaseProvingKey<PrecomputedData<Polynomial, PolynomialView>, FF> {
      public:
        ProvingKey() = default;
        ProvingKey(const size_t circuit_size,
                   const size_t num_public_inputs,
                   std::shared_ptr<ProverReferenceString> const& crs,
                   ComposerType composer_type)
        {
            this->circuit_size = circuit_size;
            this->log_circuit_size = numeric::get_msb(circuit_size);
            this->num_public_inputs = num_public_inputs;
            this->crs = crs;
            this->evaluation_domain = EvaluationDomain<FF>(circuit_size, circuit_size);
            this->composer_type = composer_type;

            for (auto& poly : this->_data) {
                auto new_poly = Polynomial(circuit_size);
                poly = new_poly;
            }
        };
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

    template <typename T> class AllData : public BaseAllData<T, NUM_ALL_ENTITIES> {
      public:
        T& q_c = std::get<0>(this->_data);
        T& q_l = std::get<1>(this->_data);
        T& q_r = std::get<2>(this->_data);
        T& q_o = std::get<3>(this->_data);
        T& q_m = std::get<4>(this->_data);
        T& sigma_1 = std::get<5>(this->_data);
        T& sigma_2 = std::get<6>(this->_data);
        T& sigma_3 = std::get<7>(this->_data);
        T& id_1 = std::get<8>(this->_data);
        T& id_2 = std::get<9>(this->_data);
        T& id_3 = std::get<10>(this->_data);
        T& lagrange_first = std::get<11>(this->_data);
        T& lagrange_last = std::get<12>(this->_data);
        T& w_l = std::get<13>(this->_data);
        T& w_r = std::get<14>(this->_data);
        T& w_o = std::get<15>(this->_data);
        T& z_perm = std::get<16>(this->_data);
        T& z_perm_shift = std::get<17>(this->_data);

        std::vector<T> get_wires() { return { w_l, w_r, w_o }; };

        std::vector<T> get_unshifted() override
        { // ...z_perm_shift is in here?
            // return { w_l,  w_r,  w_o,  z_perm,         q_m,          q_l, q_r, q_o, q_c, sigma_1, sigma_2, sigma_3,
            //          id_1, id_2, id_3, lagrange_first, lagrange_last };
            return { q_c,           q_l, q_r, q_o, q_m,   sigma_1, sigma_2, sigma_3, id_1, id_2, id_3, lagrange_first,
                     lagrange_last, w_l, w_r, w_o, z_perm };
        };

        std::vector<T> get_to_be_shifted() override { return { z_perm }; };

        std::vector<T> get_shifted() override { return { z_perm_shift }; };

        AllData() = default;

        AllData(const AllData& other)
            : BaseAllData<T, NUM_ALL_ENTITIES>(other){};

        AllData(AllData&& other)
            : BaseAllData<T, NUM_ALL_ENTITIES>(other){};

        // TODO(luke): avoiding self assignment (clang warning) here is a bit gross. Is there a better way?
        AllData& operator=(const AllData& other)
        {
            BaseAllData<T, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        AllData& operator=(AllData&& other)
        {
            BaseAllData<T, NUM_ALL_ENTITIES>::operator=(other);
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
            z_perm_shift = "__z_perm_shift";
            q_m = "__q_m";
            q_l = "__q_l";
            q_r = "__q_r";
            q_o = "__q_o";
            q_c = "__q_c";
            sigma_1 = "__sigma_1";
            sigma_2 = "__sigma_2";
            sigma_3 = "__sigma_3";
            id_1 = "__id_1";
            id_2 = "__id_2";
            id_3 = "__id_3";
            lagrange_first = "__lagrange_first";
            lagrange_last = "__lagrange_last";
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
    static constexpr size_t NUM_ALL_ENTITIES = 39;
    static constexpr size_t NUM_PRECOMPUTED_ENTITIES = 29;

    template <typename T, typename TView>
    class PrecomputedData : public BasePrecomputedData<T, TView, NUM_PRECOMPUTED_ENTITIES> {
      public:
        T& q_m = std::get<0>(this->_data);
        T& q_c = std::get<1>(this->_data);
        T& q_l = std::get<2>(this->_data);
        T& q_r = std::get<3>(this->_data);
        T& q_o = std::get<4>(this->_data);
        T& q_4 = std::get<5>(this->_data);
        T& q_arith = std::get<6>(this->_data);
        T& q_sort = std::get<7>(this->_data);
        T& q_elliptic = std::get<8>(this->_data);
        T& q_aux = std::get<9>(this->_data);
        T& q_lookuptype = std::get<10>(this->_data); // WORKTODO: rename
        T& sorted_1 = std::get<11>(this->_data);     // TODO(Cody) These shouldn't be in here, right?
        T& sorted_2 = std::get<12>(this->_data);     // WORKTODO: Should rename these to sorted_i
        T& sorted_3 = std::get<13>(this->_data);
        T& sorted_4 = std::get<14>(this->_data);
        T& sigma_1 = std::get<15>(this->_data);
        T& sigma_2 = std::get<16>(this->_data);
        T& sigma_3 = std::get<17>(this->_data);
        T& sigma_4 = std::get<18>(this->_data);
        T& id_1 = std::get<19>(this->_data);
        T& id_2 = std::get<20>(this->_data);
        T& id_3 = std::get<21>(this->_data);
        T& id_4 = std::get<22>(this->_data);
        T& table_1 = std::get<23>(this->_data);
        T& table_2 = std::get<24>(this->_data);
        T& table_3 = std::get<25>(this->_data);
        T& table_4 = std::get<26>(this->_data);
        T& lagrange_first = std::get<27>(this->_data);
        T& lagrange_last = std::get<28>(this->_data);

        std::vector<TView> get_selectors()
        {
            // return { q_c, q_l, q_r, q_o, q_4, q_m, q_arith, q_sort, q_elliptic, q_aux, q_lookuptype };
            return { q_m, q_c, q_l, q_r, q_o, q_4, q_arith, q_sort, q_elliptic, q_aux, q_lookuptype };
        };
        std::vector<TView> get_sigma_polynomials() { return { sigma_1, sigma_2, sigma_3, sigma_4 }; };
        std::vector<TView> get_table_polynomials() { return { table_1, table_2, table_3, table_4 }; };
        std::vector<TView> get_id_polynomials() { return { id_1, id_2, id_3, id_4 }; };

        virtual ~PrecomputedData() = default;
        PrecomputedData() = default;
        // TODO(Cody): are these needed?
        PrecomputedData(const PrecomputedData& other)
            : q_c(other.q_c)
            , q_l(other.q_l)
            , q_r(other.q_r)
            , q_o(other.q_o)
            , q_4(other.q_4)
            , q_m(other.q_m)
            , q_arith(other.q_arith)
            , q_sort(other.q_sort)
            , q_elliptic(other.q_elliptic)
            , q_aux(other.q_aux)
            , q_lookuptype(other.q_lookuptype)
            , sorted_1(other.sorted_1)
            , sorted_2(other.sorted_2)
            , sorted_3(other.sorted_3)
            , sorted_4(other.sorted_4)
            , sigma_1(other.sigma_1)
            , sigma_2(other.sigma_2)
            , sigma_3(other.sigma_3)
            , sigma_4(other.sigma_4)
            , id_1(other.id_1)
            , id_2(other.id_2)
            , id_3(other.id_3)
            , id_4(other.id_4)
            , table_1(other.table_1)
            , table_2(other.table_2)
            , table_3(other.table_3)
            , table_4(other.table_4)
            , lagrange_first(other.lagrange_first)
            , lagrange_last(other.lagrange_last){};

        PrecomputedData(PrecomputedData&& other)
            : q_c(other.q_c)
            , q_l(other.q_l)
            , q_r(other.q_r)
            , q_o(other.q_o)
            , q_4(other.q_4)
            , q_m(other.q_m)
            , q_arith(other.q_arith)
            , q_sort(other.q_sort)
            , q_elliptic(other.q_elliptic)
            , q_aux(other.q_aux)
            , q_lookuptype(other.q_lookuptype)
            , sorted_1(other.sorted_1)
            , sorted_2(other.sorted_2)
            , sorted_3(other.sorted_3)
            , sorted_4(other.sorted_4)
            , sigma_1(other.sigma_1)
            , sigma_2(other.sigma_2)
            , sigma_3(other.sigma_3)
            , sigma_4(other.sigma_4)
            , id_1(other.id_1)
            , id_2(other.id_2)
            , id_3(other.id_3)
            , id_4(other.id_4)
            , table_1(other.table_1)
            , table_2(other.table_2)
            , table_3(other.table_3)
            , table_4(other.table_4)
            , lagrange_first(other.lagrange_first)
            , lagrange_last(other.lagrange_last){};

        PrecomputedData& operator=(const PrecomputedData& other)
        // TODO(Cody): Doesn't work for self assignment?
        {
            q_c = other.q_c;
            q_l = other.q_l;
            q_r = other.q_r;
            q_o = other.q_o;
            q_4 = other.q_4;
            q_m = other.q_m;
            q_arith = other.q_arith;
            q_sort = other.q_sort;
            q_elliptic = other.q_elliptic;
            q_aux = other.q_aux;
            q_lookuptype = other.q_lookuptype;
            sorted_1 = other.sorted_1;
            sorted_2 = other.sorted_2;
            sorted_3 = other.sorted_3;
            sorted_4 = other.sorted_4;
            sigma_1 = other.sigma_1;
            sigma_2 = other.sigma_2;
            sigma_3 = other.sigma_3;
            sigma_4 = other.sigma_4;
            id_1 = other.id_1;
            id_2 = other.id_2;
            id_3 = other.id_3;
            id_4 = other.id_4;
            table_1 = other.table_1;
            table_2 = other.table_2;
            table_3 = other.table_3;
            table_4 = other.table_4;
            lagrange_first = other.lagrange_first;
            lagrange_last = other.lagrange_last;
            return *this;
        };

        PrecomputedData& operator=(PrecomputedData&& other)
        {
            q_c = other.q_c;
            q_l = other.q_l;
            q_r = other.q_r;
            q_o = other.q_o;
            q_4 = other.q_4;
            q_m = other.q_m;
            q_arith = other.q_arith;
            q_sort = other.q_sort;
            q_elliptic = other.q_elliptic;
            q_aux = other.q_aux;
            q_lookuptype = other.q_lookuptype;
            sorted_1 = other.sorted_1;
            sorted_2 = other.sorted_2;
            sorted_3 = other.sorted_3;
            sorted_4 = other.sorted_4;
            sigma_1 = other.sigma_1;
            sigma_2 = other.sigma_2;
            sigma_3 = other.sigma_3;
            sigma_4 = other.sigma_4;
            id_1 = other.id_1;
            id_2 = other.id_2;
            id_3 = other.id_3;
            id_4 = other.id_4;
            table_1 = other.table_1;
            table_2 = other.table_2;
            table_3 = other.table_3;
            table_4 = other.table_4;
            lagrange_first = other.lagrange_first;
            lagrange_last = other.lagrange_last;
            return *this;
        };
    };

    class ProvingKey : public BaseProvingKey<PrecomputedData<Polynomial, PolynomialView>, FF> {
      public:
        std::vector<uint32_t> memory_read_records;
        std::vector<uint32_t> memory_write_records;
        ProvingKey() = default;
        ProvingKey(const size_t circuit_size,
                   const size_t num_public_inputs,
                   std::shared_ptr<ProverReferenceString> const& crs,
                   ComposerType composer_type)
        {
            this->circuit_size = circuit_size;
            this->log_circuit_size = numeric::get_msb(circuit_size);
            this->num_public_inputs = num_public_inputs;
            this->crs = crs;
            this->evaluation_domain = EvaluationDomain<FF>(circuit_size, circuit_size);
            this->composer_type = composer_type;

            for (auto& poly : this->_data) {
                auto new_poly = Polynomial(circuit_size);
                poly = new_poly;
            }
        };
    };

    using VerificationKey = BaseVerificationKey<PrecomputedData<Commitment, CommitmentView>>;

    template <typename T> class AllData : public BaseAllData<T, NUM_ALL_ENTITIES> {
      public:
        T& q_c = std::get<0>(this->_data);
        T& q_l = std::get<1>(this->_data);
        T& q_r = std::get<2>(this->_data);
        T& q_o = std::get<3>(this->_data);
        T& q_4 = std::get<4>(this->_data);
        T& q_m = std::get<5>(this->_data);
        T& q_arith = std::get<6>(this->_data);
        T& q_sort = std::get<7>(this->_data);
        T& q_elliptic = std::get<8>(this->_data);
        T& q_aux = std::get<9>(this->_data);
        T& q_lookuptype = std::get<10>(this->_data);
        T& sigma_1 = std::get<11>(this->_data);
        T& sigma_2 = std::get<12>(this->_data);
        T& sigma_3 = std::get<13>(this->_data);
        T& sigma_4 = std::get<14>(this->_data);
        T& id_1 = std::get<15>(this->_data);
        T& id_2 = std::get<16>(this->_data);
        T& id_3 = std::get<17>(this->_data);
        T& id_4 = std::get<18>(this->_data);
        T& table_1 = std::get<19>(this->_data);
        T& table_2 = std::get<20>(this->_data);
        T& table_3 = std::get<21>(this->_data);
        T& table_4 = std::get<22>(this->_data);
        T& lagrange_first = std::get<23>(this->_data);
        T& lagrange_last = std::get<24>(this->_data);
        T& w_l = std::get<25>(this->_data);
        T& w_r = std::get<26>(this->_data);
        T& w_o = std::get<27>(this->_data);
        T& w_4 = std::get<28>(this->_data);
        T& sorted_1 = std::get<29>(this->_data);
        T& sorted_2 = std::get<30>(this->_data);
        T& sorted_3 = std::get<31>(this->_data);
        T& sorted_4 = std::get<32>(this->_data);
        T& z_perm = std::get<33>(this->_data);
        T& z_lookup = std::get<34>(this->_data);
        T& w_l_shift = std::get<35>(this->_data);
        T& w_4_shift = std::get<36>(this->_data);
        T& z_perm_shift = std::get<37>(this->_data);
        T& z_lookup_shift = std::get<38>(this->_data);

        std::vector<T> get_wires() { return { w_l, w_r, w_o, w_4 }; };

        std::vector<T> get_unshifted() override
        {
            return { q_c,           q_l,    q_r,          q_o,     q_4,     q_m,      q_arith,  q_sort,
                     q_elliptic,    q_aux,  q_lookuptype, sigma_1, sigma_2, sigma_3,  sigma_4,  id_1,
                     id_2,          id_3,   id_4,         table_1, table_2, table_3,  table_4,  lagrange_first,
                     lagrange_last, w_l,    w_r,          w_o,     w_4,     sorted_1, sorted_2, sorted_3,
                     sorted_4,      z_perm, z_lookup

            };
        };

        std::vector<T> get_to_be_shifted() override { return { w_l, w_4, z_perm, z_lookup }; };
        std::vector<T> get_shifted() override { return { w_l_shift, w_4_shift, z_perm_shift, z_lookup_shift }; };

        AllData() = default;

        AllData(const AllData& other)
            : BaseAllData<T, NUM_ALL_ENTITIES>(other){};

        AllData(AllData&& other)
            : BaseAllData<T, NUM_ALL_ENTITIES>(other){};

        // TODO(luke): avoiding self assignment (clang warning) here is a bit gross. Is there a better way?
        AllData& operator=(const AllData& other)
        {
            BaseAllData<T, NUM_ALL_ENTITIES>::operator=(other);
            return *this;
        }

        AllData& operator=(AllData&& other)
        {
            BaseAllData<T, NUM_ALL_ENTITIES>::operator=(other);
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

        // WORKTODO: stick with these names from before?
        std::string w_l = "W_1";
        std::string w_r = "W_2";
        std::string w_o = "W_3";
        std::string z_perm = "Z_PERM";
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

// Stuff
template <typename T, typename... U> concept IsAnyOf = (std::same_as<T, U> || ...);

template <typename T>
concept IsPlonkFlavor = IsAnyOf<T, plonk::flavor::Standard, plonk::flavor::Turbo, plonk::flavor::Ultra>;

template <typename T> concept IsHonkFlavor = IsAnyOf<T, honk::flavor::Standard, honk::flavor::Ultra>;

} // namespace proof_system
