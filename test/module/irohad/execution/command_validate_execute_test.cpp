/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <limits>

#include "backend/protobuf/permissions.hpp"
#include "builders/default_builders.hpp"
#include "execution/command_executor.hpp"
#include "framework/result_fixture.hpp"
#include "framework/specified_visitor.hpp"
#include "interfaces/commands/command.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

using namespace iroha;
using namespace iroha::ametsuchi;
using namespace framework::expected;
using namespace shared_model::proto::permissions;
using namespace shared_model::interface::permissions;

// TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework function with
// CommandBuilder
/**
 * Hepler function to build command and wrap it into
 * std::unique_ptr<>
 * @param builder command builder
 * @return command
 */
std::unique_ptr<shared_model::interface::Command> buildCommand(
    const TestTransactionBuilder &builder) {
  return clone(builder.build().commands().front());
}

/**
 * Helper function to get concrete command from Command container.
 * @tparam T - type of concrete command
 * @param command - Command container
 * @return concrete command extracted from container
 */
template <class T>
std::shared_ptr<T> getConcreteCommand(
    const std::unique_ptr<shared_model::interface::Command> &command) {
  return clone(
      boost::apply_visitor(framework::SpecifiedVisitor<T>(), command->get()));
}

class CommandValidateExecuteTest : public ::testing::Test {
 public:
  void SetUp() override {
    wsv_query = std::make_shared<StrictMock<MockWsvQuery>>();
    wsv_command = std::make_shared<StrictMock<MockWsvCommand>>();
    validator = std::make_unique<iroha::CommandValidator>(wsv_query);

    shared_model::builder::AccountBuilder<
        shared_model::proto::AccountBuilder,
        shared_model::validation::FieldValidator>()
        .accountId(kAdminId)
        .domainId(kDomainId)
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
        .accountId(kAccountId)
        .domainId(kDomainId)
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
        .assetId(kAssetId)
        .domainId(kDomainId)
        .precision(2)
        .build()
        .match(
            [&](expected::Value<std::shared_ptr<shared_model::interface::Asset>>
                    &v) { asset = v.value; },
            [](expected::Error<std::shared_ptr<std::string>> &e) {
              FAIL() << *e.error;
            });

    shared_model::builder::AccountAssetBuilder<
        shared_model::proto::AccountAssetBuilder,
        shared_model::validation::FieldValidator>()
        .assetId(kAssetId)
        .accountId(kAccountId)
        .balance(balance)
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

  iroha::CommandResult validate(
      const std::unique_ptr<shared_model::interface::Command> &command) {
    validator->setCreatorAccountId(creator->accountId());
    return boost::apply_visitor(*validator, command->get());
  }

  /// return result with empty error message
  WsvCommandResult makeEmptyError() {
    return WsvCommandResult(iroha::expected::makeError(""));
  }

  /// Returns error from result or throws error in case result contains value
  iroha::CommandResult::ErrorType checkErrorCase(
      const iroha::CommandResult &result) {
    return boost::get<iroha::CommandResult::ErrorType>(result);
  }

  const std::string kMaxAmountStr =
      std::numeric_limits<boost::multiprecision::uint256_t>::max().str()
      + ".00";
  const std::string kAmountWrongPrecision = "1.0000";
  const std::string kAmount = "1.00";
  const std::string kAmountOverflow = "12.04";
  const std::string kAdminId = "admin@test";
  const std::string kAccountId = "test@test";
  const std::string kNoAcountId = "noacc";
  const std::string kAssetId = "coin#test";
  const std::string kNoAssetId = "no_asset#test";
  const std::string kDomainId = "test";
  const std::string kDescription = "test transfer";
  const std::string kAdminRole = "admin";
  const std::string kMasterRole = "master";
  const std::vector<std::string> admin_roles = {kAdminRole};
  const shared_model::interface::types::PubkeyType kPubKey1 =
      shared_model::interface::types::PubkeyType(std::string(
          shared_model::crypto::DefaultCryptoAlgorithmType::kPublicKeyLength,
          '1'));
  const shared_model::interface::types::PubkeyType kPubKey2 =
      shared_model::interface::types::PubkeyType(std::string(
          shared_model::crypto::DefaultCryptoAlgorithmType::kPublicKeyLength,
          '2'));

  shared_model::interface::RolePermissionSet role_permissions;
  std::shared_ptr<shared_model::interface::Account> creator, account;
  shared_model::interface::Amount balance =
      shared_model::interface::Amount("1.50");
  std::shared_ptr<shared_model::interface::Asset> asset;
  std::shared_ptr<shared_model::interface::AccountAsset> wallet;

  std::unique_ptr<shared_model::interface::Command> command;

  std::shared_ptr<MockWsvQuery> wsv_query;
  std::shared_ptr<MockWsvCommand> wsv_command;

  std::unique_ptr<iroha::CommandValidator> validator;
};

class AddAssetQuantityTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {Role::kAddAssetQty};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(
        TestTransactionBuilder().addAssetQuantity(kAssetId, kAmount));
    add_asset_quantity =
        getConcreteCommand<shared_model::interface::AddAssetQuantity>(command);
  }

  std::shared_ptr<shared_model::interface::AddAssetQuantity> add_asset_quantity;
};

