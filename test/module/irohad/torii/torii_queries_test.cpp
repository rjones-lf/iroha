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

#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"

#include "main/server_runner.hpp"
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

using namespace iroha::network;
using namespace iroha::validation;
using namespace iroha::ametsuchi;

class ToriiServiceTest : public testing::Test {
 public:
  virtual void SetUp() {
    runner = new ServerRunner(std::string(Ip) + ":" + std::to_string(Port));
    th = std::thread([this] {
      // ----------- Command Service --------------
      pcsMock = std::make_shared<MockPeerCommunicationService>();
      statelessValidatorMock = std::make_shared<MockStatelessValidator>();
      wsv_query = std::make_shared<MockWsvQuery>();
      block_query = std::make_shared<MockBlockQuery>();

      rxcpp::subjects::subject<iroha::model::Proposal> prop_notifier;
      rxcpp::subjects::subject<Commit> commit_notifier;

      EXPECT_CALL(*pcsMock,
                  on_proposal())
          .WillRepeatedly(Return(prop_notifier.get_observable()));

      EXPECT_CALL(*pcsMock,
                  on_commit())
          .WillRepeatedly(Return(commit_notifier.get_observable()));

      auto tx_processor =
          std::make_shared<iroha::torii::TransactionProcessorImpl>(
              pcsMock, statelessValidatorMock);
      auto pb_tx_factory =
          std::make_shared<iroha::model::converters::PbTransactionFactory>();

      auto command_service =
          std::make_unique<torii::CommandService>(pb_tx_factory, tx_processor);

      //----------- Query Service ----------

      auto qpf = std::make_unique<iroha::model::QueryProcessingFactory>(
          wsv_query, block_query);

      auto qpi = std::make_shared<iroha::torii::QueryProcessorImpl>(
          std::move(qpf), statelessValidatorMock);

      auto pb_query_factory =
          std::make_shared<iroha::model::converters::PbQueryFactory>();
      auto pb_query_resp_factory =
          std::make_shared<iroha::model::converters::PbQueryResponseFactory>();

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

  std::shared_ptr<MockPeerCommunicationService> pcsMock;
  std::shared_ptr<MockStatelessValidator> statelessValidatorMock;

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockBlockQuery> block_query;
};

/**
 * Test for error response
 */

TEST_F(ToriiServiceTest, FindWhenResponseInvalid) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<std::shared_ptr<const iroha::model::Query>>()))
      .WillOnce(Return(false));

  iroha::protocol::QueryResponse response;
  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account()->set_account_id("accountB");
  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Must return Error Response
  ASSERT_EQ(response.error_response().reason(),
            iroha::model::ErrorResponse::STATELESS_INVALID);
}

/**
 * Tests for account response
 */

TEST_F(ToriiServiceTest, FindAccountWhenStatefulInvalid) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<std::shared_ptr<const iroha::model::Query>>()))
      .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountB";

  EXPECT_CALL(*wsv_query, getAccount("accountB"))
      .Times(0);  // won't be called since stateful validation should fail
  EXPECT_CALL(*wsv_query, getAccount("accountA"))
      .Times(1);  // supposed to be called once when

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account()->set_account_id("accountB");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account
  ASSERT_EQ(response.error_response().reason(),
            iroha::model::ErrorResponse::STATEFUL_INVALID);
}

TEST_F(ToriiServiceTest, FindAccountWhenHasReadPermissions) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<std::shared_ptr<const iroha::model::Query>>()))
      .WillOnce(Return(true));

  iroha::model::Account accountA;
  accountA.account_id = "accountA";
  accountA.permissions.read_all_accounts = true;

  // Should be called once, when stateful validation will be in progress
  EXPECT_CALL(*wsv_query, getAccount(accountA.account_id))
      .WillOnce(Return(accountA));

  iroha::model::Account accountB;
  accountB.account_id = "accountB";

  // Should be called once, after successful stateful validation
  EXPECT_CALL(*wsv_query, getAccount(accountB.account_id))
      .WillOnce(Return(accountB));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account()->set_account_id("accountB");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());
  ASSERT_EQ(response.account_response().account().account_id(), "accountB");
}

TEST_F(ToriiServiceTest, FindAccountWhenValid) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<std::shared_ptr<const iroha::model::Query>>()))
      .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountA";

  // Should be called once when stateful validation is in progress
  EXPECT_CALL(*wsv_query, getAccount("accountA"))
      .Times(2)
      .WillRepeatedly(Return(account));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account()->set_account_id("accountA");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());
  ASSERT_EQ(response.account_response().account().account_id(), "accountA");
}

