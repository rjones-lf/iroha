/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "validation/impl/chain_validator_impl.hpp"

#include <boost/range/adaptor/indirected.hpp>
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/consensus/yac/yac_mocks.hpp"
#include "module/shared_model/interface_mocks.hpp"

using namespace iroha;
using namespace iroha::validation;
using namespace iroha::ametsuchi;

using ::testing::_;
using ::testing::A;
using ::testing::ByRef;
using ::testing::DoAll;
using ::testing::InvokeArgument;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::ReturnRefOfCopy;
using ::testing::SaveArg;

class ChainValidationTest : public ::testing::Test {
 public:
  void SetUp() override {
    validator = std::make_shared<ChainValidatorImpl>(supermajority_checker);
    storage = std::make_shared<MockMutableStorage>();
    query = std::make_shared<MockPeerQuery>();
    peers = std::vector<std::shared_ptr<shared_model::interface::Peer>>();

    auto peer = std::make_shared<MockPeer>();
    EXPECT_CALL(*peer, pubkey())
        .WillRepeatedly(ReturnRefOfCopy(
            shared_model::interface::types::PubkeyType(std::string(32, '0'))));
    peers.push_back(peer);

    auto signature = std::make_shared<MockSignature>();
    EXPECT_CALL(*signature, publicKey())
        .WillRepeatedly(ReturnRefOfCopy(
            shared_model::interface::types::PubkeyType(std::string(32, '0'))));
    signatures.push_back(signature);

    EXPECT_CALL(*block, height()).WillRepeatedly(Return(1));
    EXPECT_CALL(*block, prevHash()).WillRepeatedly(testing::ReturnRef(hash));
    EXPECT_CALL(*block, signatures())
        .WillRepeatedly(Return(signatures | boost::adaptors::indirected));
    EXPECT_CALL(*block, payload())
        .WillRepeatedly(ReturnRefOfCopy(shared_model::crypto::Blob{"blob"}));
  }

  std::shared_ptr<iroha::consensus::yac::MockSupermajorityChecker>
      supermajority_checker =
          std::make_shared<iroha::consensus::yac::MockSupermajorityChecker>();
  std::shared_ptr<ChainValidatorImpl> validator;
  std::shared_ptr<MockMutableStorage> storage;
  std::shared_ptr<MockPeerQuery> query;

  std::vector<std::shared_ptr<shared_model::interface::Signature>> signatures;
  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers;
  shared_model::crypto::Hash hash = shared_model::crypto::Hash("valid hash");
  std::shared_ptr<MockBlock> block = std::make_shared<MockBlock>();
  rxcpp::observable<std::shared_ptr<shared_model::interface::Block>> blocks =
      rxcpp::observable<>::just(
          std::shared_ptr<shared_model::interface::Block>(block));
};

/**
 * @given valid block signed by peers
 * @when apply block
 * @then block is validated
 */
TEST_F(ChainValidationTest, ValidCase) {
  // Valid previous hash, has supermajority, correct peers subset => valid
  shared_model::interface::types::SignatureRangeType block_signatures;
  EXPECT_CALL(*supermajority_checker, hasSupermajority(_, _))
      .WillOnce(DoAll(SaveArg<0>(&block_signatures), Return(true)));

  EXPECT_CALL(*query, getLedgerPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, apply(blocks, _))
      .WillOnce(InvokeArgument<1>(ByRef(*block), ByRef(*query), ByRef(hash)));

  ASSERT_TRUE(validator->validateAndApply(blocks, *storage));
  ASSERT_EQ(block->signatures(), block_signatures);
}

/**
 * @given block with wrong hash
 * @when apply block
 * @then block is not validated
 */
TEST_F(ChainValidationTest, FailWhenDifferentPrevHash) {
  // Invalid previous hash, has supermajority, correct peers subset => invalid
  shared_model::crypto::Hash another_hash =
      shared_model::crypto::Hash(std::string(32, '1'));

  shared_model::interface::types::SignatureRangeType block_signatures;
  ON_CALL(*supermajority_checker, hasSupermajority(_, _))
      .WillByDefault(DoAll(SaveArg<0>(&block_signatures), Return(true)));

  EXPECT_CALL(*query, getLedgerPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, apply(blocks, _))
      .WillOnce(
          InvokeArgument<1>(ByRef(*block), ByRef(*query), ByRef(another_hash)));

  ASSERT_FALSE(validator->validateAndApply(blocks, *storage));
}

/**
 * @given block with wrong peers
 * @when supermajority is not achieved
 * @then block is not validated
 */
TEST_F(ChainValidationTest, FailWhenNoSupermajority) {
  // Valid previous hash, no supermajority, correct peers subset => invalid
  shared_model::interface::types::SignatureRangeType block_signatures;
  EXPECT_CALL(*supermajority_checker, hasSupermajority(_, _))
      .WillOnce(DoAll(SaveArg<0>(&block_signatures), Return(false)));

  EXPECT_CALL(*query, getLedgerPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, apply(blocks, _))
      .WillOnce(InvokeArgument<1>(ByRef(*block), ByRef(*query), ByRef(hash)));

  ASSERT_FALSE(validator->validateAndApply(blocks, *storage));
  ASSERT_EQ(block->signatures(), block_signatures);
}