/**
 * @given AddAssetQuantity where accountAsset doesn't exist at first call
 * @when command is executed, new accountAsset will be created
 * @then executor will be passed
 */
TEST_F(AddAssetQuantityTest, ValidWhenNewWallet) {
  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));

  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given AddAssetQuantity where accountAsset exists
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(AddAssetQuantityTest, ValidWhenExistingWallet) {
  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given AddAssetQuantity where command creator role is wrong
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(AddAssetQuantityTest, InvalidWhenNoRoles) {
  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
      .WillOnce(Return(boost::none));
  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given AddAssetQuantity
 * @when command references non-existing account
 * @then execute fails
 */
TEST_F(AddAssetQuantityTest, InvalidWhenNoAccount) {
  // Account to add does not exist
  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));

  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given AddAssetQuantity with wrong asset
 * @when command is executed
 * @then execute fails
 */
TEST_F(AddAssetQuantityTest, InvalidWhenNoAsset) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - IR-1276 - rework with
  // CommandBuilder
  command = buildCommand(
      TestTransactionBuilder().addAssetQuantity(kNoAssetId, kAmount));
  add_asset_quantity =
      getConcreteCommand<shared_model::interface::AddAssetQuantity>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));

  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given AddAssetQuantity
 * @when command adds value which overflows account balance
 * @then execute fails
 */
TEST_F(AddAssetQuantityTest, InvalidWhenAssetAdditionFails) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
  command = buildCommand(
      TestTransactionBuilder().addAssetQuantity(kAssetId, kMaxAmountStr));
  add_asset_quantity =
      getConcreteCommand<shared_model::interface::AddAssetQuantity>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));

  ASSERT_TRUE(val(validate(command)));
}

class SubtractAssetQuantityTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {Role::kSubtractAssetQty};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(
        TestTransactionBuilder().subtractAssetQuantity(kAssetId, kAmount));
    subtract_asset_quantity =
        getConcreteCommand<shared_model::interface::SubtractAssetQuantity>(
            command);
  }

  std::shared_ptr<shared_model::interface::SubtractAssetQuantity>
      subtract_asset_quantity;
};


/**
 * @given SubtractAssetQuantity
 * @when arguments are valid
 * @then executor will be passed
 */
TEST_F(SubtractAssetQuantityTest, ValidWhenExistingWallet) {
  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  ASSERT_TRUE(val(validate(command)));
}

class AddSignatoryTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {Role::kAddSignatory};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(
        TestTransactionBuilder().addSignatory(kAccountId, kPubKey1));
    add_signatory =
        getConcreteCommand<shared_model::interface::AddSignatory>(command);
  }

  std::shared_ptr<shared_model::interface::AddSignatory> add_signatory;
};

/**
 * @given AddSignatory creator has role permission to add signatory
 * @when command is executed
 * @then executor finishes successfully
 */
TEST_F(AddSignatoryTest, ValidWhenCreatorHasPermissions) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          kAdminId, add_signatory->accountId(), Grantable::kAddMySignatory))
      .WillOnce(Return(true));
  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given AddSignatory with valid parameters
 * @when creator is adding public key to his account
 * @then executor finishes successfully
 */
