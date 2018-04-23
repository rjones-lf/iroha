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

#include <limits>

#include "builders/default_builders.hpp"
#include "execution/command_executor.hpp"
#include "framework/result_fixture.hpp"
#include "interfaces/commands/add_asset_quantity.hpp"
#include "interfaces/commands/add_peer.hpp"
#include "interfaces/commands/add_signatory.hpp"
#include "interfaces/commands/append_role.hpp"
#include "interfaces/commands/create_account.hpp"
#include "interfaces/commands/create_asset.hpp"
#include "interfaces/commands/create_domain.hpp"
#include "interfaces/commands/create_role.hpp"
#include "interfaces/commands/detach_role.hpp"
#include "interfaces/commands/grant_permission.hpp"
#include "interfaces/commands/remove_signatory.hpp"
#include "interfaces/commands/revoke_permission.hpp"
#include "interfaces/commands/set_account_detail.hpp"
#include "interfaces/commands/set_quorum.hpp"
#include "interfaces/commands/subtract_asset_quantity.hpp"
#include "interfaces/commands/transfer_asset.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "validators/permissions.hpp"

using ::testing::Return;
using ::testing::StrictMock;
using ::testing::_;

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace framework::expected;
using namespace shared_model::permissions;

/**
 * Helper function to get concrete command from Command container.
 * @tparam T - type of concrete command
 * @param command - Command container
 * @return concrete command extracted from container
 */
template <class T>
std::shared_ptr<T> getCommand(
    std::shared_ptr<shared_model::interface::Command> command) {
  return clone(*(
      boost::get<shared_model::detail::PolymorphicWrapper<T>>(command->get())));
}

class CommandValidateExecuteTest : public ::testing::Test {
 public:
  void SetUp() override {
    wsv_query = std::make_shared<StrictMock<MockWsvQuery>>();
    wsv_command = std::make_shared<StrictMock<MockWsvCommand>>();

    executor = std::make_unique<iroha::CommandExecutor>(wsv_query, wsv_command);
    validator = std::make_unique<iroha::CommandValidator>(wsv_query);

    shared_model::builder::AccountBuilder<
        shared_model::proto::AccountBuilder,
        shared_model::validation::FieldValidator>()
        .accountId(admin_id)
        .domainId(domain_id)
        .quorum(1)
        .build()
        .match(
            [&](expected::Value<
                std::shared_ptr<shared_model::interface::Account>> &v) {
              creator = v.value;
            },
            [](expected::Error<std::shared_ptr<std::string>> &e) {
              FAIL() << *e.error;
            });

    shared_model::builder::AccountBuilder<
        shared_model::proto::AccountBuilder,
        shared_model::validation::FieldValidator>()
        .accountId(account_id)
        .domainId(domain_id)
        .quorum(1)
        .build()
        .match(
            [&](expected::Value<
                std::shared_ptr<shared_model::interface::Account>> &v) {
              account = v.value;
            },
            [](expected::Error<std::shared_ptr<std::string>> &e) {
              FAIL() << *e.error;
            });

    shared_model::builder::AssetBuilder<
        shared_model::proto::AssetBuilder,
        shared_model::validation::FieldValidator>()
        .assetId(asset_id)
        .domainId(domain_id)
        .precision(2)
        .build()
        .match(
            [&](expected::Value<std::shared_ptr<shared_model::interface::Asset>>
                    &v) { asset = v.value; },
            [](expected::Error<std::shared_ptr<std::string>> &e) {
              FAIL() << *e.error;
            });

    shared_model::builder::AmountBuilder<
        shared_model::proto::AmountBuilder,
        shared_model::validation::FieldValidator>()
        .intValue(150)
        .precision(2)
        .build()
        .match(
            [&](expected::Value<
                std::shared_ptr<shared_model::interface::Amount>> &v) {
              balance = v.value;
            },
            [](expected::Error<std::shared_ptr<std::string>> &e) {
              FAIL() << *e.error;
            });

    shared_model::builder::AccountAssetBuilder<
        shared_model::proto::AccountAssetBuilder,
        shared_model::validation::FieldValidator>()
        .assetId(asset_id)
        .accountId(account_id)
        .balance(*balance)
        .build()
        .match(
            [&](expected::Value<
                std::shared_ptr<shared_model::interface::AccountAsset>> &v) {
              wallet = v.value;
            },
            [](expected::Error<std::shared_ptr<std::string>> &e) {
              FAIL() << *e.error;
            });
  }

  iroha::ExecutionResult validateAndExecute(
      const std::shared_ptr<shared_model::interface::Command> &command) {
    validator->setCreatorAccountId(creator->accountId());
    executor->setCreatorAccountId(creator->accountId());

    if (boost::apply_visitor(*validator, command->get())) {
      return boost::apply_visitor(*executor, command->get());
    }
    return expected::makeError(
        iroha::ExecutionError{"Validate", "validation of a command failed"});
  }

  iroha::ExecutionResult execute(
      const std::shared_ptr<shared_model::interface::Command> &command) {
    executor->setCreatorAccountId(creator->accountId());
    return boost::apply_visitor(*executor, command->get());
  }

  /// return result with empty error message
  WsvCommandResult makeEmptyError() {
    return WsvCommandResult(iroha::expected::makeError(""));
  }

  /// Returns error from result or throws error in case result contains value
  iroha::ExecutionResult::ErrorType checkErrorCase(
      const iroha::ExecutionResult &result) {
    return boost::get<iroha::ExecutionResult::ErrorType>(result);
  }

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockWsvCommand> wsv_command;

  std::unique_ptr<iroha::CommandExecutor> executor;
  std::unique_ptr<iroha::CommandValidator> validator;

  std::string max_amount_str =
      std::numeric_limits<boost::multiprecision::uint256_t>::max().str()
      + ".00";
  std::string admin_id = "admin@test", account_id = "test@test",
              asset_id = "coin#test", domain_id = "test",
              description = "test transfer";
  std::string admin_role = "admin";
  std::vector<std::string> admin_roles = {admin_role};
  std::vector<std::string> role_permissions;
  std::shared_ptr<shared_model::interface::Account> creator, account;
  std::shared_ptr<shared_model::interface::Amount> balance;
  std::shared_ptr<shared_model::interface::Asset> asset;
  std::shared_ptr<shared_model::interface::AccountAsset> wallet;

  std::shared_ptr<shared_model::interface::Command> command;
};

class AddAssetQuantityTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {can_add_asset_qty};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command =
        clone(*(TestTransactionBuilder()
                    .addAssetQuantity(creator->accountId(), asset_id, "3.50")
                    .build()
                    .commands()
                    .front()));
    add_asset_quantity =
        getCommand<shared_model::interface::AddAssetQuantity>(command);
  }

  std::shared_ptr<shared_model::interface::AddAssetQuantity> add_asset_quantity;
};

/**
 * @given AddAssetQuantity where accountAsset doesn't exist at first call
 * @when command is executed, new accountAsset will be created
 * @then executor will be passed
 */
TEST_F(AddAssetQuantityTest, ValidWhenNewWallet) {
  EXPECT_CALL(*wsv_query, getAccountAsset(add_asset_quantity->accountId(), _))
      .WillOnce(Return(boost::none));
  EXPECT_CALL(*wsv_query, getAsset(add_asset_quantity->assetId()))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(add_asset_quantity->accountId()))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .WillOnce(Return(WsvCommandResult()));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given AddAssetQuantity where accountAsset exists
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(AddAssetQuantityTest, ValidWhenExistingWallet) {
  EXPECT_CALL(*wsv_query,
              getAccountAsset(add_asset_quantity->accountId(),
                              add_asset_quantity->assetId()))
      .WillOnce(Return(wallet));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(add_asset_quantity->accountId()))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .WillOnce(Return(WsvCommandResult()));
  EXPECT_CALL(*wsv_query, getAccountRoles(add_asset_quantity->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given AddAssetQuantity where command creator role is wrong
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(AddAssetQuantityTest, InvalidWhenNoRoles) {
  EXPECT_CALL(*wsv_query, getAccountRoles(add_asset_quantity->accountId()))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AddAssetQuantity with 0 amount
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(AddAssetQuantityTest, InvalidWhenZeroAmount) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command = clone(*(TestTransactionBuilder()
                        .addAssetQuantity(creator->accountId(), asset_id, "0")
                        .build()
                        .commands()
                        .front()));
  add_asset_quantity =
      getCommand<shared_model::interface::AddAssetQuantity>(command);

  EXPECT_CALL(*wsv_query, getAsset(asset->assetId())).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AddAssetQuantity with amount with wrong precision (must be 2)
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(AddAssetQuantityTest, InvalidWhenWrongPrecision) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command =
      clone(*(TestTransactionBuilder()
                  .addAssetQuantity(creator->accountId(), asset_id, "1.0000")
                  .build()
                  .commands()
                  .front()));
  add_asset_quantity =
      getCommand<shared_model::interface::AddAssetQuantity>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AddAssetQuantity
 * @when command references non-existing account
 * @then execute fails and returns false
 */
TEST_F(AddAssetQuantityTest, InvalidWhenNoAccount) {
  // Account to add does not exist
  EXPECT_CALL(*wsv_query, getAccountRoles(add_asset_quantity->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(add_asset_quantity->accountId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AddAssetQuantity with wrong asset
 * @when command is executed
 * @then execute fails and returns false
 */
TEST_F(AddAssetQuantityTest, InvalidWhenNoAsset) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command =
      clone(*(TestTransactionBuilder()
                  .addAssetQuantity(creator->accountId(), "no_asset", "3.50")
                  .build()
                  .commands()
                  .front()));
  add_asset_quantity =
      getCommand<shared_model::interface::AddAssetQuantity>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAsset(add_asset_quantity->assetId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AddAssetQuantity
 * @when command adds value which overflows account balance
 * @then execute fails and returns false
 */
TEST_F(AddAssetQuantityTest, InvalidWhenAssetAdditionFails) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command = clone(
      *(TestTransactionBuilder()
            .addAssetQuantity(creator->accountId(), asset_id, max_amount_str)
            .build()
            .commands()
            .front()));
  add_asset_quantity =
      getCommand<shared_model::interface::AddAssetQuantity>(command);

  EXPECT_CALL(*wsv_query,
              getAccountAsset(add_asset_quantity->accountId(),
                              add_asset_quantity->assetId()))
      .WillOnce(Return(wallet));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(add_asset_quantity->accountId()))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getAccountRoles(add_asset_quantity->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

class SubtractAssetQuantityTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {can_subtract_asset_qty};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(
        *(TestTransactionBuilder()
              .subtractAssetQuantity(creator->accountId(), asset_id, "1.00")
              .build()
              .commands()
              .front()));
    subtract_asset_quantity =
        getCommand<shared_model::interface::SubtractAssetQuantity>(command);
  }

  std::shared_ptr<shared_model::interface::SubtractAssetQuantity>
      subtract_asset_quantity;
};

/**
 * @given SubtractAssetQuantity
 * @when account doesn't have wallet of target asset
 * @then executor will be failed
 */
TEST_F(SubtractAssetQuantityTest, InvalidWhenNoWallet) {
  EXPECT_CALL(*wsv_query,
              getAccountAsset(subtract_asset_quantity->accountId(),
                              subtract_asset_quantity->assetId()))
      .WillOnce(Return(boost::none));
  EXPECT_CALL(*wsv_query, getAccountRoles(subtract_asset_quantity->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given SubtractAssetQuantity
 * @when correct arguments
 * @then executor will be passed
 */
TEST_F(SubtractAssetQuantityTest, ValidWhenExistingWallet) {
  EXPECT_CALL(*wsv_query,
              getAccountAsset(subtract_asset_quantity->accountId(),
                              subtract_asset_quantity->assetId()))
      .WillOnce(Return(wallet));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .WillOnce(Return(WsvCommandResult()));
  EXPECT_CALL(*wsv_query, getAccountRoles(subtract_asset_quantity->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given SubtractAssetQuantity
 * @when arguments amount is greater than wallet's amount
 * @then executor will be failed
 */
TEST_F(SubtractAssetQuantityTest, InvalidWhenOverAmount) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command = clone(
      *(TestTransactionBuilder()
            .subtractAssetQuantity(creator->accountId(), asset_id, "12.04")
            .build()
            .commands()
            .front()));
  subtract_asset_quantity =
      getCommand<shared_model::interface::SubtractAssetQuantity>(command);

  EXPECT_CALL(*wsv_query,
              getAccountAsset(subtract_asset_quantity->accountId(),
                              subtract_asset_quantity->assetId()))
      .WillOnce(Return(wallet));

  EXPECT_CALL(*wsv_query, getAccountRoles(subtract_asset_quantity->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given SubtractAssetQuantity
 * @when account doesn't have role
 * @then executor will be failed
 */
TEST_F(SubtractAssetQuantityTest, InvalidWhenNoRoles) {
  EXPECT_CALL(*wsv_query, getAccountRoles(subtract_asset_quantity->accountId()))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given SubtractAssetQuantity
 * @when arguments amount is zero
 * @then executor will be failed
 */
TEST_F(SubtractAssetQuantityTest, InvalidWhenZeroAmount) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command =
      clone(*(TestTransactionBuilder()
                  .subtractAssetQuantity(creator->accountId(), asset_id, "0")
                  .build()
                  .commands()
                  .front()));
  subtract_asset_quantity =
      getCommand<shared_model::interface::SubtractAssetQuantity>(command);

  EXPECT_CALL(*wsv_query, getAsset(asset->assetId())).WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given SubtractAssetQuantity
 * @when arguments amount precision is invalid (not equal 2)
 * @then executor will be failed
 */
TEST_F(SubtractAssetQuantityTest, InvalidWhenWrongPrecision) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command = clone(
      *(TestTransactionBuilder()
            .subtractAssetQuantity(creator->accountId(), asset_id, "1.0000")
            .build()
            .commands()
            .front()));
  subtract_asset_quantity =
      getCommand<shared_model::interface::SubtractAssetQuantity>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given SubtractAssetQuantity
 * @when account doesn't exist
 * @then executor will be failed
 */
TEST_F(SubtractAssetQuantityTest, InvalidWhenNoAccount) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command = clone(*(TestTransactionBuilder()
                        .subtractAssetQuantity("noacc", asset_id, "12.04")
                        .build()
                        .commands()
                        .front()));
  subtract_asset_quantity =
      getCommand<shared_model::interface::SubtractAssetQuantity>(command);

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given SubtractAssetQuantity
 * @when asset doesn't exist
 * @then executor will be failed
 */
TEST_F(SubtractAssetQuantityTest, InvalidWhenNoAsset) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command = clone(
      *(TestTransactionBuilder()
            .subtractAssetQuantity(creator->accountId(), "no_asset", "12.04")
            .build()
            .commands()
            .front()));
  subtract_asset_quantity =
      getCommand<shared_model::interface::SubtractAssetQuantity>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAsset(subtract_asset_quantity->assetId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

class AddSignatoryTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {can_add_signatory};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command =
        clone(*(TestTransactionBuilder()
                    .addSignatory(account_id,
                                  shared_model::interface::types::PubkeyType(
                                      std::string(32, '1')))
                    .build()
                    .commands()
                    .front()));
    add_signatory = getCommand<shared_model::interface::AddSignatory>(command);
  }

  std::shared_ptr<shared_model::interface::AddSignatory> add_signatory;
};

/**
 * @given AddSignatory creator has role permission to add signatory
 * @when command is executed
 * @then executor finishes successfully
 */
TEST_F(AddSignatoryTest, ValidWhenCreatorHasPermissions) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, add_signatory->accountId(), can_add_my_signatory))
      .WillOnce(Return(true));
  EXPECT_CALL(*wsv_command, insertSignatory(add_signatory->pubkey()))
      .WillOnce(Return(WsvCommandResult()));
  EXPECT_CALL(*wsv_command,
              insertAccountSignatory(add_signatory->accountId(),
                                     add_signatory->pubkey()))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given AddSignatory with valid parameters
 * @when creator is adding public key to his account
 * @then executor finishes successfully
 */
TEST_F(AddSignatoryTest, ValidWhenSameAccount) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command =
      clone(*(TestTransactionBuilder()
                  .addSignatory(creator->accountId(),
                                shared_model::interface::types::PubkeyType(
                                    std::string(32, '1')))
                  .build()
                  .commands()
                  .front()));
  add_signatory = getCommand<shared_model::interface::AddSignatory>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_command, insertSignatory(add_signatory->pubkey()))
      .WillOnce(Return(WsvCommandResult()));
  EXPECT_CALL(*wsv_command,
              insertAccountSignatory(add_signatory->accountId(),
                                     add_signatory->pubkey()))
      .WillOnce(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given AddSignatory creator has not permission
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(AddSignatoryTest, InvalidWhenNoPermissions) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, add_signatory->accountId(), can_add_my_signatory))
      .WillOnce(Return(false));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AddSignatory command with wrong account
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(AddSignatoryTest, InvalidWhenNoAccount) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command =
      clone(*(TestTransactionBuilder()
                  .addSignatory("noacc",
                                shared_model::interface::types::PubkeyType(
                                    std::string(32, '1')))
                  .build()
                  .commands()
                  .front()));
  add_signatory = getCommand<shared_model::interface::AddSignatory>(command);

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, add_signatory->accountId(), can_add_my_signatory))
      .WillOnce(Return(false));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AddSignatory command with public key already exists
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(AddSignatoryTest, InvalidWhenSameKey) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command =
      clone(*(TestTransactionBuilder()
                  .addSignatory(account_id,
                                shared_model::interface::types::PubkeyType(
                                    std::string(32, '2')))
                  .build()
                  .commands()
                  .front()));
  add_signatory = getCommand<shared_model::interface::AddSignatory>(command);

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, add_signatory->accountId(), can_add_my_signatory))
      .WillOnce(Return(true));
  EXPECT_CALL(*wsv_command, insertSignatory(add_signatory->pubkey()))
      .WillOnce(Return(makeEmptyError()));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

class CreateAccountTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {can_create_account};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command =
        clone(*(TestTransactionBuilder()
                    .createAccount("test",
                                   domain_id,
                                   shared_model::interface::types::PubkeyType(
                                       std::string(32, '2')))
                    .build()
                    .commands()
                    .front()));
    create_account =
        getCommand<shared_model::interface::CreateAccount>(command);

    default_domain = clone(shared_model::proto::DomainBuilder()
                               .domainId(domain_id)
                               .defaultRole(admin_role)
                               .build());
  }
  std::shared_ptr<shared_model::interface::Domain> default_domain;

  std::shared_ptr<shared_model::interface::CreateAccount> create_account;
};

/**
 * @given CreateAccount with vaild parameters
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(CreateAccountTest, ValidWhenNewAccount) {
  // Valid case
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getDomain(domain_id))
      .WillOnce(Return(default_domain));
  EXPECT_CALL(*wsv_command, insertSignatory(create_account->pubkey()))
      .Times(1)
      .WillOnce(Return(WsvCommandResult()));
  EXPECT_CALL(*wsv_command, insertAccount(_))
      .WillOnce(Return(WsvCommandResult()));
  EXPECT_CALL(*wsv_command,
              insertAccountSignatory(account_id, create_account->pubkey()))
      .WillOnce(Return(WsvCommandResult()));
  EXPECT_CALL(*wsv_command, insertAccountRole(account_id, admin_role))
      .WillOnce(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given CreateAccount command and creator has not permissions
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(CreateAccountTest, InvalidWhenNoPermissions) {
  // Creator has no permission
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given CreateAccount with invalid account name
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(CreateAccountTest, InvalidWhenLongName) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command =
      clone(*(TestTransactionBuilder()
                  .createAccount("aAccountNameMustBeLessThan64characters0000000"
                                 "0000000000000000000",
                                 domain_id,
                                 shared_model::interface::types::PubkeyType(
                                     std::string(32, '2')))
                  .build()
                  .commands()
                  .front()));
  create_account = getCommand<shared_model::interface::CreateAccount>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given CreateAccount with invalid account name
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(CreateAccountTest, InvalidWhenNameWithSystemSymbols) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command =
      clone(*(TestTransactionBuilder()
                  .createAccount("test@",
                                 domain_id,
                                 shared_model::interface::types::PubkeyType(
                                     std::string(32, '2')))
                  .build()
                  .commands()
                  .front()));

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given CreateAccount command
 * @when command tries to create account in a non-existing domain
 * @then execute fails and returns false
 */
TEST_F(CreateAccountTest, InvalidWhenNoDomain) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getDomain(domain_id)).WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

class CreateAssetTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {can_create_asset};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(TestTransactionBuilder()
                          .createAsset("fcoin", domain_id, 2)
                          .build()
                          .commands()
                          .front()));
    create_asset = getCommand<shared_model::interface::CreateAsset>(command);
  }

  std::shared_ptr<shared_model::interface::CreateAsset> create_asset;
};

