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
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/remove_signatory.hpp"
#include "model/commands/set_permissions.hpp"
#include "model/commands/set_quorum.hpp"
#include "model/commands/transfer_asset.hpp"
#include "model/execution/command_executor_factory.hpp"

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

  void SetUp() override {
    factory = CommandExecutorFactory::create().value();

    wsv_query = std::make_shared<StrictMock<MockWsvQuery>>();
    wsv_command = std::make_shared<StrictMock<MockWsvCommand>>();

    creator.account_id = admin_id;
    creator.domain_name = domain_id;
    creator.quorum = 1;

    account.account_id = account_id;
    account.domain_name = domain_id;
    account.quorum = 1;
  }

  bool validateAndExecute() {
    auto executor = factory->getCommandExecutor(command);
    return executor->validate(*command, *wsv_query, creator) and
        executor->execute(*command, *wsv_query, *wsv_command);
  }

  std::string admin_id = "admin@test", account_id = "test@test",
      asset_id = "coin#test", domain_id = "test", description = "test transfer";

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
    add_asset_quantity->account_id = account_id;
    add_asset_quantity->amount.int_part = 3;
    add_asset_quantity->amount.frac_part = 50;
    add_asset_quantity->asset_id = asset_id;

    command = add_asset_quantity;
  }

  decltype(AccountAsset().balance) balance = 150ul;
  Asset asset;
  AccountAsset wallet;

  std::shared_ptr<AddAssetQuantity> add_asset_quantity;
};

TEST_F(AddAssetQuantityTest, ValidWhenNewWallet) {
  // Add asset first time - no wallet
  // When there is no wallet - new accountAsset will be created
  creator.permissions.issue_assets = true;
  EXPECT_CALL(*wsv_query, getAccountAsset(add_asset_quantity->account_id, _))
      .WillOnce(Return(nonstd::nullopt));

  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(account_id)).WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, ValidWhenExistingWallet) {
  // There is already asset- there is a wallet
  // When there is a wallet - no new accountAsset created
  creator.permissions.issue_assets = true;
  EXPECT_CALL(*wsv_query, getAccountAsset(add_asset_quantity->account_id,
                                          add_asset_quantity->asset_id))
      .WillOnce(Return(wallet));

  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(account_id)).WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, InvalidWhenNoPermission) {
  // Creator has no permission
  creator.permissions.issue_assets = false;

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, InvalidWhenZeroAmount) {
  // Amount is zero
  add_asset_quantity->amount.int_part = 0;
  add_asset_quantity->amount.frac_part = 0;

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, InvalidWhenWrongPrecision) {
  // Amount is with wrong precision (must be 2)
  creator.permissions.issue_assets = true;
  add_asset_quantity->amount.frac_part = 300;

  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, InvalidWhenNoAccount) {
  // Account to add doesn't exist
  creator.permissions.issue_assets = true;
  add_asset_quantity->account_id = "noacc";

  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(add_asset_quantity->account_id))
      .WillOnce(Return(nonstd::nullopt));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddAssetQuantityTest, InvalidWhenNoAsset) {
  // Asset doesn't exist
  creator.permissions.issue_assets = true;
  add_asset_quantity->asset_id = "noass";

  EXPECT_CALL(*wsv_query, getAsset(add_asset_quantity->asset_id))
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

    command = add_signatory;
  }

  std::shared_ptr<AddSignatory> add_signatory;
};

