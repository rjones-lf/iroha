/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
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

#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/add_signatory.hpp"
#include "model/commands/append_role.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/create_role.hpp"
#include "model/commands/grant_permission.hpp"
#include "model/commands/remove_signatory.hpp"
#include "model/commands/revoke_permission.hpp"
#include "model/commands/set_account_detail.hpp"
#include "model/commands/set_quorum.hpp"
#include "model/commands/subtract_asset_quantity.hpp"
#include "model/commands/transfer_asset.hpp"
#include "model/execution/command_executor_factory.hpp"
#include "model/permissions.hpp"

using ::testing::Return;
using ::testing::AtLeast;
using ::testing::_;
using ::testing::AllOf;
using ::testing::StrictMock;

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace iroha::model;

class CommandValidateExecuteTest : public ::testing::Test {
 public:
  CommandValidateExecuteTest() { spdlog::set_level(spdlog::level::off); }

  void SetUp() override {
    factory = CommandExecutorFactory::create().value();

    wsv_query = std::make_shared<StrictMock<MockWsvQuery>>();
    wsv_command = std::make_shared<StrictMock<MockWsvCommand>>();

    creator.account_id = admin_id;
    creator.domain_id = domain_id;
    creator.quorum = 1;

    account.account_id = account_id;
    account.domain_id = domain_id;
    account.quorum = 1;

    default_domain.domain_id = domain_id;
    default_domain.default_role = admin_role;
  }

  bool validateAndExecute() {
    auto executor = factory->getCommandExecutor(command);
    return executor->validate(*command, *wsv_query, creator)
        and executor->execute(*command, *wsv_query, *wsv_command);
  }

  std::string admin_id = "admin@test", account_id = "test@test",
              asset_id = "coin#test", domain_id = "test",
              description = "test transfer";

  std::string admin_role = "admin";

  std::vector<std::string> admin_roles = {admin_role};
  std::vector<std::string> role_permissions;
  model::Domain default_domain;

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockWsvCommand> wsv_command;

  Account creator, account;

  std::shared_ptr<Command> command;

  std::shared_ptr<CommandExecutorFactory> factory;
};

class AddAssetQuantityTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    asset = Asset();
    asset.asset_id = asset_id;
    asset.domain_id = domain_id;
    asset.precision = 2;

    wallet = AccountAsset();
    wallet.asset_id = asset_id;
    wallet.account_id = account_id;
    wallet.balance = balance;

    add_asset_quantity = std::make_shared<AddAssetQuantity>();
    add_asset_quantity->account_id = creator.account_id;
    Amount amount(350, 2);
    add_asset_quantity->amount = amount;
    add_asset_quantity->asset_id = asset_id;

    command = add_asset_quantity;
    role_permissions = {can_add_asset_qty};
  }

  decltype(AccountAsset().balance) balance = Amount(150ul, 2);
  Asset asset;
  AccountAsset wallet;

  std::shared_ptr<AddAssetQuantity> add_asset_quantity;
};

