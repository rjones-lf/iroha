/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this/her file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

#include "builders/common_objects/account_asset_builder.hpp"
#include "builders/common_objects/amount_builder.hpp"
#include "builders/common_objects/asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_account_asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "builders/protobuf/common_objects/proto_amount_builder.hpp"
#include "builders/protobuf/common_objects/proto_asset_builder.hpp"
#include "framework/test_subscriber.hpp"
#include "model/permissions.hpp"
#include "model/query_execution.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"

using ::testing::AllOf;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::_;

using namespace iroha::ametsuchi;
using namespace iroha::model;
using namespace framework::test_subscriber;

using wTransaction = std::shared_ptr<shared_model::interface::Transaction>;

// TODO: 28/03/2018 x3medima17 remove poly wrapper, IR-1011
template <class T>
using w = shared_model::detail::PolymorphicWrapper<T>;

class QueryValidateExecuteTest : public ::testing::Test {
 public:
  QueryValidateExecuteTest() = default;

  void SetUp() override {
    wsv_query = std::make_shared<StrictMock<MockWsvQuery>>();
    block_query = std::make_shared<StrictMock<MockBlockQuery>>();
    factory = std::make_shared<QueryProcessingFactory>(wsv_query, block_query);

    EXPECT_CALL(*wsv_query, hasAccountGrantablePermission(_, _, _))
        .WillRepeatedly(Return(false));

    creator = clone(shared_model::proto::AccountBuilder()
                        .accountId(admin_id)
                        .domainId(domain_id)
                        .jsonData("{}")
                        .quorum(1)
                        .build());

    account = clone(shared_model::proto::AccountBuilder()
                        .accountId(account_id)
                        .domainId(domain_id)
                        .jsonData("{}")
                        .quorum(1)
                        .build());
  }

  std::shared_ptr<shared_model::interface::QueryResponse> validateAndExecute(
      const shared_model::interface::Query &query) {
    return factory->execute(query);
  }

  /**
   * Make transaction with specified parameters
   * @param counter
   * @param creator
   * @return wrapper with created transaction
   */
  wTransaction makeTransaction(int counter, std::string creator) {
    return clone(TestTransactionBuilder()
                     .creatorAccountId(creator)
                     .txCounter(counter)
                     .build());
  }

  /**
   * @param creator
   * @param N
   * @return observable with transactions
   */
  rxcpp::observable<wTransaction> getDefaultTransactions(
      const std::string &creator, size_t N) {
    return rxcpp::observable<>::iterate([&creator, &N, this] {
      std::vector<wTransaction> result;
      for (size_t i = 0; i < N; ++i) {
        auto current = makeTransaction(i, creator);
        result.push_back(current);
      }
      return result;
    }());
  }

  std::string admin_id = "admin@test", account_id = "test@test",
              asset_id = "coin#test", domain_id = "test";

  std::string admin_role = "admin";

  std::vector<std::string> admin_roles = {admin_role};
  std::vector<std::string> role_permissions;
  std::shared_ptr<shared_model::interface::Account> creator, account;
  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockBlockQuery> block_query;

  std::shared_ptr<QueryProcessingFactory> factory;
  std::shared_ptr<Query> query;
};

class GetAccountTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();

    role_permissions = {can_get_my_account};
  }
};

/**
 * @given initialized storage, permission to his/her account
 * @when get account information
 * @then Return account
 */
TEST_F(GetAccountTest, MyAccountValidCase) {
  // getAccount calls getAccountRoles and combines it into AccountResponse
  // In case when user is requesting her account the getAccountRoles will be
  // called twice: 1. To check permissions; 2. To create AccountResponse

  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccount(admin_id)
                   .build();

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .Times(2)
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccount(admin_id)).WillOnce(Return(creator));
  auto response = validateAndExecute(query);
  auto cast_resp =
      boost::get<w<shared_model::interface::AccountResponse>>(response->get());
  ASSERT_EQ(cast_resp->account().accountId(), admin_id);
}

