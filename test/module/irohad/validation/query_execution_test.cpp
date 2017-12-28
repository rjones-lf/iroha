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

#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "model/permissions.hpp"
#include "model/generators/command_generator.hpp"
#include "model/generators/transaction_generator.hpp"
#include "model/generators/query_generator.hpp"
#include "model/generators/command_generator.hpp"
#include "model/queries/responses/account_assets_response.hpp"
#include "model/queries/responses/account_response.hpp"
#include "model/queries/responses/asset_response.hpp"
#include "model/queries/responses/error_response.hpp"
#include "model/queries/responses/roles_response.hpp"
#include "model/queries/responses/signatories_response.hpp"
#include "model/queries/responses/transactions_response.hpp"
#include "model/queries/pager.hpp"
#include "model/query_execution.hpp"

using ::testing::Return;
using ::testing::AtLeast;
using ::testing::_;
using ::testing::AllOf;
using ::testing::StrictMock;

using namespace iroha::ametsuchi;
using namespace iroha::model;
using namespace iroha::model::generators;
using namespace framework::test_subscriber;

static const auto NO_PAGER = Pager{
  iroha::hash256_t{}, Pager::MAX_PAGER_LIMIT};

class QueryValidateExecuteTest : public ::testing::Test {
 public:
  QueryValidateExecuteTest() = default;

  void SetUp() override {
    wsv_query = std::make_shared<StrictMock<MockWsvQuery>>();
    block_query = std::make_shared<StrictMock<MockBlockQuery>>();
    factory = std::make_shared<QueryProcessingFactory>(wsv_query, block_query);

    EXPECT_CALL(*wsv_query, hasAccountGrantablePermission(_, _, _))
        .WillRepeatedly(Return(false));

    creator.account_id = admin_id;
    creator.domain_id = domain_id;
    creator.json_data = "{}";
    creator.quorum = 1;

    account.account_id = account_id;
    account.domain_id = domain_id;
    account.json_data = "{}";
    account.quorum = 1;
  }

  std::shared_ptr<QueryResponse> validateAndExecute() {
    return factory->execute(query);
  }

  std::string admin_id = "admin@test", account_id = "test@test",
              asset_id = "coin#test", domain_id = "test";

  std::string admin_role = "admin";

  std::vector<std::string> admin_roles = {admin_role};
  std::vector<std::string> role_permissions;
  Account creator, account;
  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockBlockQuery> block_query;

  std::shared_ptr<QueryProcessingFactory> factory;
  std::shared_ptr<Query> query;
};

class GetAccountTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    get_account = std::make_shared<GetAccount>();
    get_account->account_id = admin_id;
    get_account->creator_account_id = admin_id;
    query = get_account;

    role_permissions = {can_get_my_account};
  }
  std::shared_ptr<GetAccount> get_account;
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
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .Times(2)
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccount(admin_id)).WillOnce(Return(creator));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<AccountResponse>(response);
  ASSERT_EQ(cast_resp->account.account_id, admin_id);
}

/**
 * @given initialized storage, global permission
 * @when get account information about other user
 * @then Return account
 */
TEST_F(GetAccountTest, AllAccountValidCase) {
  get_account->account_id = account_id;
  get_account->creator_account_id = admin_id;
  role_permissions = {can_get_all_accounts};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccount(account_id)).WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getAccountRoles(account_id))
      .WillOnce(Return(admin_roles));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<AccountResponse>(response);
  ASSERT_EQ(cast_resp->account.account_id, account_id);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the same domain
 * @then Return account
 */
TEST_F(GetAccountTest, DomainAccountValidCase) {
  get_account->account_id = account_id;
  get_account->creator_account_id = admin_id;
  role_permissions = {can_get_domain_accounts};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccount(account_id)).WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getAccountRoles(account_id))
      .WillOnce(Return(admin_roles));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<AccountResponse>(response);
  ASSERT_EQ(cast_resp->account.account_id, account_id);
}

/**
 * @given initialized storage, granted permission
 * @when get account information about other user
 * @then Return users account
 */
TEST_F(GetAccountTest, GrantAccountValidCase) {
  get_account->account_id = account_id;
  get_account->creator_account_id = admin_id;
  role_permissions = {};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, get_account->account_id, can_get_my_account))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(account_id)).WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getAccountRoles(account_id))
      .WillOnce(Return(admin_roles));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<AccountResponse>(response);
  ASSERT_EQ(cast_resp->account.account_id, account_id);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the other domain
 * @then Return users account
 */
