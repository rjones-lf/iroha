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

#include "generator/generator.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"

// to compare pb amount and iroha amount
#include "model/converters/pb_common.hpp"

#include "main/server_runner.hpp"
#include "model/permissions.hpp"
#include "torii/processor/query_processor_impl.hpp"
#include "torii/processor/transaction_processor_impl.hpp"
#include "torii_utils/query_client.hpp"

constexpr const char *Ip = "0.0.0.0";
constexpr int Port = 50051;

constexpr size_t TimesFind = 1;
const iroha::model::Pager NO_PAGER {iroha::hash256_t{}, 100};

using ::testing::Return;
using ::testing::A;
using ::testing::_;
using ::testing::AtLeast;

using namespace iroha::network;
using namespace iroha::validation;
using namespace iroha::ametsuchi;
using namespace iroha::model;

class ToriiQueriesTest : public testing::Test {
 public:
  virtual void SetUp() {
    runner = new ServerRunner(std::string(Ip) + ":" + std::to_string(Port));
    th = std::thread([this] {
      // ----------- Command Service --------------
      pcsMock = std::make_shared<MockPeerCommunicationService>();
      statelessValidatorMock = std::make_shared<MockStatelessValidator>();
      wsv_query = std::make_shared<MockWsvQuery>();
      block_query = std::make_shared<MockBlockQuery>();
      storageMock = std::make_shared<MockStorage>();

      rxcpp::subjects::subject<iroha::model::Proposal> prop_notifier;
      rxcpp::subjects::subject<Commit> commit_notifier;

      EXPECT_CALL(*pcsMock, on_proposal())
          .WillRepeatedly(Return(prop_notifier.get_observable()));

      EXPECT_CALL(*pcsMock, on_commit())
          .WillRepeatedly(Return(commit_notifier.get_observable()));

      auto tx_processor =
          std::make_shared<iroha::torii::TransactionProcessorImpl>(
              pcsMock, statelessValidatorMock);
      auto pb_tx_factory =
          std::make_shared<iroha::model::converters::PbTransactionFactory>();

      auto command_service =
          std::make_unique<torii::CommandService>(pb_tx_factory, tx_processor, storageMock);

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
  std::shared_ptr<MockStorage> storageMock;

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockBlockQuery> block_query;

  // just random hex strings
  const std::string pubkey_test = generator::random_blob<16>(0).to_hexstring();
  const std::string signature_test =
      generator::random_blob<32>(0).to_hexstring();
};

/**
 * Test for error response
 */

TEST_F(ToriiQueriesTest, FindWhenResponseInvalid) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(false));

  iroha::protocol::QueryResponse response;
  auto query = iroha::protocol::Query();

  query.mutable_payload()->set_creator_account_id("accountA");
  query.mutable_payload()->mutable_get_account()->set_account_id("accountB");
  query.mutable_signature()->set_pubkey(pubkey_test);
  query.mutable_signature()->set_signature(signature_test);

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Must return Error Response
  ASSERT_EQ(response.error_response().reason(),
            iroha::model::ErrorResponse::STATELESS_INVALID);
}

/**
 * Tests for account response
 */

TEST_F(ToriiQueriesTest, FindAccountWhenNoGrantPermissions) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountB";
  auto creator = "accountA";

  // TODO: refactor this to use stateful validation mocks
  EXPECT_CALL(*wsv_query, hasAccountGrantablePermission(
                              creator, account.account_id, can_get_my_account))
      .WillOnce(Return(false));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillOnce(Return(nonstd::nullopt));

  EXPECT_CALL(*wsv_query, getAccount("accountB"))
      .Times(0);  // won't be called since stateful validation should fail

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();

  query.mutable_payload()->set_creator_account_id("accountA");
  query.mutable_payload()->mutable_get_account()->set_account_id("accountB");
  query.mutable_signature()->set_pubkey(pubkey_test);
  query.mutable_signature()->set_signature(signature_test);

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account
  ASSERT_EQ(response.error_response().reason(),
            iroha::model::ErrorResponse::STATEFUL_INVALID);
}

TEST_F(ToriiQueriesTest, FindAccountWhenHasReadPermissions) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(true));

  auto creator = "accountA";

  iroha::model::Account accountB;
  accountB.account_id = "accountB";

  // TODO: refactor this to use stateful validation mocks
  EXPECT_CALL(*wsv_query, hasAccountGrantablePermission(
      creator, accountB.account_id, can_get_my_account))
      .WillOnce(Return(true));

  // Should be called once, after successful stateful validation
  EXPECT_CALL(*wsv_query, getAccount(accountB.account_id))
      .WillOnce(Return(accountB));

  std::vector<std::string> roles = {"user"};
  EXPECT_CALL(*wsv_query, getAccountRoles(_))
      .WillRepeatedly(Return(roles));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();

  query.mutable_payload()->set_creator_account_id("accountA");
  query.mutable_payload()->mutable_get_account()->set_account_id("accountB");
  query.mutable_signature()->set_pubkey(pubkey_test);
  query.mutable_signature()->set_signature(signature_test);

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());
  ASSERT_EQ(response.account_response().account().account_id(), "accountB");
  ASSERT_EQ(response.account_response().account_roles().size(), 1);
}

