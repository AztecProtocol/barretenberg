<!-- $$
\newcommand{\shift}{\circlearrowleft}
\newcommand{\Fold}{\text{Fold}}
\newcommand{\GeminiFold}{\text{GeminiFold}}
\newcommand{\GeminiUnfold}{\text{GeminiUnfold}}
\newcommand{\com}{\text{com}}
\newcommand{\comGem}{\com_{\text{Gemini}}}
\newcommand{\comKZG}{\com_{\text{KZG}}}
\newcommand{\pow}{\text{pow}}
\newcommand{\idx}{\text{idx}}
\newcommand{\arith}{\text{arith}}
\newcommand{\perm}{\text{perm}}
\newcommand{\Honk}{\text{Honk}}
\newcommand{\GP}{\text{Prod}}
$$

# Minimal Sketch / Outline of Honk
The goal here is to succinctly describe Honk without zero knowledge and using a simple version of Gemini in a way that will feel familiar (I hope) to those who have spent time with the PlonK paper. To that end, I black box / am imprecise about some of the more complex constructions. The description here was based off of: Adrian's thesis, the PlonK paper, and Tohru's most basic description of Gemini.

This is a rough draft. Feel free to add and/or comment, but please don't increase the scope too much.

Some notation:
$n = 2^d$, $H = \{0,1\}$

We extend our common notations by defining $[F]_1 := \comGem(F)$ for $F$ a multivariate polynomial and $[f]_1 := \comKZG(f)$ for $f$ a univariate polynomial.


# SRS
We need an SRS where the $i$-th element is $[x^i]_1$.

# Preprocessing
- Commit to selectors $q_{m,l,r,o,c}$.
- Commit to permutation polynomials $T_{1,2,3}$.
- _Optional_: Commit to $L_0$, $\mathrm{ID} = [0,1,\ldots,n-1]$ to simplify verifier