TEST_F(AddAssetQuantityTest, ValidWhenNewWallet) {
  // Add asset first time - no wallet
  // When there is no wallet - new accountAsset will be created
  EXPECT_CALL(*wsv_query, getAccountAsset(add_asset_quantity->account_id, _))
      .WillOnce(Return(nonstd::nullopt));

  EXPECT_CALL(*wsv_query, getAsset(add_asset_quantity->asset_id))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(add_asset_quantity->account_id))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, upsertAccountAsset(_)).WillOnce(Return(true));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator.account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  ASSERT_TRUE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, ValidWhenExistingWallet) {
  // There is already asset- there is a wallet
  // When there is a wallet - no new accountAsset created
  EXPECT_CALL(*wsv_query,
              getAccountAsset(add_asset_quantity->account_id,
                              add_asset_quantity->asset_id))
      .WillOnce(Return(wallet));

  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(add_asset_quantity->account_id))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, upsertAccountAsset(_)).WillOnce(Return(true));
  EXPECT_CALL(*wsv_query, getAccountRoles(add_asset_quantity->account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  ASSERT_TRUE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, InvalidWhenNoRoles) {
  // Creator has no roles
  EXPECT_CALL(*wsv_query, getAccountRoles(add_asset_quantity->account_id))
      .WillOnce(Return(nonstd::nullopt));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, InvalidWhenZeroAmount) {
  // Amount is zero
  Amount amount(0);
  add_asset_quantity->amount = amount;
  EXPECT_CALL(*wsv_query, getAsset(asset.asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator.account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, InvalidWhenWrongPrecision) {
  // Amount is with wrong precision (must be 2)
  Amount amount(add_asset_quantity->amount.getIntValue(), 30);
  add_asset_quantity->amount = amount;
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, InvalidWhenNoAccount) {
  // Account to add doesn't exist
  add_asset_quantity->account_id = "noacc";
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, InvalidWhenNoAsset) {
  // Asset doesn't exist
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  add_asset_quantity->asset_id = "noass";

  EXPECT_CALL(*wsv_query, getAsset(add_asset_quantity->asset_id))
      .WillOnce(Return(nonstd::nullopt));

  ASSERT_FALSE(validateAndExecute());
}

class SubtractAssetQuantityTest : public CommandValidateExecuteTest {
public:
    void SetUp() override {
      CommandValidateExecuteTest::SetUp();

      asset = Asset();
      asset.asset_id = asset_id;
      asset.domain_id = domain_id;
      asset.precision = 2;

      wallet = AccountAsset();
      wallet.asset_id = asset_id;
      wallet.account_id = account_id;
      wallet.balance = balance;

      subtract_asset_quantity = std::make_shared<SubtractAssetQuantity>();
      subtract_asset_quantity->account_id = creator.account_id;
      Amount amount(100, 2);
      subtract_asset_quantity->amount = amount;
      subtract_asset_quantity->asset_id = asset_id;

      command = subtract_asset_quantity;
      role_permissions = {can_subtract_asset_qty};
    }

    decltype(AccountAsset().balance) balance = Amount(150ul, 2);
    Asset asset;
    AccountAsset wallet;

    std::shared_ptr<SubtractAssetQuantity> subtract_asset_quantity;
};

TEST_F(SubtractAssetQuantityTest, InvalidWhenNoWallet) {
  // Subtract asset - no wallet
  // When there is no wallet - Failed
  EXPECT_CALL(*wsv_query,
    getAccountAsset(subtract_asset_quantity->account_id,
                    subtract_asset_quantity->asset_id))
    .WillOnce(Return(nonstd::nullopt));

  EXPECT_CALL(*wsv_query, getAccountRoles(subtract_asset_quantity->account_id))
    .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
    .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(subtract_asset_quantity->account_id))
    .WillOnce(Return(account));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(SubtractAssetQuantityTest, ValidWhenExistingWallet) {
  // There is already asset- there is a wallet
  // When there is a wallet - no new accountAsset created
  EXPECT_CALL(*wsv_query,
      getAccountAsset(subtract_asset_quantity->account_id,
                      subtract_asset_quantity->asset_id))
      .WillOnce(Return(wallet));

  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(subtract_asset_quantity->account_id))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, upsertAccountAsset(_)).WillOnce(Return(true));
  EXPECT_CALL(*wsv_query, getAccountRoles(subtract_asset_quantity->account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  ASSERT_TRUE(validateAndExecute());
}

TEST_F(SubtractAssetQuantityTest, InvalidWhenOverAmount) {
  Amount amount(1204, 2);
  subtract_asset_quantity->amount = amount;
  EXPECT_CALL(*wsv_query,
    getAccountAsset(subtract_asset_quantity->account_id,
                    subtract_asset_quantity->asset_id))
      .WillOnce(Return(wallet));

  EXPECT_CALL(*wsv_query, getAccountRoles(subtract_asset_quantity->account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(subtract_asset_quantity->account_id))
      .WillOnce(Return(account));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(SubtractAssetQuantityTest, InvalidWhenNoRoles) {
  // Creator has no roles
  EXPECT_CALL(*wsv_query, getAccountRoles(subtract_asset_quantity->account_id))
      .WillOnce(Return(nonstd::nullopt));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(SubtractAssetQuantityTest, InvalidWhenZeroAmount) {
  // Amount is zero
  Amount amount(0);
  subtract_asset_quantity->amount = amount;
  EXPECT_CALL(*wsv_query, getAsset(asset.asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator.account_id))
    .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
    .WillOnce(Return(role_permissions));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(SubtractAssetQuantityTest, InvalidWhenWrongPrecision) {
  // Amount is with wrong precision (must be 2)
  Amount amount(subtract_asset_quantity->amount.getIntValue(), 30);
  subtract_asset_quantity->amount = amount;
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(SubtractAssetQuantityTest, InvalidWhenNoAccount) {
  // Account to subtract doesn't exist
  subtract_asset_quantity->account_id = "noacc";
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(SubtractAssetQuantityTest, InvalidWhenNoAsset) {
  // Asset doesn't exist
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  subtract_asset_quantity->asset_id = "noass";

  EXPECT_CALL(*wsv_query, getAsset(subtract_asset_quantity->asset_id))
      .WillOnce(Return(nonstd::nullopt));

  ASSERT_FALSE(validateAndExecute());
}

class AddSignatoryTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    add_signatory = std::make_shared<AddSignatory>();
    add_signatory->account_id = account_id;
    add_signatory->pubkey.fill(1);  // Such Pubkey exist
    role_permissions = {can_add_signatory};
    command = add_signatory;
  }

  std::shared_ptr<AddSignatory> add_signatory;
};

TEST_F(AddSignatoryTest, ValidWhenCreatorHasPermissions) {
  // Creator has role permissions to add signatory
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, add_signatory->account_id, can_add_signatory))
      .WillOnce(Return(true));
  EXPECT_CALL(*wsv_command, insertSignatory(add_signatory->pubkey))
      .WillOnce(Return(true));
  EXPECT_CALL(
      *wsv_command,
      insertAccountSignatory(add_signatory->account_id, add_signatory->pubkey))
      .WillOnce(Return(true));
  ASSERT_TRUE(validateAndExecute());
}

TEST_F(AddSignatoryTest, ValidWhenSameAccount) {
  // When creator is adding public keys to his account
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  add_signatory->account_id = creator.account_id;

  EXPECT_CALL(*wsv_command, insertSignatory(add_signatory->pubkey))
      .WillOnce(Return(true));
  EXPECT_CALL(
      *wsv_command,
      insertAccountSignatory(add_signatory->account_id, add_signatory->pubkey))
      .WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(AddSignatoryTest, InvalidWhenNoPermissions) {
  // Creator has no permission
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, add_signatory->account_id, can_add_signatory))
      .WillOnce(Return(false));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddSignatoryTest, InvalidWhenNoAccount) {
  // Add to nonexistent account
  add_signatory->account_id = "noacc";

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, add_signatory->account_id, can_add_signatory))
      .WillOnce(Return(false));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddSignatoryTest, InvalidWhenSameKey) {
  // Add same signatory
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, add_signatory->account_id, can_add_signatory))
      .WillOnce(Return(true));
  add_signatory->pubkey.fill(2);
  EXPECT_CALL(*wsv_command, insertSignatory(add_signatory->pubkey))
      .WillOnce(Return(false));

  ASSERT_FALSE(validateAndExecute());
}

class CreateAccountTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    create_account = std::make_shared<CreateAccount>();
    create_account->account_name = "test";
    create_account->domain_id = domain_id;
    create_account->pubkey.fill(2);

    command = create_account;
    role_permissions = {can_create_account};
  }

  std::shared_ptr<CreateAccount> create_account;
};

TEST_F(CreateAccountTest, ValidWhenNewAccount) {
  // Valid case
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getDomain(domain_id))
      .WillOnce(Return(default_domain));

  EXPECT_CALL(*wsv_command, insertSignatory(create_account->pubkey))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_command, insertAccount(_)).WillOnce(Return(true));

  EXPECT_CALL(*wsv_command,
              insertAccountSignatory(account_id, create_account->pubkey))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_command, insertAccountRole(account_id, admin_role))
      .WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(CreateAccountTest, InvalidWhenNoPermissions) {
  // Creator has no permission
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(nonstd::nullopt));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(CreateAccountTest, InvalidWhenLongName) {
  // Not valid name for account
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  create_account->account_name = "aAccountNameMustBeLessThan64characters00000000000000000000000000";
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(CreateAccountTest, InvalidWhenNameWithSystemSymbols) {
  // Not valid name for account (system symbols)
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  create_account->account_name = "test@";

  ASSERT_FALSE(validateAndExecute());
}

class CreateAssetTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    create_asset = std::make_shared<CreateAsset>();
    create_asset->asset_name = "FCoin";
    create_asset->domain_id = domain_id;
    create_asset->precision = 2;

    command = create_asset;
    role_permissions = {can_create_asset};
  }

  std::shared_ptr<CreateAsset> create_asset;
};

