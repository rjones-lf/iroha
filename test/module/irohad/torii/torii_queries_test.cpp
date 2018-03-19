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

#include "builders/protobuf/common_objects/proto_account_asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "builders/protobuf/common_objects/proto_amount_builder.hpp"
#include "builders/protobuf/common_objects/proto_asset_builder.hpp"
#include "generator/generator.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"
// to compare pb amount and iroha amount
#include "model/converters/pb_common.hpp"

#include "main/server_runner.hpp"
#include "model/permissions.hpp"
#include "torii/processor/query_processor_impl.hpp"
#include "torii/query_client.hpp"
#include "torii/query_service.hpp"

#include "builders/protobuf/queries.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"

constexpr const char *Ip = "0.0.0.0";
constexpr int Port = 50051;

constexpr size_t TimesFind = 1;

using ::testing::_;
using ::testing::A;
using ::testing::AtLeast;
using ::testing::Return;

using namespace iroha::ametsuchi;

using wTransaction = std::shared_ptr<shared_model::interface::Transaction>;

// TODO: allow dynamic port binding in ServerRunner IR-741
class ToriiQueriesTest : public testing::Test {
 public:
  virtual void SetUp() {
    runner = new ServerRunner(std::string(Ip) + ":" + std::to_string(Port));

    wsv_query = std::make_shared<MockWsvQuery>();
    block_query = std::make_shared<MockBlockQuery>();

    //----------- Query Service ----------

    auto qpf = std::make_unique<iroha::model::QueryProcessingFactory>(
        wsv_query, block_query);

    auto qpi =
        std::make_shared<iroha::torii::QueryProcessorImpl>(std::move(qpf));

    //----------- Server run ----------------
    runner->append(std::make_unique<torii::QueryService>(qpi)).run();

    runner->waitForServersReady();
  }

  virtual void TearDown() {
    delete runner;
  }

  ServerRunner *runner;

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockBlockQuery> block_query;

  // just random hex strings
  const std::string pubkey_test = generator::random_blob<16>(0).to_hexstring();
  const std::string signature_test =
      generator::random_blob<32>(0).to_hexstring();
};

/**
 * Given a Query Synchronous Client
 * When copied, moved, copy and move assigned
 * Then final object works as the first one
 */
TEST_F(ToriiQueriesTest, QueryClient) {
  iroha::protocol::QueryResponse response;
  auto query = iroha::protocol::Query();

  query.mutable_payload()->set_creator_account_id("accountA");
  query.mutable_payload()->mutable_get_account()->set_account_id("accountB");
  query.mutable_signature()->set_pubkey(pubkey_test);
  query.mutable_signature()->set_signature(signature_test);

  auto client1 = torii_utils::QuerySyncClient(Ip, Port);
  // Copy ctor
  torii_utils::QuerySyncClient client2(client1);
  // copy assignment
  auto client3 = client2;
  // move ctor
  torii_utils::QuerySyncClient client4(std::move(client3));
  // move assignment
  auto client5 = std::move(client4);
  auto stat = client5.Find(query, response);
  ASSERT_TRUE(stat.ok());
}

/**
 * Test for error response
 */

TEST_F(ToriiQueriesTest, FindWhenResponseInvalid) {
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
  ASSERT_EQ(iroha::hash(query).to_string(), response.query_hash());
}

/**
 * Tests for account response
 */

TEST_F(ToriiQueriesTest, FindAccountWhenNoGrantPermissions) {
  // TODO: kamilsa 19.02.2017 remove old model
  iroha::model::Account account;
  account.account_id = "b@domain";
  auto creator = "a@domain";

  // TODO: refactor this to use stateful validation mocks
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  creator, account.account_id, iroha::model::can_get_my_account))
      .WillOnce(Return(false));

  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillRepeatedly(Return(boost::none));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccount(account.account_id)
                         .build()
                         .signAndAddSignature(
                             shared_model::crypto::DefaultCryptoAlgorithmType::
                                 generateKeypair());

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(
      model_query.getTransport(), response);

  ASSERT_TRUE(stat.ok());

  // Must be invalid due to failed stateful validation caused by no permission
  // to read account
  ASSERT_EQ(response.error_response().reason(),
            iroha::model::ErrorResponse::STATEFUL_INVALID);
  ASSERT_EQ(iroha::hash(model_query.getTransport()).to_string(),
            response.query_hash());
}

TEST_F(ToriiQueriesTest, FindAccountWhenHasReadPermissions) {
  auto creator = "a@domain";

  auto accountB = std::shared_ptr<shared_model::interface::Account>(
      shared_model::proto::AccountBuilder()
          .accountId("b@domain")
          .build()
          .copy());

  // TODO: refactor this to use stateful validation mocks
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          creator, accountB->accountId(), iroha::model::can_get_my_account))
      .WillOnce(Return(true));

  // Should be called once, after successful stateful validation
  EXPECT_CALL(*wsv_query, getAccount(accountB->accountId()))
      .WillOnce(Return(accountB));

  std::vector<std::string> roles = {"user"};
  EXPECT_CALL(*wsv_query, getAccountRoles(_)).WillRepeatedly(Return(roles));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccount(accountB->accountId())
                         .build()
                         .signAndAddSignature(
                             shared_model::crypto::DefaultCryptoAlgorithmType::
                                 generateKeypair());

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(
      model_query.getTransport(), response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());
  ASSERT_EQ(response.account_response().account().account_id(),
            accountB->accountId());
  ASSERT_EQ(response.account_response().account_roles().size(), 1);
  ASSERT_EQ(iroha::hash(model_query.getTransport()).to_string(),
            response.query_hash());
}