TEST_F(ToriiQueriesTest, FindAccountWhenHasRolePermission) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountA";

  // Should be called once when stateful validation is in progress
  EXPECT_CALL(*wsv_query, getAccount("accountA"))
      .WillOnce(Return(account));
  // TODO: refactor this to use stateful validation mocks
  auto creator =  "accountA";
  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillRepeatedly(Return(roles));
  std::vector<std::string> perm = {can_get_my_account};
  EXPECT_CALL(*wsv_query, getRolePermissions("test"))
      .WillOnce(Return(perm));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();

  query.mutable_payload()->set_creator_account_id("accountA");
  query.mutable_payload()->mutable_get_account()->set_account_id("accountA");
  query.mutable_signature()->set_pubkey(pubkey_test);
  query.mutable_signature()->set_signature(signature_test);

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());
  ASSERT_EQ(response.account_response().account().account_id(), "accountA");
}

/**
 * Test for account asset response
 */

TEST_F(ToriiQueriesTest, FindAccountAssetWhenNoGrantPermissions) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountB";

  iroha::model::AccountAsset account_asset;
  account_asset.account_id = "accountB";
  account_asset.asset_id = "usd";
  iroha::Amount amount(100, 2);
  account_asset.balance = amount;

  iroha::model::Asset asset;
  asset.asset_id = "usd";
  asset.domain_id = "USA";
  asset.precision = 2;

  auto creator = "accountA";

  // TODO: refactor this to use stateful validation mocks
  EXPECT_CALL(*wsv_query, hasAccountGrantablePermission(
      creator, account.account_id, can_get_my_acc_ast))
      .WillOnce(Return(false));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillOnce(Return(nonstd::nullopt));

  EXPECT_CALL(*wsv_query, getAccountAsset(_, _))
      .Times(0);  // won't be called due to failed stateful validation

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();

  query.mutable_payload()->set_creator_account_id("accountA");
  query.mutable_payload()->mutable_get_account_assets()->set_account_id(
      "accountB");
  query.mutable_payload()->mutable_get_account_assets()->set_asset_id("usd");
  query.mutable_signature()->set_pubkey(pubkey_test);
  query.mutable_signature()->set_signature(signature_test);

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account asset
  ASSERT_EQ(response.error_response().reason(),
            iroha::model::ErrorResponse::STATEFUL_INVALID);
}

TEST_F(ToriiQueriesTest, FindAccountAssetWhenHasRolePermissions) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountA";

  iroha::model::AccountAsset account_asset;
  account_asset.account_id = "accountA";
  account_asset.asset_id = "usd";
  iroha::Amount amount(100, 2);
  account_asset.balance = amount;

  iroha::model::Asset asset;
  asset.asset_id = "usd";
  asset.domain_id = "USA";
  asset.precision = 2;

  // TODO: refactor this to use stateful validation mocks
  auto creator =  "accountA";
  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillOnce(Return(roles));
  std::vector<std::string> perm = {can_get_my_acc_ast};
  EXPECT_CALL(*wsv_query, getRolePermissions("test"))
      .WillOnce(Return(perm));
  EXPECT_CALL(*wsv_query, getAccountAsset(_, _))
      .WillOnce(Return(account_asset));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.mutable_payload()->set_creator_account_id("accountA");
  query.mutable_payload()->mutable_get_account_assets()->set_account_id(
      "accountA");
  query.mutable_payload()->mutable_get_account_assets()->set_asset_id("usd");
  query.mutable_signature()->set_pubkey(pubkey_test);
  query.mutable_signature()->set_signature(signature_test);

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());

  // Check if the fields in account asset response are correct
  ASSERT_EQ(response.account_assets_response().account_asset().asset_id(),
            account_asset.asset_id);
  ASSERT_EQ(response.account_assets_response().account_asset().account_id(),
            account_asset.account_id);
  auto iroha_amount_asset = iroha::model::converters::deserializeAmount(
      response.account_assets_response().account_asset().balance());
  ASSERT_EQ(iroha_amount_asset, account_asset.balance);
}

/**
 * Test for signatories response
 */