/**
 * @given initialized storage, global permission
 * @when get account information about other user
 * @then Return account
 */
TEST_F(GetAccountTest, AllAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccount(account_id)
                   .build();

  role_permissions = {can_get_all_accounts};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccount(account_id)).WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getAccountRoles(account_id))
      .WillOnce(Return(admin_roles));
  auto response = validateAndExecute(query);
  auto cast_resp =
      boost::get<w<shared_model::interface::AccountResponse>>(response->get());
  ASSERT_EQ(cast_resp->account().accountId(), account_id);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the same domain
 * @then Return account
 */
TEST_F(GetAccountTest, DomainAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccount(account_id)
                   .build();

  role_permissions = {can_get_domain_accounts};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccount(account_id)).WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getAccountRoles(account_id))
      .WillOnce(Return(admin_roles));
  auto response = validateAndExecute(query);
  auto cast_resp =
      boost::get<w<shared_model::interface::AccountResponse>>(response->get());
  ASSERT_EQ(cast_resp->account().accountId(), account_id);
}

/**
 * @given initialized storage, granted permission
 * @when get account information about other user
 * @then Return users account
 */
TEST_F(GetAccountTest, GrantAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccount(account_id)
                   .build();

  role_permissions = {};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(admin_id, account_id, can_get_my_account))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(account_id)).WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getAccountRoles(account_id))
      .WillOnce(Return(admin_roles));
  auto response = validateAndExecute(query);
  auto cast_resp =
      boost::get<w<shared_model::interface::AccountResponse>>(response->get());
  ASSERT_EQ(cast_resp->account().accountId(), account_id);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the other domain
 * @then Return users account
 */
TEST_F(GetAccountTest, DifferentDomainAccountInValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccount("test@test2")
                   .build();

  role_permissions = {can_get_domain_accounts};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(admin_id, "test@test2", can_get_my_account))
      .WillOnce(Return(false));

  auto response = validateAndExecute(query);

  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatefulFailedErrorResponse>>(
          cast_resp->get()));
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing account
 * @then Return error
 */
TEST_F(GetAccountTest, NoAccountExist) {
  auto query =
      TestQueryBuilder().creatorAccountId(admin_id).getAccount("none").build();

  role_permissions = {can_get_all_accounts};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAccount("none")).WillOnce(Return(boost::none));
  EXPECT_CALL(*wsv_query, getAccountRoles("none"))
      .WillOnce(Return(boost::none));

  auto response = validateAndExecute(query);

  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::NoAccountErrorResponse>>(
          cast_resp->get()));
}

/// --------- Get Account Assets -------------
class GetAccountAssetsTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();

    std::shared_ptr<shared_model::interface::Amount> amount;
    shared_model::builder::AmountBuilder<
        shared_model::proto::AmountBuilder,
        shared_model::validation::FieldValidator>()
        .intValue(100)
        .precision(2)
        .build()
        .match(
            [&](iroha::expected::Value<
                std::shared_ptr<shared_model::interface::Amount>> &v) {
              amount = v.value;
            },
            [](iroha::expected::Error<std::shared_ptr<std::string>>) {});

    shared_model::builder::AccountAssetBuilder<
        shared_model::proto::AccountAssetBuilder,
        shared_model::validation::FieldValidator>()
        .assetId(asset_id)
        .accountId(admin_id)
        .balance(*amount)
        .build()
        .match(
            [&](iroha::expected::Value<
                std::shared_ptr<shared_model::interface::AccountAsset>> &v) {
              accountAsset = v.value;
            },
            [](iroha::expected::Error<std::shared_ptr<std::string>>) {});

    role_permissions = {can_get_my_acc_ast};
  }
  std::shared_ptr<shared_model::interface::AccountAsset> accountAsset;
};

/**
 * @given initialized storage, permission to his/her account
 * @when get account assets
 * @then Return account asset of user
 */
