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
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "validators/permissions.hpp"

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

#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using ::testing::AllOf;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::_;

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace framework::expected;
using namespace shared_model::permissions;

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

    executor = std::make_shared<iroha::CommandExecutor>(
        iroha::CommandExecutor(wsv_query, wsv_command));
    validator = std::make_shared<iroha::CommandValidator>(
        iroha::CommandValidator(wsv_query));

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

    default_domain = clone(shared_model::proto::DomainBuilder()
                               .domainId(domain_id)
                               .defaultRole(admin_role)
                               .build());
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

  Amount max_amount{
      std::numeric_limits<boost::multiprecision::uint256_t>::max(), 2};
  std::string admin_id = "admin@test", account_id = "test@test",
              asset_id = "coin#test", domain_id = "test",
              description = "test transfer";

  std::string admin_role = "admin";

  std::vector<std::string> admin_roles = {admin_role};
  std::vector<std::string> role_permissions;
  std::shared_ptr<shared_model::interface::Domain> default_domain;

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockWsvCommand> wsv_command;

  std::shared_ptr<shared_model::interface::Account> creator, account;

  std::shared_ptr<iroha::CommandExecutor> executor;
  std::shared_ptr<iroha::CommandValidator> validator;
};

// class AddAssetQuantityTest : public CommandValidateExecuteTest {
// public:
//  void SetUp() override {
//    CommandValidateExecuteTest::SetUp();
//
//    shared_model::builder::AssetBuilder<
//        shared_model::proto::AssetBuilder,
//        shared_model::validation::FieldValidator>()
//        .assetId(asset_id)
//        .domainId(domain_id)
//        .precision(2)
//        .build()
//        .match(
//            [&](expected::Value<std::shared_ptr<shared_model::interface::Asset>>
//                    &v) { asset = v.value; },
//            [](expected::Error<std::shared_ptr<std::string>> &e) {
//              FAIL() << *e.error;
//            });
//
//    shared_model::builder::AmountBuilder<
//        shared_model::proto::AmountBuilder,
//        shared_model::validation::FieldValidator>()
//        .intValue(150)
//        .precision(2)
//        .build()
//        .match(
//            [&](expected::Value<
//                std::shared_ptr<shared_model::interface::Amount>> &v) {
//              balance = v.value;
//            },
//            [](expected::Error<std::shared_ptr<std::string>> &e) {
//              FAIL() << *e.error;
//            });
//
//    shared_model::builder::AccountAssetBuilder<
//        shared_model::proto::AccountAssetBuilder,
//        shared_model::validation::FieldValidator>()
//        .assetId(asset_id)
//        .accountId(account_id)
//        .balance(*balance)
//        .build()
//        .match(
//            [&](expected::Value<
//                std::shared_ptr<shared_model::interface::AccountAsset>> &v) {
//              wallet = v.value;
//            },
//            [](expected::Error<std::shared_ptr<std::string>> &e) {
//              FAIL() << *e.error;
//            });
//
//    add_asset_quantity = std::make_shared<AddAssetQuantity>();
//    add_asset_quantity->account_id = creator->accountId();
//    Amount amount(350, 2);
//    add_asset_quantity->amount = amount;
//    add_asset_quantity->asset_id = asset_id;
//
//    command = add_asset_quantity;
//    role_permissions = {can_add_asset_qty};
//  }
//
//  std::shared_ptr<shared_model::interface::Amount> balance;
//  std::shared_ptr<shared_model::interface::Asset> asset;
//  std::shared_ptr<shared_model::interface::AccountAsset> wallet;
//
//  std::shared_ptr<AddAssetQuantity> add_asset_quantity;
//};
//
// TEST_F(AddAssetQuantityTest, ValidWhenNewWallet) {
//  // Add asset first time - no wallet
//  // When there is no wallet - new accountAsset will be created
//  EXPECT_CALL(*wsv_query, getAccountAsset(add_asset_quantity->account_id, _))
//      .WillOnce(Return(boost::none));
//
//  EXPECT_CALL(*wsv_query, getAsset(add_asset_quantity->asset_id))
//      .WillOnce(Return(asset));
//  EXPECT_CALL(*wsv_query, getAccount(add_asset_quantity->account_id))
//      .WillOnce(Return(account));
//  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
//      .WillOnce(Return(WsvCommandResult()));
//  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//
//  ASSERT_NO_THROW(checkValueCase(validateAndExecute()));
//}
//
// TEST_F(AddAssetQuantityTest, ValidWhenExistingWallet) {
//  // There is already asset- there is a wallet
//  // When there is a wallet - no new accountAsset created
//  EXPECT_CALL(*wsv_query,
//              getAccountAsset(add_asset_quantity->account_id,
//                              add_asset_quantity->asset_id))
//      .WillOnce(Return(wallet));
//
//  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
//  EXPECT_CALL(*wsv_query, getAccount(add_asset_quantity->account_id))
//      .WillOnce(Return(account));
//  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
//      .WillOnce(Return(WsvCommandResult()));
//  EXPECT_CALL(*wsv_query, getAccountRoles(add_asset_quantity->account_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  ASSERT_NO_THROW(checkValueCase(validateAndExecute()));
//}
//
// TEST_F(AddAssetQuantityTest, InvalidWhenNoRoles) {
//  // Creator has no roles
//  EXPECT_CALL(*wsv_query, getAccountRoles(add_asset_quantity->account_id))
//      .WillOnce(Return(boost::none));
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
// TEST_F(AddAssetQuantityTest, InvalidWhenZeroAmount) {
//  // Amount is zero
//  Amount amount(0);
//  add_asset_quantity->amount = amount;
//  EXPECT_CALL(*wsv_query, getAsset(asset->assetId())).WillOnce(Return(asset));
//  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
// TEST_F(AddAssetQuantityTest, InvalidWhenWrongPrecision) {
//  // Amount is with wrong precision (must be 2)
//  Amount amount(add_asset_quantity->amount.getIntValue(), 30);
//  add_asset_quantity->amount = amount;
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
///**
// * @given AddAssetQuantity
// * @when command references non-existing account
// * @then execute fails and returns false
// */
// TEST_F(AddAssetQuantityTest, InvalidWhenNoAccount) {
//  // Account to add does not exist
//  EXPECT_CALL(*wsv_query, getAccountRoles(add_asset_quantity->account_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
//  EXPECT_CALL(*wsv_query, getAccount(add_asset_quantity->account_id))
//      .WillOnce(Return(boost::none));
//
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
// TEST_F(AddAssetQuantityTest, InvalidWhenNoAsset) {
//  // Asset doesn't exist
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  add_asset_quantity->asset_id = "noass";
//
//  EXPECT_CALL(*wsv_query, getAsset(add_asset_quantity->asset_id))
//      .WillOnce(Return(boost::none));
//
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
///**
// * @given AddAssetQuantity
// * @when command adds value which overflows account balance
// * @then execute fails and returns false
// */
// TEST_F(AddAssetQuantityTest, InvalidWhenAssetAdditionFails) {
//  // amount overflows
//  add_asset_quantity->amount = max_amount;
//
//  EXPECT_CALL(*wsv_query,
//              getAccountAsset(add_asset_quantity->account_id,
//                              add_asset_quantity->asset_id))
//      .WillOnce(Return(wallet));
//
//  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
//  EXPECT_CALL(*wsv_query, getAccount(add_asset_quantity->account_id))
//      .WillOnce(Return(account));
//  EXPECT_CALL(*wsv_query, getAccountRoles(add_asset_quantity->account_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
// class SubtractAssetQuantityTest : public CommandValidateExecuteTest {
// public:
//  void SetUp() override {
//    CommandValidateExecuteTest::SetUp();
//
//    shared_model::builder::AmountBuilder<
//        shared_model::proto::AmountBuilder,
//        shared_model::validation::FieldValidator>()
//        .intValue(150ul)
//        .precision(2)
//        .build()
//        .match(
//            [&](expected::Value<
//                std::shared_ptr<shared_model::interface::Amount>> &v) {
//              balance = v.value;
//            },
//            [](expected::Error<std::shared_ptr<std::string>> &e) {
//              FAIL() << *e.error;
//            });
//    ;
//
//    shared_model::builder::AssetBuilder<
//        shared_model::proto::AssetBuilder,
//        shared_model::validation::FieldValidator>()
//        .assetId(asset_id)
//        .domainId(domain_id)
//        .precision(2)
//        .build()
//        .match(
//            [&](expected::Value<std::shared_ptr<shared_model::interface::Asset>>
//                    &v) { asset = v.value; },
//            [](expected::Error<std::shared_ptr<std::string>> &e) {
//              FAIL() << *e.error;
//            });
//
//    shared_model::builder::AccountAssetBuilder<
//        shared_model::proto::AccountAssetBuilder,
//        shared_model::validation::FieldValidator>()
//        .assetId(asset_id)
//        .accountId(account_id)
//        .balance(*balance)
//        .build()
//        .match(
//            [&](expected::Value<
//                std::shared_ptr<shared_model::interface::AccountAsset>> &v) {
//              wallet = v.value;
//            },
//            [](expected::Error<std::shared_ptr<std::string>> &e) {
//              FAIL() << *e.error;
//            });
//
//    subtract_asset_quantity = std::make_shared<SubtractAssetQuantity>();
//    subtract_asset_quantity->account_id = creator->accountId();
//    Amount amount(100, 2);
//    subtract_asset_quantity->amount = amount;
//    subtract_asset_quantity->asset_id = asset_id;
//
//    command = subtract_asset_quantity;
//    role_permissions = {can_subtract_asset_qty};
//  }
//
//  std::shared_ptr<shared_model::interface::Amount> balance;
//  std::shared_ptr<shared_model::interface::Asset> asset;
//  std::shared_ptr<shared_model::interface::AccountAsset> wallet;
//
//  std::shared_ptr<SubtractAssetQuantity> subtract_asset_quantity;
//};
//
///**
// * @given SubtractAssetQuantity
// * @when account doesn't have wallet of target asset
// * @then executor will be failed
// */
// TEST_F(SubtractAssetQuantityTest, InvalidWhenNoWallet) {
//  // Subtract asset - no wallet
//  // When there is no wallet - Failed
//  EXPECT_CALL(*wsv_query,
//              getAccountAsset(subtract_asset_quantity->account_id,
//                              subtract_asset_quantity->asset_id))
//      .WillOnce(Return(boost::none));
//
//  EXPECT_CALL(*wsv_query,
//  getAccountRoles(subtract_asset_quantity->account_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
///**
// * @given SubtractAssetQuantity
// * @when correct arguments
// * @then executor will be passed
// */
// TEST_F(SubtractAssetQuantityTest, ValidWhenExistingWallet) {
//  // There is already asset- there is a wallet
//  // When there is a wallet - no new accountAsset created
//  EXPECT_CALL(*wsv_query,
//              getAccountAsset(subtract_asset_quantity->account_id,
//                              subtract_asset_quantity->asset_id))
//      .WillOnce(Return(wallet));
//
//  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
//  EXPECT_CALL(*wsv_command, upsertAccountAsset(_))
//      .WillOnce(Return(WsvCommandResult()));
//  EXPECT_CALL(*wsv_query,
//  getAccountRoles(subtract_asset_quantity->account_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  ASSERT_NO_THROW(checkValueCase(validateAndExecute()));
//}
//
///**
// * @given SubtractAssetQuantity
// * @when arguments amount is greater than wallet's amount
// * @then executor will be failed
// */
// TEST_F(SubtractAssetQuantityTest, InvalidWhenOverAmount) {
//  Amount amount(1204, 2);
//  subtract_asset_quantity->amount = amount;
//  EXPECT_CALL(*wsv_query,
//              getAccountAsset(subtract_asset_quantity->account_id,
//                              subtract_asset_quantity->asset_id))
//      .WillOnce(Return(wallet));
//
//  EXPECT_CALL(*wsv_query,
//  getAccountRoles(subtract_asset_quantity->account_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
//
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
///**
// * @given SubtractAssetQuantity
// * @when account doesn't have role
// * @then executor will be failed
// */
// TEST_F(SubtractAssetQuantityTest, InvalidWhenNoRoles) {
//  // Creator has no roles
//  EXPECT_CALL(*wsv_query,
//  getAccountRoles(subtract_asset_quantity->account_id))
//      .WillOnce(Return(boost::none));
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
///**
// * @given SubtractAssetQuantity
// * @when arguments amount is zero
// * @then executor will be failed
// */
// TEST_F(SubtractAssetQuantityTest, InvalidWhenZeroAmount) {
//  // Amount is zero
//  Amount amount(0);
//  subtract_asset_quantity->amount = amount;
//  EXPECT_CALL(*wsv_query, getAsset(asset->assetId())).WillOnce(Return(asset));
//  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
///**
// * @given SubtractAssetQuantity
// * @when arguments amount precision is invalid
// * @then executor will be failed
// */
// TEST_F(SubtractAssetQuantityTest, InvalidWhenWrongPrecision) {
//  // Amount is with wrong precision (must be 2)
//  Amount amount(subtract_asset_quantity->amount.getIntValue(), 30);
//  subtract_asset_quantity->amount = amount;
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  EXPECT_CALL(*wsv_query, getAsset(asset_id)).WillOnce(Return(asset));
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
///**
// * @given SubtractAssetQuantity
// * @when account doesn't exist
// * @then executor will be failed
// */
// TEST_F(SubtractAssetQuantityTest, InvalidWhenNoAccount) {
//  // Account to subtract doesn't exist
//  subtract_asset_quantity->account_id = "noacc";
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
///**
// * @given SubtractAssetQuantity
// * @when asset doesn't exist
// * @then executor will be failed
// */
// TEST_F(SubtractAssetQuantityTest, InvalidWhenNoAsset) {
//  // Asset doesn't exist
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  subtract_asset_quantity->asset_id = "noass";
//
//  EXPECT_CALL(*wsv_query, getAsset(subtract_asset_quantity->asset_id))
//      .WillOnce(Return(boost::none));
//
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
// class AddSignatoryTest : public CommandValidateExecuteTest {
// public:
//  void SetUp() override {
//    CommandValidateExecuteTest::SetUp();
//
//    add_signatory = std::make_shared<AddSignatory>();
//    add_signatory->account_id = account_id;
//    add_signatory->pubkey.fill(1);  // Such Pubkey exist
//    role_permissions = {can_add_signatory};
//    command = add_signatory;
//  }
//
//  std::shared_ptr<AddSignatory> add_signatory;
//};
//
// TEST_F(AddSignatoryTest, ValidWhenCreatorHasPermissions) {
//  // Creator has role permissions to add signatory
//  EXPECT_CALL(*wsv_query,
//              hasAccountGrantablePermission(
//                  admin_id, add_signatory->account_id, can_add_signatory))
//      .WillOnce(Return(true));
//  EXPECT_CALL(
//      *wsv_command,
//      insertSignatory(shared_model::crypto::PublicKey(
//          {add_signatory->pubkey.begin(), add_signatory->pubkey.end()})))
//      .WillOnce(Return(WsvCommandResult()));
//  EXPECT_CALL(*wsv_command,
//              insertAccountSignatory(add_signatory->account_id,
//                                     shared_model::crypto::PublicKey(
//                                         {add_signatory->pubkey.begin(),
//                                          add_signatory->pubkey.end()})))
//      .WillOnce(Return(WsvCommandResult()));
//  ASSERT_NO_THROW(checkValueCase(validateAndExecute()));
//}
//
// TEST_F(AddSignatoryTest, ValidWhenSameAccount) {
//  // When creator is adding public keys to his account
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  add_signatory->account_id = creator->accountId();
//
//  EXPECT_CALL(
//      *wsv_command,
//      insertSignatory(shared_model::crypto::PublicKey(
//          {add_signatory->pubkey.begin(), add_signatory->pubkey.end()})))
//      .WillOnce(Return(WsvCommandResult()));
//  EXPECT_CALL(*wsv_command,
//              insertAccountSignatory(add_signatory->account_id,
//                                     shared_model::crypto::PublicKey(
//                                         {add_signatory->pubkey.begin(),
//                                          add_signatory->pubkey.end()})))
//      .WillOnce(Return(WsvCommandResult()));
//  ASSERT_NO_THROW(checkValueCase(validateAndExecute()));
//}
//
// TEST_F(AddSignatoryTest, InvalidWhenNoPermissions) {
//  // Creator has no permission
//  EXPECT_CALL(*wsv_query,
//              hasAccountGrantablePermission(
//                  admin_id, add_signatory->account_id, can_add_signatory))
//      .WillOnce(Return(false));
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
// TEST_F(AddSignatoryTest, InvalidWhenNoAccount) {
//  // Add to nonexistent account
//  add_signatory->account_id = "noacc";
//
//  EXPECT_CALL(*wsv_query,
//              hasAccountGrantablePermission(
//                  admin_id, add_signatory->account_id, can_add_signatory))
//      .WillOnce(Return(false));
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
// TEST_F(AddSignatoryTest, InvalidWhenSameKey) {
//  // Add same signatory
//  EXPECT_CALL(*wsv_query,
//              hasAccountGrantablePermission(
//                  admin_id, add_signatory->account_id, can_add_signatory))
//      .WillOnce(Return(true));
//  add_signatory->pubkey.fill(2);
//  EXPECT_CALL(
//      *wsv_command,
//      insertSignatory(shared_model::crypto::PublicKey(
//          {add_signatory->pubkey.begin(), add_signatory->pubkey.end()})))
//      .WillOnce(Return(makeEmptyError()));
//
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
// class CreateAccountTest : public CommandValidateExecuteTest {
// public:
//  void SetUp() override {
//    CommandValidateExecuteTest::SetUp();
//
//    create_account = std::make_shared<CreateAccount>();
//    create_account->account_name = "test";
//    create_account->domain_id = domain_id;
//    create_account->pubkey.fill(2);
//
//    command = create_account;
//    role_permissions = {can_create_account};
//  }
//
//  std::shared_ptr<CreateAccount> create_account;
//};
//
// TEST_F(CreateAccountTest, ValidWhenNewAccount) {
//  // Valid case
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  EXPECT_CALL(*wsv_query, getDomain(domain_id))
//      .WillOnce(Return(default_domain));
//
//  EXPECT_CALL(
//      *wsv_command,
//      insertSignatory(shared_model::crypto::PublicKey(
//          {create_account->pubkey.begin(), create_account->pubkey.end()})))
//      .Times(1)
//      .WillOnce(Return(WsvCommandResult()));
//
//  EXPECT_CALL(*wsv_command, insertAccount(_))
//      .WillOnce(Return(WsvCommandResult()));
//
//  EXPECT_CALL(*wsv_command,
//              insertAccountSignatory(account_id,
//                                     shared_model::crypto::PublicKey(
//                                         {create_account->pubkey.begin(),
//                                          create_account->pubkey.end()})))
//      .WillOnce(Return(WsvCommandResult()));
//  EXPECT_CALL(*wsv_command, insertAccountRole(account_id, admin_role))
//      .WillOnce(Return(WsvCommandResult()));
//
//  ASSERT_NO_THROW(checkValueCase(validateAndExecute()));
//}
//
// TEST_F(CreateAccountTest, InvalidWhenNoPermissions) {
//  // Creator has no permission
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(boost::none));
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
// TEST_F(CreateAccountTest, InvalidWhenLongName) {
//  // Not valid name for account
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  create_account->account_name =
//      "aAccountNameMustBeLessThan64characters00000000000000000000000000";
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
// TEST_F(CreateAccountTest, InvalidWhenNameWithSystemSymbols) {
//  // Not valid name for account (system symbols)
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  create_account->account_name = "test@";
//
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
///**
// * @given CreateAccountCommand
// * @when command tries to create account in a non-existing domain
// * @then execute fails and returns false
// */
// TEST_F(CreateAccountTest, InvalidWhenNoDomain) {
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//  EXPECT_CALL(*wsv_query, getDomain(domain_id)).WillOnce(Return(boost::none));
//
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
// class CreateAssetTest : public CommandValidateExecuteTest {
// public:
//  void SetUp() override {
//    CommandValidateExecuteTest::SetUp();
//
//    create_asset = std::make_shared<CreateAsset>();
//    create_asset->asset_name = "fcoin";
//    create_asset->domain_id = domain_id;
//    create_asset->precision = 2;
//
//    command = create_asset;
//    role_permissions = {can_create_asset};
//  }
//
//  std::shared_ptr<CreateAsset> create_asset;
//};
//
// TEST_F(CreateAssetTest, ValidWhenCreatorHasPermissions) {
//  // Creator is money creator
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//
//  EXPECT_CALL(*wsv_command, insertAsset(_))
//      .WillOnce(Return(WsvCommandResult()));
//
//  ASSERT_NO_THROW(checkValueCase(validateAndExecute()));
//}
//
// TEST_F(CreateAssetTest, InvalidWhenNoPermissions) {
//  // Creator has no permissions
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(boost::none));
//
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
///**
// * @given CreateAsset
// * @when command tries to create asset, but insertion fails
// * @then execute() fails
// */
// TEST_F(CreateAssetTest, InvalidWhenAssetInsertionFails) {
//  EXPECT_CALL(*wsv_command,
//  insertAsset(_)).WillOnce(Return(makeEmptyError()));
//
//  ASSERT_NO_THROW(checkErrorCase(execute()));
//}
//
// class CreateDomainTest : public CommandValidateExecuteTest {
// public:
//  void SetUp() override {
//    CommandValidateExecuteTest::SetUp();
//
//    create_domain = std::make_shared<CreateDomain>();
//    create_domain->domain_id = "cn";
//    create_domain->user_default_role = "default";
//
//    command = create_domain;
//    role_permissions = {can_create_domain};
//  }
//
//  std::shared_ptr<CreateDomain> create_domain;
//};
//
// TEST_F(CreateDomainTest, ValidWhenCreatorHasPermissions) {
//  // Valid case
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .WillOnce(Return(role_permissions));
//
//  EXPECT_CALL(*wsv_command, insertDomain(_))
//      .WillOnce(Return(WsvCommandResult()));
//
//  ASSERT_NO_THROW(checkValueCase(validateAndExecute()));
//}
//
// TEST_F(CreateDomainTest, InvalidWhenNoPermissions) {
//  // Creator has no permissions
//  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
//      .WillOnce(Return(boost::none));
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute()));
//}
//
///**
// * @given CreateDomain
// * @when command tries to create domain, but insertion fails
// * @then execute() fails
// */
// TEST_F(CreateDomainTest, InvalidWhenDomainInsertionFails) {
//  EXPECT_CALL(*wsv_command,
//  insertDomain(_)).WillOnce(Return(makeEmptyError()));
//
//  ASSERT_NO_THROW(checkErrorCase(execute()));
//}

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
  std::shared_ptr<shared_model::interface::Command> command;
  std::shared_ptr<shared_model::interface::RemoveSignatory> remove_signatory;
};

