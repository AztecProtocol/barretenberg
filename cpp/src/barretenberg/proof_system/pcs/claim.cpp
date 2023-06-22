#include "claim.hpp"

namespace proof_system::pcs {
template <typename Params>
bool OpeningClaim<Params>::verify(std::shared_ptr<typename Params::CommitmentKey> ck,
                                  const barretenberg::Polynomial<typename Params::Fr>& polynomial) const
{
    Fr real_eval = polynomial.evaluate(opening_pair.challenge);
    if (real_eval != opening_pair.evaluation) {
        return false;
    }
    // Note: real_commitment is a raw type, while commitment may be a linear combination.
    auto real_commitment = ck->commit(polynomial);
    return (real_commitment == commitment);
};

template class OpeningPair<kzg::Params>;
template class OpeningClaim<kzg::Params>;
template class MLEOpeningClaim<kzg::Params>;
template class OpeningPair<ipa::Params>;
template class OpeningClaim<ipa::Params>;
template class MLEOpeningClaim<ipa::Params>;
} // namespace proof_system::pcs