TEST_F(GetAccountTest, DifferentDomainAccountInValidCase) {
  get_account->account_id = "test@test2";  // other domain
  get_account->creator_account_id = admin_id;
  role_permissions = {can_get_domain_accounts};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, get_account->account_id, can_get_my_account))
      .WillOnce(Return(false));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::STATEFUL_INVALID);
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing account
 * @then Return error
 */
TEST_F(GetAccountTest, NoAccountExist) {
  get_account->account_id = "none";
  get_account->creator_account_id = admin_id;
  role_permissions = {can_get_all_accounts};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAccount(get_account->account_id))
      .WillOnce(Return(nonstd::nullopt));
  EXPECT_CALL(*wsv_query, getAccountRoles(get_account->account_id))
      .WillOnce(Return(nonstd::nullopt));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::NO_ACCOUNT);
}

/// --------- Get Account Assets -------------
class GetAccountAssetsTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    get_account_assets = std::make_shared<GetAccountAssets>();
    get_account_assets->account_id = admin_id;
    get_account_assets->asset_id = asset_id;
    get_account_assets->creator_account_id = admin_id;
    query = get_account_assets;

    accountAsset.asset_id = asset_id;
    accountAsset.account_id = admin_id;
    iroha::Amount amount(100, 2);
    accountAsset.balance = amount;

    role_permissions = {can_get_my_acc_ast};
  }
  std::shared_ptr<GetAccountAssets> get_account_assets;
  AccountAsset accountAsset;
};

/**
 * @given initialized storage, permission to his/her account
 * @when get account assets
 * @then Return account asset of user
 */
TEST_F(GetAccountAssetsTest, MyAccountValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccountAsset(admin_id, asset_id))
      .WillOnce(Return(accountAsset));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<AccountAssetResponse>(response);
  ASSERT_EQ(cast_resp->acct_asset.account_id, admin_id);
  ASSERT_EQ(cast_resp->acct_asset.asset_id, asset_id);
}

/**
 * @given initialized storage, global permission
 * @when get account information about other user
 * @then Return account asset
 */
TEST_F(GetAccountAssetsTest, AllAccountValidCase) {
  get_account_assets->account_id = account_id;
  accountAsset.account_id = account_id;
  role_permissions = {can_get_all_acc_ast};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccountAsset(account_id, asset_id))
      .WillOnce(Return(accountAsset));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<AccountAssetResponse>(response);
  ASSERT_EQ(cast_resp->acct_asset.account_id, account_id);
  ASSERT_EQ(cast_resp->acct_asset.asset_id, asset_id);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the same domain
 * @then Return account
 */
TEST_F(GetAccountAssetsTest, DomainAccountValidCase) {
  get_account_assets->account_id = account_id;
  accountAsset.account_id = account_id;
  role_permissions = {can_get_domain_acc_ast};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccountAsset(account_id, asset_id))
      .WillOnce(Return(accountAsset));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<AccountAssetResponse>(response);
  ASSERT_EQ(cast_resp->acct_asset.account_id, account_id);
  ASSERT_EQ(cast_resp->acct_asset.asset_id, asset_id);
}

/**
 * @given initialized storage, granted permission
 * @when get account information about other user
 * @then Return account assets
 */
TEST_F(GetAccountAssetsTest, GrantAccountValidCase) {
  get_account_assets->account_id = account_id;
  accountAsset.account_id = account_id;
  role_permissions = {};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, get_account_assets->account_id, can_get_my_acc_ast))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccountAsset(account_id, asset_id))
      .WillOnce(Return(accountAsset));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<AccountAssetResponse>(response);
  ASSERT_EQ(cast_resp->acct_asset.account_id, account_id);
  ASSERT_EQ(cast_resp->acct_asset.asset_id, asset_id);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the other domain
 * @then Return account assets
 */
TEST_F(GetAccountAssetsTest, DifferentDomainAccountInValidCase) {
  get_account_assets->account_id = "test@test2";
  accountAsset.account_id = account_id;
  role_permissions = {can_get_domain_acc_ast};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, get_account_assets->account_id, can_get_my_acc_ast))
      .WillOnce(Return(false));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::STATEFUL_INVALID);
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing account
 * @then Return error
 */
