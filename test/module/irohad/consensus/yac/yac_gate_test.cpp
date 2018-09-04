/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <memory>
#include <rxcpp/rx-observable.hpp>

#include "builders/protobuf/transaction.hpp"
#include "consensus/consensus_block_cache.hpp"
#include "consensus/yac/impl/yac_gate_impl.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/specified_visitor.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/simulator/simulator_mocks.hpp"
#include "module/shared_model/builders/protobuf/block.hpp"
#include "module/shared_model/builders/protobuf/common_objects/proto_signature_builder.hpp"

using namespace iroha::consensus::yac;
using namespace iroha::network;
using namespace iroha::simulator;
using namespace framework::test_subscriber;
using namespace shared_model::crypto;
using namespace std;
using iroha::consensus::ConsensusResultCache;

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using ::testing::Return;

class YacGateTest : public ::testing::Test {
 public:
  void SetUp() override {
    auto keypair =
        shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

    expected_hash = YacHash("proposal", "block");
    auto tx = shared_model::proto::TransactionBuilder()
                  .creatorAccountId("account@domain")
                  .setAccountQuorum("account@domain", 1)
                  .createdTime(iroha::time::now())
                  .quorum(1)
                  .build()
                  .signAndAddSignature(keypair)
                  .finish();
    shared_model::proto::Block tmp =
        shared_model::proto::BlockBuilder()
            .height(1)
            .createdTime(iroha::time::now())
            .transactions(std::vector<shared_model::proto::Transaction>{tx})
            .prevHash(Sha3_256::makeHash(Blob("block")))
            .build()
            .signAndAddSignature(keypair)
            .finish();

    expected_block = clone(tmp);
    const auto &signature = *(expected_block->signatures().begin());

    expected_hash.block_signature = clone(signature);
    message.hash = expected_hash;
    message.signature = clone(signature);
    commit_message = CommitMessage({message});
    expected_commit = rxcpp::observable<>::just(Answer(commit_message));

    hash_gate = make_unique<MockHashGate>();
    peer_orderer = make_unique<MockYacPeerOrderer>();
    hash_provider = make_shared<MockYacHashProvider>();
    block_creator = make_shared<MockBlockCreator>();
    block_loader = make_shared<MockBlockLoader>();
    block_cache = make_shared<ConsensusResultCache>();
  }

  void init() {
    gate = std::make_shared<YacGateImpl>(std::move(hash_gate),
                                         std::move(peer_orderer),
                                         hash_provider,
                                         block_creator,
                                         block_loader,
                                         block_cache);
  }

  YacHash expected_hash;
  std::shared_ptr<shared_model::interface::Block> expected_block;
  VoteMessage message;
  CommitMessage commit_message;
  rxcpp::observable<Answer> expected_commit;

  unique_ptr<MockHashGate> hash_gate;
  unique_ptr<MockYacPeerOrderer> peer_orderer;
  shared_ptr<MockYacHashProvider> hash_provider;
  shared_ptr<MockBlockCreator> block_creator;
  shared_ptr<MockBlockLoader> block_loader;
  shared_ptr<ConsensusResultCache> block_cache;

  shared_ptr<YacGateImpl> gate;

 protected:
  YacGateTest() : commit_message(std::vector<VoteMessage>{}) {}
};

/**
 * @given yac gate
 * @when voting for the block @and receiving it on commit
 * @then yac gate will emit this block
 */