/**
 * @given CreateAsset with valid parameters
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(CreateAssetTest, ValidWhenCreatorHasPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_command, insertAsset(_))
      .WillOnce(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given CreateAsset and creator ha not permissions
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(CreateAssetTest, InvalidWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given CreateAsset
 * @when command tries to create asset, but insertion fails
 * @then execute() fails
 */
TEST_F(CreateAssetTest, InvalidWhenAssetInsertionFails) {
  EXPECT_CALL(*wsv_command, insertAsset(_)).WillOnce(Return(makeEmptyError()));

  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

class CreateDomainTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {can_create_domain};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(TestTransactionBuilder()
                          .createDomain("cn", "default")
                          .build()
                          .commands()
                          .front()));
    create_domain = getCommand<shared_model::interface::CreateDomain>(command);
  }

  std::shared_ptr<shared_model::interface::CreateDomain> create_domain;
};

/**
 * @given CreateDomain with valid parameters
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(CreateDomainTest, ValidWhenCreatorHasPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_command, insertDomain(_))
      .WillOnce(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given CreateDomain and creator has not permissions
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(CreateDomainTest, InvalidWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given CreateDomain
 * @when command tries to create domain, but insertion fails
 * @then execute() fails
 */
TEST_F(CreateDomainTest, InvalidWhenDomainInsertionFails) {
  EXPECT_CALL(*wsv_command, insertDomain(_)).WillOnce(Return(makeEmptyError()));

  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

class RemoveSignatoryTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    auto creator_key =
        shared_model::interface::types::PubkeyType(std::string(32, '1'));
    auto account_key =
        shared_model::interface::types::PubkeyType(std::string(32, '2'));

    account_pubkeys = {shared_model::interface::types::PubkeyType(account_key)};

    many_pubkeys = {shared_model::interface::types::PubkeyType(creator_key),
                    shared_model::interface::types::PubkeyType(account_key)};

    role_permissions = {can_remove_signatory};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command =
        clone(*(TestTransactionBuilder()
                    .removeSignatory(account_id,
                                     shared_model::interface::types::PubkeyType(
                                         std::string(32, '1')))
                    .build()
                    .commands()
                    .front()));
    remove_signatory =
        getCommand<shared_model::interface::RemoveSignatory>(command);
  }

  std::vector<shared_model::interface::types::PubkeyType> account_pubkeys;
  std::vector<shared_model::interface::types::PubkeyType> many_pubkeys;

  std::shared_ptr<shared_model::interface::RemoveSignatory> remove_signatory;
};

