#pragma once
#include "composer_base.hpp"
#include "barretenberg/proof_system/types/merkle_hash_type.hpp"
#include "barretenberg/proof_system/types/pedersen_commitment_type.hpp"

namespace proof_system::plonk {
class TurboComposer : public ComposerBase {
  public:
    static constexpr ComposerType type = ComposerType::TURBO;
    static constexpr merkle::HashType merkle_hash_type = merkle::HashType::FIXED_BASE_PEDERSEN;
    static constexpr pedersen::CommitmentType commitment_type = pedersen::CommitmentType::FIXED_BASE_PEDERSEN;
    static constexpr size_t UINT_LOG2_BASE = 2;
    enum TurboSelectors { QM, QC, Q1, Q2, Q3, Q4, Q5, QARITH, QFIXED, QRANGE, QLOGIC, NUM };

    TurboComposer();
    TurboComposer(std::string const& crs_path, const size_t size_hint = 0);
    TurboComposer(std::shared_ptr<ReferenceStringFactory> const& crs_factory, const size_t size_hint = 0);
    TurboComposer(std::shared_ptr<proving_key> const& p_key,
                  std::shared_ptr<verification_key> const& v_key,
                  size_t size_hint = 0);
    TurboComposer(TurboComposer&& other) = default;
    TurboComposer& operator=(TurboComposer&& other) = default;
    ~TurboComposer() {}

    virtual std::shared_ptr<proving_key> compute_proving_key() override;
    std::shared_ptr<verification_key> compute_verification_key() override;

    virtual size_t get_total_circuit_size() const override { return num_gates; };

    void compute_witness() override;

    TurboProver create_prover();
    TurboVerifier create_verifier();

    void create_add_gate(const add_triple& in) override;

    void create_big_add_gate(const add_quad& in);
    void create_big_add_gate_with_bit_extraction(const add_quad& in);
    void create_big_mul_gate(const mul_quad& in);
    void create_balanced_add_gate(const add_quad& in);

    void create_mul_gate(const mul_triple& in) override;
    void create_bool_gate(const uint32_t a) override;
    void create_poly_gate(const poly_triple& in) override;
    void create_fixed_group_add_gate(const fixed_group_add_quad& in);
    void create_fixed_group_add_gate_with_init(const fixed_group_add_quad& in, const fixed_group_init_quad& init);
    void create_fixed_group_add_gate_final(const add_quad& in);
    void fix_witness(const uint32_t witness_index, const barretenberg::fr& witness_value);

    bool check_circuit();

    void add_recursive_proof(const std::vector<uint32_t>& proof_output_witness_indices)
    {
        if (contains_recursive_proof) {
            failure("added recursive proof when one already exists");
        }
        contains_recursive_proof = true;

        for (const auto& idx : proof_output_witness_indices) {
            set_public_input(idx);
            recursive_proof_public_input_indices.push_back((uint32_t)(public_inputs.size() - 1));
        }
    }
    std::vector<uint32_t> recursive_proof_public_input_indices;
    bool contains_recursive_proof = false;

    std::vector<uint32_t> decompose_into_base4_accumulators(const uint32_t witness_index,
                                                            const size_t num_bits,
                                                            std::string const& msg);

    void create_range_constraint(const uint32_t variable_index, const size_t num_bits, std::string const& msg)
    {
        decompose_into_base4_accumulators(variable_index, num_bits, msg);
    }

    accumulator_triple create_logic_constraint(const uint32_t a,
                                               const uint32_t b,
                                               const size_t num_bits,
                                               bool is_xor_gate);
    accumulator_triple create_and_constraint(const uint32_t a, const uint32_t b, const size_t num_bits);
    accumulator_triple create_xor_constraint(const uint32_t a, const uint32_t b, const size_t num_bits);

    uint32_t put_constant_variable(const barretenberg::fr& variable);

    size_t get_num_constant_gates() const override { return 0; }

    void assert_equal_constant(const uint32_t a_idx,
                               const barretenberg::fr& b,
                               std::string const& msg = "assert_equal_constant")
    {
        if (variables[a_idx] != b && !failed()) {
            failure(msg);
        }
        auto b_idx = put_constant_variable(b);
        assert_equal(a_idx, b_idx, msg);
    }

    /**
     * For any type other than uint32_t (presumed to be a witness index), we call normalize first.
     */
    template <typename T>
    void assert_equal_constant(T const& in, const barretenberg::fr& b, std::string const& msg = "assert_equal_constant")
    {
        assert_equal_constant(in.normalize().witness_index, b, msg);
    }

    // these are variables that we have used a gate on, to enforce that they are equal to a defined value
    std::map<barretenberg::fr, uint32_t> constant_variable_indices;