TEST_F(RemoveSignatoryTest, ValidWhenMultipleKeys) {
  // Creator is admin
  // Add same signatory
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, remove_signatory->accountId(), can_remove_signatory))
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

TEST_F(RemoveSignatoryTest, InvalidWhenSingleKey) {
  // Creator is admin
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, remove_signatory->accountId(), can_remove_signatory))
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

TEST_F(RemoveSignatoryTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  // Add same signatory
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, remove_signatory->accountId(), can_remove_signatory))
      .WillOnce(Return(false));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

TEST_F(RemoveSignatoryTest, InvalidWhenNoKey) {
  // Remove signatory not present in account

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
                                    can_remove_signatory))
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
          admin_id, remove_signatory->accountId(), can_remove_signatory))
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
 any
 * signatories
 * @then execute fails and returns false
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoSignatories) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, remove_signatory->accountId(), can_remove_signatory))
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
 has
 * no signatories
 * @then execute fails and returns false
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoAccountAndSignatories) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          admin_id, remove_signatory->accountId(), can_remove_signatory))
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
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(admin_id, admin_id, can_remove_signatory))
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
  std::shared_ptr<shared_model::interface::Command> command;
  std::shared_ptr<shared_model::interface::SetQuorum> set_quorum;
  std::shared_ptr<shared_model::interface::Command> creator_command;
  std::shared_ptr<shared_model::interface::SetQuorum> creator_set_quorum;
};