/**
 * @given RemoveSignatory with valid parameters
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(RemoveSignatoryTest, ValidWhenMultipleKeys) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, remove_signatory->accountId(), can_remove_my_signatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->accountId()))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->accountId()))
      .WillOnce(Return(many_pubkeys));

  EXPECT_CALL(*wsv_command,
              deleteAccountSignatory(remove_signatory->accountId(),
                                     remove_signatory->pubkey()))
      .WillOnce(Return(WsvCommandResult()));
  EXPECT_CALL(*wsv_command, deleteSignatory(remove_signatory->pubkey()))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given RemoveSignatory with valid parameters
 * @when command is executed and return single signatory pubkey
 * @then executor will be failed
 */
TEST_F(RemoveSignatoryTest, InvalidWhenSingleKey) {
  // Creator is admin
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, remove_signatory->accountId(), can_remove_my_signatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->accountId()))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->accountId()))
      .WillOnce(Return(account_pubkeys));

  // delete methods must not be called because the account quorum is 1.
  EXPECT_CALL(*wsv_command,
              deleteAccountSignatory(remove_signatory->accountId(),
                                     remove_signatory->pubkey()))
      .Times(0);
  EXPECT_CALL(*wsv_command, deleteSignatory(remove_signatory->pubkey()))
      .Times(0);

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given RemoveSignatory and creator has not permissons
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoPermissions) {
  // Add same signatory
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, remove_signatory->accountId(), can_remove_my_signatory))
      .WillOnce(Return(false));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given RemoveSignatory with signatory not present in account
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoKey) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  std::shared_ptr<shared_model::interface::Command> wrong_key_command =
      clone(*(TestTransactionBuilder()
                  .removeSignatory(account_id,
                                   shared_model::interface::types::PubkeyType(
                                       std::string(32, 0xF)))
                  .build()
                  .commands()
                  .front()));
  auto wrong_key_remove_signatory =
      getCommand<shared_model::interface::RemoveSignatory>(wrong_key_command);

  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(admin_id,
                                    wrong_key_remove_signatory->accountId(),
                                    can_remove_my_signatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(wrong_key_remove_signatory->accountId()))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_query,
              getSignatories(wrong_key_remove_signatory->accountId()))
      .WillOnce(Return(account_pubkeys));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(wrong_key_command)));
}