TEST_F(AddSignatoryTest, ValidWhenSameAccount) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
  command = buildCommand(
      TestTransactionBuilder().addSignatory(creator->accountId(), kPubKey1));
  add_signatory =
      getConcreteCommand<shared_model::interface::AddSignatory>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given AddSignatory creator has not grantable permissions
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(AddSignatoryTest, InvalidWhenNoPermissions) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          kAdminId, add_signatory->accountId(), Grantable::kAddMySignatory))
      .WillOnce(Return(false));

  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given AddSignatory command with wrong account
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(AddSignatoryTest, InvalidWhenNoAccount) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
  command = buildCommand(
      TestTransactionBuilder().addSignatory(kNoAcountId, kPubKey1));
  add_signatory =
      getConcreteCommand<shared_model::interface::AddSignatory>(command);

  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          kAdminId, add_signatory->accountId(), Grantable::kAddMySignatory))
      .WillOnce(Return(false));

  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given AddSignatory command with an existed public key
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(AddSignatoryTest, InvalidWhenSameKey) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
  command =
      buildCommand(TestTransactionBuilder().addSignatory(kAccountId, kPubKey2));
  add_signatory =
      getConcreteCommand<shared_model::interface::AddSignatory>(command);

  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          kAdminId, add_signatory->accountId(), Grantable::kAddMySignatory))
      .WillOnce(Return(true));

  ASSERT_TRUE(val(validate(command)));
}

class CreateAccountTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {Role::kCreateAccount};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(
        TestTransactionBuilder().createAccount("test", kDomainId, kPubKey2));
    create_account =
        getConcreteCommand<shared_model::interface::CreateAccount>(command);

    default_domain = clone(shared_model::proto::DomainBuilder()
                               .domainId(kDomainId)
                               .defaultRole(kAdminRole)
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
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));

  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given CreateAccount command and creator has not roles
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(CreateAccountTest, InvalidWhenNoPermissions) {
  // Creator has no permission
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(boost::none));
  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given CreateAccount command
 * @when command tries to create account in a non-existing domain
 * @then execute fails
 */
TEST_F(CreateAccountTest, InvalidWhenNoDomain) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));

  ASSERT_TRUE(val(validate(command)));
}

class CreateAssetTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {Role::kCreateAsset};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(
        TestTransactionBuilder().createAsset("fcoin", kDomainId, 2));
    create_asset =
        getConcreteCommand<shared_model::interface::CreateAsset>(command);
  }

  std::shared_ptr<shared_model::interface::CreateAsset> create_asset;
};

/**
 * @given CreateAsset with valid parameters
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(CreateAssetTest, ValidWhenCreatorHasPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given CreateAsset and creator has not role permissions
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(CreateAssetTest, InvalidWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(boost::none));

  ASSERT_TRUE(err(validate(command)));
}

class CreateDomainTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {Role::kCreateDomain};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command =
        buildCommand(TestTransactionBuilder().createDomain("cn", kDomainId));
    create_domain =
        getConcreteCommand<shared_model::interface::CreateDomain>(command);
  }

  std::shared_ptr<shared_model::interface::CreateDomain> create_domain;
};

/**
 * @given CreateDomain with valid parameters
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(CreateDomainTest, ValidWhenCreatorHasPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));

  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given CreateDomain and creator has not account roles
 * @when command is executed
 * @then executor will be failed
 */
TEST_F(CreateDomainTest, InvalidWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(boost::none));
  ASSERT_TRUE(err(validate(command)));
}

class RemoveSignatoryTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    auto creator_key = kPubKey1;
    auto account_key = kPubKey2;

    account_pubkeys = {account_key};

    many_pubkeys = {creator_key, account_key};

    role_permissions = {Role::kRemoveSignatory};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(
        TestTransactionBuilder().removeSignatory(kAccountId, kPubKey1));
    remove_signatory =
        getConcreteCommand<shared_model::interface::RemoveSignatory>(command);
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
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(kAdminId,
                                            remove_signatory->accountId(),
                                            Grantable::kRemoveMySignatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->accountId()))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->accountId()))
      .WillOnce(Return(many_pubkeys));

  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given RemoveSignatory with valid parameters
 * @when command is executed and return single signatory pubkey
 * @then executor will be failed
 */