TEST_F(GetAccountAssetsTest, NoAccountExist) {
  get_account_assets->account_id = "none";
  accountAsset.account_id = account_id;
  role_permissions = {can_get_all_acc_ast};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_query,
              getAccountAsset(get_account_assets->account_id, asset_id))
      .WillOnce(Return(nonstd::nullopt));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::NO_ACCOUNT_ASSETS);
}

/// --------- Get Signatories-------------
class GetSignatoriesTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    get_signatories = std::make_shared<GetSignatories>();
    get_signatories->account_id = admin_id;
    get_signatories->creator_account_id = admin_id;
    query = get_signatories;
    signs = {iroha::pubkey_t()};
    role_permissions = {can_get_my_signatories};
  }
  std::shared_ptr<GetSignatories> get_signatories;
  std::vector<iroha::pubkey_t> signs;
};

/**
 * @given initialized storage, permission to his/her account
 * @when get account assets
 * @then Return account asset of user
 */
TEST_F(GetSignatoriesTest, MyAccountValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getSignatories(admin_id)).WillOnce(Return(signs));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<::SignatoriesResponse>(response);
  ASSERT_EQ(cast_resp->keys.size(), 1);
}

/**
 * @given initialized storage, global permission
 * @when get account information about other user
 * @then Return account asset
 */
TEST_F(GetSignatoriesTest, AllAccountValidCase) {
  get_signatories->account_id = account_id;
  role_permissions = {can_get_all_signatories};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getSignatories(account_id)).WillOnce(Return(signs));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<::SignatoriesResponse>(response);
  ASSERT_EQ(cast_resp->keys.size(), 1);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the same domain
 * @then Return account
 */
TEST_F(GetSignatoriesTest, DomainAccountValidCase) {
  get_signatories->account_id = account_id;
  role_permissions = {can_get_domain_signatories};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getSignatories(account_id)).WillOnce(Return(signs));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<::SignatoriesResponse>(response);
  ASSERT_EQ(cast_resp->keys.size(), 1);
}

/**
 * @given initialized storage, granted permission
 * @when get account information about other user
 * @then Return signatories
 */
TEST_F(GetSignatoriesTest, GrantAccountValidCase) {
  get_signatories->account_id = account_id;
  role_permissions = {};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, get_signatories->account_id, can_get_my_signatories))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getSignatories(account_id)).WillOnce(Return(signs));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<::SignatoriesResponse>(response);
  ASSERT_EQ(cast_resp->keys.size(), 1);
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the other domain
 * @then Return signatories
 */
TEST_F(GetSignatoriesTest, DifferentDomainAccountInValidCase) {
  get_signatories->account_id = "test@test2";
  role_permissions = {can_get_domain_signatories};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, get_signatories->account_id, can_get_my_signatories))
      .WillOnce(Return(false));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<::ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::STATEFUL_INVALID);
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing account
 * @then Return error
 */
TEST_F(GetSignatoriesTest, NoAccountExist) {
  get_signatories->account_id = "none";
  role_permissions = {can_get_all_signatories};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getSignatories(get_signatories->account_id))
      .WillOnce(Return(nonstd::nullopt));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<::ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::NO_SIGNATORIES);
}

/// --------- Get Account Transactions-------------
class GetAccountTransactionsTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    get_tx = std::make_shared<GetAccountTransactions>();
    get_tx->account_id = admin_id;
    get_tx->creator_account_id = admin_id;
    get_tx->pager = NO_PAGER;
    query = get_tx;
    role_permissions = {can_get_my_acc_txs};
    txs_observable = getDefaultTransactions(account_id);
  }

  rxcpp::observable<Transaction> getDefaultTransactions(
      const std::string &creator) {
    return rxcpp::observable<>::iterate([&creator, this] {
      std::vector<::Transaction> result;
      for (size_t i = 0; i < N; ++i) {
        Transaction current;
        current.creator_account_id = creator;
        current.tx_counter = i;
        result.push_back(current);
      }
      return result;
    }());
  }

  rxcpp::observable<Transaction> txs_observable;
  std::shared_ptr<GetAccountTransactions> get_tx;
  size_t N = 3;
};

/**
 * @given initialized storage, permission to his/her account
 * @when get account assets
 * @then Return account asset of user
 */
