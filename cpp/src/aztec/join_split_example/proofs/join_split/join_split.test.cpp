#include "../../constants.hpp"
#include "../inner_proof_data/inner_proof_data.hpp"
#include "index.hpp"
#include "../notes/native/index.hpp"
#include "join_split_example/proofs/join_split/join_split_circuit.hpp"
#include <common/streams.hpp>
#include <common/test.hpp>
#include <gtest/gtest.h>
#include <proof_system/proving_key/serialize.hpp>
#include <stdlib/merkle_tree/index.hpp>

namespace join_split_example::proofs::join_split {

template <typename Composer> class join_split : public testing::Test {

  protected:
    join_split_example::fixtures::user_context user;
    std::unique_ptr<MemoryStore> store;
    std::unique_ptr<MerkleTree<MemoryStore>> tree;

    virtual void SetUp()
    {
        store = std::make_unique<MemoryStore>();
        tree = std::make_unique<MerkleTree<MemoryStore>>(*store, 32);
        user = join_split_example::fixtures::create_user_context();
    }

  public:
    /**
     * Return a join split tx with 0-valued input notes.
     */
    join_split_tx zero_input_setup()
    {
        using namespace join_split_example::proofs::notes::native;

        value::value_note input_note1 = {
            0, 0, false, user.owner.public_key, user.note_secret, 0, fr::random_element()
        };
        value::value_note input_note2 = {
            0, 0, false, user.owner.public_key, user.note_secret, 0, fr::random_element()
        };
        auto input_nullifier1 = compute_nullifier(input_note1.commit(), user.owner.private_key, false);
        auto input_nullifier2 = compute_nullifier(input_note2.commit(), user.owner.private_key, false);
        value::value_note output_note1 = { 0, 0, false, user.owner.public_key, user.note_secret, 0, input_nullifier1 };
        value::value_note output_note2 = { 0, 0, false, user.owner.public_key, user.note_secret, 0, input_nullifier2 };

        join_split_tx tx;
        tx.proof_id = ProofIds::SEND;
        tx.public_value = 0;
        tx.public_owner = 0;
        tx.asset_id = 0;
        tx.num_input_notes = 0;
        tx.input_index = { 0, 1 };
        tx.old_data_root = tree->root();
        tx.input_path = { tree->get_hash_path(0), tree->get_hash_path(1) };
        tx.input_note = { input_note1, input_note2 };
        tx.output_note = { output_note1, output_note2 };
        tx.partial_claim_note.input_nullifier = 0;
        tx.account_private_key = user.owner.private_key;
        tx.alias_hash = join_split_example::fixtures::generate_alias_hash("penguin");
        tx.account_required = false;
        tx.account_note_index = 0;
        tx.account_note_path = tree->get_hash_path(0);
        tx.signing_pub_key = user.signing_keys[0].public_key;
        tx.backward_link = 0;
        tx.allow_chain = 0;
        return tx;
    }
};

using ComposerTypes =
    testing::Types<waffle::UltraComposer, waffle::TurboComposer, waffle::StandardComposer, honk::StandardHonkComposer>;

TYPED_TEST_SUITE(join_split, ComposerTypes);

// This is derived from the Aztec Connect test
// join_split_tests.test_deposit_construct_proof
TYPED_TEST(join_split, deposit)
{
    join_split_tx tx = TestFixture::zero_input_setup();
    tx.proof_id = ProofIds::DEPOSIT;
    tx.public_value = 10;
    tx.public_owner = fr::random_element();
    tx.output_note[0].value = 7;

    /**
     * DEPOSIT tx represents:
     *   - public_value = 10
     *   - out1 = 7
     *   - fee = 3
     */

    // sign_and_create_proof
    tx.signature = sign_join_split_tx(tx, TestFixture::user.owner);

    auto composer = Composer();
    join_split_circuit(composer, tx);

    BenchmarkInfoCollator benchmark_collator;
    Timer timer;
    auto prover = composer.create_prover();
    auto build_time = timer.toString();
    benchmark_collator.benchmark_info_deferred(
        GET_COMPOSER_NAME_STRING(Composer::type), "Core", "join split", "Build time", build_time);

    auto proof = prover.construct_proof();

    auto verifier = composer.create_verifier();
    bool verified = verifier.verify_proof(proof);

    ASSERT_TRUE(verified);
}
} // namespace join_split_example::proofs::join_split