TEST_F(RemoveSignatoryTest, InvalidWhenSingleKey) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(kAdminId,
                                            remove_signatory->accountId(),
                                            Grantable::kRemoveMySignatory))
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

  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given RemoveSignatory and creator has not grantable permissions
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoPermissions) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(kAdminId,
                                            remove_signatory->accountId(),
                                            Grantable::kRemoveMySignatory))
      .WillOnce(Return(false));

  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given RemoveSignatory with signatory is not present in account
 * @when command is executed
 * @then executor will be passed
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoKey) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
  std::unique_ptr<shared_model::interface::Command> wrong_key_command =
      buildCommand(
          TestTransactionBuilder().removeSignatory(kAccountId, kPubKey1));
  auto wrong_key_remove_signatory =
      getConcreteCommand<shared_model::interface::RemoveSignatory>(
          wrong_key_command);

  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(kAdminId,
                                    wrong_key_remove_signatory->accountId(),
                                    Grantable::kRemoveMySignatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(wrong_key_remove_signatory->accountId()))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_query,
              getSignatories(wrong_key_remove_signatory->accountId()))
      .WillOnce(Return(account_pubkeys));

  ASSERT_TRUE(err(validate(wrong_key_command)));
}

/**
 * @given RemoveSignatory
 * @when command tries to remove signatory from non-existing account
 * @then execute fails
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoAccount) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(kAdminId,
                                            remove_signatory->accountId(),
                                            Grantable::kRemoveMySignatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->accountId()))
      .WillOnce(Return(boost::none));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->accountId()))
      .WillOnce(Return(many_pubkeys));

  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given RemoveSignatory
 * @when command tries to remove signatory from account which does not have
 * any signatories
 * @then execute fails
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoSignatories) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(kAdminId,
                                            remove_signatory->accountId(),
                                            Grantable::kRemoveMySignatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->accountId()))
      .WillOnce(Return(account));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->accountId()))
      .WillOnce(Return(boost::none));

  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given RemoveSignatory
 * @when command tries to remove signatory from non-existing account and it
 * has no signatories
 * @then execute fails
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoAccountAndSignatories) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(kAdminId,
                                            remove_signatory->accountId(),
                                            Grantable::kRemoveMySignatory))
      .WillOnce(Return(true));

  EXPECT_CALL(*wsv_query, getAccount(remove_signatory->accountId()))
      .WillOnce(Return(boost::none));

  EXPECT_CALL(*wsv_query, getSignatories(remove_signatory->accountId()))
      .WillOnce(Return(boost::none));

  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given RemoveSignatory
 * @when command tries to remove signatory from creator's account but has no
 * permissions and no grantable permissions to do that
 * @then execute fails
 */
TEST_F(RemoveSignatoryTest, InvalidWhenNoPermissionToRemoveFromSelf) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
  command = buildCommand(
      TestTransactionBuilder().removeSignatory(creator->accountId(), kPubKey1));
  auto remove_signatory =
      getConcreteCommand<shared_model::interface::RemoveSignatory>(command);

  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(shared_model::interface::RolePermissionSet{}));
  EXPECT_CALL(*wsv_query, getAccountRoles(creator->accountId()))
      .WillOnce(Return(std::vector<std::string>{kAdminRole}));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  kAdminId, kAdminId, Grantable::kRemoveMySignatory))
      .WillOnce(Return(false));

  ASSERT_TRUE(err(validate(command)));
}

class SetQuorumTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    account_pubkeys = {kPubKey1, kPubKey2};
    role_permissions = {Role::kSetQuorum};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command =
        buildCommand(TestTransactionBuilder().setAccountQuorum(kAccountId, 2));
    set_quorum =
        getConcreteCommand<shared_model::interface::SetQuorum>(command);

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    creator_command = buildCommand(
        TestTransactionBuilder().setAccountQuorum(creator->accountId(), 2));
    creator_set_quorum =
        getConcreteCommand<shared_model::interface::SetQuorum>(creator_command);
  }

  std::vector<shared_model::interface::types::PubkeyType> account_pubkeys;

  std::shared_ptr<shared_model::interface::SetQuorum> set_quorum;
  std::unique_ptr<shared_model::interface::Command> creator_command;
  std::shared_ptr<shared_model::interface::SetQuorum> creator_set_quorum;
};

/**
 * @given SetQuorum and command creator is admin
 * @when command executes
 * @then execute successes
 */
TEST_F(SetQuorumTest, ValidWhenCreatorHasPermissions) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  kAdminId, set_quorum->accountId(), Grantable::kSetMyQuorum))
      .WillOnce(Return(true));
  EXPECT_CALL(*wsv_query, getSignatories(set_quorum->accountId()))
      .WillOnce(Return(account_pubkeys));
  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given SetQuorum and creator is the account parameter
 * @when command executes
 * @then execute successes
 */