TEST_F(GetAccountTransactionsTest, MyAccountValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  txs_observable = getDefaultTransactions(admin_id);

  EXPECT_CALL(*block_query, getAccountTransactions(admin_id, NO_PAGER))
      .WillOnce(Return(txs_observable));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<TransactionsResponse>(response);

  auto TxWrapper = make_test_subscriber<CallExact>(txs_observable, N);
  TxWrapper.subscribe(
      [this](auto val) { EXPECT_EQ(admin_id, val.creator_account_id); });
  ASSERT_TRUE(TxWrapper.validate());
}

/**
 * @given initialized storage, global permission
 * @when get account information about other user
 * @then Return account asset
 */
TEST_F(GetAccountTransactionsTest, AllAccountValidCase) {
  get_tx->account_id = account_id;
  role_permissions = {can_get_all_acc_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountTransactions(account_id, NO_PAGER))
      .WillOnce(Return(txs_observable));
  auto response = validateAndExecute();

  auto TxWrapper = make_test_subscriber<CallExact>(txs_observable, N);
  TxWrapper.subscribe(
      [this](auto val) { EXPECT_EQ(account_id, val.creator_account_id); });
  ASSERT_TRUE(TxWrapper.validate());
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the same domain
 * @then Return account
 */
TEST_F(GetAccountTransactionsTest, DomainAccountValidCase) {
  get_tx->account_id = account_id;
  role_permissions = {can_get_domain_acc_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountTransactions(account_id, NO_PAGER))
      .WillOnce(Return(txs_observable));
  auto response = validateAndExecute();

  auto TxWrapper = make_test_subscriber<CallExact>(txs_observable, N);
  TxWrapper.subscribe(
      [this](auto val) { EXPECT_EQ(account_id, val.creator_account_id); });
  ASSERT_TRUE(TxWrapper.validate());
}

/**
 * @given initialized storage, granted permission
 * @when get account information about other user
 * @then Return error
 */
TEST_F(GetAccountTransactionsTest, GrantAccountValidCase) {
  get_tx->account_id = account_id;
  role_permissions = {};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, get_tx->account_id, can_get_my_acc_txs))
      .WillOnce(Return(true));

  EXPECT_CALL(*block_query, getAccountTransactions(account_id, NO_PAGER))
      .WillOnce(Return(txs_observable));
  auto response = validateAndExecute();

  auto TxWrapper = make_test_subscriber<CallExact>(txs_observable, N);
  TxWrapper.subscribe(
      [this](auto val) { EXPECT_EQ(account_id, val.creator_account_id); });
  ASSERT_TRUE(TxWrapper.validate());
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the other domain
 * @then Return error
 */
TEST_F(GetAccountTransactionsTest, DifferentDomainAccountInValidCase) {
  get_tx->account_id = "test@test2";
  role_permissions = {can_get_domain_acc_ast};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, get_tx->account_id, can_get_my_acc_txs))
      .WillOnce(Return(false));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::STATEFUL_INVALID);
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing account
 * @then Return empty response
 */
TEST_F(GetAccountTransactionsTest, NoAccountExist) {
  get_tx->account_id = "none";
  role_permissions = {can_get_all_acc_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountTransactions(get_tx->account_id, NO_PAGER))
      .WillOnce(Return(rxcpp::observable<>::empty<Transaction>()));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<TransactionsResponse>(response);
}

/// --------- Get Account Asset Transactions -------------
class GetAccountAssetTransactionsTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    get_tx = std::make_shared<GetAccountAssetTransactions>();
    get_tx->pager = NO_PAGER;
    get_tx->assets_id.emplace_back(asset_id);
    get_tx->account_id = account_id;
    get_tx->creator_account_id = admin_id;
    query = get_tx;
    role_permissions = {can_get_my_acc_ast_txs};
    txs_observable = getDefaultTransactions(account_id, asset_id);
  }

  rxcpp::observable<Transaction> getDefaultTransactions(
      const std::string &creator_id, const std::string &asset_id) {
    return rxcpp::observable<>::iterate([&creator_id, asset_id, this] {
      std::vector<::Transaction> result;
      for (size_t i = 0; i < N; ++i) {
        Transaction current;
        current.creator_account_id = creator_id;
        current.tx_counter = i;
        result.push_back(current);
      }
      return result;
    }());
  }

  std::vector<std::string> assets = {asset_id};
  rxcpp::observable<Transaction> txs_observable;
  std::shared_ptr<GetAccountAssetTransactions> get_tx;
  size_t N = 3;
};