TEST_F(SetQuorumTest, ValidWhenCreatorHasPermissions) {
  // Creator is admin
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, set_quorum->accountId(), can_set_quorum))
      .WillOnce(Return(true));
  EXPECT_CALL(*wsv_query, getAccount(set_quorum->accountId()))
      .WillOnce(Return(account));
  EXPECT_CALL(*wsv_query, getSignatories(set_quorum->accountId()))
      .WillOnce(Return(account_pubkeys));
  EXPECT_CALL(*wsv_command, updateAccount(_))
      .WillOnce(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

TEST_F(SetQuorumTest, ValidWhenSameAccount) {
  // Creator is the account
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

TEST_F(SetQuorumTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, set_quorum->accountId(), can_set_quorum))
      .WillOnce(Return(false));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

TEST_F(SetQuorumTest, InvalidWhenNoAccount) {
  // No such account exists

  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
  command = clone(*(TestTransactionBuilder()
                        .setAccountQuorum("noacc", 2)
                        .build()
                        .commands()
                        .front()));
  set_quorum = getCommand<shared_model::interface::SetQuorum>(command);

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, set_quorum->accountId(), can_set_quorum))
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

TEST_F(SetQuorumTest, InvalidWhenNotEnoughSignatories) {
  // Creator is the account
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccount(creator_set_quorum->accountId())).Times(0);
  pubkey_t key;
  key.fill(0x1);
  std::vector<shared_model::interface::types::PubkeyType> acc_pubkeys = {
      shared_model::interface::types::PubkeyType(key.to_string())};
  EXPECT_CALL(*wsv_query, getSignatories(creator_set_quorum->accountId()))
      .WillOnce(Return(acc_pubkeys));
  EXPECT_CALL(*wsv_command, updateAccount(_)).Times(0);

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(creator_command)));
}

class TransferAssetTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    asset = clone(shared_model::proto::AssetBuilder()
                      .assetId(asset_id)
                      .domainId(domain_id)
                      .precision(2)
                      .build());

    balance = clone(shared_model::proto::AmountBuilder()
                        .intValue(150)
                        .precision(2)
                        .build());

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

  std::shared_ptr<shared_model::interface::Amount> balance;
  std::shared_ptr<shared_model::interface::Asset> asset;
  std::shared_ptr<shared_model::interface::AccountAsset> src_wallet, dst_wallet;

  std::shared_ptr<shared_model::interface::Command> command;
  std::shared_ptr<shared_model::interface::TransferAsset> transfer_asset;
};

TEST_F(TransferAssetTest, ValidWhenNewWallet) {
  // When there is no wallet - new accountAsset will be created
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

TEST_F(TransferAssetTest, InvalidWhenNoPermissions) {
  // Creator has no permissions
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

TEST_F(TransferAssetTest, InvalidWhenNoDestAccount) {
  // No destination account exists
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

TEST_F(TransferAssetTest, InvalidWhenNoSrcAccountAsset) {
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

TEST_F(TransferAssetTest, InvalidWhenInsufficientFunds) {
  // No sufficient funds
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));

  Amount amount(155, 2);

  EXPECT_CALL(*wsv_query,
              getAccountAsset(transfer_asset->srcAccountId(),
                              transfer_asset->assetId()))
      .WillOnce(Return(src_wallet));
  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
      .WillOnce(Return(asset));
  EXPECT_CALL(*wsv_query, getAccount(transfer_asset->destAccountId()))
      .WillOnce(Return(boost::none));

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @given TransferAsset
 * @when command tries to transfer amount which is less than source balance
 * @then execute fails and returns false
 */
TEST_F(TransferAssetTest, InvalidWhenInsufficientFundsDuringExecute) {
  // More than account balance
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

// TEST_F(TransferAssetTest, InvalidWhenWrongPrecision) {
//  // Amount has wrong precision
//  // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
//  std::shared_ptr<shared_model::interface::Command> command = clone(*(
//      TestTransactionBuilder()
//          .transferAsset(admin_id, account_id, asset_id, description,
//          "155.000000000000000000000000") .build() .commands() .front()));
//  auto transfer_asset =
//      getCommand<shared_model::interface::TransferAsset>(command);
//
//  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
//      .WillOnce(Return(admin_roles));
//  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
//      .Times(2)
//      .WillRepeatedly(Return(role_permissions));
//
//  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
//      .WillOnce(Return(asset));
//
//  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
//}

///**
// * @given TransferAsset
// * @when command tries to transfer amount with wrong precision
// * @then execute fails and returns false
// */
// TEST_F(TransferAssetTest, InvalidWhenWrongPrecisionDuringExecute) {
//  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
//      .WillOnce(Return(asset));
//  EXPECT_CALL(
//      *wsv_query,
//      getAccountAsset(transfer_asset->srcAccountId(),
//      transfer_asset->assetId())) .WillOnce(Return(src_wallet));
//  EXPECT_CALL(*wsv_query,
//              getAccountAsset(transfer_asset->destAccountId(),
//                              transfer_asset->assetId()))
//      .WillOnce(Return(dst_wallet));
//
//  Amount amount(transfer_asset->amount.getIntValue(), 30);
//  transfer_asset->amount = amount;
//  ASSERT_NO_THROW(checkErrorCase(execute(command)));
//}

///**
// * @given TransferAsset
// * @when command tries to transfer asset which overflows destination balance
// * @then execute fails and returns false
// */
// TEST_F(TransferAssetTest, InvalidWhenAmountOverflow) {
//  std::shared_ptr<shared_model::interface::Amount> max_balance = clone(
//      shared_model::proto::AmountBuilder()
//          .intValue(
//              std::numeric_limits<boost::multiprecision::uint256_t>::max())
//          .precision(2)
//          .build());
//
//  src_wallet = clone(shared_model::proto::AccountAssetBuilder()
//                         .assetId(src_wallet->assetId())
//                         .accountId(src_wallet->accountId())
//                         .balance(*max_balance)
//                         .build());
//
//  EXPECT_CALL(*wsv_query, getAsset(transfer_asset->assetId()))
//      .WillOnce(Return(asset));
//  EXPECT_CALL(
//      *wsv_query,
//      getAccountAsset(transfer_asset->srcAccountId(),
//      transfer_asset->assetId())) .WillOnce(Return(src_wallet));
//  EXPECT_CALL(*wsv_query,
//              getAccountAsset(transfer_asset->destAccountId(),
//                              transfer_asset->assetId()))
//      .WillOnce(Return(dst_wallet));
//
//  // More than account balance
//  transfer_asset->amount = (max_amount - Amount(100, 2)).value();
//
//  ASSERT_NO_THROW(checkErrorCase(execute(command)));
//}

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
              hasAccountGrantablePermission(admin_id, account_id, can_transfer))
      .WillOnce(Return(false));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

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
              hasAccountGrantablePermission(admin_id, account_id, can_transfer))
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
    add_peer = getCommand<shared_model::interface::AddPeer>(command);
  }

  std::shared_ptr<shared_model::interface::Command> command;
  std::shared_ptr<shared_model::interface::AddPeer> add_peer;
};