TEST_F(CreateAssetTest, ValidWhenCreatorHasPermissions) {
  // Creator is money creator
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_command, insertAsset(_)).WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(CreateAssetTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(nonstd::nullopt));

  ASSERT_FALSE(validateAndExecute());
}

class CreateDomainTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    create_domain = std::make_shared<CreateDomain>();
    create_domain->domain_id = "CN";

    command = create_domain;
    role_permissions = {can_create_domain};
  }

  std::shared_ptr<CreateDomain> create_domain;
};

TEST_F(CreateDomainTest, ValidWhenCreatorHasPermissions) {
  // Valid case
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_command, insertDomain(_)).WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(CreateDomainTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(nonstd::nullopt));
  ASSERT_FALSE(validateAndExecute());
}

class RemoveSignatoryTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    pubkey_t creator_key, account_key;
    creator_key.fill(0x1);
    account_key.fill(0x2);

    account_pubkeys = {account_key};
    many_pubkeys = {creator_key, account_key};

    remove_signatory = std::make_shared<RemoveSignatory>();
    remove_signatory->account_id = account_id;
    remove_signatory->pubkey.fill(1);

    command = remove_signatory;
    role_permissions = {can_remove_signatory};
  }

  std::vector<pubkey_t> account_pubkeys;
  std::vector<pubkey_t> many_pubkeys;
  std::shared_ptr<RemoveSignatory> remove_signatory;
};

TEST_F(RemoveSignatoryTest, ValidWhenMultipleKeys) {
  // Creator is admin
  // Add same signatory

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, remove_signatory->account_id, can_remove_signatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->account_id))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->account_id))
      .WillOnce(Return(many_pubkeys));

  EXPECT_CALL(*wsv_command,
              deleteAccountSignatory(remove_signatory->account_id,
                                     remove_signatory->pubkey))
      .WillOnce(Return(true));
  EXPECT_CALL(*wsv_command, deleteSignatory(remove_signatory->pubkey))
      .WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(RemoveSignatoryTest, InvalidWhenSingleKey) {
  // Creator is admin

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, remove_signatory->account_id, can_remove_signatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->account_id))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->account_id))
      .WillOnce(Return(account_pubkeys));

  // delete methods must not be called because the account quorum is 1.
  EXPECT_CALL(*wsv_command,
              deleteAccountSignatory(remove_signatory->account_id,
                                     remove_signatory->pubkey))
      .Times(0);
  EXPECT_CALL(*wsv_command, deleteSignatory(remove_signatory->pubkey)).Times(0);

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(RemoveSignatoryTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  // Add same signatory
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, remove_signatory->account_id, can_remove_signatory))
      .WillOnce(Return(false));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(RemoveSignatoryTest, InvalidWhenNoKey) {
  // Remove signatory not present in account
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, remove_signatory->account_id, can_remove_signatory))
      .WillOnce(Return(true));
  remove_signatory->pubkey.fill(0xF);

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->account_id))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->account_id))

      .WillOnce(Return(account_pubkeys));

  ASSERT_FALSE(validateAndExecute());
}

class SetQuorumTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    pubkey_t creator_key, account_key;
    creator_key.fill(0x1);
    account_key.fill(0x2);
    account_pubkeys = {creator_key, account_key};
    set_quorum = std::make_shared<SetQuorum>();
    set_quorum->account_id = account_id;
    set_quorum->new_quorum = 2;

    command = set_quorum;
    role_permissions = {can_set_quorum};
  }

  std::vector<pubkey_t> account_pubkeys;
  std::shared_ptr<SetQuorum> set_quorum;
};

TEST_F(SetQuorumTest, ValidWhenCreatorHasPermissions) {
  // Creator is admin
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, set_quorum->account_id, can_set_quorum))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(set_quorum->account_id))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_query, getSignatories(set_quorum->account_id))
      .WillOnce(Return(account_pubkeys));

  EXPECT_CALL(*wsv_command, updateAccount(_)).WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(SetQuorumTest, ValidWhenSameAccount) {
  // Creator is the account
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  set_quorum->account_id = creator.account_id;
  EXPECT_CALL(*wsv_query, getAccount(set_quorum->account_id))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getSignatories(set_quorum->account_id))
      .WillOnce(Return(account_pubkeys));
  EXPECT_CALL(*wsv_command, updateAccount(_)).WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(SetQuorumTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, set_quorum->account_id, can_set_quorum))
      .WillOnce(Return(false));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(SetQuorumTest, InvalidWhenNoAccount) {
  // No such account exists
  set_quorum->account_id = "noacc";
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, set_quorum->account_id, can_set_quorum))
      .WillOnce(Return(false));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(SetQuorumTest, InvalidWhenNotEnoughSignatories) {
  // Creator is the account
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  set_quorum->account_id = creator.account_id;
  EXPECT_CALL(*wsv_query, getAccount(set_quorum->account_id)).Times(0);
  pubkey_t key;
  key.fill(0x1);
  std::vector<pubkey_t> acc_pubkeys = {key};
  EXPECT_CALL(*wsv_query, getSignatories(set_quorum->account_id))
      .WillOnce(Return(acc_pubkeys));
  EXPECT_CALL(*wsv_command, updateAccount(_)).Times(0);

  ASSERT_FALSE(validateAndExecute());
}

class TransferAssetTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    asset = Asset();
    asset.asset_id = asset_id;
    asset.domain_id = domain_id;
    asset.precision = 2;

    src_wallet = AccountAsset();
    src_wallet.asset_id = asset_id;
    src_wallet.account_id = admin_id;
    src_wallet.balance = balance;

    dst_wallet = src_wallet;
    dst_wallet.account_id = account_id;

    transfer_asset = std::make_shared<TransferAsset>();
    transfer_asset->src_account_id = admin_id;
    transfer_asset->dest_account_id = account_id;
    transfer_asset->asset_id = asset_id;
    transfer_asset->description = description;
    Amount amount(150, 2);
    transfer_asset->amount = amount;
    command = transfer_asset;
    role_permissions = {can_transfer, can_receive};
  }

  Amount balance = Amount(150, 2);
  Asset asset;
  AccountAsset src_wallet, dst_wallet;

  std::shared_ptr<TransferAsset> transfer_asset;
};

