/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "torii/impl/command_service_impl.hpp"

#include <gtest/gtest.h>
#include "backend/protobuf/proto_tx_status_factory.hpp"
#include "cryptography/hash.hpp"
#include "cryptography/public_key.hpp"
#include "framework/test_logger.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/ametsuchi/mock_storage.hpp"
#include "module/irohad/ametsuchi/mock_tx_presence_cache.hpp"
#include "module/irohad/torii/torii_mocks.hpp"
#include "module/shared_model/interface_mocks.hpp"

using namespace testing;

class CommandServiceTest : public Test {
 public:
  void SetUp() override {
    transaction_processor_ =
        std::make_shared<iroha::torii::MockTransactionProcessor>();
    storage_ = std::make_shared<iroha::ametsuchi::MockStorage>();

    status_bus_ = std::make_shared<iroha::torii::MockStatusBus>();

    tx_status_factory_ =
        std::make_shared<shared_model::proto::ProtoTxStatusFactory>();
    cache_ = std::make_shared<iroha::torii::CommandServiceImpl::CacheType>();
    tx_presence_cache_ =
        std::make_shared<iroha::ametsuchi::MockTxPresenceCache>();

    log_ = getTestLogger("CommandServiceTest");
  }

  void bindCommandService() {
    command_service_ = std::make_shared<iroha::torii::CommandServiceImpl>(
        transaction_processor_,
        storage_,
        status_bus_,
        tx_status_factory_,
        cache_,
        tx_presence_cache_,
        log_);
  }

  std::shared_ptr<iroha::torii::MockTransactionProcessor>
      transaction_processor_;
  std::shared_ptr<iroha::ametsuchi::MockStorage> storage_;
  std::shared_ptr<iroha::torii::MockStatusBus> status_bus_;
  std::shared_ptr<shared_model::interface::TxStatusFactory> tx_status_factory_;
  std::shared_ptr<iroha::ametsuchi::MockTxPresenceCache> tx_presence_cache_;
  logger::LoggerPtr log_;
  std::shared_ptr<iroha::torii::CommandServiceImpl::CacheType> cache_;
  std::shared_ptr<iroha::torii::CommandService> command_service_;
};

/**
 * @given intialized command service
 *        @and hash with passed consensus but not present in runtime cache
 * @when  invoke getStatusStream by hash
 * @then  verify that code checks run-time and persistent caches for the hash
 *        @and return notReceived status
 */
TEST_F(CommandServiceTest, getStatusStreamWithAbsentHash) {
  using HashType = shared_model::crypto::Hash;
  auto hash = HashType("a");
  auto opt_hash = boost::optional<HashType>("a");
  iroha::ametsuchi::TxCacheStatusType ret_value{
      iroha::ametsuchi::tx_cache_status_responses::Committed{hash}};

  // TODO: 2019-03-13 @muratovv add expect call for runtime cache invocation
  // IR-397
  EXPECT_CALL(*tx_presence_cache_,
              check(Matcher<const shared_model::crypto::Hash &>(_)))
      .Times(1)
      .WillOnce(Return(ret_value));
  EXPECT_CALL(*status_bus_, statuses())
      .Times(2)
      .WillRepeatedly(Return(
          rxcpp::observable<>::empty<iroha::torii::StatusBus::Objects>()));

  bindCommandService();
  auto wrapper = framework::test_subscriber::make_test_subscriber<
      framework::test_subscriber::CallExact>(
      command_service_->getStatusStream(hash), 0);
  ASSERT_TRUE(wrapper.validate());
}
