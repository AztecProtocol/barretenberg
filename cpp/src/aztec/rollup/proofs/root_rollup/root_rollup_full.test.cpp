#include <common/test.hpp>
#include "index.hpp"
#include "../join_split/index.hpp"
#include "../rollup/index.hpp"
#include "../notes/native/index.hpp"
#include "../../fixtures/test_context.hpp"
#include "../../fixtures/compute_or_load_fixture.hpp"
#include <filesystem>

namespace rollup {
namespace proofs {
namespace root_rollup {

using namespace barretenberg;
using namespace notes::native;
using namespace plonk::stdlib::merkle_tree;

namespace {
#ifdef CI
constexpr bool CIRCUIT_CHANGE_EXPECTED = false;
bool persist = false;
#else
// During development, if the circuit vk hash/gate count is expected to change, set the following to true.
constexpr bool CIRCUIT_CHANGE_EXPECTED = false;
bool persist = false;
#endif

std::shared_ptr<waffle::DynamicFileReferenceStringFactory> srs;
numeric::random::Engine* rand_engine = &numeric::random::get_debug_engine(true);
fixtures::user_context user = fixtures::create_user_context(rand_engine);
join_split::circuit_data js_cd;
proofs::account::circuit_data account_cd;
proofs::claim::circuit_data claim_cd;
proofs::rollup::circuit_data tx_rollup2_cd;
proofs::rollup::circuit_data tx_rollup3_cd;
} // namespace

class root_rollup_full_tests : public ::testing::Test {
  protected:
    static constexpr auto CRS_PATH = "../srs_db/ignition";
    static constexpr auto FIXTURE_PATH = "../src/aztec/rollup/proofs/root_rollup/fixtures";
    static constexpr auto TEST_PROOFS_PATH = "../src/aztec/rollup/proofs/root_rollup/fixtures/test_proofs";

    typedef std::vector<std::vector<std::vector<uint8_t>>> RollupStructure;

    root_rollup_full_tests()
        : context(js_cd, account_cd, claim_cd)
        , js_proofs(get_js_proofs(5))
    {
        rand_engine = &numeric::random::get_debug_engine(true);
        user = fixtures::create_user_context(rand_engine);
    }

    static void SetUpTestCase()
    {
        std::filesystem::create_directories(FIXTURE_PATH);
        std::filesystem::create_directories(TEST_PROOFS_PATH);
        srs = std::make_shared<waffle::DynamicFileReferenceStringFactory>(CRS_PATH);
        account_cd = proofs::account::get_circuit_data(srs);
        js_cd = join_split::get_circuit_data(srs);
        claim_cd = proofs::claim::get_circuit_data(srs);
        tx_rollup2_cd =
            rollup::get_circuit_data(2, js_cd, account_cd, claim_cd, srs, FIXTURE_PATH, true, persist, persist);
        tx_rollup3_cd =
            rollup::get_circuit_data(3, js_cd, account_cd, claim_cd, srs, FIXTURE_PATH, true, persist, persist);
    }

    std::vector<std::vector<uint8_t>> get_js_proofs(uint32_t n)
    {
        std::vector<std::vector<uint8_t>> proofs;
        for (uint32_t i = 0; i < n; ++i) {
            auto js_proof = fixtures::compute_or_load_fixture(TEST_PROOFS_PATH, format("js", i), [&] {
                return context.create_join_split_proof({}, {}, { 100, 50 }, 150);
            });
            proofs.push_back(js_proof);
        }
        return proofs;
    }

    root_rollup_tx create_root_rollup_tx(std::string const& test_name,
                                         uint32_t rollup_id,
                                         rollup::circuit_data const& tx_rollup_cd,
                                         RollupStructure const& rollup_structure)
    {
        std::vector<std::vector<uint8_t>> rollups_data;

        for (auto txs : rollup_structure) {
            auto name = format(test_name, "_rollup", rollups_data.size() + 1);
            auto rollup = rollup::create_rollup_tx(context.world_state, tx_rollup_cd.rollup_size, txs, {}, { 0 });
            auto rollup_data = fixtures::compute_or_load_fixture(
                TEST_PROOFS_PATH, name, [&] { return rollup::verify(rollup, tx_rollup_cd).proof_data; });
            assert(!rollup_data.empty());
            rollups_data.push_back(rollup_data);
        }

        auto old_defi_path = context.world_state.defi_tree.get_hash_path(rollup_id * NUM_INTERACTION_RESULTS_PER_BLOCK);

        return root_rollup::create_root_rollup_tx(context.world_state,
                                                  rollup_id,
                                                  context.world_state.defi_tree.root(),
                                                  old_defi_path,
                                                  rollups_data,
                                                  {},
                                                  { 0 });
    }

