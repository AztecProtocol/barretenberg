#include "barretenberg/ecc/curves/bn254/fq.hpp"
#include "barretenberg/ecc/curves/secp256k1/secp256k1.hpp"
#include "barretenberg/ecc/curves/secp256r1/secp256r1.hpp"
#include "barretenberg/stdlib/primitives/circuit_builders/circuit_builders_fwd.hpp"
#include "bigfield_impl.hpp"

namespace bb::stdlib {
// 2 builders x (Fr, Fq)
template class bigfield<UltraCircuitBuilder, secp256r1::FqParams>;
template class bigfield<UltraCircuitBuilder, secp256r1::FrParams>;
template class bigfield<MegaCircuitBuilder, secp256r1::FqParams>;
template class bigfield<MegaCircuitBuilder, secp256r1::FrParams>;
} // namespace bb::stdlib