TEST_F(GetAccountAssetsTest, MyAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssets(admin_id, asset_id)
                   .build();

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccountAsset(admin_id, asset_id))
      .WillOnce(Return(accountAsset));
  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::AccountAssetResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->accountAsset().accountId(), admin_id);
  ASSERT_EQ(cast_resp->accountAsset().assetId(), asset_id);
}

/**
 * @given initialized storage, global permission
 * @when get account information about other user
 * @then Return account asset
 */
TEST_F(GetAccountAssetsTest, AllAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssets(account_id, asset_id)
                   .build();

  shared_model::builder::AccountAssetBuilder<
      shared_model::proto::AccountAssetBuilder,
      shared_model::validation::FieldValidator>()
      .assetId(accountAsset->assetId())
      .accountId(account_id)
      .balance(accountAsset->balance())
      .build()
      .match(
          [&](iroha::expected::Value<
              std::shared_ptr<shared_model::interface::AccountAsset>> &v) {
            accountAsset = v.value;
          },
          [](iroha::expected::Error<std::shared_ptr<std::string>>) {});
  role_permissions = {can_get_all_acc_ast};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccountAsset(account_id, asset_id))
      .WillOnce(Return(accountAsset));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::AccountAssetResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->accountAsset().accountId(), account_id);
  ASSERT_EQ(cast_resp->accountAsset().assetId(), asset_id);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the same domain
 * @then Return account
 */
TEST_F(GetAccountAssetsTest, DomainAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssets(account_id, asset_id)
                   .build();

  shared_model::builder::AccountAssetBuilder<
      shared_model::proto::AccountAssetBuilder,
      shared_model::validation::FieldValidator>()
      .assetId(accountAsset->assetId())
      .accountId(account_id)
      .balance(accountAsset->balance())
      .build()
      .match(
          [&](iroha::expected::Value<
              std::shared_ptr<shared_model::interface::AccountAsset>> &v) {
            accountAsset = v.value;
          },
          [](iroha::expected::Error<std::shared_ptr<std::string>>) {});
  role_permissions = {can_get_domain_acc_ast};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccountAsset(account_id, asset_id))
      .WillOnce(Return(accountAsset));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::AccountAssetResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->accountAsset().accountId(), account_id);
  ASSERT_EQ(cast_resp->accountAsset().assetId(), asset_id);
}

/**
 * @given initialized storage, granted permission
 * @when get account information about other user
 * @then Return account assets
 */
TEST_F(GetAccountAssetsTest, GrantAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssets(account_id, asset_id)
                   .build();

  accountAsset = clone(shared_model::proto::AccountAssetBuilder()
                           .assetId(accountAsset->assetId())
                           .accountId(account_id)
                           .balance(accountAsset->balance())
                           .build());
  role_permissions = {};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(admin_id, account_id, can_get_my_acc_ast))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccountAsset(account_id, asset_id))
      .WillOnce(Return(accountAsset));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::AccountAssetResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->accountAsset().accountId(), account_id);
  ASSERT_EQ(cast_resp->accountAsset().assetId(), asset_id);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the other domain
 * @then Return account assets
 */
TEST_F(GetAccountAssetsTest, DifferentDomainAccountInValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssets("test@test2", asset_id)
                   .build();

  shared_model::builder::AccountAssetBuilder<
      shared_model::proto::AccountAssetBuilder,
      shared_model::validation::FieldValidator>()
      .assetId(accountAsset->assetId())
      .accountId(account_id)
      .balance(accountAsset->balance())
      .build()
      .match(
          [&](iroha::expected::Value<
              std::shared_ptr<shared_model::interface::AccountAsset>> &v) {
            accountAsset = v.value;
          },
          [](iroha::expected::Error<std::shared_ptr<std::string>>) {});
  ;
  role_permissions = {can_get_domain_acc_ast};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(admin_id, "test@test2", can_get_my_acc_ast))
      .WillOnce(Return(false));

  auto response = validateAndExecute(query);

  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatefulFailedErrorResponse>>(
          cast_resp->get()));
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing account
 * @then Return error
 */
