/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/pending_txs_storage/pending_txs_storage_mock.hpp"
#include "module/irohad/torii/torii_mocks.hpp"
#include "module/irohad/validation/validation_mocks.hpp"

#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "builders/protobuf/common_objects/proto_account_asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "builders/protobuf/common_objects/proto_asset_builder.hpp"
#include "builders/protobuf/queries.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

#include "execution/query_execution_impl.hpp"
#include "framework/specified_visitor.hpp"
#include "main/server_runner.hpp"
#include "torii/processor/query_processor_impl.hpp"
#include "torii/query_client.hpp"
#include "torii/query_service.hpp"
#include "utils/query_error_response_visitor.hpp"

constexpr size_t TimesFind = 1;

using ::testing::_;
using ::testing::A;
using ::testing::AtLeast;
using ::testing::Return;

using namespace iroha::ametsuchi;
using namespace iroha::torii;
using namespace shared_model::interface::permissions;

using wTransaction = std::shared_ptr<shared_model::interface::Transaction>;

// TODO kamilsa 22.06.18 IR-1472 rework this test so that query service is
// mocked
class ToriiQueriesTest : public testing::Test {
 public:
  virtual void SetUp() {
    runner = std::make_unique<ServerRunner>(ip + ":0");
    wsv_query = std::make_shared<MockWsvQuery>();
    block_query = std::make_shared<MockBlockQuery>();
    storage = std::make_shared<MockStorage>();
    pending_txs_storage =
        std::make_shared<iroha::MockPendingTransactionStorage>();

    //----------- Query Service ----------

    EXPECT_CALL(*storage, getWsvQuery()).WillRepeatedly(Return(wsv_query));
    EXPECT_CALL(*storage, getBlockQuery()).WillRepeatedly(Return(block_query));

    auto qpi = std::make_shared<iroha::torii::QueryProcessorImpl>(
        storage,
        std::make_shared<iroha::QueryExecutionImpl>(storage,
                                                    pending_txs_storage));

    //----------- Server run ----------------
    runner->append(std::make_unique<torii::QueryService>(qpi))
        .run()
        .match(
            [this](iroha::expected::Value<int> port) {
              this->port = port.value;
            },
            [](iroha::expected::Error<std::string> err) {
              FAIL() << err.error;
            });

    runner->waitForServersReady();
  }

  std::unique_ptr<ServerRunner> runner;
  shared_model::crypto::Keypair pair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
  std::vector<shared_model::interface::types::PubkeyType> signatories = {
      pair.publicKey()};

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockBlockQuery> block_query;
  std::shared_ptr<MockStorage> storage;
  std::shared_ptr<iroha::MockPendingTransactionStorage> pending_txs_storage;

  const std::string ip = "127.0.0.1";
  int port;
};

/**
 * Given a Query Synchronous Client
 * When copied, moved, copy and move assigned
 * Then final object works as the first one
 */
TEST_F(ToriiQueriesTest, QueryClient) {
  iroha::protocol::QueryResponse response;
  auto query = TestUnsignedQueryBuilder()
                   .creatorAccountId("accountA")
                   .getAccount("accountB")
                   .build()
                   .signAndAddSignature(pair)
                   .finish();

  auto client1 = torii_utils::QuerySyncClient(ip, port);
  // Copy ctor
  torii_utils::QuerySyncClient client2(client1);
  // copy assignment
  auto client3 = client2;
  // move ctor
  torii_utils::QuerySyncClient client4(std::move(client3));
  // move assignment
  auto client5 = std::move(client4);
  auto stat = client5.Find(query.getTransport(), response);
  ASSERT_TRUE(stat.ok());
}

/**
 * Test for error response
 */

TEST_F(ToriiQueriesTest, FindWhenResponseInvalid) {
  iroha::protocol::QueryResponse response;
  auto query = TestUnsignedQueryBuilder()
                   .creatorAccountId("accountA")
                   .getAccount("accountB")
                   .build()
                   .signAndAddSignature(pair)
                   .finish();

  auto stat = torii_utils::QuerySyncClient(ip, port).Find(query.getTransport(),
                                                          response);
  auto resp = shared_model::proto::QueryResponse(response);
  ASSERT_TRUE(stat.ok());
  // Must return Error Response
  ASSERT_TRUE(boost::apply_visitor(
      shared_model::interface::QueryErrorResponseChecker<
          shared_model::interface::StatelessFailedErrorResponse>(),
      resp.get()));

  ASSERT_EQ(query.hash(), resp.queryHash());
}

