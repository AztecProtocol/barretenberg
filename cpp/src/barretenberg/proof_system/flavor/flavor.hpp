#pragma once
#include <array>
#include <concepts>
#include <span>
#include <string>
#include <type_traits>
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/honk/sumcheck/polynomials/univariate.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/plonk/proof_system/proving_key/proving_key.hpp"
#include "barretenberg/polynomials/evaluation_domain.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/proof_system/circuit_constructors/standard_circuit_constructor.hpp"
#include "barretenberg/proof_system/circuit_constructors/turbo_circuit_constructor.hpp"
#include "barretenberg/proof_system/circuit_constructors/ultra_circuit_constructor.hpp"
#include "barretenberg/srs/reference_string/reference_string.hpp"
#include "barretenberg/proof_system/polynomial_store/polynomial_store.hpp"

namespace proof_system::honk::flavor {

template <typename T, typename TView, size_t NUM_ENTITIES> class Data {
  public:
    using DataType = std::array<T, NUM_ENTITIES>;
    DataType _data;

    // TODO(Cody): now it's safe to inherit from this... right?
    virtual ~Data() = default;

    T& operator[](size_t idx) { return _data[idx]; };
    typename DataType::iterator begin() { return _data.begin(); };
    typename DataType::iterator end() { return _data.end(); };

    consteval size_t size() { return _data.size(); };
};

template <typename T, typename TView, size_t NUM_PRECOMPUTED_ENTITIES>
class BasePrecomputedData : public Data<T, TView, NUM_PRECOMPUTED_ENTITIES> {
  public:
    virtual std::vector<TView> get_selectors() = 0;
    virtual std::vector<TView> get_sigma_polynomials() = 0;
    virtual std::vector<TView> get_id_polynomials() = 0;
};

// TODO(Cody): Made this public derivation so that I could iterate through
// the selectors
template <typename PrecomputedData, typename FF> class BaseProvingKey : public PrecomputedData {
  public:
    const size_t circuit_size;
    const size_t log_circuit_size;
    const size_t num_public_inputs;
    std::shared_ptr<ProverReferenceString> crs;
    EvaluationDomain<FF> evaluation_domain;
    const ComposerType composer_type;     // TODO(Cody): Get rid of this
    PolynomialStore<FF> polynomial_store; // TODO(Cody): Get rid of this

    BaseProvingKey(const size_t circuit_size,
                   const size_t num_public_inputs,
                   std::shared_ptr<ProverReferenceString> const& crs,
                   ComposerType composer_type)
        : circuit_size(circuit_size)
        , log_circuit_size(numeric::get_msb(circuit_size))
        , num_public_inputs(num_public_inputs)
        , crs(crs)
        ,
        //   evaluation_domain(circuit_size, circuit_size),
        composer_type(composer_type){};
};

template <typename PrecomputedData> using BaseVerificationKey = PrecomputedData;

template <typename T, size_t NUM_ALL_ENTITIES> class BaseAllData {
  public:
    virtual std::vector<T> get_not_to_be_shifted() = 0;
    virtual std::vector<T> get_to_be_shifted() = 0;

    // TODO(Cody): Look for a better solution?
    std::vector<T> get_in_order()
    {
        std::vector<T> result{ get_not_to_be_shifted() };
        std::vector<T> to_be_shifted{ get_to_be_shifted() };
        result.insert(result.end(), to_be_shifted.begin(), to_be_shifted.end());
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
    using Commitment = G1;
    using CommitmentView = G1; // TODO(Cody): make a pointer?
    using PCSParams = pcs::kzg::Params;

    static constexpr size_t num_wires = CircuitConstructor::num_wires;
    static constexpr size_t NUM_PRECOMPUTED_ENTITIES = 18;
    static constexpr size_t NUM_ALL_ENTITIES = 13;
    static constexpr size_t minimum_circuit_size = 3; // TODO(Cody): what is this?/s

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

        std::vector<TView> get_selectors() { return { q_m, q_l, q_r, q_o, q_c }; };
        std::vector<TView> get_sigma_polynomials() { return { sigma_1, sigma_2, sigma_3 }; };
        std::vector<TView> get_id_polynomials() { return { id_1, id_2, id_3 }; };
    };

    using ProvingKey = BaseProvingKey<PrecomputedData<Polynomial, PolynomialView>, FF>;

    using VerificationKey = BaseVerificationKey<PrecomputedData<Commitment, CommitmentView>>;

    template <typename T> class AllData : BaseAllData<T, NUM_ALL_ENTITIES> {
      public:
        T& w_l = std::get<0>(this->_data);
        T& w_r = std::get<1>(this->_data);
        T& w_o = std::get<2>(this->_data);
        T& z_perm = std::get<3>(this->_data);
        T& z_perm_shift = std::get<4>(this->_data);
        T& q_m = std::get<5>(this->_data);
        T& q_l = std::get<6>(this->_data);
        T& q_r = std::get<7>(this->_data);
        T& q_o = std::get<8>(this->_data);
        T& q_c = std::get<9>(this->_data);
        T& sigma_1 = std::get<10>(this->_data);
        T& sigma_2 = std::get<11>(this->_data);
        T& sigma_3 = std::get<12>(this->_data);
        T& id_1 = std::get<13>(this->_data);
        T& id_2 = std::get<14>(this->_data);
        T& id_3 = std::get<15>(this->_data);
        T& lagrange_first = std::get<16>(this->_data);
        T& lagrange_last = std::get<17>(this->_data);

        std::vector<T> get_not_to_be_shifted()
        { // ...z_perm_shift is in here?
            return { w_l,  w_r,  w_o,  z_perm_shift,   q_m,          q_l, q_r, q_o, q_c, sigma_1, sigma_2, sigma_3,
                     id_1, id_2, id_3, lagrange_first, lagrange_last };
        };

        std::vector<T> get_to_be_shifted() { return { z_perm }; };
    };

    // These are classes are views of data living in different entities. They
    // provide the utility of grouping these and ranged `for` loops over
    // subsets.
    using ProverPolynomials = AllData<PolynomialView>;
    using VerifierCommitments = AllData<CommitmentView>;

    // WORKTODOXIxI: Handle univariates right
    using ExtendedEdges = AllData<sumcheck::Univariate<FF, 5>>;

    using PurportedEvaluations = AllData<FF>;

    class CommitmentLabels : public AllData<std::string> {
      public:
        // this does away with the ENUM_TO_COMM array while preserving the
        // transcript interface, which takes a string
        // note: we could consider "enriching" the transcript interface to not use
        // strings in the future, but I leave it this way for simplicity
        std::string w_l = "w_l";
        std::string w_r = "w_r";
        std::string w_o = "w_o";
        std::string p_0 = "p_0";
        std::string p_1 = "p_1";
        std::string q_0 = "q_0";
        std::string q_1 = "q_1";
        std::string s_0 = "s_0";
    };
};

class Ultra {
  public:
    using CircuitConstructor = UltraCircuitConstructor;
    using FF = CircuitConstructor::FF;