TEST_F(GetAccountAssetsTest, NoAccountExist) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssets("none", asset_id)
                   .build();

  shared_model::builder::AccountAssetBuilder<
      shared_model::proto::AccountAssetBuilder,
      shared_model::validation::FieldValidator>()
      .assetId(accountAsset->assetId())
      .accountId(account_id)
      .balance(accountAsset->balance())
      .build()
      .match(
          [&](iroha::expected::Value<
              std::shared_ptr<shared_model::interface::AccountAsset>> &v) {
            accountAsset = v.value;
          },
          [](iroha::expected::Error<std::shared_ptr<std::string>>) {});
  ;
  role_permissions = {can_get_all_acc_ast};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAccountAsset("none", asset_id))
      .WillOnce(Return(boost::none));

  auto response = validateAndExecute(query);

  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::NoAccountAssetsErrorResponse>>(
          cast_resp->get()));
}

/// --------- Get Signatories-------------
class GetSignatoriesTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    signs = {shared_model::interface::types::PubkeyType(std::string(32, '0'))};
    role_permissions = {can_get_my_signatories};
  }
  std::vector<shared_model::interface::types::PubkeyType> signs;
};

/**
 * @given initialized storage, permission to his/her account
 * @when get account assets
 * @then Return account asset of user
 */
TEST_F(GetSignatoriesTest, MyAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getSignatories(admin_id)
                   .build();

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getSignatories(admin_id)).WillOnce(Return(signs));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::SignatoriesResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->keys().size(), 1);
}

/**
 * @given initialized storage, global permission
 * @when get account information about other user
 * @then Return account asset
 */
TEST_F(GetSignatoriesTest, AllAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getSignatories(account_id)
                   .build();

  role_permissions = {can_get_all_signatories};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getSignatories(account_id)).WillOnce(Return(signs));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::SignatoriesResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->keys().size(), 1);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the same domain
 * @then Return account
 */
TEST_F(GetSignatoriesTest, DomainAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getSignatories(account_id)
                   .build();

  role_permissions = {can_get_domain_signatories};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getSignatories(account_id)).WillOnce(Return(signs));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::SignatoriesResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->keys().size(), 1);
}

/**
 * @given initialized storage, granted permission
 * @when get account information about other user
 * @then Return signatories
 */
TEST_F(GetSignatoriesTest, GrantAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getSignatories(account_id)
                   .build();

  role_permissions = {};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, account_id, can_get_my_signatories))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getSignatories(account_id)).WillOnce(Return(signs));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::SignatoriesResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->keys().size(), 1);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the other domain
 * @then Return signatories
 */
TEST_F(GetSignatoriesTest, DifferentDomainAccountInValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getSignatories("test@test2")
                   .build();

  role_permissions = {can_get_domain_signatories};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, "test@test2", can_get_my_signatories))
      .WillOnce(Return(false));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatefulFailedErrorResponse>>(
          cast_resp->get()));
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing account
 * @then Return error
 */
TEST_F(GetSignatoriesTest, NoAccountExist) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getSignatories("none")
                   .build();

  role_permissions = {can_get_all_signatories};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getSignatories("none")).WillOnce(Return(boost::none));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::NoSignatoriesErrorResponse>>(
          cast_resp->get()));
}

/// --------- Get Account Transactions-------------
class GetAccountTransactionsTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    role_permissions = {can_get_my_acc_txs};
    txs_observable = getDefaultTransactions(account_id, N);
  }

  rxcpp::observable<wTransaction> txs_observable;
  size_t N = 3;
};

/**
 * @given initialized storage, permission to his/her account
 * @when get account assets
 * @then Return account asset of user
 */