/**
 * Tests for account response
 */

TEST_F(ToriiQueriesTest, FindAccountWhenNoGrantPermissions) {
  auto account =
      shared_model::proto::AccountBuilder().accountId("b@domain").build();
  auto creator = "a@domain";

  EXPECT_CALL(*wsv_query, getSignatories(creator))
      .WillRepeatedly(Return(signatories));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillRepeatedly(Return(boost::none));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccount(account.accountId())
                         .build()
                         .signAndAddSignature(pair)
                         .finish();

  auto stat = torii_utils::QuerySyncClient(ip, port).Find(
      model_query.getTransport(), response);

  ASSERT_TRUE(stat.ok());
  auto resp = shared_model::proto::QueryResponse(response);
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account
  ASSERT_TRUE(boost::apply_visitor(
      shared_model::interface::QueryErrorResponseChecker<
          shared_model::interface::StatefulFailedErrorResponse>(),
      resp.get()));

  ASSERT_EQ(model_query.hash(), resp.queryHash());
}

TEST_F(ToriiQueriesTest, FindAccountWhenHasReadPermissions) {
  auto creator = "a@domain";

  std::shared_ptr<shared_model::interface::Account> accountB = clone(
      shared_model::proto::AccountBuilder().accountId("b@domain").build());

  EXPECT_CALL(*wsv_query, getSignatories(creator))
      .WillRepeatedly(Return(signatories));

  // Should be called once, after successful stateful validation
  EXPECT_CALL(*wsv_query, getAccount(accountB->accountId()))
      .WillOnce(Return(accountB));

  std::vector<std::string> roles = {"user"};
  EXPECT_CALL(*wsv_query, getAccountRoles(_)).WillRepeatedly(Return(roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(_))
      .WillOnce(Return(shared_model::interface::RolePermissionSet(
          {shared_model::interface::permissions::Role::kGetAllAccounts})));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccount(accountB->accountId())
                         .build()
                         .signAndAddSignature(pair)
                         .finish();

  auto stat = torii_utils::QuerySyncClient(ip, port).Find(
      model_query.getTransport(), response);
  auto resp = shared_model::proto::QueryResponse(response);

  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());

  ASSERT_NO_THROW({
    const auto &account_resp = boost::apply_visitor(
        framework::SpecifiedVisitor<shared_model::interface::AccountResponse>(),
        resp.get());

    ASSERT_EQ(account_resp.account().accountId(), accountB->accountId());
    ASSERT_EQ(account_resp.roles().size(), 1);
    ASSERT_EQ(model_query.hash(), resp.queryHash());
  });
}

TEST_F(ToriiQueriesTest, FindAccountWhenHasRolePermission) {
  std::shared_ptr<shared_model::interface::Account> account = clone(
      shared_model::proto::AccountBuilder().accountId("accountA").build());

  auto creator = "a@domain";
  EXPECT_CALL(*wsv_query, getAccount(creator)).WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getSignatories(creator))
      .WillRepeatedly(Return(signatories));
  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillRepeatedly(Return(roles));
  shared_model::interface::RolePermissionSet perm;
  perm.set(Role::kGetMyAccount);
  EXPECT_CALL(*wsv_query, getRolePermissions("test")).WillOnce(Return(perm));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccount(creator)
                         .build()
                         .signAndAddSignature(pair)
                         .finish();

  auto stat = torii_utils::QuerySyncClient(ip, port).Find(
      model_query.getTransport(), response);
  auto resp = shared_model::proto::QueryResponse(response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());

  ASSERT_NO_THROW({
    const auto &detail_resp = boost::apply_visitor(
        framework::SpecifiedVisitor<shared_model::interface::AccountResponse>(),
        resp.get());

    ASSERT_EQ(detail_resp.account().accountId(), account->accountId());
    ASSERT_EQ(detail_resp.account().domainId(), account->domainId());
    ASSERT_EQ(model_query.hash(), resp.queryHash());
  });
}

/**
 * Test for account asset response
 */