    using Polynomial = barretenberg::Polynomial<FF>;
    using PolynomialView = std::span<FF>;
    using G1 = barretenberg::g1;
    using Commitment = G1;
    using CommitmentView = G1; // TODO(Cody): make a pointer?
    using PCSParams = pcs::kzg::Params;

    static constexpr size_t num_wires = CircuitConstructor::num_wires;
    static constexpr size_t NUM_PRECOMPUTED_ENTITIES = 18;
    static constexpr size_t NUM_ALL_ENTITIES = 13;
    static constexpr size_t minimum_circuit_size = 3; // TODO(Cody): what is this?

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

        std::vector<TView> get_selectors() { return { q_m, q_l, q_r, q_o, q_c }; };
        std::vector<TView> get_sigma_polynomials() { return { sigma_1, sigma_2, sigma_3 }; };
        std::vector<TView> get_id_polynomials() { return { id_1, id_2, id_3 }; };
    };

    // TODO(Cody): Made this public derivation so that I could iterate through
    // the selectors

    using ProvingKey = BaseProvingKey<PrecomputedData<Polynomial, PolynomialView>, FF>;

    using VerificationKey = BaseVerificationKey<PrecomputedData<Commitment, CommitmentView>>;

    template <typename T> class AllData : BaseAllData<T, NUM_ALL_ENTITIES> {
      public:
        T& w_l = std::get<0>(this->_data);
        T& w_r = std::get<1>(this->_data);
        T& w_o = std::get<2>(this->_data);
        T& z_perm = std::get<3>(this->_data);
        T& z_perm_shift = std::get<4>(this->_data);
        T& q_m = std::get<5>(this->_data);
        T& q_l = std::get<6>(this->_data);
        T& q_r = std::get<7>(this->_data);
        T& q_o = std::get<8>(this->_data);
        T& q_c = std::get<9>(this->_data);
        T& sigma_1 = std::get<10>(this->_data);
        T& sigma_2 = std::get<11>(this->_data);
        T& sigma_3 = std::get<12>(this->_data);
        T& id_1 = std::get<13>(this->_data);
        T& id_2 = std::get<14>(this->_data);
        T& id_3 = std::get<15>(this->_data);
        T& lagrange_first = std::get<16>(this->_data);
        T& lagrange_last = std::get<17>(this->_data);

        std::vector<T> get_not_to_be_shifted()
        { // ...z_perm_shift is in here?
            return { w_l,  w_r,  w_o,  z_perm_shift,   q_m,          q_l, q_r, q_o, q_c, sigma_1, sigma_2, sigma_3,
                     id_1, id_2, id_3, lagrange_first, lagrange_last };
        };

        std::vector<T> get_to_be_shifted() { return { z_perm }; };
    };

    // These are classes are views of data living in different entities. They
    // provide the utility of grouping these and ranged `for` loops over
    // subsets.
    using ProverPolynomials = AllData<PolynomialView>;
    using VerifierCommitments = AllData<CommitmentView>;

    // TODO: Handle univariates right
    using ExtendedEdges = AllData<sumcheck::Univariate<FF, 5>>;

    using PurportedEvaluations = AllData<FF>;

    class CommitmentLabels : public AllData<std::string> {
      public:
        // this does away with the ENUM_TO_COMM array while preserving the
        // transcript interface, which takes a string
        // note: we could consider "enriching" the transcript interface to not use
        // strings in the future, but I leave it this way for simplicity
        std::string w_l = "w_l";
        std::string w_r = "w_r";
        std::string w_o = "w_o";
        std::string p_0 = "p_0";
        std::string p_1 = "p_1";
        std::string q_0 = "q_0";
        std::string q_1 = "q_1";
        std::string s_0 = "s_0";
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

template <typename T> concept IsHonkFlavor = IsAnyOf<T, honk::flavor::Standard>;

} // namespace proof_system