TEST_F(TransferAssetTest, ValidWhenNewWallet) {
  // When there is no wallet - new accountAsset will be created
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->dest_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->src_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAccountAsset(transfer_asset->dest_account_id, _))
      .WillOnce(Return(nonstd::nullopt));

  EXPECT_CALL(
      *wsv_query,
      getAccountAsset(transfer_asset->src_account_id, transfer_asset->asset_id))
      .Times(2)
      .WillRepeatedly(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .Times(2)
      .WillRepeatedly(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->dest_account_id))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .Times(2)
      .WillRepeatedly(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(TransferAssetTest, ValidWhenExistingWallet) {
  // When there is a wallet - no new accountAsset created
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->dest_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->src_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->dest_account_id,
                              transfer_asset->asset_id))
      .WillOnce(Return(dst_wallet));

  EXPECT_CALL(
      *wsv_query,
      getAccountAsset(transfer_asset->src_account_id, transfer_asset->asset_id))
      .Times(2)
      .WillRepeatedly(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .Times(2)
      .WillRepeatedly(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->dest_account_id))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .Times(2)
      .WillRepeatedly(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->src_account_id))
      .WillOnce(Return(nonstd::nullopt));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenNoDestAccount) {
  // No destination account exists
  transfer_asset->dest_account_id = "noacc";
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->dest_account_id))
      .WillOnce(Return(nonstd::nullopt));

  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->src_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenNoSrcAccountAsset) {
  // No source account asset exists
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->dest_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->src_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .WillOnce(Return(asset));
  EXPECT_CALL(
      *wsv_query,
      getAccountAsset(transfer_asset->src_account_id, transfer_asset->asset_id))
      .WillOnce(Return(nonstd::nullopt));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenInsufficientFunds) {
  // No sufficient funds
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->dest_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->src_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  Amount amount(155, 2);

  EXPECT_CALL(
      *wsv_query,
      getAccountAsset(transfer_asset->src_account_id, transfer_asset->asset_id))
      .WillOnce(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->dest_account_id))
      .WillOnce(Return(nonstd::nullopt));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenWrongPrecision) {
  // Amount has wrong precision
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->dest_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->src_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  Amount amount(transfer_asset->amount.getIntValue(), 30);
  transfer_asset->amount = amount;

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .WillOnce(Return(asset));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenCreatorHasNoPermission) {
  // Transfer creator is not connected to account
  transfer_asset->src_account_id = account_id;
  transfer_asset->dest_account_id = admin_id;
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(admin_id, account_id, can_transfer))
      .WillOnce(Return(false));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, ValidWhenCreatorHasPermission) {
  // Transfer creator is not connected to account
  transfer_asset->src_account_id = account_id;
  transfer_asset->dest_account_id = admin_id;
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(admin_id, account_id, can_transfer))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->dest_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAccountAsset(transfer_asset->dest_account_id, _))
      .WillOnce(Return(nonstd::nullopt));

  EXPECT_CALL(
      *wsv_query,
      getAccountAsset(transfer_asset->src_account_id, transfer_asset->asset_id))
      .Times(2)
      .WillRepeatedly(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .Times(2)
      .WillRepeatedly(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->dest_account_id))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .Times(2)
      .WillRepeatedly(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenZeroAmount) {
  // Transfer zero assets
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->dest_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->src_account_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  Amount amount(0, 2);
  transfer_asset->amount = amount;

  ASSERT_FALSE(validateAndExecute());
}

class AddPeerTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    add_peer = std::make_shared<AddPeer>();
    add_peer->address = "iroha_node:10001";
    add_peer->peer_key.fill(4);

    command = add_peer;
    role_permissions = {can_add_peer};
  }

  std::shared_ptr<AddPeer> add_peer;
};

TEST_F(AddPeerTest, ValidCase) {
  // Valid case
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_command, insertPeer(_)).WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(AddPeerTest, InvalidCaseWhenNoPermissions) {
  // Valid case
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(nonstd::nullopt));
  ASSERT_FALSE(validateAndExecute());
}

class CreateRoleTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();
    std::set<std::string> perm = {can_create_role};
    create_role = std::make_shared<CreateRole>("master", perm);
    command = create_role;
    role_permissions = {can_create_role};
  }
  std::shared_ptr<CreateRole> create_role;
};

