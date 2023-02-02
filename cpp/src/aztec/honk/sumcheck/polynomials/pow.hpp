#pragma once

namespace honk::sumcheck {

/**
 * @brief Simulates the pow polynomial for both the prover and the verifier.
 *
 * @details Let
 * - m be the number of variables (usually d but no subscript unicode exists)
 * - j be the current Sumcheck round
 * - u₀, …, uⱼ₋₁ the challenges sent by the verifier in the first j rounds.
 * - Note: In this explanation, we reverse the order of the variables to ease the notation.
 *   This all works when we reverse the order of the rounds.
 *
 * We define
 * - pow(X) = ∏ₗ ( (1−Xₗ) + Xₗ⋅ζ^{ 2ˡ } ) is the multi-linear polynomial whose evaluation at the i-th index
 *   of the full hypercube, equals ζⁱ.
 *   We can also see it as the multi-linear extension of the vector (1, ζ, ζ², ...).
 * - Sʲᵢ(Xⱼ) is the univariate of the full relation at edge pair i
 *      i.e. it is the alpha-linear-combination of the relations evaluated in the i-th edge.
 * - powʲᵢ(Xⱼ) = pow(u₀, …, uⱼ₋₁, Xⱼ, i) is pow(u₀, …, uⱼ₋₁, Xⱼ, Xⱼ₊₁, …, Xₘ₋₁) evaluated at the i-th
 *      edge. It can be written a   cⱼ⋅ ( (1−Xⱼ) + ζ 2ʲ ) ⋅ { ζ^{2ʲ⁺¹} }ⁱ, where cⱼ is the `partial_evaluation_constant`
 *      equal to ∏ₗ ( (1−uₗ) + uₗ⋅ζ^{ 2ˡ } ), with l<j.
 * - Sʲ(Xⱼ) = ∑ᵢ powʲ(Xⱼ)⋅ Sʲᵢ(Xⱼ) be the Sumcheck univariate at round j
 *      Notice that we can use the structure of powʲᵢ(Xⱼ) to factorize the linear term ( (1−Xⱼ) + ζ 2ʲ ):
 *      Sʲ(Xⱼ) = ( (1−Xⱼ) + ζ 2ʲ )⋅ ∑ᵢ cⱼ⋅{ ζ^{2ʲ⁺¹} }ⁱ⋅ Sʲᵢ(Xⱼ) = ( (1−Xⱼ) + ζ 2ʲ )⋅Tʲ(Xⱼ).
 *      The univariate Tʲ(Xⱼ) has the same degree as the univariate we would've constructed without the pow
 *      multiplication. The verifier is able to Sʲ(Xⱼ) given  Tʲ(Xⱼ) since it knows ζ.
 *      The verification equation is modified as
 *      - σⱼ₋₁ == Sʲ(0) + Sʲ(1) = Tʲ(0) + ζ^{2ʲ} Tʲ(1)
 *      - σⱼ = ( (1−uⱼ) + ζ^2ʲ )⋅Tʲ(uⱼ)
 */
template <typename FF> struct PowUnivariate {
    FF zeta_pow;
    FF zeta_pow_sqr;
    FF partial_evaluation_constant = FF::one();

    // Initialize with the random zeta
    PowUnivariate(FF zeta_pow)
        : zeta_pow(zeta_pow)
        , zeta_pow_sqr(zeta_pow.sqr())
    {}

    // Compute ( (1−uⱼ) + ζ^2ʲ )
    FF univariate_eval(FF challenge) const { return (FF::one() + (challenge * (zeta_pow - FF::one()))); };

    // Parially evaluate the polynomial in the new challenge, by updating the constant.
    void fold(FF challenge)
    {
        FF current_univariate_eval = univariate_eval(challenge);
        zeta_pow = zeta_pow_sqr;
        zeta_pow_sqr.self_sqr();
        partial_evaluation_constant *= current_univariate_eval;
    }
};
} // namespace honk::sumcheck