TEST_F(AddSignatoryTest, ValidWhenCreatorHasPermissions) {
  // Creator has permissions
  creator.permissions.add_signatory = true;
  EXPECT_CALL(*wsv_command, insertAccountSignatory(add_signatory->account_id,
                                                   add_signatory->pubkey))
      .WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(AddSignatoryTest, ValidWhenSameAccount) {
  // When creator is adding public keys to his account
  creator.permissions.add_signatory = false;
  add_signatory->account_id = creator.account_id;

  EXPECT_CALL(*wsv_command, insertAccountSignatory(add_signatory->account_id,
                                                   add_signatory->pubkey))
      .WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(AddSignatoryTest, InvalidWhenNoPermissions) {
  // Creator has no permission
  creator.permissions.add_signatory = false;

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddSignatoryTest, InvalidWhenNoKey) {
  // Trying to add nonexistent public key
  creator.permissions.add_signatory = true;
  add_signatory->pubkey.fill(0xF);

  EXPECT_CALL(*wsv_command, insertAccountSignatory(add_signatory->account_id,
                                                   add_signatory->pubkey))
      .WillOnce(Return(false));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddSignatoryTest, InvalidWhenNoAccount) {
  // Add to nonexistent account
  creator.permissions.add_signatory = true;
  add_signatory->account_id = "noacc";

  EXPECT_CALL(*wsv_command, insertAccountSignatory(add_signatory->account_id,
                                                   add_signatory->pubkey))
      .WillOnce(Return(false));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(AddSignatoryTest, InvalidWhenSameKey) {
  // Add same signatory
  creator.permissions.add_signatory = true;
  add_signatory->pubkey.fill(2);

  EXPECT_CALL(*wsv_command, insertAccountSignatory(add_signatory->account_id,
                                                   add_signatory->pubkey))
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
  }

  std::shared_ptr<CreateAccount> create_account;
};

TEST_F(CreateAccountTest, ValidWhenNewAccount) {
  // Valid case
  creator.permissions.create_accounts = true;

  EXPECT_CALL(*wsv_command, insertSignatory(create_account->pubkey))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_command, insertAccount(_)).WillOnce(Return(true));

  EXPECT_CALL(*wsv_command,
              insertAccountSignatory(account_id, create_account->pubkey))
      .WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(CreateAccountTest, InvalidWhenNoPermissions) {
  // Creator has no permission
  creator.permissions.create_accounts = false;

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(CreateAccountTest, InvalidWhenLongName) {
  // Not valid name for account
  creator.permissions.create_accounts = true;
  create_account->account_name = "thisisaverybigname";

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(CreateAccountTest, InvalidWhenBadName) {
  // Not valid name for account (system symbols)
  creator.permissions.create_accounts = true;
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
  }

  std::shared_ptr<CreateAsset> create_asset;
};

TEST_F(CreateAssetTest, ValidWhenCreatorHasPermissions) {
  // Creator is money creator
  creator.permissions.create_assets = true;

  EXPECT_CALL(*wsv_command, insertAsset(_)).WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(CreateAssetTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  creator.permissions.create_assets = false;

  ASSERT_FALSE(validateAndExecute());
}


class CreateDomainTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    create_domain = std::make_shared<CreateDomain>();
    create_domain->domain_name = "CN";

    command = create_domain;
  }

  std::shared_ptr<CreateDomain> create_domain;
};

TEST_F(CreateDomainTest, ValidWhenCreatorHasPermissions) {
  // Valid case
  creator.permissions.create_domains = true;

  EXPECT_CALL(*wsv_command, insertDomain(_)).WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(CreateDomainTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  creator.permissions.create_domains = false;

  ASSERT_FALSE(validateAndExecute());
}


class RemoveSignatoryTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    remove_signatory = std::make_shared<RemoveSignatory>();
    remove_signatory->account_id = account_id;
    remove_signatory->pubkey.fill(1);

    command = remove_signatory;
  }

  std::shared_ptr<RemoveSignatory> remove_signatory;
};

TEST_F(RemoveSignatoryTest, ValidWhenCreatorHasPermissions) {
  // Creator is admin
  creator.permissions.remove_signatory = true;

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->account_id))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, deleteAccountSignatory(remove_signatory->account_id,
                                                   remove_signatory->pubkey))
      .WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(RemoveSignatoryTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  creator.permissions.remove_signatory = false;

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(RemoveSignatoryTest, InvalidWhenNoKey) {
  // Remove signatory not present in account
  creator.permissions.remove_signatory = true;
  remove_signatory->pubkey.fill(0xF);

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->account_id))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, deleteAccountSignatory(remove_signatory->account_id,
                                                   remove_signatory->pubkey))
      .WillOnce(Return(false));

  ASSERT_FALSE(validateAndExecute());
}


class SetAccountPermissionsTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    set_account_permissions = std::make_shared<SetAccountPermissions>();
    set_account_permissions->account_id = account_id;
    set_account_permissions->new_permissions.create_assets = true;

    command = set_account_permissions;
  }

  std::shared_ptr<SetAccountPermissions> set_account_permissions;
};

TEST_F(SetAccountPermissionsTest, ValidWhenCreatorHasPermissions) {
  // Creator is admin
  creator.permissions.set_permissions = true;

  EXPECT_CALL(*wsv_query, getAccount(set_account_permissions->account_id))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, updateAccount(_)).WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(SetAccountPermissionsTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  creator.permissions.set_permissions = false;

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(SetAccountPermissionsTest, InvalidWhenNoAccount) {
  // No such account exists
  creator.permissions.set_permissions = true;
  set_account_permissions->account_id = "noacc";

  EXPECT_CALL(*wsv_query, getAccount(set_account_permissions->account_id))
      .WillOnce(Return(nonstd::nullopt));

  ASSERT_FALSE(validateAndExecute());
}


class SetQuorumTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    set_quorum = std::make_shared<SetQuorum>();
    set_quorum->account_id = account_id;
    set_quorum->new_quorum = 2;

    command = set_quorum;
  }

  std::shared_ptr<SetQuorum> set_quorum;
};