TEST_F(AddPeerTest, ValidCase) {
  // Valid case
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_command, insertPeer(_)).WillOnce(Return(WsvCommandResult()));

  ASSERT_NO_THROW(checkValueCase(validateAndExecute(command)));
}

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
    exact_command = clone(*(TestTransactionBuilder()
                                .createRole("yoda", perm)
                                .build()
                                .commands()
                                .front()));
    create_role =
        getCommand<shared_model::interface::CreateRole>(exact_command);

    std::set<std::string> master_perms = {can_add_peer, can_append_role};
    master_command = clone(*(TestTransactionBuilder()
                                 .createRole("master", master_perms)
                                 .build()
                                 .commands()
                                 .front()));
  }
  std::shared_ptr<shared_model::interface::Command> exact_command;
  std::shared_ptr<shared_model::interface::CreateRole> create_role;
  std::shared_ptr<shared_model::interface::Command> master_command;
};

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
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(exact_command)));
}

TEST_F(CreateRoleTest, InvalidCaseWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillRepeatedly(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(exact_command)));
}

TEST_F(CreateRoleTest, InvalidCaseWhenRoleSuperset) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillRepeatedly(Return(role_permissions));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(master_command)));
}

/**
 * @given CreateRole
 * @when command tries to create new role, but insertion fails
 * @then execute returns false
 */