TEST_F(SetQuorumTest, ValidWhenSameAccount) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getSignatories(creator_set_quorum->accountId()))
      .WillOnce(Return(account_pubkeys));
  ASSERT_TRUE(val(validate(creator_command)));
}
/**
 * @given SetQuorum and creator has not grantable permissions
 * @when command executes
 * @then execute fails
 */
TEST_F(SetQuorumTest, InvalidWhenNoPermissions) {
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  kAdminId, set_quorum->accountId(), Grantable::kSetMyQuorum))
      .WillOnce(Return(false));

  ASSERT_TRUE(err(validate(command)));
}
/**
 * @given SetQuorum and account parameter is invalid
 * @when command executes
 * @then execute fails
 */
TEST_F(SetQuorumTest, InvalidWhenNoAccount) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
  command =
      buildCommand(TestTransactionBuilder().setAccountQuorum(kNoAcountId, 2));
  set_quorum = getConcreteCommand<shared_model::interface::SetQuorum>(command);

  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  kAdminId, set_quorum->accountId(), Grantable::kSetMyQuorum))
      .WillOnce(Return(false));

  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given SetQuorum
 * @when command tries to set quorum for non-existing account
 * @then execute fails
 */
TEST_F(SetQuorumTest, InvalidWhenNoAccountButPassedPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getSignatories(creator_set_quorum->accountId()))
      .WillOnce(Return(account_pubkeys));

  ASSERT_TRUE(val(validate(creator_command)));
}

/**
 * @given SetQuorum
 * @when command tries to set quorum for account which does not have any
 * signatories
 * @then execute fails
 */
TEST_F(SetQuorumTest, InvalidWhenNoSignatories) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getSignatories(creator_set_quorum->accountId()))
      .WillOnce(Return(boost::none));

  ASSERT_TRUE(err(validate(creator_command)));
}

/**
 * @given SetQuorum
 * @when command tries to set quorum for account which does not have enough
 * signatories
 * @then execute fails
 */
TEST_F(SetQuorumTest, InvalidWhenNotEnoughSignatories) {
  // Creator is the account
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getAccount(creator_set_quorum->accountId())).Times(0);
  std::vector<shared_model::interface::types::PubkeyType> acc_pubkeys = {
      kPubKey1};
  EXPECT_CALL(*wsv_query, getSignatories(creator_set_quorum->accountId()))
      .WillOnce(Return(acc_pubkeys));
  EXPECT_CALL(*wsv_command, updateAccount(_)).Times(0);

  ASSERT_TRUE(err(validate(creator_command)));
}

class TransferAssetTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    src_wallet = clone(shared_model::proto::AccountAssetBuilder()
                           .assetId(kAssetId)
                           .accountId(kAdminId)
                           .balance(balance)
                           .build());

    dst_wallet = clone(shared_model::proto::AccountAssetBuilder()
                           .assetId(kAssetId)
                           .accountId(kAccountId)
                           .balance(balance)
                           .build());

    role_permissions = {Role::kTransfer, Role::kReceive};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(TestTransactionBuilder().transferAsset(
        kAdminId, kAccountId, kAssetId, kDescription, kAmount));
    transfer_asset =
        getConcreteCommand<shared_model::interface::TransferAsset>(command);
  }

  std::shared_ptr<shared_model::interface::AccountAsset> src_wallet, dst_wallet;

  std::shared_ptr<shared_model::interface::TransferAsset> transfer_asset;
};

/**
 * @given TransferAsset
 * @when command is executed
 * @then execute successes
 */
TEST_F(TransferAssetTest, ValidWhenExistingWallet) {
  // When there is a wallet - no new accountAsset created
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->destAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getAccountRoles(transfer_asset->srcAccountId()))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
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
  ASSERT_TRUE(val(validate(command)));
}

class AddPeerTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {Role::kAddPeer};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(
        TestTransactionBuilder().addPeer("iroha_node:10001", kPubKey1));
  }
};

/**
 * @given AddPeer and all parameters are valid
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(AddPeerTest, ValidCase) {
  // Valid case
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));

  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given AddPeer and creator has not role permissions
 * @when command tries to transfer
 * @then execute failed
 */
TEST_F(AddPeerTest, InvalidCaseWhenNoPermissions) {
  // Valid case
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(boost::none));
  ASSERT_TRUE(err(validate(command)));
}


class CreateRoleTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    shared_model::interface::RolePermissionSet perm = {Role::kCreateRole};
    role_permissions = {Role::kCreateRole};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command =
        buildCommand(TestTransactionBuilder().createRole(kAccountId, perm));
    create_role =
        getConcreteCommand<shared_model::interface::CreateRole>(command);
  }
  std::shared_ptr<shared_model::interface::CreateRole> create_role;
};

/**
 * @given CreateRole and all parameters are valid
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(CreateRoleTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillRepeatedly(Return(role_permissions));
  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given CreateRole and creator has not role permissions
 * @when command tries to transfer
 * @then execute is failed
 */
TEST_F(CreateRoleTest, InvalidCaseWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillRepeatedly(Return(boost::none));
  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given CreateRole with wrong permissions
 * @when command tries to transfer
 * @then execute is failed
 */
TEST_F(CreateRoleTest, InvalidCaseWhenRoleSuperset) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
  shared_model::interface::RolePermissionSet master_perms = {Role::kAddPeer,
                                                             Role::kAppendRole};
  command = buildCommand(
      TestTransactionBuilder().createRole(kMasterRole, master_perms));

  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillRepeatedly(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillRepeatedly(Return(role_permissions));
  ASSERT_TRUE(err(validate(command)));
}

class AppendRoleTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {Role::kAppendRole};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(
        TestTransactionBuilder().appendRole(kAccountId, kMasterRole));
    append_role =
        getConcreteCommand<shared_model::interface::AppendRole>(command);
  }
  std::shared_ptr<shared_model::interface::AppendRole> append_role;
};

/**
 * @given CreateRole and all parameters are valid
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(AppendRoleTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .Times(2)
      .WillRepeatedly(Return(admin_roles));

  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .Times(2)
      .WillRepeatedly(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRolePermissions(kMasterRole))
      .WillOnce(Return(role_permissions));

  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given CreateRole and creator has not role permissions
 * @when command tries to transfer
 * @then execute failed
 */
TEST_F(AppendRoleTest, InvalidCaseNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(boost::none));
  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given AppendRole
 * @when command tries to append non-existing role
 * @then execute() fails
 */
TEST_F(AppendRoleTest, InvalidCaseNoAccountRole) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .Times(2)
      .WillOnce(Return(admin_roles))
      .WillOnce((Return(boost::none)));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRolePermissions(kMasterRole))
      .WillOnce(Return(role_permissions));
  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given AppendRole
 * @when command tries to append non-existing role and creator does not have
 any
 * roles
 * @then execute() fails
 */
TEST_F(AppendRoleTest, InvalidCaseNoAccountRoleAndNoPermission) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .Times(2)
      .WillOnce(Return(admin_roles))
      .WillOnce((Return(boost::none)));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  EXPECT_CALL(*wsv_query, getRolePermissions(kMasterRole))
      .WillOnce(Return(boost::none));
  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given AppendRole
 * @when command tries to append role, but creator account does not have
 * necessary permission
 * @then execute() fails
 */
TEST_F(AppendRoleTest, InvalidCaseRoleHasNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .Times(2)
      .WillOnce(Return(admin_roles))
      .WillOnce((Return(admin_roles)));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .Times(2)
      .WillOnce(Return(role_permissions))
      .WillOnce(Return(boost::none));
  EXPECT_CALL(*wsv_query, getRolePermissions(kMasterRole))
      .WillOnce(Return(role_permissions));

  ASSERT_TRUE(err(validate(command)));
}

class DetachRoleTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {Role::kDetachRole};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(
        TestTransactionBuilder().detachRole(kAccountId, kMasterRole));
    detach_role =
        getConcreteCommand<shared_model::interface::DetachRole>(command);
  }
  std::shared_ptr<shared_model::interface::DetachRole> detach_role;
};

/**
 * @given DetachRole and all parameters are valid
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(DetachRoleTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given DetachRole and creator has not role permissions
 * @when command tries to transfer
 * @then execute failed
 */
TEST_F(DetachRoleTest, InvalidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(boost::none));
  ASSERT_TRUE(err(validate(command)));
}

class GrantPermissionTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    role_permissions = {permissionFor(expected_permission)};

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with
    // CommandBuilder
    command = buildCommand(TestTransactionBuilder().grantPermission(
        kAccountId, expected_permission));
    grant_permission =
        getConcreteCommand<shared_model::interface::GrantPermission>(command);
  }
  std::shared_ptr<shared_model::interface::GrantPermission> grant_permission;
  const Grantable expected_permission = Grantable::kAddMySignatory;
};