TEST_F(SetQuorumTest, ValidWhenCreatorHasPermissions) {
  // Creator is admin
  creator.permissions.set_quorum = true;

  EXPECT_CALL(*wsv_query, getAccount(set_quorum->account_id))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, updateAccount(_)).WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(SetQuorumTest, ValidWhenSameAccount) {
  // Creator is the account
  creator = account;

  EXPECT_CALL(*wsv_query, getAccount(set_quorum->account_id))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, updateAccount(_)).WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(SetQuorumTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  creator.permissions.set_quorum = false;

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(SetQuorumTest, InvalidWhenNoAccount) {
  // No such account exists
  creator.permissions.set_quorum = true;
  set_quorum->account_id = "noacc";

  EXPECT_CALL(*wsv_query, getAccount(set_quorum->account_id))
      .WillOnce(Return(nonstd::nullopt));

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
    transfer_asset->amount.int_part = 1;
    transfer_asset->amount.frac_part = 50;

    command = transfer_asset;
  }

  decltype(AccountAsset().balance) balance = 150ul;
  Asset asset;
  AccountAsset src_wallet, dst_wallet;

  std::shared_ptr<TransferAsset> transfer_asset;
};

TEST_F(TransferAssetTest, ValidWhenNewWallet) {
  // When there is no wallet - new accountAsset will be created
  creator.permissions.can_transfer = true;
  EXPECT_CALL(*wsv_query, getAccountAsset(transfer_asset->dest_account_id, _))
      .WillOnce(Return(nonstd::nullopt));

  EXPECT_CALL(*wsv_query, getAccountAsset(transfer_asset->src_account_id,
                                          transfer_asset->asset_id))
      .Times(2).WillRepeatedly(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .Times(2).WillRepeatedly(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->dest_account_id))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .Times(2)
      .WillRepeatedly(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(TransferAssetTest, ValidWhenExistingWallet) {
  // When there is a wallet - no new accountAsset created
  creator.permissions.can_transfer = true;
  EXPECT_CALL(*wsv_query, getAccountAsset(transfer_asset->dest_account_id,
                                          transfer_asset->asset_id))
      .WillOnce(Return(dst_wallet));

  EXPECT_CALL(*wsv_query, getAccountAsset(transfer_asset->src_account_id,
                                          transfer_asset->asset_id))
      .Times(2).WillRepeatedly(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .Times(2).WillRepeatedly(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->dest_account_id))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .Times(2)
      .WillRepeatedly(Return(true));

  ASSERT_TRUE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  creator.permissions.can_transfer = false;

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenNoDestAccount) {
  // No destination account exists
  creator.permissions.can_transfer = true;
  transfer_asset->dest_account_id = "noacc";

  EXPECT_CALL(*wsv_query, getAccountAsset(transfer_asset->src_account_id,
                                          transfer_asset->asset_id))
      .WillOnce(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->dest_account_id))
      .WillOnce(Return(nonstd::nullopt));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenNoSrcAccountAsset) {
  // No source account asset exists
  creator.permissions.can_transfer = true;

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccountAsset(transfer_asset->src_account_id,
                                          transfer_asset->asset_id))
      .WillOnce(Return(nonstd::nullopt));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenInsufficientFunds) {
  // No sufficient funds
  creator.permissions.can_transfer = true;
  transfer_asset->amount.int_part = 1;
  transfer_asset->amount.frac_part = 55;

  EXPECT_CALL(*wsv_query, getAccountAsset(transfer_asset->src_account_id,
                                          transfer_asset->asset_id))
      .WillOnce(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->dest_account_id))
      .WillOnce(Return(nonstd::nullopt));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenWrongPrecision) {
  // Amount has wrong precision
  creator.permissions.can_transfer = true;
  transfer_asset->amount.frac_part = 555;

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->asset_id))
      .WillOnce(Return(asset));

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenDifferentCreator) {
  // Transfer creator is not connected to account
  creator.permissions.can_transfer = true;
  creator = account;

  ASSERT_FALSE(validateAndExecute());
}

TEST_F(TransferAssetTest, InvalidWhenZeroAmount) {
  // Transfer zero assets
  creator.permissions.can_transfer = true;
  transfer_asset->amount.int_part = 0;
  transfer_asset->amount.frac_part = 0;

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
  }

  std::shared_ptr<AddPeer> add_peer;
};

TEST_F(AddPeerTest, ValidCase) {
  // Valid case
  EXPECT_CALL(*wsv_command, insertPeer(_)).WillOnce(Return(true));

  ASSERT_TRUE(validateAndExecute());
}