TEST_F(ToriiQueriesTest, FindAccountAssetWhenNoGrantPermissions) {
  auto creator = "a@domain";
  auto accountb_id = "b@domain";

  EXPECT_CALL(*wsv_query, getSignatories(creator))
      .WillRepeatedly(Return(signatories));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillOnce(Return(boost::none));

  EXPECT_CALL(*wsv_query, getAccountAsset(_, _))
      .Times(0);  // won't be called due to failed stateful validation

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccountAssets(accountb_id)
                         .build()
                         .signAndAddSignature(pair)
                         .finish();

  auto stat = torii_utils::QuerySyncClient(ip, port).Find(
      model_query.getTransport(), response);
  auto resp = shared_model::proto::QueryResponse(response);
  ASSERT_TRUE(stat.ok());
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account asset
  ASSERT_TRUE(boost::apply_visitor(
      shared_model::interface::QueryErrorResponseChecker<
          shared_model::interface::StatefulFailedErrorResponse>(),
      resp.get()));

  ASSERT_EQ(model_query.hash(), resp.queryHash());
}

TEST_F(ToriiQueriesTest, FindAccountAssetWhenHasRolePermissions) {
  auto account =
      shared_model::proto::AccountBuilder().accountId("accountA").build();

  std::shared_ptr<shared_model::interface::AccountAsset> account_asset =
      clone(shared_model::proto::AccountAssetBuilder()
                .accountId("accountA")
                .assetId("usd")
                .balance(shared_model::interface::Amount("1.00"))
                .build());

  auto asset = shared_model::proto::AssetBuilder()
                   .assetId("usd")
                   .domainId("USA")
                   .precision(2)
                   .build();

  auto creator = "a@domain";
  EXPECT_CALL(*wsv_query, getSignatories(creator))
      .WillRepeatedly(Return(signatories));
  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles(creator)).WillOnce(Return(roles));
  shared_model::interface::RolePermissionSet perm;
  perm.set(Role::kGetMyAccAst);
  EXPECT_CALL(*wsv_query, getRolePermissions("test")).WillOnce(Return(perm));
  EXPECT_CALL(*wsv_query, getAccountAssets(_))
      .WillOnce(Return(
          std::vector<std::shared_ptr<shared_model::interface::AccountAsset>>(
              {account_asset})));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccountAssets(creator)
                         .build()
                         .signAndAddSignature(pair)
                         .finish();

  auto stat = torii_utils::QuerySyncClient(ip, port).Find(
      model_query.getTransport(), response);

  auto hash = response.query_hash();

  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());

  auto resp = shared_model::proto::QueryResponse(response);
  ASSERT_NO_THROW({
    const auto &asset_resp = boost::apply_visitor(
        framework::SpecifiedVisitor<
            shared_model::interface::AccountAssetResponse>(),
        resp.get());
    // Check if the fields in account asset response are correct
    ASSERT_EQ(asset_resp.accountAssets()[0].assetId(),
              account_asset->assetId());
    ASSERT_EQ(asset_resp.accountAssets()[0].accountId(),
              account_asset->accountId());
    ASSERT_EQ(asset_resp.accountAssets()[0].balance(),
              account_asset->balance());
    ASSERT_EQ(model_query.hash(), resp.queryHash());
  });
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

  auto creator = "a@domain";
  EXPECT_CALL(*wsv_query, getSignatories(creator))
      .WillRepeatedly(Return(signatories));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator))
      .WillOnce(Return(boost::none));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getSignatories("b@domain")
                         .build()
                         .signAndAddSignature(pair)
                         .finish();

  auto stat = torii_utils::QuerySyncClient(ip, port).Find(
      model_query.getTransport(), response);
  ASSERT_TRUE(stat.ok());
  // Must be invalid due to failed stateful validation caused by no permission
  // to read account
  auto resp = shared_model::proto::QueryResponse(response);
  ASSERT_TRUE(boost::apply_visitor(
      shared_model::interface::QueryErrorResponseChecker<
          shared_model::interface::StatefulFailedErrorResponse>(),
      resp.get()));

  ASSERT_EQ(model_query.hash(), resp.queryHash());
}