TEST_F(GetAccountTransactionsTest, MyAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountTransactions(admin_id)
                   .build();

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  txs_observable = getDefaultTransactions(admin_id, N);

  EXPECT_CALL(*block_query, getAccountTransactions(admin_id))
      .WillOnce(Return(txs_observable));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::TransactionsResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->transactions().size(), N);
  for (const auto &tx : cast_resp->transactions()) {
    EXPECT_EQ(admin_id, tx->creatorAccountId());
  }
}

/**
 * @given initialized storage, global permission
 * @when get account information about other user
 * @then Return account asset
 */
TEST_F(GetAccountTransactionsTest, AllAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountTransactions(account_id)
                   .build();

  role_permissions = {can_get_all_acc_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountTransactions(account_id))
      .WillOnce(Return(txs_observable));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::TransactionsResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->transactions().size(), N);
  for (const auto &tx : cast_resp->transactions()) {
    EXPECT_EQ(account_id, tx->creatorAccountId());
  }
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the same domain
 * @then Return account
 */
TEST_F(GetAccountTransactionsTest, DomainAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountTransactions(account_id)
                   .build();

  role_permissions = {can_get_domain_acc_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountTransactions(account_id))
      .WillOnce(Return(txs_observable));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::TransactionsResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->transactions().size(), N);
  for (const auto &tx : cast_resp->transactions()) {
    EXPECT_EQ(account_id, tx->creatorAccountId());
  }
}

/**
 * @given initialized storage, granted permission
 * @when get account information about other user
 * @then Return error
 */
TEST_F(GetAccountTransactionsTest, GrantAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountTransactions(account_id)
                   .build();

  role_permissions = {};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(admin_id, account_id, can_get_my_acc_txs))
      .WillOnce(Return(true));

  EXPECT_CALL(*block_query, getAccountTransactions(account_id))
      .WillOnce(Return(txs_observable));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::TransactionsResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->transactions().size(), N);
  for (const auto &tx : cast_resp->transactions()) {
    EXPECT_EQ(account_id, tx->creatorAccountId());
  }
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the other domain
 * @then Return error
 */
TEST_F(GetAccountTransactionsTest, DifferentDomainAccountInValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountTransactions("test@test2")
                   .build();

  role_permissions = {can_get_domain_acc_ast};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(admin_id, "test@test2", can_get_my_acc_txs))
      .WillOnce(Return(false));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatefulFailedErrorResponse>>(
          cast_resp->get()));
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing account
 * @then Return empty response
 */
TEST_F(GetAccountTransactionsTest, NoAccountExist) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountTransactions("none")
                   .build();

  role_permissions = {can_get_all_acc_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountTransactions("none"))
      .WillOnce(Return(rxcpp::observable<>::empty<wTransaction>()));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::TransactionsResponse>>(
      response->get());
}

/// --------- Get Account Assets Transactions-------------
class GetAccountAssetsTransactionsTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    role_permissions = {can_get_my_acc_ast_txs};
    txs_observable = getDefaultTransactions(account_id, N);
  }

  rxcpp::observable<wTransaction> txs_observable;
  size_t N = 3;
};

/**
 * @given initialized storage, permission to his/her account
 * @when get account assets
 * @then Return account asset of user
 */
TEST_F(GetAccountAssetsTransactionsTest, MyAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssetTransactions(admin_id, asset_id)
                   .build();

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  txs_observable = getDefaultTransactions(admin_id, N);

  EXPECT_CALL(*block_query, getAccountAssetTransactions(admin_id, asset_id))
      .WillOnce(Return(txs_observable));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::TransactionsResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->transactions().size(), N);
  for (const auto &tx : cast_resp->transactions()) {
    EXPECT_EQ(admin_id, tx->creatorAccountId());
  }
}

/**
 * @given initialized storage, global permission
 * @when get account information about other user
 * @then Return account asset
 */
