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

inline bb::fr bn254_fr_decode(bb::fr* f_)
{
    if (!f_->get_bit(255)) {
        f_->self_to_montgomery_form();
        f_->set_bit(255, true);
    }

    auto f = *f_;
    f.set_bit(255, false);
    return f;
}

WASM_EXPORT void blackbox_pedersen_commit(bb::fr* inputs,
                                          size_t const size,
                                          uint32_t const hash_index,
                                          std::pair<bb::fr, bb::fr>* output)
{
    std::vector<grumpkin::fq> to_hash;
    to_hash.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        to_hash.emplace_back(bn254_fr_decode(&inputs[i]));
    }
    // std::vector<grumpkin::fq> to_hash(inputs, inputs + size);
    crypto::GeneratorContext<curve::Grumpkin> ctx;
    ctx.offset = static_cast<size_t>(hash_index);
    auto r = crypto::pedersen_commitment::commit_native(to_hash, ctx);
    r.x.set_bit(255, true);
    r.y.set_bit(255, true);
    (*output).first = r.x;
    (*output).second = r.y;
}