/**
 * Test for account asset response
 */

TEST_F(ToriiServiceTest, FindAccountAssetWhenStatefulInvalid) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<std::shared_ptr<const iroha::model::Query>>()))
      .WillOnce(Return(true));

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

  EXPECT_CALL(*wsv_query, getAccount("accountA")).WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getAccountAsset(_, _))
      .Times(0);  // won't be called due to failed stateful validation

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account_assets()->set_account_id("accountB");
  query.mutable_get_account_assets()->set_asset_id("usd");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account asset
  ASSERT_EQ(response.error_response().reason(),
            iroha::model::ErrorResponse::STATEFUL_INVALID);
}

TEST_F(ToriiServiceTest, FindAccountAssetWhenValid) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<std::shared_ptr<const iroha::model::Query>>()))
      .WillOnce(Return(true));

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

  EXPECT_CALL(*wsv_query, getAccount("accountA")).WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getAccountAsset(_, _)).WillOnce(Return(account_asset));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account_assets()->set_account_id("accountA");
  query.mutable_get_account_assets()->set_asset_id("usd");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());

  // Check if the fields in account asset response are correct
  ASSERT_EQ(response.account_assets_response().account_asset().asset_id(),
            account_asset.asset_id);
  ASSERT_EQ(response.account_assets_response().account_asset().account_id(),
            account_asset.account_id);
  ASSERT_EQ(response.account_assets_response().account_asset().balance(),
            account_asset.balance);
}

/**
 * Test for signatories response
 */

TEST_F(ToriiServiceTest, FindSignatoriesWhenStatefulInvalid) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<std::shared_ptr<const iroha::model::Query>>()))
      .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountB";

  iroha::ed25519::pubkey_t pubkey;
  std::fill(pubkey.begin(), pubkey.end(), 0x1);
  std::vector<iroha::ed25519::pubkey_t> keys;
  keys.push_back(pubkey);

  EXPECT_CALL(*wsv_query, getAccount("accountA")).WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getSignatories(_)).Times(0);

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account_signatories()->set_account_id("accountB");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account
  ASSERT_EQ(response.error_response().reason(),
            iroha::model::ErrorResponse::STATEFUL_INVALID);
}

TEST_F(ToriiServiceTest, FindSignatoriesWhenValid) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<std::shared_ptr<const iroha::model::Query>>()))
      .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountA";

  iroha::ed25519::pubkey_t pubkey;
  std::fill(pubkey.begin(), pubkey.end(), 0x1);
  std::vector<iroha::ed25519::pubkey_t> keys;
  keys.push_back(pubkey);

  EXPECT_CALL(*wsv_query, getAccount("accountA")).WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getSignatories(_)).WillOnce(Return(keys));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id("accountA");
  query.mutable_get_account_signatories()->set_account_id("accountA");

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  /// Should not return Error Response because tx is stateless and stateful
  /// valid
  ASSERT_FALSE(response.has_error_response());
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
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<std::shared_ptr<const iroha::model::Query>>()))
      .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountA";

  auto txs_observable  =
          rxcpp::observable<>::iterate([account] {
              std::vector<iroha::model::Transaction> result;
              for (size_t i = 0; i < 3; ++i) {
                iroha::model::Transaction current;
                current.creator_account_id = account.account_id;
                current.tx_counter = i;
                result.push_back(current);
              }
              return result;
          }());

  EXPECT_CALL(*wsv_query, getAccount(_)).WillOnce(Return(account));
  EXPECT_CALL(*block_query, getAccountTransactions(account.account_id))
      .WillOnce(Return(txs_observable));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.set_creator_account_id(account.account_id);
  query.mutable_get_account_transactions()->set_account_id(account.account_id);

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());
  for (auto i = 0; i < response.transactions_response().transactions_size();
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
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<std::shared_ptr<const iroha::model::Query>>()))
      .WillOnce(Return(false));

  for (size_t i = 0; i < TimesFind; ++i) {
    iroha::protocol::QueryResponse response;
    auto query = iroha::protocol::Query();
    query.set_creator_account_id("accountA");
    query.mutable_get_account()->set_account_id("accountB");
    query.set_query_counter(i);

    auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
    ASSERT_TRUE(stat.ok());
    // Must return Error Response
    ASSERT_EQ(response.error_response().reason(),
              iroha::model::ErrorResponse::STATELESS_INVALID);
  }
}