TEST_F(GetAccountAssetsTransactionsTest, AllAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssetTransactions(account_id, asset_id)
                   .build();

  role_permissions = {can_get_all_acc_ast_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountAssetTransactions(account_id, asset_id))
      .WillOnce(Return(txs_observable));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::TransactionsResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->transactions().size(), N);
  for (const auto &tx : cast_resp->transactions()) {
    EXPECT_EQ(account_id, tx->creatorAccountId());
  }
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the same domain
 * @then Return account
 */
TEST_F(GetAccountAssetsTransactionsTest, DomainAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssetTransactions(account_id, asset_id)
                   .build();

  role_permissions = {can_get_domain_acc_ast_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountAssetTransactions(account_id, asset_id))
      .WillOnce(Return(txs_observable));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::TransactionsResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->transactions().size(), N);
  for (const auto &tx : cast_resp->transactions()) {
    EXPECT_EQ(account_id, tx->creatorAccountId());
  }
}

/**
 * @given initialized storage, granted permission
 * @when get account information about other user
 * @then Return error
 */
TEST_F(GetAccountAssetsTransactionsTest, GrantAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssetTransactions(account_id, asset_id)
                   .build();

  role_permissions = {};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, account_id, can_get_my_acc_ast_txs))
      .WillOnce(Return(true));

  EXPECT_CALL(*block_query, getAccountAssetTransactions(account_id, asset_id))
      .WillOnce(Return(txs_observable));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::TransactionsResponse>>(
      response->get());

  ASSERT_EQ(cast_resp->transactions().size(), N);
  for (const auto &tx : cast_resp->transactions()) {
    EXPECT_EQ(account_id, tx->creatorAccountId());
  }
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the other domain
 * @then Return error
 */
TEST_F(GetAccountAssetsTransactionsTest, DifferentDomainAccountInValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssetTransactions("test@test2", asset_id)
                   .build();

  role_permissions = {can_get_domain_acc_ast_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, "test@test2", can_get_my_acc_ast_txs))
      .WillOnce(Return(false));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatefulFailedErrorResponse>>(
          cast_resp->get()));
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing account
 * @then Return empty response
 */
TEST_F(GetAccountAssetsTransactionsTest, NoAccountExist) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssetTransactions("none", asset_id)
                   .build();

  role_permissions = {can_get_all_acc_ast_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountAssetTransactions("none", asset_id))
      .WillOnce(Return(rxcpp::observable<>::empty<wTransaction>()));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::TransactionsResponse>>(
      response->get());
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing asset
 * @then Return empty response
 */
TEST_F(GetAccountAssetsTransactionsTest, NoAssetExist) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAccountAssetTransactions(account_id, "none")
                   .build();

  role_permissions = {can_get_all_acc_ast_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountAssetTransactions(account_id, "none"))
      .WillOnce(Return(rxcpp::observable<>::empty<wTransaction>()));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::TransactionsResponse>>(
      response->get());
}

/// --------- Get Asset Info -------------
class GetAssetInfoTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    role_permissions = {can_read_assets};
    asset = clone(shared_model::proto::AssetBuilder()
                      .assetId(asset_id)
                      .domainId("test")
                      .precision(2)
                      .build());
  }
  std::shared_ptr<shared_model::interface::Asset> asset;
};

/**
 * @given initialized storage, permission to read all system assets
 * @when get asset info
 * @then Return asset
 */
TEST_F(GetAssetInfoTest, MyAccountValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAssetInfo(asset_id)
                   .build();

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));

  auto response = validateAndExecute(query);
  auto cast_resp =
      boost::get<w<shared_model::interface::AssetResponse>>(response->get());

  ASSERT_EQ(cast_resp->asset().assetId(), asset_id);
}

/**
 * @given initialized storage, no permissions
 * @when get asset info
 * @then Error
 */
TEST_F(GetAssetInfoTest, PermissionsInvalidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAssetInfo(asset_id)
                   .build();

  role_permissions = {};
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatefulFailedErrorResponse>>(
          cast_resp->get()));
}

/**
 * @given initialized storage, all permissions
 * @when get asset info of non existing asset
 * @then Error
 */