/**
 * @given initialized storage, permission to his/her account
 * @when get account assets
 * @then Return account asset of user
 */
TEST_F(GetAccountAssetTransactionsTest, MyAccountValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  get_tx->account_id = admin_id;
  txs_observable = getDefaultTransactions(admin_id, asset_id);

  EXPECT_CALL(*block_query, getAccountAssetTransactions(admin_id, assets, NO_PAGER))
      .WillOnce(Return(txs_observable));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<TransactionsResponse>(response);

  auto TxWrapper = make_test_subscriber<CallExact>(txs_observable, N);
  TxWrapper.subscribe(
      [this](auto val) { EXPECT_EQ(admin_id, val.creator_account_id); });
  ASSERT_TRUE(TxWrapper.validate());
}

/**
 * @given initialized storage, global permission
 * @when get account information about other user
 * @then Return account asset
 */
TEST_F(GetAccountAssetTransactionsTest, AllAccountValidCase) {
  get_tx->account_id = account_id;
  role_permissions = {can_get_all_acc_ast_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountAssetTransactions(account_id, assets, NO_PAGER))
      .WillOnce(Return(txs_observable));
  auto response = validateAndExecute();

  auto TxWrapper = make_test_subscriber<CallExact>(txs_observable, N);
  TxWrapper.subscribe(
      [this](auto val) { EXPECT_EQ(account_id, val.creator_account_id); });
  ASSERT_TRUE(TxWrapper.validate());
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the same domain
 * @then Return account
 */
TEST_F(GetAccountAssetTransactionsTest, DomainAccountValidCase) {
  get_tx->account_id = account_id;
  role_permissions = {can_get_domain_acc_ast_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query, getAccountAssetTransactions(account_id, assets, NO_PAGER))
      .WillOnce(Return(txs_observable));
  auto response = validateAndExecute();

  auto TxWrapper = make_test_subscriber<CallExact>(txs_observable, N);
  TxWrapper.subscribe(
      [this](auto val) { EXPECT_EQ(account_id, val.creator_account_id); });
  ASSERT_TRUE(TxWrapper.validate());
}

/**
 * @given initialized storage, granted permission
 * @when get account information about other user
 * @then Return error
 */
TEST_F(GetAccountAssetTransactionsTest, GrantAccountValidCase) {
  get_tx->account_id = account_id;
  role_permissions = {};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, get_tx->account_id, can_get_my_acc_ast_txs))
      .WillOnce(Return(true));

  EXPECT_CALL(*block_query, getAccountAssetTransactions(account_id, assets, NO_PAGER))
      .WillOnce(Return(txs_observable));
  auto response = validateAndExecute();

  auto TxWrapper = make_test_subscriber<CallExact>(txs_observable, N);
  TxWrapper.subscribe(
      [this](auto val) { EXPECT_EQ(account_id, val.creator_account_id); });
  ASSERT_TRUE(TxWrapper.validate());
}

/**
 * @given initialized storage, domain permission
 * @when get account information about other user in the other domain
 * @then Return error
 */
TEST_F(GetAccountAssetTransactionsTest, DifferentDomainAccountInValidCase) {
  get_tx->account_id = "test@test2";
  role_permissions = {can_get_domain_acc_ast_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, get_tx->account_id, can_get_my_acc_ast_txs))
      .WillOnce(Return(false));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::STATEFUL_INVALID);
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing account
 * @then Return empty response
 */
TEST_F(GetAccountAssetTransactionsTest, NoAccountExist) {
  get_tx->account_id = "none";
  role_permissions = {can_get_all_acc_ast_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query,
              getAccountAssetTransactions(get_tx->account_id, assets, NO_PAGER))
      .WillOnce(Return(rxcpp::observable<>::empty<Transaction>()));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<TransactionsResponse>(response);
}

/**
 * @given initialized storage, permission
 * @when get account information about non existing asset
 * @then Return empty response
 */
