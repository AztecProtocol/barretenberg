#include "c_bind.hpp"
#include "barretenberg/common/mem.hpp"
#include "barretenberg/common/serialize.hpp"
#include "barretenberg/common/streams.hpp"
#include "pedersen.hpp"

using namespace bb;

WASM_EXPORT void pedersen_hash(fr::vec_in_buf inputs_buffer, uint32_t const* hash_index, fr::out_buf output)
{
    std::vector<grumpkin::fq> to_hash;
    read(inputs_buffer, to_hash);
    crypto::GeneratorContext<curve::Grumpkin> ctx;
    ctx.offset = static_cast<size_t>(ntohl(*hash_index));
    auto r = crypto::pedersen_hash::hash(to_hash, ctx);
    fr::serialize_to_buffer(r, output);
}

WASM_EXPORT void pedersen_hashes(fr::vec_in_buf inputs_buffer, uint32_t const* hash_index, fr::out_buf output)
{
    std::vector<grumpkin::fq> to_hash;
    read(inputs_buffer, to_hash);
    crypto::GeneratorContext<curve::Grumpkin> ctx;
    ctx.offset = static_cast<size_t>(ntohl(*hash_index));
    const size_t numHashes = to_hash.size() / 2;
    std::vector<grumpkin::fq> results;
    size_t count = 0;
    while (count < numHashes) {
        auto r = crypto::pedersen_hash::hash({ to_hash[count * 2], to_hash[count * 2 + 1] }, ctx);
        results.push_back(r);
        ++count;
    }
    write(output, results);
}

WASM_EXPORT void pedersen_hash_buffer(uint8_t const* input_buffer, uint32_t const* hash_index, fr::out_buf output)
{
    std::vector<uint8_t> to_hash;
    read(input_buffer, to_hash);
    crypto::GeneratorContext<curve::Grumpkin> ctx;
    ctx.offset = static_cast<size_t>(ntohl(*hash_index));
    auto r = crypto::pedersen_hash::hash_buffer(to_hash, ctx);
    fr::serialize_to_buffer(r, output);
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

WASM_EXPORT void blackbox_pedersen_hash(bb::fr* inputs, size_t const size, uint32_t const hash_index, bb::fr* output)
{
    std::vector<grumpkin::fq> to_hash;
    to_hash.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        to_hash.emplace_back(bn254_fr_decode(&inputs[i]));
    }
    crypto::GeneratorContext<curve::Grumpkin> ctx;
    ctx.offset = static_cast<size_t>(hash_index);
    auto r = crypto::pedersen_hash::hash(to_hash, ctx);
    *output = r;
    output->set_bit(255, true);
}