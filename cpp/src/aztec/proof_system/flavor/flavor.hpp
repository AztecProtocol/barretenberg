#pragma once
#include <array>
#include <string>
#include <common/log.hpp>
#include <transcript/manifest.hpp>

#define STANDARD_HONK_WIDTH 3
// TODO(Cody): "bonk" is short for "both plonk and honk". Just need a short and non-vague temporary name.
namespace bonk {
struct StandardArithmetization {
    /**
     * @brief All of the multivariate polynomials used by the Standard Honk Prover.
     * @details The polynomials are broken into three categories: precomputed, witness, and shifted.
     * This separation must be maintained to allow for programmatic access, but the ordering of the
     * polynomials can be permuted within each category if necessary. Polynomials can also be added
     * or removed (assuming consistency with the prover algorithm) but the constants describing the
     * number of poynomials in each category must be manually updated.
     *
     */
    enum POLYNOMIAL {
        /* --- PRECOMPUTED POLYNOMIALS --- */
        Q_C,
        Q_L,
        Q_R,
        Q_O,
        Q_M,
        SIGMA_1,
        SIGMA_2,
        SIGMA_3,
        ID_1,
        ID_2,
        ID_3,
        LAGRANGE_FIRST,
        LAGRANGE_LAST, // = LAGRANGE_N-1 whithout ZK, but can be less
        /* --- WITNESS POLYNOMIALS --- */
        W_L,
        W_R,
        W_O,
        Z_PERM,
        /* --- SHIFTED POLYNOMIALS --- */
        Z_PERM_SHIFT,
        /* --- --- */
        COUNT // for programmatic determination of NUM_POLYNOMIALS
    };

    static constexpr size_t NUM_POLYNOMIALS = POLYNOMIAL::COUNT;
    static constexpr size_t NUM_SHIFTED_POLYNOMIALS = 1;
    static constexpr size_t NUM_PRECOMPUTED_POLYNOMIALS = 13;
    static constexpr size_t NUM_UNSHIFTED_POLYNOMIALS = NUM_POLYNOMIALS - NUM_SHIFTED_POLYNOMIALS;

    // *** WARNING: The order of this array must be manually updated to match POLYNOMIAL enum ***
    // TODO(luke): This is a temporary measure to associate the above enum with sting tags. Its only needed because
    // the
    // polynomials/commitments in the prover/verifier are stored in maps. This storage could be converted to simple
    // arrays at which point these string tags can be removed.
    inline static const std::array<std::string, 18> ENUM_TO_COMM = {
        "Q_C",           "Q_1",     "Q_2",  "Q_3",  "Q_M",    "SIGMA_1",
        "SIGMA_2",       "SIGMA_3", "ID_1", "ID_2", "ID_3",   "LAGRANGE_FIRST",
        "LAGRANGE_LAST", "W_1",     "W_2",  "W_3",  "Z_PERM", "Z_PERM_SHIFT"
    };
};
} // namespace bonk

namespace honk {
struct StandardHonk {
  public:
    using Arithmetization = bonk::StandardArithmetization;
    using MULTIVARIATE = Arithmetization::POLYNOMIAL;
    // // TODO(Cody): Where to specify? is this polynomial manifest size?
    // static constexpr size_t STANDARD_HONK_MANIFEST_SIZE = 16;
    // TODO(Cody): Maybe relation should be supplied and this should be computed as is done in sumcheck?
    // Then honk::StandardHonk (or whatever we rename it) would become an alias for a Honk flavor with a
    // certain set of parameters, including the relations?
    static constexpr size_t MAX_RELATION_LENGTH = 5;

    // TODO(Cody): should extract this from the parameter pack. Maybe that should be done here?

    // num_sumcheck_rounds = 1 if using quotient polynomials, otherwise = number of sumcheck rounds
};
} // namespace honk