/**
 * @given RemoveSignatory
 * @when command tries to remove signatory from non-existing account
 * @then execute fails and returns false
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoAccount) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, remove_signatory->accountId(), can_remove_my_signatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->accountId()))
      .WillOnce(Return(boost::none));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->accountId()))
      .WillOnce(Return(many_pubkeys));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given RemoveSignatory
 * @when command tries to remove signatory from account which does not have
 * any signatories
 * @then execute fails and returns false
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoSignatories) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, remove_signatory->accountId(), can_remove_my_signatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->accountId()))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->accountId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given RemoveSignatory
 * @when command tries to remove signatory from non-existing account and it
 * has no signatories
 * @then execute fails and returns false
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoAccountAndSignatories) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, remove_signatory->accountId(), can_remove_my_signatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->accountId()))
      .WillOnce(Return(boost::none));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->accountId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given RemoveSignatory
 * @when command tries to remove signatory from creator's account but has no
 * permissions and no grantable permissions to do that
 * @then execute fails and returns false
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoPermissionToRemoveFromSelf) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  std::shared_ptr<shared_model::interface::Command> command =
      clone(*(TestTransactionBuilder()
                  .removeSignatory(creator->accountId(),
                                   shared_model::interface::types::PubkeyType(
                                       std::string(32, '1')))
                  .build()
                  .commands()
                  .front()));
  auto remove_signatory =
      getCommand<shared_model::interface::RemoveSignatory>(command);

  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(std::vector<std::string>{}));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
      .WillOnce(Return(std::vector<std::string>{admin_role}));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, admin_id, can_remove_my_signatory))
      .WillOnce(Return(false));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given RemoveSignatory
 * @when command tries to remove signatory but deletion fails
 * @then execute() fails
 */
TEST_F(RemoveSignatoryTest, InvalidWhenAccountSignatoryDeletionFails) {
  EXPECT_CALL(*wsv_command,
              deleteAccountSignatory(remove_signatory->accountId(),
                                     remove_signatory->pubkey()))
      .WillOnce(Return(makeEmptyError()));

  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

class SetQuorumTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    account_pubkeys = {
        shared_model::interface::types::PubkeyType(std::string(32, '0')),
        shared_model::interface::types::PubkeyType(std::string(32, '1'))};
    role_permissions = {can_set_quorum};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(TestTransactionBuilder()
                          .setAccountQuorum(account_id, 2)
                          .build()
                          .commands()
                          .front()));
    set_quorum = getCommand<shared_model::interface::SetQuorum>(command);

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    creator_command = clone(*(TestTransactionBuilder()
                                  .setAccountQuorum(creator->accountId(), 2)
                                  .build()
                                  .commands()
                                  .front()));
    creator_set_quorum =
        getCommand<shared_model::interface::SetQuorum>(creator_command);
  }

  std::vector<shared_model::interface::types::PubkeyType> account_pubkeys;

  std::shared_ptr<shared_model::interface::SetQuorum> set_quorum;
  std::shared_ptr<shared_model::interface::Command> creator_command;
  std::shared_ptr<shared_model::interface::SetQuorum> creator_set_quorum;
};

/**
 * @given SetQuorum
 * @when command executes
 * @then execute successes
 */
TEST_F(SetQuorumTest, ValidWhenCreatorHasPermissions) {
  // Creator is admin
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, set_quorum->accountId(), can_set_my_quorum))
      .WillOnce(Return(true));
  EXPECT_CALL(*wsv_query, getAccount(set_quorum->accountId()))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getSignatories(set_quorum->accountId()))
      .WillOnce(Return(account_pubkeys));
  EXPECT_CALL(*wsv_command, updateAccount(_))
      .WillOnce(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given SetQuorum and creator is the account parameter
 * @when command executes
 * @then execute successes
 */
TEST_F(SetQuorumTest, ValidWhenSameAccount) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccount(creator_set_quorum->accountId()))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getSignatories(creator_set_quorum->accountId()))
      .WillOnce(Return(account_pubkeys));
  EXPECT_CALL(*wsv_command, updateAccount(_))
      .WillOnce(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(creator_command)));
}
/**
 * @given SetQuorum and creator has not permissions
 * @when command executes
 * @then execute fails
 */
TEST_F(SetQuorumTest, InvalidWhenNoPermissions) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, set_quorum->accountId(), can_set_my_quorum))
      .WillOnce(Return(false));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}
/**
 * @given SetQuorum and account parameter is invalid
 * @when command executes
 * @then execute fails
 */
TEST_F(SetQuorumTest, InvalidWhenNoAccount) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command = clone(*(TestTransactionBuilder()
                        .setAccountQuorum("noacc", 2)
                        .build()
                        .commands()
                        .front()));
  set_quorum = getCommand<shared_model::interface::SetQuorum>(command);

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, set_quorum->accountId(), can_set_my_quorum))
      .WillOnce(Return(false));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given SetQuorum
 * @when command tries to set quorum for non-existing account
 * @then execute fails and returns false
 */
TEST_F(SetQuorumTest, InvalidWhenNoAccountButPassedPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getSignatories(creator_set_quorum->accountId()))
      .WillOnce(Return(account_pubkeys));

  EXPECT_CALL(*wsv_query, getAccount(creator_set_quorum->accountId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(creator_command)));
}

/**
 * @given SetQuorum
 * @when command tries to set quorum for account which does not have any
 * signatories
 * @then execute fails and returns false
 */
TEST_F(SetQuorumTest, InvalidWhenNoSignatories) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getSignatories(creator_set_quorum->accountId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(creator_command)));
}

/**
 * @given SetQuorum
 * @when command tries to set quorum for account which does not have enough
 * signatories
 * @then execute fails and returns false
 */
TEST_F(SetQuorumTest, InvalidWhenNotEnoughSignatories) {
  // Creator is the account
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccount(creator_set_quorum->accountId())).Times(0);
  std::vector<shared_model::interface::types::PubkeyType> acc_pubkeys = {
      shared_model::interface::types::PubkeyType(std::string(32, 0x1))};
  EXPECT_CALL(*wsv_query, getSignatories(creator_set_quorum->accountId()))
      .WillOnce(Return(acc_pubkeys));
  EXPECT_CALL(*wsv_command, updateAccount(_)).Times(0);

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(creator_command)));
}

class TransferAssetTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    src_wallet = clone(shared_model::proto::AccountAssetBuilder()
                           .assetId(asset_id)
                           .accountId(admin_id)
                           .balance(*balance)
                           .build());

    dst_wallet = clone(shared_model::proto::AccountAssetBuilder()
                           .assetId(asset_id)
                           .accountId(account_id)
                           .balance(*balance)
                           .build());

    role_permissions = {can_transfer, can_receive};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(
        TestTransactionBuilder()
            .transferAsset(admin_id, account_id, asset_id, description, "1.50")
            .build()
            .commands()
            .front()));
    transfer_asset =
        getCommand<shared_model::interface::TransferAsset>(command);
  }

  std::shared_ptr<shared_model::interface::AccountAsset> src_wallet, dst_wallet;

  std::shared_ptr<shared_model::interface::TransferAsset> transfer_asset;
};

/**
 * @given TransferAsset and destination account has not AccountAsset
 * @when command is executed and new AccountAsset will be created
 * @then execute successes and returns true
 */
TEST_F(TransferAssetTest, ValidWhenNewWallet) {
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAccountAsset(transfer_asset->destAccountId(), _))
      .WillOnce(Return(boost::none));

  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->srcAccountId(),
                              transfer_asset->assetId()))
      .Times(2)
      .WillRepeatedly(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .Times(2)
      .WillRepeatedly(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->destAccountId()))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .Times(2)
      .WillRepeatedly(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset
 * @when command is executed
 * @then execute successes and returns true
 */
TEST_F(TransferAssetTest, ValidWhenExistingWallet) {
  // When there is a wallet - no new accountAsset created
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->destAccountId(),
                              transfer_asset->assetId()))
      .WillOnce(Return(dst_wallet));

  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->srcAccountId(),
                              transfer_asset->assetId()))
      .Times(2)
      .WillRepeatedly(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .Times(2)
      .WillRepeatedly(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->destAccountId()))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .Times(2)
      .WillRepeatedly(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset and creator has not permission
 * @when command is executed
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset and destination account doesn't have any role
 * @when command is executed
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenNoDestAccount) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  std::shared_ptr<shared_model::interface::Command> command = clone(
      *(TestTransactionBuilder()
            .transferAsset(admin_id, "noacc", asset_id, description, "150.00")
            .build()
            .commands()
            .front()));
  auto transfer_asset =
      getCommand<shared_model::interface::TransferAsset>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(boost::none));

  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset and source account doesn't have asset
 * @when command is executed
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenNoSrcAccountAsset) {
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->srcAccountId(),
                              transfer_asset->assetId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset
 * @when command tries to transfer asset from non-existing account
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenNoSrcAccountAssetDuringExecute) {
  // No source account asset exists
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->srcAccountId(),
                              transfer_asset->assetId()))
      .Times(2)
      .WillOnce(Return(src_wallet))
      .WillOnce(Return(boost::none));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->destAccountId()))
      .WillOnce(Return(account));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset
 * @when command tries to transfer non-existent asset
 * @then isValid fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenNoAssetDuringValidation) {
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset
 * @when command tries to transfer non-existent asset
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenNoAssetId) {
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .WillOnce(Return(asset))
      .WillOnce(Return(boost::none));
  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->srcAccountId(),
                              transfer_asset->assetId()))
      .Times(2)
      .WillRepeatedly(Return(src_wallet));
  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->destAccountId(),
                              transfer_asset->assetId()))
      .WillOnce(Return(dst_wallet));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->destAccountId()))
      .WillOnce(Return(account));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset
 * @when command tries to transfer amount which is less than source balance
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenInsufficientFunds) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  std::shared_ptr<shared_model::interface::Command> command = clone(*(
      TestTransactionBuilder()
          .transferAsset(admin_id, account_id, asset_id, description, "155.00")
          .build()
          .commands()
          .front()));
  auto transfer_asset =
      getCommand<shared_model::interface::TransferAsset>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->srcAccountId(),
                              transfer_asset->assetId()))
      .WillOnce(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->destAccountId()))
      .WillOnce(Return(account));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset
 * @when command tries to transfer amount which is less than source balance
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenInsufficientFundsDuringExecute) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  std::shared_ptr<shared_model::interface::Command> command = clone(*(
      TestTransactionBuilder()
          .transferAsset(admin_id, account_id, asset_id, description, "155.00")
          .build()
          .commands()
          .front()));
  auto transfer_asset =
      getCommand<shared_model::interface::TransferAsset>(command);

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->srcAccountId(),
                              transfer_asset->assetId()))
      .WillOnce(Return(src_wallet));
  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->destAccountId(),
                              transfer_asset->assetId()))
      .WillOnce(Return(dst_wallet));

  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