TEST_F(YacGateTest, YacGateSubscriptionTest) {
  cout << "----------| BlockCreator (block)=> YacGate (vote)=> "
          "HashGate (commit) => YacGate => on_commit() |----------"
       << endl;

  // yac consensus
  EXPECT_CALL(*hash_gate, vote(expected_hash, _)).Times(1);

  EXPECT_CALL(*hash_gate, onOutcome()).WillOnce(Return(expected_commit));

  // generate order of peers
  EXPECT_CALL(*peer_orderer, getOrdering(_))
      .WillOnce(Return(ClusterOrdering::create({mk_peer("fake_node")})));

  // make hash from block
  EXPECT_CALL(*hash_provider, makeHash(_)).WillOnce(Return(expected_hash));

  // make blocks
  EXPECT_CALL(*block_creator, on_block())
      .WillOnce(Return(
          rxcpp::observable<>::just<shared_model::interface::BlockVariant>(
              expected_block)));

  init();

  // verify that block we voted for is in the cache
  ASSERT_NO_THROW({
    auto cache_block = boost::apply_visitor(
        framework::SpecifiedVisitor<decltype(expected_block)>(),
        *block_cache->get());
    ASSERT_EQ(*cache_block, *expected_block);
  });

  // verify that yac gate emit expected block
  auto gate_wrapper = make_test_subscriber<CallExact>(gate->on_commit(), 1);
  gate_wrapper.subscribe([this](const auto &block_variant) {
    ASSERT_NO_THROW({
      auto block = boost::apply_visitor(
          framework::SpecifiedVisitor<decltype(expected_block)>(),
          block_variant);
      ASSERT_EQ(*block, *expected_block);

      // verify that gate has put to cache block received from consensus
      ASSERT_EQ(block_variant, *block_cache->get());
    });
  });

  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given yac gate
 * @when unsuccesfully trying to retrieve peers order
 * @then system will not crash
 */
TEST_F(YacGateTest, YacGateSubscribtionTestFailCase) {
  cout << "----------| Fail case of retrieving cluster order  |----------"
       << endl;

  // yac consensus
  EXPECT_CALL(*hash_gate, vote(_, _)).Times(0);

  EXPECT_CALL(*hash_gate, onOutcome()).Times(0);

  // generate order of peers
  EXPECT_CALL(*peer_orderer, getOrdering(_)).WillOnce(Return(boost::none));

  // make hash from block
  EXPECT_CALL(*hash_provider, makeHash(_)).WillOnce(Return(expected_hash));

  // make blocks
  EXPECT_CALL(*block_creator, on_block())
      .WillOnce(Return(
          rxcpp::observable<>::just<shared_model::interface::BlockVariant>(
              expected_block)));

  init();
}

/**
 * @given yac gate
 * @when voting for one block @and receiving another
 * @then yac gate will load the block, for which consensus voted, @and emit it
 */
TEST_F(YacGateTest, LoadBlockWhenDifferentCommit) {
  // make blocks
  EXPECT_CALL(*block_creator, on_block())
      .WillOnce(Return(
          rxcpp::observable<>::just<shared_model::interface::BlockVariant>(
              expected_block)));

  // make hash from block
  EXPECT_CALL(*hash_provider, makeHash(_)).WillOnce(Return(expected_hash));

  // generate order of peers
  EXPECT_CALL(*peer_orderer, getOrdering(_))
      .WillOnce(Return(ClusterOrdering::create({mk_peer("fake_node")})));

  EXPECT_CALL(*hash_gate, vote(expected_hash, _)).Times(1);

  // create another block, which will be "received", and generate a commit
  // message with it
  auto keypair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
  auto tx = shared_model::proto::TransactionBuilder()
                .creatorAccountId("doge@meme")
                .setAccountQuorum("doge@meme", 1)
                .createdTime(iroha::time::now())
                .quorum(1)
                .build()
                .signAndAddSignature(keypair)
                .finish();
  std::shared_ptr<shared_model::interface::Block> actual_block =
      clone(shared_model::proto::BlockBuilder()
                .height(1)
                .createdTime(iroha::time::now())
                .transactions(std::vector<shared_model::proto::Transaction>{tx})
                .prevHash(Sha3_256::makeHash(Blob("actual_block")))
                .build()
                .signAndAddSignature(keypair)
                .finish());
  const auto &signature = *(actual_block->signatures().begin());

  message.hash = YacHash("actual_proposal", "actual_block");
  message.signature = clone(signature);
  commit_message = CommitMessage({message});
  expected_commit = rxcpp::observable<>::just(Answer(commit_message));

  // yac consensus
  EXPECT_CALL(*hash_gate, onOutcome()).WillOnce(Return(expected_commit));

  // convert yac hash to model hash
  EXPECT_CALL(*hash_provider, toModelHash(message.hash))
      .WillOnce(Return(actual_block->hash()));

  // load different block
  auto sig = actual_block->signatures().begin();
  auto &pubkey = sig->publicKey();
  EXPECT_CALL(*block_loader, retrieveBlock(pubkey, actual_block->hash()))
      .WillOnce(Return(shared_model::interface::BlockVariant{actual_block}));

  init();

  // verify that block we voted for is in the cache
  ASSERT_NO_THROW({
    auto cache_block = boost::apply_visitor(
        framework::SpecifiedVisitor<decltype(expected_block)>(),
        *block_cache->get());
    ASSERT_EQ(*cache_block, *expected_block);
  });

  // verify that yac gate emit expected block
  std::shared_ptr<shared_model::interface::BlockVariant> yac_emitted_block;
  auto gate_wrapper = make_test_subscriber<CallExact>(gate->on_commit(), 1);
  gate_wrapper.subscribe(
      [actual_block, &yac_emitted_block](const auto &block_variant) {
        ASSERT_NO_THROW({
          auto block = boost::apply_visitor(
              framework::SpecifiedVisitor<decltype(expected_block)>(),
              block_variant);
          ASSERT_EQ(*block, *actual_block);

          // memorize the block came from the consensus for future
          yac_emitted_block =
              std::make_shared<shared_model::interface::BlockVariant>(
                  block_variant);
        });
      });

  // verify that block, which was received from consensus, is now in the
  // cache
  ASSERT_EQ(*block_cache->get(), *yac_emitted_block);

  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given yac gate
 * @when receives new commit different to the one it voted for
 * @then polls nodes for the block with corresponding hash until it succeed,
 * (receiving none on the first poll)
 */
TEST_F(YacGateTest, LoadBlockWhenDifferentCommitFailFirst) {
  // Vote for block => receive different block => load committed block

  // make blocks
  EXPECT_CALL(*block_creator, on_block())
      .WillOnce(Return(
          rxcpp::observable<>::just<shared_model::interface::BlockVariant>(
              expected_block)));

  // make hash from block
  EXPECT_CALL(*hash_provider, makeHash(_)).WillOnce(Return(expected_hash));

  // generate order of peers
  EXPECT_CALL(*peer_orderer, getOrdering(_))
      .WillOnce(Return(ClusterOrdering::create({mk_peer("fake_node")})));

  EXPECT_CALL(*hash_gate, vote(expected_hash, _)).Times(1);

  // expected values
  expected_hash = YacHash("actual_proposal", "actual_block");

  message.hash = expected_hash;

  commit_message = CommitMessage({message});
  expected_commit = rxcpp::observable<>::just(Answer(commit_message));

  // yac consensus
  EXPECT_CALL(*hash_gate, onOutcome()).WillOnce(Return(expected_commit));

  // convert yac hash to model hash
  EXPECT_CALL(*hash_provider, toModelHash(expected_hash))
      .WillOnce(Return(expected_block->hash()));

  // load block
  auto sig = expected_block->signatures().begin();
  auto &pubkey = sig->publicKey();
  EXPECT_CALL(*block_loader, retrieveBlock(pubkey, expected_block->hash()))
      .WillOnce(Return(boost::none))
      .WillOnce(Return(shared_model::interface::BlockVariant{expected_block}));

  init();

  // verify that yac gate emit expected block
  auto gate_wrapper = make_test_subscriber<CallExact>(gate->on_commit(), 1);
  gate_wrapper.subscribe([this](const auto &block_variant) {
    ASSERT_NO_THROW({
      auto block = boost::apply_visitor(
          framework::SpecifiedVisitor<decltype(expected_block)>(),
          block_variant);
      ASSERT_EQ(*block, *expected_block);
    });
  });

  ASSERT_TRUE(gate_wrapper.validate());
}
