/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <memory>

#include <rxcpp/rx-observable.hpp>
#include "consensus/consensus_block_cache.hpp"
#include "consensus/yac/impl/yac_gate_impl.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/specified_visitor.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/simulator/simulator_mocks.hpp"
#include "module/shared_model/interface_mocks.hpp"

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
using ::testing::ReturnRefOfCopy;

class YacGateTest : public ::testing::Test {
 public:
  void SetUp() override {
    auto keypair =
        shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

    expected_hash = YacHash(iroha::consensus::Round{1, 1}, "proposal", "block");

    auto block = std::make_shared<MockBlock>();
    EXPECT_CALL(*block, payload())
        .WillRepeatedly(ReturnRefOfCopy(Blob(std::string())));
    EXPECT_CALL(*block, addSignature(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*block, height()).WillRepeatedly(Return(1));
    EXPECT_CALL(*block, txsNumber()).WillRepeatedly(Return(0));
    EXPECT_CALL(*block, createdTime()).WillRepeatedly(Return(1));
    EXPECT_CALL(*block, transactions())
        .WillRepeatedly(
            Return<shared_model::interface::types::TransactionsCollectionType>(
                {}));
    EXPECT_CALL(*block, signatures())
        .WillRepeatedly(
            Return<shared_model::interface::types::SignatureRangeType>({}));
    auto prev_hash = Hash("prev hash");
    EXPECT_CALL(*block, prevHash())
        .WillRepeatedly(testing::ReturnRefOfCopy(prev_hash));
    expected_block = block;

    auto signature = std::make_shared<MockSignature>();
    EXPECT_CALL(*signature, publicKey())
        .WillRepeatedly(ReturnRefOfCopy(expected_pubkey));
    EXPECT_CALL(*signature, signedData())
        .WillRepeatedly(ReturnRefOfCopy(expected_signed));

    expected_hash.block_signature = signature;
    message.hash = expected_hash;
    message.signature = signature;
    commit_message = CommitMessage({message});
    expected_commit = rxcpp::observable<>::just(Answer(commit_message));

    hash_gate = make_unique<MockHashGate>();
    peer_orderer = make_unique<MockYacPeerOrderer>();
    hash_provider = make_shared<MockYacHashProvider>();
    block_creator = make_shared<MockBlockCreator>();
    block_cache = make_shared<ConsensusResultCache>();
  }

  void init() {
    gate = std::make_shared<YacGateImpl>(std::move(hash_gate),
                                         std::move(peer_orderer),
                                         hash_provider,
                                         block_creator,
                                         block_cache);
  }

  PublicKey expected_pubkey{"expected_pubkey"};
  Signed expected_signed{"expected_signed"};
  Hash prev_hash{"prev hash"};
  YacHash expected_hash;
  std::shared_ptr<shared_model::interface::Block> expected_block;
  VoteMessage message;
  CommitMessage commit_message;
  rxcpp::observable<Answer> expected_commit;

  unique_ptr<MockHashGate> hash_gate;
  unique_ptr<MockYacPeerOrderer> peer_orderer;
  shared_ptr<MockYacHashProvider> hash_provider;
  shared_ptr<MockBlockCreator> block_creator;
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
      .WillOnce(Return(rxcpp::observable<>::just(expected_block)));

  init();

  // verify that block we voted for is in the cache
  auto cache_block = block_cache->get();
  ASSERT_EQ(cache_block, expected_block);

  // verify that yac gate emit expected block
  auto gate_wrapper = make_test_subscriber<CallExact>(gate->onOutcome(), 1);
  gate_wrapper.subscribe([this](auto outcome) {
    auto block = boost::get<PairValid>(outcome).block;
    ASSERT_EQ(block, expected_block);

    // verify that gate has put to cache block received from consensus
    auto cache_block = block_cache->get();
    ASSERT_EQ(block, cache_block);
  });

  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given yac gate
 * @when unsuccesfully trying to retrieve peers order
 * @then system will not crash
 */
TEST_F(YacGateTest, YacGateSubscribtionTestFailCase) {
  // yac consensus
  EXPECT_CALL(*hash_gate, vote(_, _)).Times(0);

  EXPECT_CALL(*hash_gate, onOutcome()).Times(0);

  // generate order of peers
  EXPECT_CALL(*peer_orderer, getOrdering(_)).WillOnce(Return(boost::none));

  // make hash from block
  EXPECT_CALL(*hash_provider, makeHash(_)).WillOnce(Return(expected_hash));

  // make blocks
  EXPECT_CALL(*block_creator, on_block())
      .WillOnce(Return(rxcpp::observable<>::just(expected_block)));

  init();
}

/**
 * @given yac gate
 * @when voted on nothing
 * @then cache isn't changed
 */
TEST_F(YacGateTest, AgreementOnNone) {
  EXPECT_CALL(*hash_gate, vote(_, _)).Times(1);
  EXPECT_CALL(*block_creator, on_block())
      .WillOnce(Return(rxcpp::observable<>::empty<
                       std::shared_ptr<shared_model::interface::Block>>()));
  EXPECT_CALL(*peer_orderer, getOrdering(_))
      .WillOnce(Return(ClusterOrdering::create({mk_peer("fake_node")})));

  init();

  ASSERT_EQ(block_cache->get(), nullptr);
  gate->vote(boost::none, boost::none, {});
  ASSERT_EQ(block_cache->get(), nullptr);
}