# Prover
- Commit to wires $w_{1,2,3}$.
- CHALLENGE: $\beta, \gamma$
- Generate permutation grand product polynomial $\GP$, modified to include public inputs as described [here](https://raw.githubusercontent.com/arielgabizon/plonk-addendum/master/plonk-pubinputs.pdf), and and commit to it. ($\GP^\shift$ is just a reindexed copy of $\GP$.)
- CHALLENGE: $\zeta, \alpha$
- Sumcheck the Honk constraint polynomial $$ \Honk = \pow_\zeta \cdot (\Honk_\arith + \alpha \Honk_{\perm, 1} + \alpha^2 \Honk_{\perm, 2})$$
  - For $\ell$ from $d$ to $1$:
    - Construct univariate $S_\ell$ and output coefficients (or evals? Rep TBD)
    - CHALLENGE: $u_\ell$
    - Partially evaluate multivariate.
  - Send multilinear evaluations (at $u$) $\bar q_{m,l,r,o,c}$, $\overline{T}_{1,2,3}$, $\overline{\GP}$, $\bar{ w}_{l,r,o}$, ($\bar{L}_0$, $\overline{\mathrm{ID}}$), and shifted evaluation $\overline{\GP}^\shift$. 
    - Relabel the evaluations to $v_0,\ldots,v_{13},v_{14}^\shift$, and polynomials to $F_0, \ldots, F_{13}, F^\shift_{14}$, where $F^\shift_{14} = \GP$.
- Gemini:
  - CHALLENGE: $\rho$ for batching the evaluations
  - Compute folded polynomials $\Fold^{(0)}, \ldots, \Fold^{(d-1)}$:
    - Compute $B = \sum_{i=0}^{13} \rho^i F_i$ and $B^\shift = \rho^{14} F^\shift_{14}$
    - $\Fold^{(0)}_i = B_i + B^\shift_{i+1}$ for $i=0,1,\ldots,{n-1}$ (letting $B^\shift_n =0$)
    - $\Fold^{(\ell+1)} = \GeminiFold(\Fold^{(\ell)}, u_\ell)$ for $\ell = 0,\ldots, d-1$
  - Commit to $\Fold^{(1)}, \ldots, \Fold^{(d-1)}$
  - Add to transcript: $[\Fold^{(1)}], \ldots, [\Fold^{(d-1)}]$
  - CHALLENGE: $r$
  - Compute partially evaluated folded polynomials
    $$ \Fold^{(0)}_{r} = B + \tfrac{1}{r}B^\shift, \Fold^{(0)}_{-r} = B + \tfrac{1}{-r}B^\shift$$
  - Send univariate openings 
    $$\begin{gathered}
    a_{0} = \Fold^{(0)}_{-r}(-r) \\
    a_{1} = \Fold^{(1)}(-r^{2}) \\
    \ldots \\
    a_{d-1} = \Fold^{(d-1)}(-r^{2^{d-1}})
    \end{gathered}$$
  - Evaluate $\hat{a}_{0} = \Fold^{(0)}_{r}(r)$
  - Add to transcript: $a_0, \ldots, a_{d-1}$, $\hat{a}_{0}$, and $[\Fold_r^{(0)}], [\Fold_{-r}^{(0)}]$
- Shplonk (simplifed):
    - CHALLENGE: $\nu$ for batching the quotients
    - Compute and commit to batched quotient 
        $$ Q(X) = \frac{\Fold^{(0)}_{r}(X) - \hat{a}_{0}}{X-r} + \nu\frac{\Fold^{(0)}_{-r}(X) - a_{0}}{X+r} + \sum_{\ell=1}^{d-1} \nu^{\ell+1} \frac{\Fold^{(\ell)}(X) - a_{\ell}}{X+r^{2^{\ell}}}$$
    - CHALLENGE: $z$ 
    - Compute the partial evaluation of $Q$ 
        $$ Q_z(X) =  \frac{\Fold^{(0)}_{r}(X) - \hat{a}_{0}}{z-r} + \nu\frac{\Fold^{(0)}_{-r}(X) - a_{0}}{z+r} + \sum_{\ell=1}^{d-1} \nu^{\ell+1} \frac{\Fold^{(\ell)}(X) - a_{\ell}}{z+r^{2^{\ell}}}$$
    - Compute and commit to 
        $$ W(X) = \frac{Q(X)-Q_z(X)}{X-z}$$

    

# Verifier
- Validate commmitments lie on curve and validate that evaluations lie in field.
- Compute challenges.
- Compute $\pow_\zeta(u)$, $\idx(u)$, $L_2(u)$.
- Sumcheck: Set $\sigma_d = 0$. 
  - For $\ell$ from $d$ to $1$:
    - Check $\sigma_\ell = S_\ell(0) + S_\ell(1)$
    - Evaluate $S_\ell$ at challenge to get $\sigma_{\ell-1}$ for $\ell > 1$.
  - Check that $\sigma_0 = \pow_\zeta(u) (\overline{\Honk}_\arith +
                                          \alpha \overline{\Honk}_{\perm, 1} + \alpha^2 \overline{\Honk}_{\perm, 2})$, 
    where each $\overline{\Honk}_\star$ is the expected value of $\Honk_\star$ given the purported (i.e., provider-supplied) and verifier-computed evaluations of the components. This uses all of the sumcheck evaluations as well as $\idx(u)$ and $L_2(u)$.
- Set $v = \sum_{i=0}^{13} \rho^i v_i + \rho^{14}v^\shift_{14}$ as the batched multi-linear evaluation
- Compute simulated folded commitments 
    $$  [\Fold^{(0)}_{r}] = [B] + \tfrac{1}{r}[B^\shift], [\Fold^{(0)}_{-r}] = [B] + \tfrac{1}{-r}[B^\shift],$$
    where $[B] = \sum_{i=0}^{13} \rho^i [F_i]$ and $[B^\shift] = \rho^{14} [F^\shift_{14}]$
- Compute purported evaluation of $\Fold^{(0)}(r)$ as $\hat{a}_{0} = \GeminiUnfold(u, r, a_{0},\ldots,a_{d-1},v)$
- Compute simulated commitment to $[Q_z]$ 
    $$[Q_z] =  \frac{[\Fold^{(0)}_{r}] - \hat{a}_{0}[1]}{z-r} + \nu\frac{[\Fold^{(0)}_{-r}] - a_{0}[1]}{z+r} + \sum_{\ell=1}^{d-1} \nu^{\ell+1} \frac{[\Fold^{(\ell)}] - a_{\ell}[1]}{z+r^{2^{\ell}}}$$
- Check that 
   $$ e([Q] - [Q_z] + z[W], [1]_2])  = e([W], [x]_2) $$
       (or rearrange for efficiency as in the PlonK paper).


-------------------------------------------------------------------------------------------



Here is a table representing the prover's algorithm:


| protocol                    | transcript                                                                      | challenge         |
|-----------------------------|---------------------------------------------------------------------------------|-------------------|
| Commit to wires             | $[w_l]_1$, $[w_r]_1$, $[w_o]_1$                                                 |                   |
|                             |                                                                                 | $\beta, \gamma$   |
| Compute grand product polys | $[\GP]_1$                                                                       |                   |
|                             |                                                                                 | $\zeta$, $\alpha$ |
| Sumcheck Round $1$          | $S_{d}(X)$                                                                      |                   |
|                             |                                                                                 | $u_d$             |
|                             | $\vdots$                                                                        | $\vdots$          |
| Sumcheck Round $d$          | $S_{1}(X)$                                                                      |                   |
|                             |                                                                                 | $u_1$             |
| Sumcheck evaluations        | $\bar q_{m,l,r,o,c}$, $\bar T_{1,2,3}$, $\bar S^{\_, \shift}$, $\bar w_{l,r,o}$ |                   |
|                             |                                                                                 | $\rho$            |
| Gemini folding              | $[\Fold^{(1)}]_1$,..., $[\Fold^{(d-1)}]_1$                                      |                   |
|                             |                                                                                 | $r$               |
| Gemini evaluations          | $a_0, \ldots,a_{d-1}$                                                           |                   |
|                             |                                                                                 | $\nu$             |
| Shplonk opening             | $[Q]_1$                                                                         |                   |
|                             |                                                                                 | $z$               |
| KZG quotient                | $[W]_1$                                                                         |                   | -->