TEST_F(ToriiQueriesTest, FindAccountWhenHasRolePermission) {
  auto account = std::shared_ptr<shared_model::interface::Account>(
      shared_model::proto::AccountBuilder()
          .accountId("accountA")
          .build()
          .copy());
  ;

  auto creator = "a@domain";
  EXPECT_CALL(*wsv_query, getAccount(creator)).WillOnce(Return(account));
  // TODO: refactor this to use stateful validation mocks
  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillRepeatedly(Return(roles));
  std::vector<std::string> perm = {iroha::model::can_get_my_account};
  EXPECT_CALL(*wsv_query, getRolePermissions("test")).WillOnce(Return(perm));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccount(creator)
                         .build()
                         .signAndAddSignature(
                             shared_model::crypto::DefaultCryptoAlgorithmType::
                                 generateKeypair());

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(
      model_query.getTransport(), response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());
  //  ASSERT_EQ(response.account_detail_response().detail(), "value");
  ASSERT_EQ(response.account_response().account().account_id(),
            account->accountId());
  ASSERT_EQ(response.account_response().account().domain_id(),
            account->domainId());
  ASSERT_EQ(iroha::hash(model_query.getTransport()).to_string(),
            response.query_hash());
}

/**
 * Test for account asset response
 */

TEST_F(ToriiQueriesTest, FindAccountAssetWhenNoGrantPermissions) {
  auto creator = "a@domain";
  auto accountb_id = "b@domain";

  // TODO: refactor this to use stateful validation mocks
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  creator, accountb_id, iroha::model::can_get_my_acc_ast))
      .WillOnce(Return(false));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillOnce(Return(boost::none));

  EXPECT_CALL(*wsv_query, getAccountAsset(_, _))
      .Times(0);  // won't be called due to failed stateful validation

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccountAssets(accountb_id, "usd#domain")
                         .build()
                         .signAndAddSignature(
                             shared_model::crypto::DefaultCryptoAlgorithmType::
                                 generateKeypair());

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(
      model_query.getTransport(), response);
  ASSERT_TRUE(stat.ok());
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account asset
  ASSERT_EQ(response.error_response().reason(),
            iroha::model::ErrorResponse::STATEFUL_INVALID);
  ASSERT_EQ(iroha::hash(model_query.getTransport()).to_string(),
            response.query_hash());
}

TEST_F(ToriiQueriesTest, FindAccountAssetWhenHasRolePermissions) {
  auto account =
      shared_model::proto::AccountBuilder().accountId("accountA").build();

  auto amount =
      shared_model::proto::AmountBuilder().intValue(100).precision(2).build();

  auto account_asset = std::shared_ptr<shared_model::interface::AccountAsset>(
      shared_model::proto::AccountAssetBuilder()
          .accountId("accountA")
          .assetId("usd")
          .balance(amount)
          .build()
          .copy());

  auto asset = shared_model::proto::AssetBuilder()
                   .assetId("usd")
                   .domainId("USA")
                   .precision(2)
                   .build();

  // TODO: refactor this to use stateful validation mocks
  auto creator = "a@domain";
  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles(creator)).WillOnce(Return(roles));
  std::vector<std::string> perm = {iroha::model::can_get_my_acc_ast};
  EXPECT_CALL(*wsv_query, getRolePermissions("test")).WillOnce(Return(perm));
  EXPECT_CALL(*wsv_query, getAccountAsset(_, _))
      .WillOnce(Return(account_asset));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccountAssets(creator, "usd#domain")
                         .build()
                         .signAndAddSignature(
                             shared_model::crypto::DefaultCryptoAlgorithmType::
                                 generateKeypair());

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(
      model_query.getTransport(), response);

  auto hash = response.query_hash();

  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());

  // Check if the fields in account asset response are correct
  ASSERT_EQ(response.account_assets_response().account_asset().asset_id(),
            account_asset->assetId());
  ASSERT_EQ(response.account_assets_response().account_asset().account_id(),
            account_asset->accountId());
  auto iroha_amount_asset = iroha::model::converters::deserializeAmount(
      response.account_assets_response().account_asset().balance());
  ASSERT_EQ(iroha_amount_asset,
            *std::unique_ptr<iroha::Amount>(
                account_asset->balance().makeOldModel()));
  ASSERT_EQ(iroha::hash(model_query.getTransport()).to_string(),
            response.query_hash());
}

/**
 * Test for signatories response
 */