TEST_F(CreateRoleTest, InvalidCaseWhenRoleInsertionFails) {
  EXPECT_CALL(*wsv_command, insertRole(create_role->roleName()))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(exact_command)));
}

class AppendRoleTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {can_append_role};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    exact_command = clone(*(TestTransactionBuilder()
                                .appendRole("yoda", "master")
                                .build()
                                .commands()
                                .front()));
    exact_cmd = getCommand<shared_model::interface::AppendRole>(exact_command);
  }
  std::shared_ptr<shared_model::interface::Command> exact_command;
  std::shared_ptr<shared_model::interface::AppendRole> exact_cmd;
};

TEST_F(AppendRoleTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .Times(2)
      .WillRepeatedly(Return(admin_roles));

  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRolePermissions("master"))
      .WillOnce(Return(role_permissions));

  EXPECT_CALL(*wsv_command,
              insertAccountRole(exact_cmd->accountId(), exact_cmd->roleName()))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(exact_command)));
}

TEST_F(AppendRoleTest, InvalidCaseNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(exact_command)));
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
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(exact_command)));
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
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(exact_command)));
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

  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(exact_command)));
}

/**
 * @given AppendRole
 * @when command tries to append role, but insertion of account fails
 * @then execute() fails
 */
TEST_F(AppendRoleTest, InvalidCaseInsertAccountRoleFails) {
  EXPECT_CALL(*wsv_command,
              insertAccountRole(exact_cmd->accountId(), exact_cmd->roleName()))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(exact_command)));
}

class DetachRoleTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {can_detach_role};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    exact_command = clone(*(TestTransactionBuilder()
                                .detachRole("yoda", "master")
                                .build()
                                .commands()
                                .front()));
    exact_cmd = getCommand<shared_model::interface::DetachRole>(exact_command);
  }
  std::shared_ptr<shared_model::interface::Command> exact_command;
  std::shared_ptr<shared_model::interface::DetachRole> exact_cmd;
};

TEST_F(DetachRoleTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_command,
              deleteAccountRole(exact_cmd->accountId(), exact_cmd->roleName()))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(exact_command)));
}

TEST_F(DetachRoleTest, InvalidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(exact_command)));
}

/**
 * @given DetachRole
 * @when deletion of account role fails
 * @then execute fails()
 */
TEST_F(DetachRoleTest, InvalidCaseWhenDeleteAccountRoleFails) {
  EXPECT_CALL(*wsv_command,
              deleteAccountRole(exact_cmd->accountId(), exact_cmd->roleName()))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(exact_command)));
}

class GrantPermissionTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    expected_permission = can_add_my_signatory;
    role_permissions = {can_grant + expected_permission};

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    exact_command = clone(*(TestTransactionBuilder()
                                .grantPermission("yoda", expected_permission)
                                .build()
                                .commands()
                                .front()));
    exact_cmd =
        getCommand<shared_model::interface::GrantPermission>(exact_command);
  }
  std::shared_ptr<shared_model::interface::Command> exact_command;
  std::shared_ptr<shared_model::interface::GrantPermission> exact_cmd;
  std::string expected_permission;
};

