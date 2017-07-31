/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <gtest/gtest.h>
#include "main/server_runner.hpp"
#include "mock_classes.hpp"
#include "torii/command_service.hpp"
#include "torii/processor/query_processor_impl.hpp"
#include "torii/processor/transaction_processor_impl.hpp"
#include "torii_utils/query_client.hpp"

constexpr const char *Ip = "0.0.0.0";
constexpr int Port = 50051;

constexpr size_t TimesToriiBlocking = 5;
constexpr size_t TimesToriiNonBlocking = 5;
constexpr size_t TimesFind = 1;

using ::testing::Return;
using ::testing::A;
using ::testing::_;
using ::testing::AtLeast;

class ToriiServiceTest : public testing::Test {
 public:
  virtual void SetUp() {
    runner = new ServerRunner(Ip, Port);
    th = std::thread([ this, runner = runner ] {
      // ----------- Command Service --------------

      auto tx_processor =
          iroha::torii::TransactionProcessorImpl(this->pcsMock, this->svMock);
      iroha::model::converters::PbTransactionFactory pb_tx_factory;
      auto command_service =  // std::unique_ptr<torii::CommandService>(new
                              // torii::CommandService(pb_tx_factory,
                              // tx_processor));
          std::make_unique<torii::CommandService>(pb_tx_factory, tx_processor);

      //----------- Query Service ----------

      iroha::model::QueryProcessingFactory qpf(this->wsv_query,
                                               this->block_query);

      iroha::torii::QueryProcessorImpl qpi(qpf, this->svMock);

      iroha::model::converters::PbQueryFactory pb_query_factory;
      iroha::model::converters::PbQueryResponseFactory pb_query_resp_factory;

      auto query_service = std::make_unique<torii::QueryService>(
          pb_query_factory, pb_query_resp_factory, qpi);

      //----------- Server run ----------------
      runner->run(std::move(command_service), std::move(query_service));
    });

    runner->waitForServersReady();
  }

  virtual void TearDown() {
    runner->shutdown();
    delete runner;
    th.join();
  }

  ServerRunner *runner;
  std::thread th;

  PCSMock pcsMock;
  StatelessValidatorMock svMock;
  WsvQueryMock wsv_query;
  BlockQueryMock block_query;
};

/**
 * Test for error response
 */

TEST_F(ToriiServiceTest, FindWhenResponseInvalid) {
  EXPECT_CALL(svMock, validate(A<const iroha::model::Query &>()))
      .WillRepeatedly(Return(false));

  iroha::protocol::QueryResponse response;
  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account()->set_account_id("accountB");
  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Must return Error Response
  ASSERT_EQ(response.error_response().reason(), "Not valid");
}

/**
 * Tests for account response
 */

TEST_F(ToriiServiceTest, FindAccountWhenStatefulInvalid) {
  EXPECT_CALL(svMock, validate(A<const iroha::model::Query &>()))
      .WillRepeatedly(Return(true));

  iroha::model::Account account;
  account.account_id = "accountB";

  EXPECT_CALL(wsv_query, getAccount(_)).WillRepeatedly(Return(account));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account()->set_account_id("accountB");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because stateless valid
  ASSERT_NE(response.error_response().reason(), "Not valid");
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account
  ASSERT_EQ(response.error_response().reason(), "Not valid query");
}

TEST_F(ToriiServiceTest, FindAccountWhenValid) {
  EXPECT_CALL(svMock, validate(A<const iroha::model::Query &>()))
      .WillRepeatedly(Return(true));

  iroha::model::Account account;
  account.account_id = "accountA";

  EXPECT_CALL(wsv_query, getAccount(_)).WillRepeatedly(Return(account));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account()->set_account_id("accountA");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because stateless valid
  ASSERT_NE(response.error_response().reason(), "Not valid");
  // Must be stateful valid
  ASSERT_NE(response.error_response().reason(), "Not valid query");
  ASSERT_EQ(response.account_response().account().account_id(), "accountA");
}

/**
 * Test for account asset response
 */

TEST_F(ToriiServiceTest, FindAccountAssetWhenStatefulInvalid) {
  EXPECT_CALL(svMock, validate(A<const iroha::model::Query &>()))
      .WillRepeatedly(Return(true));

  iroha::model::Account account;
  account.account_id = "accountB";

  iroha::model::AccountAsset account_asset;
  account_asset.account_id = "accountB";
  account_asset.asset_id = "usd";
  account_asset.balance = 100;

  iroha::model::Asset asset;
  asset.asset_id = "usd";
  asset.domain_id = "USA";
  asset.precision = 2;

  EXPECT_CALL(wsv_query, getAccount(_)).WillRepeatedly(Return(account));
  EXPECT_CALL(wsv_query, getAccountAsset(_, _)).WillRepeatedly(Return(account_asset));
  EXPECT_CALL(wsv_query, getAsset(_)).WillRepeatedly(Return(asset));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account_assets()->set_account_id("accountB");
  query.mutable_get_account_assets()->set_asset_id("usd");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because stateless valid
  ASSERT_NE(response.error_response().reason(), "Not valid");
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account asset
  ASSERT_EQ(response.error_response().reason(), "Not valid query");
}