TEST_F(GetAccountAssetTransactionsTest, NoAssetExist) {
  get_tx->account_id = account_id;
  get_tx->assets_id.emplace_back("none");
  role_permissions = {can_get_all_acc_ast_txs};

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*block_query,
              getAccountAssetTransactions(get_tx->account_id, get_tx->assets_id, NO_PAGER))
      .WillOnce(Return(rxcpp::observable<>::empty<Transaction>()));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<TransactionsResponse>(response);
}

/// --------- Get Asset Info -------------
class GetAssetInfoTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    qry = std::make_shared<GetAssetInfo>();
    qry->asset_id = asset_id;
    qry->creator_account_id = admin_id;
    query = qry;
    role_permissions = {can_read_assets};
    asset = Asset(asset_id, "test", 2);
  }
  Asset asset;
  std::shared_ptr<GetAssetInfo> qry;
};

/**
 * @given initialized storage, permission to read all system assets
 * @when get asset info
 * @then Return asset
 */
TEST_F(GetAssetInfoTest, MyAccountValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<AssetResponse>(response);
  ASSERT_EQ(cast_resp->asset.asset_id, asset_id);
}

/**
 * @given initialized storage, no permissions
 * @when get asset info
 * @then Error
 */
TEST_F(GetAssetInfoTest, PermissionsInvalidCase) {
  role_permissions = {};
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::STATEFUL_INVALID);
}

/**
 * @given initialized storage, all permissions
 * @when get asset info of non existing asset
 * @then Error
 */
TEST_F(GetAssetInfoTest, AssetInvalidCase) {
  qry->asset_id = "none";
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset(qry->asset_id))
      .WillOnce(Return(nonstd::nullopt));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::NO_ASSET);
}

/// --------- Get Roles -------------
class GetRolesTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    qry = std::make_shared<GetRoles>();
    qry->creator_account_id = admin_id;
    query = qry;
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
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRoles()).WillOnce(Return(roles));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<RolesResponse>(response);
  ASSERT_EQ(cast_resp->roles.size(), roles.size());
  for (size_t i = 0; i < roles.size(); ++i) {
    ASSERT_EQ(cast_resp->roles.at(i), roles.at(i));
  }
}

/**
 * @given initialized storage, no permission to read all roles
 * @when get system roles
 * @then Return Error
 */
TEST_F(GetRolesTest, InValidCaseNoPermissions) {
  role_permissions = {};
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::STATEFUL_INVALID);
}

/**
 * @given initialized storage, no roles exist
 * @when get system roles
 * @then Return Error
 */
TEST_F(GetRolesTest, InValidCaseNoRoles) {
  admin_roles = {};
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::STATEFUL_INVALID);
}

/// --------- Get Role Permissions -------------
class GetRolePermissionsTest : public QueryValidateExecuteTest {
 public:
  void SetUp() override {
    QueryValidateExecuteTest::SetUp();
    qry = std::make_shared<GetRolePermissions>();
    qry->creator_account_id = admin_id;
    qry->role_id = "user";
    query = qry;
    role_permissions = {can_get_roles};
    perms = {can_get_my_account, can_get_my_signatories};
  }
  std::vector<std::string> perms;
  std::shared_ptr<GetRolePermissions> qry;
};

/**
 * @given initialized storage, permission to read all roles
 * @when get system roles
 * @then Return roles
 */
TEST_F(GetRolePermissionsTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRolePermissions(qry->role_id))
      .WillOnce(Return(perms));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<RolePermissionsResponse>(response);
  ASSERT_EQ(cast_resp->role_permissions.size(), perms.size());
  for (size_t i = 0; i < perms.size(); ++i) {
    ASSERT_EQ(cast_resp->role_permissions.at(i), perms.at(i));
  }
}

/**
 * @given initialized storage, no permission to read all roles
 * @when get system roles
 * @then Return Error
 */
TEST_F(GetRolePermissionsTest, InValidCaseNoPermissions) {
  role_permissions = {};
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::STATEFUL_INVALID);
}


/**
 * @given initialized storage,  permission to read all roles, no role exist
 * @when get system roles
 * @then Return Error
 */
TEST_F(GetRolePermissionsTest, InValidCaseNoRole) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRolePermissions(qry->role_id))
      .WillOnce(Return(nonstd::nullopt));
  auto response = validateAndExecute();
  auto cast_resp = std::static_pointer_cast<ErrorResponse>(response);
  ASSERT_EQ(cast_resp->reason, ErrorResponse::NO_ROLES);
}