TEST_F(ToriiQueriesTest, FindSignatoriesWhenNoGrantPermissions) {
  iroha::pubkey_t pubkey;
  std::fill(pubkey.begin(), pubkey.end(), 0x1);
  std::vector<shared_model::interface::types::PubkeyType> keys;
  keys.push_back(
      shared_model::interface::types::PubkeyType(pubkey.to_string()));

  // TODO: refactor this to use stateful validation mocks
  auto creator = "a@domain";
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  creator, "b@domain", iroha::model::can_get_my_signatories))
      .WillOnce(Return(false));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillOnce(Return(boost::none));
  EXPECT_CALL(*wsv_query, getSignatories(_)).Times(0);

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getSignatories("b@domain")
                         .build()
                         .signAndAddSignature(
                             shared_model::crypto::DefaultCryptoAlgorithmType::
                                 generateKeypair());

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(
      model_query.getTransport(), response);
  ASSERT_TRUE(stat.ok());
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account
  ASSERT_EQ(response.error_response().reason(),
            iroha::model::ErrorResponse::STATEFUL_INVALID);
  ASSERT_EQ(iroha::hash(model_query.getTransport()).to_string(),
            response.query_hash());
}

TEST_F(ToriiQueriesTest, FindSignatoriesHasRolePermissions) {
  // TODO: refactor this to use stateful validation mocks
  iroha::pubkey_t pubkey;
  std::fill(pubkey.begin(), pubkey.end(), 0x1);
  std::vector<shared_model::interface::types::PubkeyType> keys;
  keys.push_back(
      shared_model::interface::types::PubkeyType(pubkey.to_string()));

  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles("a@domain")).WillOnce(Return(roles));
  std::vector<std::string> perm = {iroha::model::can_get_my_signatories};
  EXPECT_CALL(*wsv_query, getRolePermissions("test")).WillOnce(Return(perm));
  EXPECT_CALL(*wsv_query, getSignatories(_)).WillOnce(Return(keys));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId("a@domain")
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getSignatories("a@domain")
                         .build()
                         .signAndAddSignature(
                             shared_model::crypto::DefaultCryptoAlgorithmType::
                                 generateKeypair());

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(
      model_query.getTransport(), response);
  ASSERT_TRUE(stat.ok());
  /// Should not return Error Response because tx is stateless and stateful
  /// valid
  ASSERT_FALSE(response.has_error_response());
  // check if fields in response are valid
  auto signatory = response.signatories_response().keys(0);
  decltype(pubkey) response_pubkey;
  std::copy(signatory.begin(), signatory.end(), response_pubkey.begin());
  ASSERT_EQ(response_pubkey, pubkey);
  ASSERT_EQ(iroha::hash(model_query.getTransport()).to_string(),
            response.query_hash());
}

/**
 * Test for transactions response
 */

TEST_F(ToriiQueriesTest, FindTransactionsWhenValid) {
  // TODO 19.02.2018 kamilsa remove old model
  iroha::model::Account account;
  account.account_id = "accountA";

  auto txs_observable = rxcpp::observable<>::iterate([account] {
    std::vector<wTransaction> result;
    for (size_t i = 0; i < 3; ++i) {
      auto current = wTransaction(TestTransactionBuilder()
                                      .creatorAccountId(account.account_id)
                                      .txCounter(i)
                                      .build()
                                      .copy());
      result.push_back(current);
    }
    return result;
  }());

  // TODO: refactor this to use stateful validation mocks
  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles("a@domain")).WillOnce(Return(roles));
  std::vector<std::string> perm = {iroha::model::can_get_my_acc_txs};
  EXPECT_CALL(*wsv_query, getRolePermissions("test")).WillOnce(Return(perm));
  EXPECT_CALL(*block_query, getAccountTransactions("a@domain"))
      .WillOnce(Return(txs_observable));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId("a@domain")
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccountTransactions("a@domain")
                         .build()
                         .signAndAddSignature(
                             shared_model::crypto::DefaultCryptoAlgorithmType::
                                 generateKeypair());

  auto stat = torii_utils::QuerySyncClient(Ip, Port).Find(
      model_query.getTransport(), response);
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
  ASSERT_EQ(iroha::hash(model_query.getTransport()).to_string(),
            response.query_hash());
}

TEST_F(ToriiQueriesTest, FindManyTimesWhereQueryServiceSync) {
  auto client = torii_utils::QuerySyncClient(Ip, Port);

  for (size_t i = 0; i < TimesFind; ++i) {
    iroha::protocol::QueryResponse response;

    // stateless invalid query
    auto model_query = TestQueryBuilder()
                           .creatorAccountId("a@domain")
                           .queryCounter(i)
                           .createdTime(iroha::time::now())
                           .getAccountTransactions("a@2domain")
                           .build();

    auto stat = client.Find(model_query.getTransport(), response);
    ASSERT_TRUE(stat.ok());
    // Must return Error Response
    ASSERT_EQ(response.error_response().reason(),
              iroha::model::ErrorResponse::STATELESS_INVALID);
    ASSERT_EQ(iroha::hash(model_query.getTransport()).to_string(),
              response.query_hash());
  }
}