TEST_F(ToriiQueriesTest, FindSignatoriesWhenNoGrantPermissions) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountB";

  iroha::pubkey_t pubkey;
  std::fill(pubkey.begin(), pubkey.end(), 0x1);
  std::vector<iroha::pubkey_t> keys;
  keys.push_back(pubkey);

  // TODO: refactor this to use stateful validation mocks
  auto creator =  "accountA";
  EXPECT_CALL(*wsv_query, hasAccountGrantablePermission(
      creator, account.account_id, can_get_my_signatories))
      .WillOnce(Return(false));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillOnce(Return(nonstd::nullopt));
  EXPECT_CALL(*wsv_query, getSignatories(_)).Times(0);

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();
  query.mutable_payload()->set_creator_account_id("accountA");
  query.mutable_payload()->mutable_get_account_signatories()->set_account_id(
      "accountB");
  query.mutable_signature()->set_pubkey(pubkey_test);
  query.mutable_signature()->set_signature(signature_test);

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account
  ASSERT_EQ(response.error_response().reason(),
            iroha::model::ErrorResponse::STATEFUL_INVALID);
}

TEST_F(ToriiQueriesTest, FindSignatoriesHasRolePermissions) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountA";

  iroha::pubkey_t pubkey;
  std::fill(pubkey.begin(), pubkey.end(), 0x1);
  std::vector<iroha::pubkey_t> keys;
  keys.push_back(pubkey);

  // TODO: refactor this to use stateful validation mocks
  auto creator =  "accountA";
  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillOnce(Return(roles));
  std::vector<std::string> perm = {can_get_my_signatories};
  EXPECT_CALL(*wsv_query, getRolePermissions("test"))
      .WillOnce(Return(perm));
  EXPECT_CALL(*wsv_query, getSignatories(_)).WillOnce(Return(keys));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();

  query.mutable_payload()->set_creator_account_id("accountA");
  query.mutable_payload()->mutable_get_account_signatories()->set_account_id(
      "accountA");
  query.mutable_signature()->set_pubkey(pubkey_test);
  query.mutable_signature()->set_signature(signature_test);

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
TEST_F(ToriiQueriesTest, FindTransactionsWhenValid) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<const iroha::model::Query &>()))
    .WillOnce(Return(true));

  iroha::model::Account account;
  account.account_id = "accountA";

  auto txs_observable = rxcpp::observable<>::iterate([account] {
    std::vector<iroha::model::Transaction> result;
    for (size_t i = 0; i < 3; ++i) {
      iroha::model::Transaction current;
      current.creator_account_id = account.account_id;
      current.tx_counter = i;
      result.push_back(current);
    }
    return result;
  }());

  // TODO: refactor this to use stateful validation mocks
  auto creator =  "accountA";
  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
    .WillOnce(Return(roles));
  std::vector<std::string> perm = {can_get_my_acc_txs};
  EXPECT_CALL(*wsv_query, getRolePermissions("test"))
    .WillOnce(Return(perm));
  EXPECT_CALL(*block_query, getAccountTransactions(account.account_id, NO_PAGER))
    .WillOnce(Return(txs_observable));

  iroha::protocol::QueryResponse response;

  auto query = iroha::protocol::Query();

  query.mutable_payload()->set_creator_account_id(account.account_id);
  auto get_account_tx = query.mutable_payload()->mutable_get_account_transactions();
  get_account_tx->set_account_id(account.account_id);
  *get_account_tx->mutable_pager()->mutable_tx_hash() = NO_PAGER.tx_hash.to_string();
  get_account_tx->mutable_pager()->set_limit(NO_PAGER.limit);
  query.mutable_signature()->set_pubkey(pubkey_test);
  query.mutable_signature()->set_signature(signature_test);

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());
  for (auto i = 0; i < response.transactions_response().transactions_size();
       i++) {
    ASSERT_EQ(response.transactions_response()
                .transactions(i)
                .payload()
                .creator_account_id(),
              account.account_id);
    ASSERT_EQ(
      response.transactions_response().transactions(i).payload().tx_counter(),
      i);
  }
}

TEST_F(ToriiQueriesTest, FindManyTimesWhereQueryServiceSync) {
  EXPECT_CALL(*statelessValidatorMock,
              validate(A<const iroha::model::Query &>()))
      .WillOnce(Return(false));

  for (size_t i = 0; i < TimesFind; ++i) {
    iroha::protocol::QueryResponse response;
    auto query = iroha::protocol::Query();

    query.mutable_payload()->set_creator_account_id("accountA");
    query.mutable_payload()->mutable_get_account()->set_account_id("accountB");
    query.mutable_payload()->set_query_counter(i);
    query.mutable_signature()->set_pubkey(pubkey_test);
    query.mutable_signature()->set_signature(signature_test);

    auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(query, response);
    ASSERT_TRUE(stat.ok());
    // Must return Error Response
    ASSERT_EQ(response.error_response().reason(),
              iroha::model::ErrorResponse::STATELESS_INVALID);
  }
}