    static transcript::Manifest create_manifest(const size_t num_public_inputs)
    {
        // add public inputs....
        constexpr size_t g1_size = 64;
        constexpr size_t fr_size = 32;
        const size_t public_input_size = fr_size * num_public_inputs;
        const transcript::Manifest output = transcript::Manifest(
            { transcript::Manifest::RoundManifest(
                  { { "circuit_size", 4, true }, { "public_input_size", 4, true } }, "init", 1),

              transcript::Manifest::RoundManifest({}, "eta", 0),

              transcript::Manifest::RoundManifest(
                  {
                      { "public_inputs", public_input_size, false },
                      { "W_1", g1_size, false },
                      { "W_2", g1_size, false },
                      { "W_3", g1_size, false },
                      { "W_4", g1_size, false },
                  },
                  "beta",
                  2),
              transcript::Manifest::RoundManifest({ { "Z_PERM", g1_size, false } }, "alpha", 1),
              transcript::Manifest::RoundManifest(
                  {
                      { "T_1", g1_size, false },
                      { "T_2", g1_size, false },
                      { "T_3", g1_size, false },
                      { "T_4", g1_size, false },
                  },
                  "z",
                  1),

              transcript::Manifest::RoundManifest(
                  {
                      { "t", fr_size, true, -1 },         { "w_1", fr_size, false, 0 },
                      { "w_2", fr_size, false, 1 },       { "w_3", fr_size, false, 2 },
                      { "w_4", fr_size, false, 3 },       { "sigma_1", fr_size, false, 4 },
                      { "sigma_2", fr_size, false, 5 },   { "sigma_3", fr_size, false, 6 },
                      { "sigma_4", fr_size, false, 7 },   { "q_1", fr_size, false, 8 },
                      { "q_2", fr_size, false, 9 },       { "q_3", fr_size, false, 10 },
                      { "q_4", fr_size, false, 11 },      { "q_5", fr_size, false, 12 },
                      { "q_m", fr_size, false, 13 },      { "q_c", fr_size, false, 14 },
                      { "q_arith", fr_size, false, 15 },  { "q_logic", fr_size, false, 16 },
                      { "q_range", fr_size, false, 17 },  { "q_fixed_base", fr_size, false, 18 },
                      { "z_perm", fr_size, false, 19 },   { "z_perm_omega", fr_size, false, 19 },
                      { "w_1_omega", fr_size, false, 0 }, { "w_2_omega", fr_size, false, 1 },
                      { "w_3_omega", fr_size, false, 2 }, { "w_4_omega", fr_size, false, 3 },
                  },
                  "nu",
                  TURBO_MANIFEST_SIZE,
                  true),

              transcript::Manifest::RoundManifest(
                  { { "PI_Z", g1_size, false }, { "PI_Z_OMEGA", g1_size, false } }, "separator", 3) });

        return output;
    }
};
/**
 * CheckGetter class is used to evaluate widget operations for circuit_checking
 *
 * */
class CheckGetter {
  public:
    static constexpr barretenberg::fr random_value = barretenberg::fr(0xdead);
    static constexpr barretenberg::fr zero = barretenberg::fr::zero();

    /**
     * Get a reference to a value of a witness/selector
     *
     * @param composer Composer object
     * @param index Index of the value in polynomial (array)
     *
     * @tparam value_type Controls if we shift index to the right or not
     * @tparam id The id of the selector/witness polynomial being used
     * */
    template <EvaluationType value_type, PolynomialIndex id>
    inline static const barretenberg::fr& get_value(const TurboComposer& composer, const size_t index = 0)
    {
        size_t actual_index = index;
        if constexpr (EvaluationType::SHIFTED == value_type) {
            actual_index += 1;
            if (actual_index >= composer.num_gates) {
                return zero;
            }
        }
        switch (id) {
        case PolynomialIndex::Q_1:
            return composer.selectors[TurboComposer::TurboSelectors::Q1][actual_index];
        case PolynomialIndex::Q_2:
            return composer.selectors[TurboComposer::TurboSelectors::Q2][actual_index];
        case PolynomialIndex::Q_3:
            return composer.selectors[TurboComposer::TurboSelectors::Q3][actual_index];
        case PolynomialIndex::Q_4:
            return composer.selectors[TurboComposer::TurboSelectors::Q4][actual_index];
        case PolynomialIndex::Q_5:
            return composer.selectors[TurboComposer::TurboSelectors::Q5][actual_index];
        case PolynomialIndex::Q_M:
            return composer.selectors[TurboComposer::TurboSelectors::QM][actual_index];
        case PolynomialIndex::Q_C:
            return composer.selectors[TurboComposer::TurboSelectors::QC][actual_index];
        case PolynomialIndex::Q_ARITHMETIC:
            return composer.selectors[TurboComposer::TurboSelectors::QARITH][actual_index];
        case PolynomialIndex::Q_LOGIC:
            return composer.selectors[TurboComposer::TurboSelectors::QLOGIC][actual_index];
        case PolynomialIndex::Q_RANGE:
            return composer.selectors[TurboComposer::TurboSelectors::QRANGE][actual_index];
        case PolynomialIndex::Q_FIXED_BASE:
            return composer.selectors[TurboComposer::TurboSelectors::QFIXED][actual_index];
        case PolynomialIndex::W_1:
            return composer.get_variable_reference(composer.w_l[actual_index]);
        case PolynomialIndex::W_2:
            return composer.get_variable_reference(composer.w_r[actual_index]);
        case PolynomialIndex::W_3:
            return composer.get_variable_reference(composer.w_o[actual_index]);
        case PolynomialIndex::W_4:
            return composer.get_variable_reference(composer.w_4[actual_index]);
        }
        return CheckGetter::random_value;
    }
};

using TurboArithmeticChecker = widget::TurboArithmeticKernel<barretenberg::fr, CheckGetter, TurboComposer>;
using TurboRangeChecker = widget::TurboRangeKernel<barretenberg::fr, CheckGetter, TurboComposer>;
using TurboLogicChecker = widget::TurboLogicKernel<barretenberg::fr, CheckGetter, TurboComposer>;
using TurboFixedBaseChecker = widget::TurboFixedBaseKernel<barretenberg::fr, CheckGetter, TurboComposer>;
} // namespace proof_system::plonk
