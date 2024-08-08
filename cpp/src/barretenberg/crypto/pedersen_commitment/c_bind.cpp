#include "c_bind.hpp"
#include "../pedersen_hash/pedersen.hpp"
#include "barretenberg/common/serialize.hpp"
#include "pedersen.hpp"

using namespace bb;

WASM_EXPORT void pedersen_commit(fr::vec_in_buf inputs_buffer, grumpkin::g1::affine_element::out_buf output)
{
    std::vector<grumpkin::fq> to_commit;
    read(inputs_buffer, to_commit);
    grumpkin::g1::affine_element pedersen_commitment = crypto::pedersen_commitment::commit_native(to_commit);

    write(output, pedersen_commitment);
}

WASM_EXPORT void blackbox_pedersen_commit(uint256_t* inputs,
                                          size_t const size,
                                          uint32_t const hash_index,
                                          std::pair<uint256_t, uint256_t>* output)
{
    std::vector<grumpkin::fq> to_hash(inputs, inputs + size);
    crypto::GeneratorContext<curve::Grumpkin> ctx;
    ctx.offset = static_cast<size_t>(hash_index);
    auto r = crypto::pedersen_commitment::commit_native(to_hash, ctx);
    (*output).first = r.x;
    (*output).second = r.y;
}