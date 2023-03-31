#pragma once
#include <array>
#include <span>
#include <string>
#include <type_traits>
#include "barretenberg/honk/sumcheck/polynomials/univariate.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include "barretenberg/polynomials/polynomial.hpp"

// could be shared, but will it?
namespace proof_system::honk::flavor {
template <typename T, size_t NUM_ENTITIES> class Data {
  public:
    using DataType = std::array<T, NUM_ENTITIES>;
    DataType _data;

    // now it's safe to inherit from this... right?
    virtual ~Data() = default;

    T& operator[](size_t idx) { return _data[idx]; };
    typename DataType::iterator begin() { return _data.begin(); };
    typename DataType::iterator end() { return _data.end(); };

    consteval size_t size() { return _data.size(); };
};

class Standard {
    static constexpr size_t NUM_ALL_ENTITIES = 18;
    static constexpr size_t NUM_PRECOMPUTED_ENTITIES = 13;

  public:
    using FF = barretenberg::fr;
    using Polynomial = barretenberg::Polynomial<FF>;
    using PolynomialView = std::span<FF>;
    using G1 = barretenberg::g1;
    using Commitment = G1;
    using CommitmentView = G1; // TODO(Cody): make a pointer?

    template <typename T> class PrecomputedData : Data<T, NUM_PRECOMPUTED_ENTITIES> {
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
    };

    // these can differ, but it's ideal to share co
    using ProvingKey = PrecomputedData<Polynomial>;
    using VerificationKey = PrecomputedData<Commitment>;

    template <typename T> struct AllData : Data<T, NUM_ALL_ENTITIES> {
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

        // TODO(Cody): Look for a better solution?
        std::vector<T> get_in_order()
        {
            std::vector<T> result{ get_not_to_be_shifted() };
            std::vector<T> to_be_shifted{ get_to_be_shifted() };
            result.insert(result.end(), to_be_shifted.begin(), to_be_shifted.end());
            return result;
        };
    };
    // These are classes are views of data living in different entities. They
    // provide the utility of grouping these and ranged `for` loops over subsets.
    using ProverPolynomials = AllData<PolynomialView>;
    using VerifierCommitments = AllData<CommitmentView>;

    // TODO: Handle univariates right
    using ExtendedEdges = AllData<sumcheck::Univariate<FF, 5>>;

    using PurportedEvaluations = AllData<FF>;

    class CommitmentLabels : public AllData<std::string> {
      public:
        // this does away with the ENUM_TO_COMM array while preserving the
        // transcript interface, which takes a string
        //
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