TEST_F(GrantPermissionTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(
      *wsv_command,
      insertAccountGrantablePermission(
          exact_cmd->accountId(), creator->accountId(), expected_permission))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(exact_command)));
}

TEST_F(GrantPermissionTest, InvalidCaseWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(admin_id))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(admin_role))
      .WillOnce(Return(boost::none));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(exact_command)));
}

/**
 * @given GrantPermission
 * @when command tries to grant permission but insertion fails
 * @then execute() fails
 */
TEST_F(GrantPermissionTest, InvalidCaseWhenInsertGrantablePermissionFails) {
  EXPECT_CALL(
      *wsv_command,
      insertAccountGrantablePermission(
          exact_cmd->accountId(), creator->accountId(), expected_permission))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(exact_command)));
}

class RevokePermissionTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    expected_permission = "can_add_my_signatory";

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    exact_command = clone(*(TestTransactionBuilder()
                                .revokePermission("yoda", expected_permission)
                                .build()
                                .commands()
                                .front()));
    exact_cmd =
        getCommand<shared_model::interface::RevokePermission>(exact_command);
  }

  std::shared_ptr<shared_model::interface::Command> exact_command;
  std::shared_ptr<shared_model::interface::RevokePermission> exact_cmd;
  std::string expected_permission;
};

TEST_F(RevokePermissionTest, ValidCase) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  exact_cmd->accountId(), admin_id, expected_permission))
      .WillOnce(Return(true));
  EXPECT_CALL(
      *wsv_command,
      deleteAccountGrantablePermission(
          exact_cmd->accountId(), creator->accountId(), expected_permission))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(exact_command)));
}

TEST_F(RevokePermissionTest, InvalidCaseNoPermissions) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  exact_cmd->accountId(), admin_id, expected_permission))
      .WillOnce(Return(false));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(exact_command)));
}

/**
 * @given RevokePermission
 * @when deleting permission fails
 * @then execute fails
 */
TEST_F(RevokePermissionTest, InvalidCaseDeleteAccountPermissionvFails) {
  EXPECT_CALL(
      *wsv_command,
      deleteAccountGrantablePermission(
          exact_cmd->accountId(), creator->accountId(), expected_permission))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(exact_command)));
}

class SetAccountDetailTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    adminCommand = clone(*(TestTransactionBuilder()
                               .setAccountDetail(admin_id, "key", "val")
                               .build()
                               .commands()
                               .front()));
    adminCmd =
        getCommand<shared_model::interface::SetAccountDetail>(adminCommand);

    // TODO 2018-04-20 Alexey Chernyshov - rework with CommandBuilder
    command = clone(*(TestTransactionBuilder()
                          .setAccountDetail(account_id, "key", "val")
                          .build()
                          .commands()
                          .front()));
    cmd = getCommand<shared_model::interface::SetAccountDetail>(command);

    role_permissions = {can_set_quorum};
  }

  std::shared_ptr<shared_model::interface::Command> adminCommand;
  std::shared_ptr<shared_model::interface::SetAccountDetail> adminCmd;
  std::shared_ptr<shared_model::interface::Command> command;
  std::shared_ptr<shared_model::interface::SetAccountDetail> cmd;
  std::string needed_permission = can_set_detail;
};

/**
 * @when creator is setting details to their account
 * @then successfully execute the command
 */
TEST_F(SetAccountDetailTest, ValidWhenSetOwnAccount) {
  EXPECT_CALL(*wsv_command,
              setAccountKV(adminCmd->accountId(),
                           creator->accountId(),
                           adminCmd->key(),
                           adminCmd->value()))
      .WillOnce(Return(WsvCommandResult()));
  ASSERT_NO_THROW(checkValueCase(validateAndExecute(adminCommand)));
}

/**
 * @when creator is setting details to their account
 * @then successfully execute the command
 */
TEST_F(SetAccountDetailTest, InValidWhenOtherCreator) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, cmd->accountId(), needed_permission))
      .WillOnce(Return(false));
  ASSERT_NO_THROW(checkErrorCase(validateAndExecute(command)));
}

/**
 * @when creator is setting details to their account
 * @then successfully execute the command
 */
TEST_F(SetAccountDetailTest, ValidWhenHasPermissions) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  admin_id, cmd->accountId(), needed_permission))
      .WillOnce(Return(true));
  EXPECT_CALL(
      *wsv_command,
      setAccountKV(
          cmd->accountId(), creator->accountId(), cmd->key(), cmd->value()))
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
              setAccountKV(adminCmd->accountId(),
                           creator->accountId(),
                           adminCmd->key(),
                           adminCmd->value()))
      .WillOnce(Return(makeEmptyError()));
  ASSERT_NO_THROW(checkErrorCase(execute(adminCommand)));
}