TEST_F(ToriiServiceTest, FindAccountAssetWhenValid) {
  EXPECT_CALL(svMock, validate(A<const iroha::model::Query &>()))
      .WillRepeatedly(Return(true));

  iroha::model::Account account;
  account.account_id = "accountA";

  iroha::model::AccountAsset account_asset;
  account_asset.account_id = "accountA";
  account_asset.asset_id = "usd";
  account_asset.balance = 100;

  iroha::model::Asset asset;
  asset.asset_id = "usd";
  asset.domain_id = "USA";
  asset.precision = 2;

  EXPECT_CALL(wsv_query, getAccount(_)).WillRepeatedly(Return(account));
  EXPECT_CALL(wsv_query, getAccountAsset(_, _)).WillRepeatedly(Return(account_asset));
  EXPECT_CALL(wsv_query, getAsset(_)).WillRepeatedly(Return(asset));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account_assets()->set_account_id("accountA");
  query.mutable_get_account_assets()->set_asset_id("usd");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because stateless valid
  ASSERT_NE(response.error_response().reason(), "Not valid");
  // Must be valid
  ASSERT_NE(response.error_response().reason(), "Not valid query");

  // Check if the fields in account asset response are correct
  ASSERT_EQ(response.account_assets_response().account_asset().asset_id(), account_asset.asset_id);
  ASSERT_EQ(response.account_assets_response().account_asset().account_id(), account_asset.account_id);
  ASSERT_EQ(response.account_assets_response().account_asset().balance(), account_asset.balance);
}

/**
 * Test for signatories response
 */

TEST_F(ToriiServiceTest, FindSignatoriesWhenStatefulInvalid) {
  EXPECT_CALL(svMock, validate(A<const iroha::model::Query &>()))
      .WillRepeatedly(Return(true));

  iroha::model::Account account;
  account.account_id = "accountB";

  iroha::ed25519::pubkey_t pubkey;
  std::fill(pubkey.begin(), pubkey.end(), 0x1);
  std::vector<iroha::ed25519::pubkey_t> keys;
  keys.push_back(pubkey);

  EXPECT_CALL(wsv_query, getAccount(_)).WillRepeatedly(Return(account));
  EXPECT_CALL(wsv_query, getSignatories(_)).WillRepeatedly(Return(keys));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account_signatories()->set_account_id("accountB");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because stateless valid
  ASSERT_NE(response.error_response().reason(), "Not valid");
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account
  ASSERT_EQ(response.error_response().reason(), "Not valid query");
}

TEST_F(ToriiServiceTest, FindSignatoriesWhenValid) {
  EXPECT_CALL(svMock, validate(A<const iroha::model::Query &>()))
      .WillRepeatedly(Return(true));

  iroha::model::Account account;
  account.account_id = "accountA";

  iroha::ed25519::pubkey_t pubkey;
  std::fill(pubkey.begin(), pubkey.end(), 0x1);
  std::vector<iroha::ed25519::pubkey_t> keys;
  keys.push_back(pubkey);

  EXPECT_CALL(wsv_query, getAccount(_)).WillRepeatedly(Return(account));
  EXPECT_CALL(wsv_query, getSignatories(_)).WillRepeatedly(Return(keys));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account_signatories()->set_account_id("accountA");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because stateless valid
  ASSERT_NE(response.error_response().reason(), "Not valid");
  // Must be valid
  ASSERT_NE(response.error_response().reason(), "Not valid query");
  // check if fields in response are valid
  auto signatory = response.signatories_response().keys(0);
  decltype(pubkey) response_pubkey;
  std::copy(signatory.begin(), signatory.end(), response_pubkey.begin());
  ASSERT_EQ(response_pubkey, pubkey);
}

/**
 * Test for transactions response
 */

TEST_F(ToriiServiceTest, FindTransactionsWhenValid) {
  EXPECT_CALL(svMock, validate(A<const iroha::model::Query &>()))
      .WillRepeatedly(Return(true));

  iroha::model::Account account;
  account.account_id = "accountA";

  auto txs_observable = rxcpp::observable<>::from(
      iroha::model::Transaction().setTxCounter(0).setCreatorAccountId(
          account.account_id),
      iroha::model::Transaction().setTxCounter(1).setCreatorAccountId(
          account.account_id),
      iroha::model::Transaction().setTxCounter(2).setCreatorAccountId(
          account.account_id));

  EXPECT_CALL(wsv_query, getAccount(_)).WillRepeatedly(Return(account));
  EXPECT_CALL(block_query, getAccountTransactions(account.account_id))
      .WillRepeatedly(Return(txs_observable));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id(account.account_id);
  query.mutable_get_account_transactions()->set_account_id(account.account_id);

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because stateless valid
  ASSERT_NE(response.error_response().reason(), "Not valid");
  // Must be stateful valid
  ASSERT_NE(response.error_response().reason(), "Not valid query");
  for (size_t i = 0; i < response.transactions_response().transactions_size();
       i++) {
    ASSERT_EQ(response.transactions_response()
                  .transactions(i)
                  .meta()
                  .creator_account_id(),
              account.account_id);
    ASSERT_EQ(
        response.transactions_response().transactions(i).meta().tx_counter(),
        i);
  }
}

TEST_F(ToriiServiceTest, FindManyTimesWhereQueryServiceSync) {
  EXPECT_CALL(svMock, validate(A<const iroha::model::Query &>()))
      .WillRepeatedly(Return(false));

  for (size_t i = 0; i < TimesFind; ++i) {
    iroha::protocol::QueryResponse response;
    auto query = iroha::protocol::Query();
    query.set_creator_account_id("accountA");
    query.mutable_get_account()->set_account_id("accountB");
    query.set_query_counter(i);

    auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
    ASSERT_TRUE(stat.ok());
    // Must return Error Response
    ASSERT_EQ(response.error_response().reason(), "Not valid");
  }
}