/**
 * @given TransferAsset
 * @when command tries to transfer amount which has wrong precesion (must be 2)
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenWrongPrecision) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  std::shared_ptr<shared_model::interface::Command> command =
      clone(*(TestTransactionBuilder()
                  .transferAsset(
                      admin_id, account_id, asset_id, description, "155.0000")
                  .build()
                  .commands()
                  .front()));
  auto transfer_asset =
      getCommand<shared_model::interface::TransferAsset>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .WillOnce(Return(asset));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset
 * @when command tries to transfer amount with wrong precision
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenWrongPrecisionDuringExecute) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  std::shared_ptr<shared_model::interface::Command> command = clone(*(
      TestTransactionBuilder()
          .transferAsset(admin_id, account_id, asset_id, description, "1.5000")
          .build()
          .commands()
          .front()));
  auto transfer_asset =
      getCommand<shared_model::interface::TransferAsset>(command);

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->srcAccountId(),
                              transfer_asset->assetId()))
      .WillOnce(Return(src_wallet));
  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->destAccountId(),
                              transfer_asset->assetId()))
      .WillOnce(Return(dst_wallet));

  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

/**
 * @given TransferAsset
 * @when command tries to transfer asset which overflows destination balance
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenAmountOverflow) {
  std::shared_ptr<shared_model::interface::Amount> max_balance = clone(
      shared_model::proto::AmountBuilder()
          .intValue(
              std::numeric_limits<boost::multiprecision::uint256_t>::max())
          .precision(2)
          .build());

  std::shared_ptr<shared_model::interface::AccountAsset> max_wallet =
      clone(shared_model::proto::AccountAssetBuilder()
                .assetId(src_wallet->assetId())
                .accountId(src_wallet->accountId())
                .balance(*max_balance)
                .build());

  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->srcAccountId(),
                              transfer_asset->assetId()))
      .WillOnce(Return(src_wallet));
  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->destAccountId(),
                              transfer_asset->assetId()))
      .WillOnce(Return(max_wallet));

  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

/**
 * @given TransferAsset and creator has not permissions
 * @when command tries to transfer
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenCreatorHasNoPermission) {
  // Transfer creator is not connected to account
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  std::shared_ptr<shared_model::interface::Command> command = clone(*(
      TestTransactionBuilder()
          .transferAsset(account_id, admin_id, asset_id, description, "155.00")
          .build()
          .commands()
          .front()));
  auto transfer_asset =
      getCommand<shared_model::interface::TransferAsset>(command);

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, account_id, can_transfer_my_assets))
      .WillOnce(Return(false));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset and creator has permissions
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(TransferAssetTest, ValidWhenCreatorHasPermission) {
  // Transfer creator is not connected to account
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  std::shared_ptr<shared_model::interface::Command> command = clone(
      *(TestTransactionBuilder()
            .transferAsset(account_id, admin_id, asset_id, description, "1.50")
            .build()
            .commands()
            .front()));
  auto transfer_asset =
      getCommand<shared_model::interface::TransferAsset>(command);

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, account_id, can_transfer_my_assets))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_query, getAccountAsset(transfer_asset->destAccountId(), _))
      .WillOnce(Return(boost::none));

  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->srcAccountId(),
                              transfer_asset->assetId()))
      .Times(2)
      .WillRepeatedly(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .Times(2)
      .WillRepeatedly(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->destAccountId()))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
      .Times(2)
      .WillRepeatedly(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

class AddPeerTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {can_add_peer};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(
        TestTransactionBuilder()
            .addPeer("iroha_node:10001", shared_model::crypto::PublicKey("key"))
            .build()
            .commands()
            .front()));
  }
};

/**
 * @given AddPeer and all parameters are valid
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(AddPeerTest, ValidCase) {
  // Valid case
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_command, insertPeer(_)).WillOnce(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given AddPeer and creator has not permissions
 * @when command tries to transfer
 * @then execute failed
 */
TEST_F(AddPeerTest, InvalidCaseWhenNoPermissions) {
  // Valid case
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AddPeer
 * @when command tries to insert peer but insertion fails
 * @then execute returns false
 */
TEST_F(AddPeerTest, InvalidCaseWhenInsertPeerFails) {
  EXPECT_CALL(*wsv_command, insertPeer(_)).WillOnce(Return(makeEmptyError()));

  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

class CreateRoleTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    std::set<std::string> perm = {can_create_role};
    role_permissions = {can_create_role};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(TestTransactionBuilder()
                          .createRole("yoda", perm)
                          .build()
                          .commands()
                          .front()));
    create_role = getCommand<shared_model::interface::CreateRole>(command);
  }
  std::shared_ptr<shared_model::interface::CreateRole> create_role;
};

/**
 * @given CreateRole and all parameters are valid
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(CreateRoleTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillRepeatedly(Return(role_permissions));
  EXPECT_CALL(*wsv_command, insertRole(create_role->roleName()))
      .WillOnce(Return(WsvCommandResult()));
  EXPECT_CALL(*wsv_command,
              insertRolePermissions(create_role->roleName(),
                                    create_role->rolePermissions()))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given CreateRole and creator has not permissions
 * @when command tries to transfer
 * @then execute is failed
 */
TEST_F(CreateRoleTest, InvalidCaseWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillRepeatedly(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given CreateRole with wrong permissions
 * @when command tries to transfer
 * @then execute is failed
 */
TEST_F(CreateRoleTest, InvalidCaseWhenRoleSuperset) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  std::set<std::string> master_perms = {can_add_peer, can_append_role};
  command = clone(*(TestTransactionBuilder()
                        .createRole("master", master_perms)
                        .build()
                        .commands()
                        .front()));

  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillRepeatedly(Return(role_permissions));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given CreateRole
 * @when command tries to create new role, but insertion fails
 * @then execute returns false
 */
TEST_F(CreateRoleTest, InvalidCaseWhenRoleInsertionFails) {
  EXPECT_CALL(*wsv_command, insertRole(create_role->roleName()))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

class AppendRoleTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {can_append_role};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(TestTransactionBuilder()
                          .appendRole("yoda", "master")
                          .build()
                          .commands()
                          .front()));
    append_role = getCommand<shared_model::interface::AppendRole>(command);
  }
  std::shared_ptr<shared_model::interface::AppendRole> append_role;
};

/**
 * @given CreateRole and all parameters are valid
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(AppendRoleTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .Times(2)
      .WillRepeatedly(Return(admin_roles));

  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRolePermissions("master"))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(
      *wsv_command,
      insertAccountRole(append_role->accountId(), append_role->roleName()))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given CreateRole and creator has not permissions
 * @when command tries to transfer
 * @then execute failed
 */
TEST_F(AppendRoleTest, InvalidCaseNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AppendRole
 * @when command tries to append non-existing role
 * @then execute() fails and returns false
 */
TEST_F(AppendRoleTest, InvalidCaseNoAccountRole) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .Times(2)
      .WillOnce(Return(admin_roles))
      .WillOnce((Return(boost::none)));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRolePermissions("master"))
      .WillOnce(Return(role_permissions));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AppendRole
 * @when command tries to append non-existing role and creator does not have
 any
 * roles
 * @then execute() fails and returns false
 */
TEST_F(AppendRoleTest, InvalidCaseNoAccountRoleAndNoPermission) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .Times(2)
      .WillOnce(Return(admin_roles))
      .WillOnce((Return(boost::none)));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRolePermissions("master"))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AppendRole
 * @when command tries to append role, but creator account does not have
 * necessary permission
 * @then execute() fails and returns false
 */
