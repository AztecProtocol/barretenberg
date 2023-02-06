#pragma once
#include <stdlib/types/turbo.hpp>
#include <stdlib/hash/pedersen/pedersen.hpp>
#include "../../constants.hpp"

namespace rollup {
namespace proofs {
namespace notes {
namespace circuit {
namespace value {

using namespace plonk::stdlib::types::turbo;

inline auto create_partial_commitment(field_ct const& secret,
                                      point_ct const& owner,
                                      bool_ct const& account_required,
                                      field_ct const& creator_pubkey)
{
    return pedersen::compress({ secret, owner.x, owner.y, account_required, creator_pubkey },
                              GeneratorIndex::VALUE_NOTE_PARTIAL_COMMITMENT);
}

} // namespace value
} // namespace circuit
} // namespace notes
} // namespace proofs
} // namespace rollup