TEST_F(GetAssetInfoTest, AssetInvalidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getAssetInfo("none")
                   .build();

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset("none")).WillOnce(Return(boost::none));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(boost::get<w<shared_model::interface::NoAssetErrorResponse>>(
      cast_resp->get()));
}

/// --------- Get Roles -------------
class GetRolesTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    role_permissions = {can_get_roles};
    roles = {admin_role, "some_role"};
  }
  std::vector<std::string> roles;
  std::shared_ptr<GetRoles> qry;
};

/**
 * @given initialized storage, permission to read all roles
 * @when get system roles
 * @then Return roles
 */
TEST_F(GetRolesTest, ValidCase) {
  auto query = TestQueryBuilder().creatorAccountId(admin_id).getRoles().build();

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRoles()).WillOnce(Return(roles));

  auto response = validateAndExecute(query);
  auto cast_resp =
      boost::get<w<shared_model::interface::RolesResponse>>(response->get());

  ASSERT_EQ(cast_resp->roles().size(), roles.size());

  for (size_t i = 0; i < roles.size(); ++i) {
    ASSERT_EQ(cast_resp->roles().at(i), roles.at(i));
  }
}

/**
 * @given initialized storage, no permission to read all roles
 * @when get system roles
 * @then Return Error
 */
TEST_F(GetRolesTest, InValidCaseNoPermissions) {
  auto query = TestQueryBuilder().creatorAccountId(admin_id).getRoles().build();

  role_permissions = {};
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatefulFailedErrorResponse>>(
          cast_resp->get()));
}

/**
 * @given initialized storage, no roles exist
 * @when get system roles
 * @then Return Error
 */
TEST_F(GetRolesTest, InValidCaseNoRoles) {
  auto query = TestQueryBuilder().creatorAccountId(admin_id).getRoles().build();

  admin_roles = {};
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatefulFailedErrorResponse>>(
          cast_resp->get()));
}

/// --------- Get Role Permissions -------------
class GetRolePermissionsTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    role_permissions = {can_get_roles};
    perms = {can_get_my_account, can_get_my_signatories};
  }
  std::string role_id = "user";
  std::vector<std::string> perms;
  std::shared_ptr<GetRolePermissions> qry;
};

/**
 * @given initialized storage, permission to read all roles
 * @when get system roles
 * @then Return roles
 */
TEST_F(GetRolePermissionsTest, ValidCase) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getRolePermissions(role_id)
                   .build();

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRolePermissions(role_id)).WillOnce(Return(perms));

  auto response = validateAndExecute(query);
  auto cast_resp =
      boost::get<w<shared_model::interface::RolePermissionsResponse>>(
          response->get());

  ASSERT_EQ(cast_resp->rolePermissions().size(), perms.size());
  for (size_t i = 0; i < perms.size(); ++i) {
    ASSERT_EQ(cast_resp->rolePermissions().at(i), perms.at(i));
  }
}

/**
 * @given initialized storage, no permission to read all roles
 * @when get system roles
 * @then Return Error
 */
TEST_F(GetRolePermissionsTest, InValidCaseNoPermissions) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getRolePermissions(role_id)
                   .build();

  role_permissions = {};
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatefulFailedErrorResponse>>(
          cast_resp->get()));
}

/**
 * @given initialized storage,  permission to read all roles, no role exist
 * @when get system roles
 * @then Return Error
 */
TEST_F(GetRolePermissionsTest, InValidCaseNoRole) {
  auto query = TestQueryBuilder()
                   .creatorAccountId(admin_id)
                   .getRolePermissions(role_id)
                   .build();

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRolePermissions(role_id))
      .WillOnce(Return(boost::none));

  auto response = validateAndExecute(query);
  auto cast_resp = boost::get<w<shared_model::interface::ErrorQueryResponse>>(
      response->get());

  ASSERT_NO_THROW(boost::get<w<shared_model::interface::NoRolesErrorResponse>>(
      cast_resp->get()));
}