TEST_F(AppendRoleTest, InvalidCaseRoleHasNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .Times(2)
      .WillOnce(Return(admin_roles))
      .WillOnce((Return(admin_roles)));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillOnce(Return(role_permissions))
      .WillOnce(Return(boost::none));
  EXPECT_CALL(*wsv_query, getRolePermissions("master"))
      .WillOnce(Return(role_permissions));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given AppendRole
 * @when command tries to append role, but insertion of account fails
 * @then execute() fails
 */
TEST_F(AppendRoleTest, InvalidCaseInsertAccountRoleFails) {
  EXPECT_CALL(
      *wsv_command,
      insertAccountRole(append_role->accountId(), append_role->roleName()))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

class DetachRoleTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {can_detach_role};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(TestTransactionBuilder()
                          .detachRole("yoda", "master")
                          .build()
                          .commands()
                          .front()));
    detach_role = getCommand<shared_model::interface::DetachRole>(command);
  }
  std::shared_ptr<shared_model::interface::DetachRole> detach_role;
};

/**
 * @given DetachRole and all parameters are valid
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(DetachRoleTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(
      *wsv_command,
      deleteAccountRole(detach_role->accountId(), detach_role->roleName()))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given DetachRole and creator has not permissions
 * @when command tries to transfer
 * @then execute failed
 */
TEST_F(DetachRoleTest, InvalidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given DetachRole
 * @when deletion of account role fails
 * @then execute fails()
 */
TEST_F(DetachRoleTest, InvalidCaseWhenDeleteAccountRoleFails) {
  EXPECT_CALL(
      *wsv_command,
      deleteAccountRole(detach_role->accountId(), detach_role->roleName()))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

class GrantPermissionTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    expected_permission = can_add_my_signatory;
    role_permissions = {can_grant + expected_permission};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(TestTransactionBuilder()
                          .grantPermission("yoda", expected_permission)
                          .build()
                          .commands()
                          .front()));
    grant_permission =
        getCommand<shared_model::interface::GrantPermission>(command);
  }
  std::shared_ptr<shared_model::interface::GrantPermission> grant_permission;
  std::string expected_permission;
};

/**
 * @given GrantPermission and all parameters are valid
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(GrantPermissionTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_command,
              insertAccountGrantablePermission(grant_permission->accountId(),
                                               creator->accountId(),
                                               expected_permission))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given GrantPermission and creator has not permissions
 * @when command tries to transfer
 * @then execute failed
 */
TEST_F(GrantPermissionTest, InvalidCaseWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given GrantPermission
 * @when command tries to grant permission but insertion fails
 * @then execute() fails
 */
TEST_F(GrantPermissionTest, InvalidCaseWhenInsertGrantablePermissionFails) {
  EXPECT_CALL(*wsv_command,
              insertAccountGrantablePermission(grant_permission->accountId(),
                                               creator->accountId(),
                                               expected_permission))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

class RevokePermissionTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    expected_permission = "can_add_my_signatory";

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(TestTransactionBuilder()
                          .revokePermission("yoda", expected_permission)
                          .build()
                          .commands()
                          .front()));
    revoke_permission =
        getCommand<shared_model::interface::RevokePermission>(command);
  }

  std::shared_ptr<shared_model::interface::RevokePermission> revoke_permission;
  std::string expected_permission;
};

/**
 * @given RevokePermission and all parameters are valid
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(RevokePermissionTest, ValidCase) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          revoke_permission->accountId(), admin_id, expected_permission))
      .WillOnce(Return(true));
  EXPECT_CALL(*wsv_command,
              deleteAccountGrantablePermission(revoke_permission->accountId(),
                                               creator->accountId(),
                                               expected_permission))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given RevokePermission and creator has not permissions
 * @when command tries to transfer
 * @then execute failed
 */
TEST_F(RevokePermissionTest, InvalidCaseNoPermissions) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          revoke_permission->accountId(), admin_id, expected_permission))
      .WillOnce(Return(false));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given RevokePermission
 * @when deleting permission fails
 * @then execute fails
 */
TEST_F(RevokePermissionTest, InvalidCaseDeleteAccountPermissionvFails) {
  EXPECT_CALL(*wsv_command,
              deleteAccountGrantablePermission(revoke_permission->accountId(),
                                               creator->accountId(),
                                               expected_permission))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}

class SetAccountDetailTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(TestTransactionBuilder()
                          .setAccountDetail(admin_id, "key", "val")
                          .build()
                          .commands()
                          .front()));
    set_aacount_detail =
        getCommand<shared_model::interface::SetAccountDetail>(command);

    role_permissions = {can_set_quorum};
  }

  std::string needed_permission = can_set_my_account_detail;

  std::shared_ptr<shared_model::interface::SetAccountDetail> set_aacount_detail;
};

/**
 * @given SetAccountDetail and all parameters are valid
 * @when creator is setting details to their account
 * @then successfully execute the command
 */
TEST_F(SetAccountDetailTest, ValidWhenSetOwnAccount) {
  EXPECT_CALL(*wsv_command,
              setAccountKV(set_aacount_detail->accountId(),
                           creator->accountId(),
                           set_aacount_detail->key(),
                           set_aacount_detail->value()))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given SetAccountDetail
 * @when creator is setting details to their account
 * @then execute fails
 */
TEST_F(SetAccountDetailTest, InValidWhenOtherCreator) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command = clone(*(TestTransactionBuilder()
                        .setAccountDetail(account_id, "key", "val")
                        .build()
                        .commands()
                        .front()));
  set_aacount_detail =
      getCommand<shared_model::interface::SetAccountDetail>(command);

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, set_aacount_detail->accountId(), needed_permission))
      .WillOnce(Return(false));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given SetAccountDetail
 * @when creator is setting details to their account
 * @then successfully execute the command
 */
TEST_F(SetAccountDetailTest, ValidWhenHasPermissions) {
  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command = clone(*(TestTransactionBuilder()
                        .setAccountDetail(account_id, "key", "val")
                        .build()
                        .commands()
                        .front()));
  set_aacount_detail =
      getCommand<shared_model::interface::SetAccountDetail>(command);

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, set_aacount_detail->accountId(), needed_permission))
      .WillOnce(Return(true));
  EXPECT_CALL(*wsv_command,
              setAccountKV(set_aacount_detail->accountId(),
                           creator->accountId(),
                           set_aacount_detail->key(),
                           set_aacount_detail->value()))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

/**
 * @given SetAccountDetail
 * @when command tries to set details, but setting key-value fails
 * @then execute fails
 */
TEST_F(SetAccountDetailTest, InvalidWhenSetAccountKVFails) {
  EXPECT_CALL(*wsv_command,
              setAccountKV(set_aacount_detail->accountId(),
                           creator->accountId(),
                           set_aacount_detail->key(),
                           set_aacount_detail->value()))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(command)));
}