TEST_F(ToriiQueriesTest, FindSignatoriesHasRolePermissions) {
  auto creator = "a@domain";
  iroha::pubkey_t pubkey;
  std::fill(pubkey.begin(), pubkey.end(), 0x1);
  std::vector<shared_model::interface::types::PubkeyType> keys;
  keys.push_back(
      shared_model::interface::types::PubkeyType(pubkey.to_string()));

  EXPECT_CALL(*wsv_query, getSignatories(creator))
      .WillRepeatedly(Return(signatories));

  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles(creator)).WillOnce(Return(roles));
  shared_model::interface::RolePermissionSet perm;
  perm.set(Role::kGetMySignatories);
  EXPECT_CALL(*wsv_query, getRolePermissions("test")).WillOnce(Return(perm));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getSignatories(creator)
                         .build()
                         .signAndAddSignature(pair)
                         .finish();

  auto stat = torii_utils::QuerySyncClient(ip, port).Find(
      model_query.getTransport(), response);
  auto shared_response = shared_model::proto::QueryResponse(response);
  ASSERT_NO_THROW({
    auto resp_pubkey = *boost::apply_visitor(
                            framework::SpecifiedVisitor<
                                shared_model::interface::SignatoriesResponse>(),
                            shared_response.get())
                            .keys()
                            .begin();

    ASSERT_TRUE(stat.ok());
    /// Should not return Error Response because tx is stateless and stateful
    /// valid
    ASSERT_FALSE(response.has_error_response());
    // check if fields in response are valid
    ASSERT_EQ(resp_pubkey, signatories.back());
    ASSERT_EQ(model_query.hash(), shared_response.queryHash());
  });
}

/**
 * Test for transactions response
 */

TEST_F(ToriiQueriesTest, FindTransactionsWhenValid) {
  auto account =
      shared_model::proto::AccountBuilder().accountId("accountA").build();
  auto creator = "a@domain";
  std::vector<wTransaction> txs;
  for (size_t i = 0; i < 3; ++i) {
    std::shared_ptr<shared_model::interface::Transaction> current = clone(
        TestTransactionBuilder().creatorAccountId(account.accountId()).build());
    txs.push_back(current);
  }

  EXPECT_CALL(*wsv_query, getSignatories(creator))
      .WillRepeatedly(Return(signatories));
  std::vector<std::string> roles = {"test"};
  EXPECT_CALL(*wsv_query, getAccountRoles(creator)).WillOnce(Return(roles));
  shared_model::interface::RolePermissionSet perm;
  perm.set(Role::kGetMyAccTxs);
  EXPECT_CALL(*wsv_query, getRolePermissions("test")).WillOnce(Return(perm));
  EXPECT_CALL(*block_query, getAccountTransactions(creator))
      .WillOnce(Return(txs));

  iroha::protocol::QueryResponse response;

  auto model_query = shared_model::proto::QueryBuilder()
                         .creatorAccountId(creator)
                         .queryCounter(1)
                         .createdTime(iroha::time::now())
                         .getAccountTransactions(creator)
                         .build()
                         .signAndAddSignature(pair)
                         .finish();

  auto stat = torii_utils::QuerySyncClient(ip, port).Find(
      model_query.getTransport(), response);
  ASSERT_TRUE(stat.ok());
  // Should not return Error Response because tx is stateless and stateful valid
  ASSERT_FALSE(response.has_error_response());
  auto resp = shared_model::proto::QueryResponse(response);
  ASSERT_NO_THROW({
    const auto &tx_resp = boost::apply_visitor(
        framework::SpecifiedVisitor<
            shared_model::interface::TransactionsResponse>(),
        resp.get());

    const auto &txs = tx_resp.transactions();
    for (const auto &tx : txs) {
      ASSERT_EQ(tx.creatorAccountId(), account.accountId());
    }
    ASSERT_EQ(model_query.hash(), resp.queryHash());
  });
}

TEST_F(ToriiQueriesTest, FindManyTimesWhereQueryServiceSync) {
  auto client = torii_utils::QuerySyncClient(ip, port);

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
    auto resp = shared_model::proto::QueryResponse(response);
    // Must return Error Response
    ASSERT_TRUE(boost::apply_visitor(
        shared_model::interface::QueryErrorResponseChecker<
            shared_model::interface::StatelessFailedErrorResponse>(),
        resp.get()));

    ASSERT_EQ(model_query.hash(), resp.queryHash());
  }
}