/**
 * @given GrantPermission and all parameters are valid
 * @when command tries to transfer
 * @then execute succeses
 */
TEST_F(GrantPermissionTest, ValidCase) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));
  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given GrantPermission and creator has not role permissions
 * @when command tries to transfer
 * @then execute failed
 */
TEST_F(GrantPermissionTest, InvalidCaseWhenNoPermissions) {
  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(boost::none));
  ASSERT_TRUE(err(validate(command)));
}

class RevokePermissionTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    expected_permission = Grantable::kAddMySignatory;

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(TestTransactionBuilder().revokePermission(
        kAccountId, expected_permission));
    revoke_permission =
        getConcreteCommand<shared_model::interface::RevokePermission>(command);
  }

  std::shared_ptr<shared_model::interface::RevokePermission> revoke_permission;
  Grantable expected_permission;
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
          revoke_permission->accountId(), kAdminId, expected_permission))
      .WillOnce(Return(true));
  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given RevokePermission and creator has not grantable permissions
 * @when command tries to transfer
 * @then execute failed
 */
TEST_F(RevokePermissionTest, InvalidCaseNoPermissions) {
  EXPECT_CALL(
      *wsv_query,
      hasAccountGrantablePermission(
          revoke_permission->accountId(), kAdminId, expected_permission))
      .WillOnce(Return(false));
  ASSERT_TRUE(err(validate(command)));
}

class SetAccountDetailTest : public CommandValidateExecuteTest {
 public:
  void SetUp() override {
    CommandValidateExecuteTest::SetUp();

    // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
    command = buildCommand(
        TestTransactionBuilder().setAccountDetail(kAdminId, kKey, kValue));
    set_account_detail =
        getConcreteCommand<shared_model::interface::SetAccountDetail>(command);

    role_permissions = {Role::kSetDetail};
  }

  const std::string kKey = "key";
  const std::string kValue = "val";
  const Grantable kNeededPermission = Grantable::kSetMyAccountDetail;

  std::shared_ptr<shared_model::interface::SetAccountDetail> set_account_detail;
};

/**
 * @given SetAccountDetail and all parameters are valid
 * @when creator is setting details to their account
 * @then successfully execute the command
 */
TEST_F(SetAccountDetailTest, ValidWhenSetOwnAccount) {
  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given SetAccountDetail
 * @when creator is setting details to their account
 * @then execute fails
 */
TEST_F(SetAccountDetailTest, InvalidWhenOtherCreator) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
  command = buildCommand(
      TestTransactionBuilder().setAccountDetail(kAccountId, kKey, kValue));
  set_account_detail =
      getConcreteCommand<shared_model::interface::SetAccountDetail>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(std::vector<std::string>{}));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  kAdminId, set_account_detail->accountId(), kNeededPermission))
      .WillOnce(Return(false));
  ASSERT_TRUE(err(validate(command)));
}

/**
 * @given SetAccountDetail
 * @when creator is setting details to their account
 * @then successfully execute the command
 */
TEST_F(SetAccountDetailTest, ValidWhenHasRolePermission) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
  command = buildCommand(
      TestTransactionBuilder().setAccountDetail(kAccountId, kKey, kValue));
  set_account_detail =
      getConcreteCommand<shared_model::interface::SetAccountDetail>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(admin_roles));
  EXPECT_CALL(*wsv_query, getRolePermissions(kAdminRole))
      .WillOnce(Return(role_permissions));

  ASSERT_TRUE(val(validate(command)));
}

/**
 * @given SetAccountDetail
 * @when creator is setting details to their account
 * @then successfully execute the command
 */
TEST_F(SetAccountDetailTest, ValidWhenHasGrantblePermission) {
  // TODO 2018-04-20 Alexey Chernyshov - IR-1276 - rework with CommandBuilder
  command = buildCommand(
      TestTransactionBuilder().setAccountDetail(kAccountId, kKey, kValue));
  set_account_detail =
      getConcreteCommand<shared_model::interface::SetAccountDetail>(command);

  EXPECT_CALL(*wsv_query, getAccountRoles(kAdminId))
      .WillOnce(Return(std::vector<std::string>{}));
  EXPECT_CALL(*wsv_query,
              hasAccountGrantablePermission(
                  kAdminId, set_account_detail->accountId(), kNeededPermission))
      .WillOnce(Return(true));
  ASSERT_TRUE(val(validate(command)));
}