TEST_F(CreateRoleTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillRepeatedly(Return(role_permissions));
  EXPECT_CALL(*wsv_command, insertRole(_)).WillOnce(Return(true));
  EXPECT_CALL(*wsv_command, insertRolePermissions(_, _)).WillOnce(Return(true));
  ASSERT_TRUE(validateAndExecute());
}

TEST_F(CreateRoleTest, InvalidCaseWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillRepeatedly(Return(nonstd::nullopt));
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(CreateRoleTest, InvalidCaseWhenRoleSuperset) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillRepeatedly(Return(role_permissions));
  std::set<std::string> perms = {can_add_peer, can_append_role};
  command = std::make_shared<CreateRole>("master", perms);
  ASSERT_FALSE(validateAndExecute());
}

TEST_F(CreateRoleTest, InvalidCaseWhenWrongRoleName) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillRepeatedly(Return(role_permissions));
  std::set<std::string> perms = {can_create_role};
  command = std::make_shared<CreateRole>("m!Aster", perms);
  ASSERT_FALSE(validateAndExecute());
}

class AppendRoleTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();
    exact_command = std::make_shared<AppendRole>("yoda", "master");
    command = exact_command;
    role_permissions = {can_append_role};
  }
  std::shared_ptr<AppendRole> exact_command;
};

TEST_F(AppendRoleTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_command, insertAccountRole(_, _)).WillOnce(Return(true));
  ASSERT_TRUE(validateAndExecute());
}

TEST_F(AppendRoleTest, InvalidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(nonstd::nullopt));
  ASSERT_FALSE(validateAndExecute());
}

class GrantPermissionTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();
    exact_command = std::make_shared<GrantPermission>("yoda", "CanTeach");
    command = exact_command;
    role_permissions = {"CanGrantCanTeach"};
  }
  std::shared_ptr<GrantPermission> exact_command;
};

TEST_F(GrantPermissionTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_command, insertAccountGrantablePermission(_, _, _))
      .WillOnce(Return(true));
  ASSERT_TRUE(validateAndExecute());
}

TEST_F(GrantPermissionTest, InvalidCaseWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(nonstd::nullopt));
  ASSERT_FALSE(validateAndExecute());
}

class RevokePermissionTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();
    exact_command = std::make_shared<RevokePermission>("yoda", "CanTeach");
    command = exact_command;
  }
  std::shared_ptr<RevokePermission> exact_command;
};

TEST_F(RevokePermissionTest, ValidCase) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          exact_command->account_id, admin_id, exact_command->permission_name))
      .WillOnce(Return(true));
  EXPECT_CALL(*wsv_command, deleteAccountGrantablePermission(_, _, _))
      .WillOnce(Return(true));
  ASSERT_TRUE(validateAndExecute());
}

TEST_F(RevokePermissionTest, InvalidCaseNoPermissions) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          exact_command->account_id, admin_id, exact_command->permission_name))
      .WillOnce(Return(false));
  ASSERT_FALSE(validateAndExecute());
}

class SetAccountDetailTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();
    cmd = std::make_shared<SetAccountDetail>();
    cmd->account_id = admin_id;
    cmd->key = "key";
    cmd->value = "val";
    command = cmd;
    role_permissions = {can_set_quorum};
  }
  std::shared_ptr<SetAccountDetail> cmd;
};

/**
 * @when creator is setting details to their account
 * @then successfully execute the command
 */
TEST_F(SetAccountDetailTest, ValidWhenCreatorHasPermissions) {
  EXPECT_CALL(*wsv_command, setAccountKV(_, _, _)).WillOnce(Return(true));
  ASSERT_TRUE(validateAndExecute());
}

/**
 * @when creator is setting details to their account
 * @then successfully execute the command
 */
TEST_F(SetAccountDetailTest, InValidWhenOtherCreator) {
  cmd->account_id = account_id;
  ASSERT_FALSE(validateAndExecute());
}