    fixtures::TestContext context;
    std::vector<std::vector<uint8_t>> js_proofs;
};

HEAVY_TEST_F(root_rollup_full_tests, test_root_rollup_3x2_and_detect_circuit_change)
{
    static constexpr auto rollups_per_rollup = 3U;

    auto root_rollup_cd = get_circuit_data(rollups_per_rollup, tx_rollup2_cd, srs, FIXTURE_PATH, true, false, false);

    auto old_data_root = context.world_state.data_tree.root();
    auto old_null_root = context.world_state.null_tree.root();
    auto old_root_root = context.world_state.root_tree.root();

    auto tx_data = create_root_rollup_tx(
        "test_root_rollup_3x2", 0, tx_rollup2_cd, { { js_proofs[0], js_proofs[1] }, { js_proofs[2] } });
    auto result = verify(tx_data, root_rollup_cd);
    ASSERT_TRUE(result.verified);

    auto rollup_data = root_rollup_broadcast_data(result.broadcast_data);
    EXPECT_EQ(rollup_data.rollup_id, 0U);
    EXPECT_EQ(rollup_data.rollup_size, 8U);
    EXPECT_EQ(rollup_data.data_start_index, 0U);
    EXPECT_EQ(rollup_data.old_data_root, old_data_root);
    EXPECT_EQ(rollup_data.old_null_root, old_null_root);
    EXPECT_EQ(rollup_data.old_data_roots_root, old_root_root);
    EXPECT_EQ(rollup_data.new_data_root, context.world_state.data_tree.root());
    EXPECT_EQ(rollup_data.new_null_root, context.world_state.null_tree.root());
    EXPECT_EQ(rollup_data.new_data_roots_root, context.world_state.root_tree.root());

    auto inner_data = rollup_data.tx_data[3];
    EXPECT_EQ(inner_data.note_commitment1, fr(0));
    EXPECT_EQ(inner_data.note_commitment2, fr(0));
    EXPECT_EQ(inner_data.nullifier1, fr(0));
    EXPECT_EQ(inner_data.nullifier2, fr(0));
    EXPECT_EQ(inner_data.public_value, fr(0));
    EXPECT_EQ(inner_data.public_owner, fr(0));
    EXPECT_EQ(inner_data.asset_id, fr(0));

    // The below assertions detect changes in the root rollup circuit
    constexpr uint32_t CIRCUIT_GATE_COUNT = 5424685;
    constexpr uint32_t GATES_NEXT_POWER_OF_TWO = 8388608;
    const uint256_t VK_HASH("6f6d58bfe23a31ea15dcc612c6a96d89bf211a192f52386673a0af1ef0fd3745");

    size_t number_of_gates_root_rollup = result.number_of_gates;
    auto vk_hash_root_rollup = result.verification_key->sha256_hash();

    if (!CIRCUIT_CHANGE_EXPECTED) {
        EXPECT_EQ(number_of_gates_root_rollup, CIRCUIT_GATE_COUNT)
            << "The gate count for the root rollup circuit is changed.";
        EXPECT_EQ(from_buffer<uint256_t>(vk_hash_root_rollup), VK_HASH)
            << "The verification key hash for the root rollup circuit is changed.";
    }
    // For the next power of two limit, we need to consider that we reserve four gates for adding
    // randomness/zero-knowledge
    EXPECT_LE(number_of_gates_root_rollup, GATES_NEXT_POWER_OF_TWO - waffle::ComposerBase::NUM_RESERVED_GATES)
        << "You have exceeded the next power of two limit for the root rollup circuit.";
}

HEAVY_TEST_F(root_rollup_full_tests, test_root_rollup_2x3)
{
    static constexpr auto rollups_per_rollup = 2U;

    auto root_rollup_cd = get_circuit_data(rollups_per_rollup, tx_rollup3_cd, srs, FIXTURE_PATH, true, false, false);

    auto old_data_root = context.world_state.data_tree.root();
    auto old_null_root = context.world_state.null_tree.root();
    auto old_root_root = context.world_state.root_tree.root();

    auto tx_data = create_root_rollup_tx("test_root_rollup_2x3", 0, tx_rollup3_cd, { { js_proofs[0] } });
    auto result = verify(tx_data, root_rollup_cd);
    ASSERT_TRUE(result.verified);

    auto rollup_data = root_rollup_broadcast_data(result.broadcast_data);
    EXPECT_EQ(rollup_data.rollup_id, 0U);
    EXPECT_EQ(rollup_data.rollup_size, 8U);
    EXPECT_EQ(rollup_data.data_start_index, 0U);
    EXPECT_EQ(rollup_data.old_data_root, old_data_root);
    EXPECT_EQ(rollup_data.old_null_root, old_null_root);
    EXPECT_EQ(rollup_data.old_data_roots_root, old_root_root);
    EXPECT_EQ(rollup_data.new_data_root, context.world_state.data_tree.root());
    EXPECT_EQ(rollup_data.new_null_root, context.world_state.null_tree.root());
    EXPECT_EQ(rollup_data.new_data_roots_root, context.world_state.root_tree.root());

    for (size_t i = 1; i < rollup_data.tx_data.size(); ++i) {
        auto inner_data = rollup_data.tx_data[i];
        EXPECT_EQ(inner_data.note_commitment1, fr(0));
        EXPECT_EQ(inner_data.note_commitment2, fr(0));
        EXPECT_EQ(inner_data.nullifier1, fr(0));
        EXPECT_EQ(inner_data.nullifier2, fr(0));
        EXPECT_EQ(inner_data.public_value, fr(0));
        EXPECT_EQ(inner_data.public_owner, fr(0));
        EXPECT_EQ(inner_data.asset_id, fr(0));
    }
}

HEAVY_TEST_F(root_rollup_full_tests, test_bad_js_proof_fails)
{
    static constexpr auto rollups_per_rollup = 1U;

    // Create a bad js proof.
    auto bad_proof = join_split::create_noop_join_split_proof(js_cd, context.world_state.data_tree.root(), false);

    // Our inner rollup should fail.
    auto tx_rollup_cd = tx_rollup2_cd;
    auto inner_rollup_tx =
        rollup::create_rollup_tx(context.world_state, tx_rollup_cd.rollup_size, { js_proofs[0], bad_proof });
    Composer inner_composer = Composer(tx_rollup_cd.proving_key, tx_rollup_cd.verification_key, tx_rollup_cd.num_gates);
    rollup::pad_rollup_tx(inner_rollup_tx, tx_rollup_cd.num_txs, tx_rollup_cd.join_split_circuit_data.padding_proof);
    rollup::rollup_circuit(inner_composer, inner_rollup_tx, tx_rollup_cd.verification_keys, tx_rollup_cd.num_txs);
    ASSERT_FALSE(inner_composer.failed());
    auto inner_prover = inner_composer.create_unrolled_prover();
    auto inner_proof = inner_prover.construct_proof();
    auto inner_verifier = inner_composer.create_unrolled_verifier();
    ASSERT_FALSE(inner_verifier.verify_proof(inner_proof));

    // Root rollup should fail.
    auto root_rollup_cd = get_circuit_data(rollups_per_rollup, tx_rollup_cd, srs, FIXTURE_PATH, true, false, false);
    auto root_rollup_tx = root_rollup::create_root_rollup_tx(context.world_state,
                                                             0,
                                                             context.world_state.defi_tree.root(),
                                                             context.world_state.defi_tree.get_hash_path(0),
                                                             { inner_proof.proof_data });
    Composer root_composer =
        Composer(root_rollup_cd.proving_key, root_rollup_cd.verification_key, root_rollup_cd.num_gates);
    pad_root_rollup_tx(root_rollup_tx, root_rollup_cd);
    root_rollup_circuit(root_composer,
                        root_rollup_tx,
                        root_rollup_cd.inner_rollup_circuit_data.rollup_size,
                        root_rollup_cd.rollup_size,
                        root_rollup_cd.inner_rollup_circuit_data.verification_key);
    ASSERT_FALSE(root_composer.failed());
    auto root_prover = root_composer.create_prover();
    auto root_proof = root_prover.construct_proof();
    auto root_verifier = root_composer.create_verifier();
    ASSERT_FALSE(root_verifier.verify_proof(root_proof));
}

} // namespace root_rollup
} // namespace proofs
} // namespace rollup