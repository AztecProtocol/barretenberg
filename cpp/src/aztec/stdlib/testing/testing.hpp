#pragma once
#include <numeric/random/engine.hpp>
#include <plonk/composer/standard_composer.hpp>
#include <plonk/composer/turbo_composer.hpp>
#include <plonk/composer/ultra_composer.hpp>
#include <stdlib/types/types.hpp>

#include <gtest/gtest.h>

namespace plonk::stdlib::test {

/**
 * @brief Concept to differentiate between composers which have a 'check_circuit'
 * function. This will usually be more efficient than constructing the proof directly
 *
 * @tparam Composer
 */
template <typename Composer>
concept HasCheckCircuit = requires(Composer composer) {
                              {
                                  composer.check_circuit()
                                  } -> std::same_as<bool>;
                          };

template <typename Composer> class StdlibTest : public testing::Test {

  protected:
    // before each individual test, we reset the random engine with a new seed.
    // this method is called automatically
    void SetUp() override
    {
        uint64_t seed = numeric::random::get_engine().get_random_uint64();
        std::cerr << "\033[0;35m"
                  << "[----------] "
                  << "\033[0;0m"
                  << "using random seed = " << seed << "\n";

        engine = &(numeric::random::get_debug_engine(seed));
    }

    /**
     * @brief Call this method at the start of the test to fix a specific seed
     * for the `debug_engine`.
     *
     * @param seed a fixed seed to get deterministic values from the `debug_engine`.
     */
    void OverrideSeed(uint64_t seed)
    {
        std::cerr << "\033[0;35m"
                  << "[----------] "
                  << "\033[0;0m"
                  << "overriding random engine with seed = " << seed << "\n";

        engine = &(numeric::random::get_debug_engine(seed));
    }

  public:
    /**
     * @brief Ensures that the circuit defined by the `composer` is not valid.
     *
     * @details If the composer has error-ed out, then we know the circuit is invalid and we return early.
     * Otherwise we run `check_circuit()` (if it's available) and return this result.
     * In cases where no `check_circuit()` is available, we construct the full proof and check that it fails.
     *
     * @warning This method should only be used to test failures.
     * `EXPECT_FALSE(circuit_fails(c))` may yield false positives.
     *
     * @param composer
     * @param check_composer_failed set this to false to ignore the result of `composer.failed()`
     * @return testing::AssertionResult
     */
    testing::AssertionResult circuit_fails(Composer& composer, bool check_composer_failed = true)
    {
        if (check_composer_failed && composer.failed()) {
            return testing::AssertionSuccess();
        }
        if constexpr (HasCheckCircuit<Composer>) {
            // Since we are expecting an invalid circuit, we would expect `check_circuit()`
            // to catch this.
            // If the test calling this function does not pass because we return an AssertionFailure,
            // then we should improve `check_circuit()` to catch this this error.
            if (composer.check_circuit()) {
                return testing::AssertionFailure() << "circuit is valid";
            } else {
                return testing::AssertionSuccess();
            }
        } else {
            auto prover = composer.create_prover();
            auto verifier = composer.create_verifier();
            auto proof = prover.construct_proof();
            if (verifier.verify_proof(proof)) {
                return testing::AssertionFailure() << "proof is valid";
            } else {
                return testing::AssertionSuccess();
            }
        }
    }

    /**
     * @brief Ensures that the circuit defined by the `composer` is valid and
     * a valid proof can be successfully created.
     *
     * @details To ensure the accuracy of this test, we create a full proof that must be valid.
     * If case the proof fails, then we also run `check_circuit()` (when available) to ensure the
     * results are consistent.
     *
     * @warning This method should only be used to test the validity of circuits.
     * `EXPECT_FALSE(circuit_verifies(c))` may yield false positives, and will be
     * slower than running `EXPECT_TRUE(circuit_fails(c))`.
     *
     * @param composer
     * @param check_composer_failed set this to false to ignore the result of `composer.failed()`
     * @return testing::AssertionResult
     */
    testing::AssertionResult circuit_verifies(Composer& composer, bool check_composer_failed = true)
    {
        // TODO: Add cases for DEBUG/RELEASE/CI
        if (check_composer_failed && composer.failed()) {
            return testing::AssertionFailure() << "composer failed with error: " << composer.err();
        }

        // Test whether we are creating a valid proof
        auto prover = composer.create_prover();
        auto verifier = composer.create_verifier();
        auto proof = prover.construct_proof();
        if (verifier.verify_proof(proof)) {
            return testing::AssertionSuccess();
        } else {
            // This branch should never be taken. If it is, then this test would fail,
            // and we assume that all tests must pass.
            if constexpr (HasCheckCircuit<Composer>) {
                // If the proof is not valid, we provide feedback about whether
                // `check_circuit()` can detect this failure.
                if (composer.check_circuit()) {
                    return testing::AssertionFailure() << "circuit is valid but proof verification failed";
                } else {
                    return testing::AssertionFailure() << "circuit and proof verification failed";
                }
            } else {
                return testing::AssertionFailure() << "proof verification failed";
            }
        }
    }

    numeric::random::Engine* engine;
};

using ComposerTypes = testing::Types<waffle::UltraComposer, waffle::TurboComposer, waffle::StandardComposer>;

#define TYPED_STDLIB_TEST(CaseName, TestName)                                                                          \
    template <typename gtest_TypeParam_>                                                                               \
    class GTEST_TEST_CLASS_NAME_(CaseName, TestName)                                                                   \
        : public CaseName<gtest_TypeParam_> {                                                                          \
      private:                                                                                                         \
        typedef CaseName<gtest_TypeParam_> TestFixture;                                                                \
        typedef gtest_TypeParam_ TypeParam;                                                                            \
        using Composer = TypeParam;                                                                                    \
        STDLIB_TYPE_ALIASES                                                                                            \
        virtual void TestBody();                                                                                       \
    };                                                                                                                 \
    static bool gtest_##CaseName##_##TestName##_registered_ GTEST_ATTRIBUTE_UNUSED_ =                                  \
        ::testing::internal::TypeParameterizedTest<                                                                    \
            CaseName,                                                                                                  \
            ::testing::internal::TemplateSel<GTEST_TEST_CLASS_NAME_(CaseName, TestName)>,                              \
            GTEST_TYPE_PARAMS_(CaseName)>::                                                                            \
            Register(                                                                                                  \
                "",                                                                                                    \
                ::testing::internal::CodeLocation(__FILE__, __LINE__),                                                 \
                #CaseName,                                                                                             \
                #TestName,                                                                                             \
                0,                                                                                                     \
                ::testing::internal::GenerateNames<GTEST_NAME_GENERATOR_(CaseName), GTEST_TYPE_PARAMS_(CaseName)>());  \
    template <typename gtest_TypeParam_> void GTEST_TEST_CLASS_NAME_(CaseName, TestName)<gtest_TypeParam_>::TestBody()

} // namespace plonk::stdlib::test