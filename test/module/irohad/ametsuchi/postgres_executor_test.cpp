/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_command_executor.hpp"
#include "ametsuchi/impl/postgres_query_executor.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "backend/protobuf/proto_permission_to_string.hpp"
#include "framework/result_fixture.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_account_builder.hpp"
#include "module/shared_model/builders/protobuf/test_asset_builder.hpp"
#include "module/shared_model/builders/protobuf/test_domain_builder.hpp"
#include "module/shared_model/builders/protobuf/test_peer_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

namespace iroha {
  namespace ametsuchi {

    using ::testing::HasSubstr;

    using namespace framework::expected;

    class CommandExecutorTest : public AmetsuchiTest {
      // TODO [IR-1831] Akvinikym 31.10.18: rework the CommandExecutorTest
     public:
      CommandExecutorTest() {
        domain = clone(
            TestDomainBuilder().domainId("domain").defaultRole(role).build());

        account = clone(TestAccountBuilder()
                            .domainId(domain->domainId())
                            .accountId("id@" + domain->domainId())
                            .quorum(1)
                            .jsonData(R"({"id@domain": {"key": "value"}})")
                            .build());
        role_permissions.set(
            shared_model::interface::permissions::Role::kAddMySignatory);
        grantable_permission =
            shared_model::interface::permissions::Grantable::kAddMySignatory;
        pubkey = std::make_unique<shared_model::interface::types::PubkeyType>(
            std::string('1', 32));
      }

      void SetUp() override {
        AmetsuchiTest::SetUp();
        sql = std::make_unique<soci::session>(soci::postgresql, pgopt_);

        auto factory =
            std::make_shared<shared_model::proto::ProtoCommonObjectsFactory<
                shared_model::validation::FieldValidator>>();
        query = std::make_unique<PostgresWsvQuery>(*sql, factory);
        PostgresCommandExecutor::prepareStatements(*sql);
        executor =
            std::make_unique<PostgresCommandExecutor>(*sql, perm_converter);

        *sql << init_;
      }

      void TearDown() override {
        sql->close();
        AmetsuchiTest::TearDown();
      }

      CommandResult execute(
          const std::unique_ptr<shared_model::interface::Command> &command,
          bool do_validation = false,
          const shared_model::interface::types::AccountIdType &creator =
              "id@domain") {
        executor->doValidation(not do_validation);
        executor->setCreatorAccountId(creator);
        return boost::apply_visitor(*executor, command->get());
      }

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

      void addAllPerms(
          const shared_model::interface::types::AccountIdType account_id =
              "id@domain",
          const shared_model::interface::types::RoleIdType role_id = "all") {
        shared_model::interface::RolePermissionSet permissions;
        permissions.set();
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role_id, permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().appendRole(
                            account_id, role_id)),
                        true)));
      }

      /**
       * Check that command result contains specific error code and error
       * message
       * @param cmd_result to be checked
       * @param expected_code to be in the result
       * @param expected_substrings - collection of strings, which are expected
       * to be in command error
       */
#define CHECK_ERROR_CODE_AND_MESSAGE(                \
    cmd_result, expected_code, expected_substrings)  \
  auto error = err(cmd_result);                      \
  ASSERT_TRUE(error);                                \
  EXPECT_EQ(error->error.error_code, expected_code); \
  auto str_error = error->error.error_extra;         \
  for (auto substring : expected_substrings) {       \
    EXPECT_THAT(str_error, HasSubstr(substring));    \
  }

      const std::string role = "role";
      const std::string another_role = "role2";
      shared_model::interface::RolePermissionSet role_permissions;
      shared_model::interface::permissions::Grantable grantable_permission;
      std::unique_ptr<shared_model::interface::Account> account;
      std::unique_ptr<shared_model::interface::Domain> domain;
      std::unique_ptr<shared_model::interface::types::PubkeyType> pubkey;
      std::unique_ptr<soci::session> sql;

      std::unique_ptr<shared_model::interface::Command> command;

      std::unique_ptr<WsvQuery> query;
      std::unique_ptr<CommandExecutor> executor;

      std::shared_ptr<shared_model::interface::PermissionToString>
          perm_converter =
              std::make_shared<shared_model::proto::ProtoPermissionToString>();

      const std::string uint256_halfmax =
          "57896044618658097711785492504343953926634992332820282019728792003956"
          "5648"
          "19966.0";  // 2**255
      const std::string asset_amount_one_zero = "1.0";
    };

    class AddAccountAssetTest : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();

        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
      }
      /**
       * Add default asset and check that it is done
       */
      void addAsset() {
        auto asset = clone(TestAccountAssetBuilder()
                               .domainId(domain->domainId())
                               .assetId(asset_id)
                               .precision(1)
                               .build());

        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAsset(
                            "coin", domain->domainId(), 1)),
                        true)));
      }

      shared_model::interface::types::AssetIdType asset_id =
          "coin#" + domain->domainId();
    };

    /**
     * @given command
     * @when trying to add account asset
     * @then account asset is successfully added
     */
    TEST_F(AddAccountAssetTest, Valid) {
      addAsset();
      addAllPerms();
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())))));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ(asset_amount_one_zero,
                account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())))));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("2.0", account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given command
     * @when trying to add account asset without permission
     * @then account asset not added
     */
    TEST_F(AddAccountAssetTest, NoPerms) {
      addAsset();
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())),
          true)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ(asset_amount_one_zero,
                account_asset.get()->balance().toStringRepr());

      auto cmd_result = execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())));

      std::vector<std::string> query_args{
          account->accountId(), asset_amount_one_zero, asset_id, "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);
    }

    /**
     * @given command
     * @when trying to add account asset with non-existing asset
     * @then account asset fails to be added
     */
    TEST_F(AddAccountAssetTest, InvalidAsset) {
      auto cmd_result = execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())),
          true);

      std::vector<std::string> query_args{
          account->accountId(), asset_amount_one_zero, asset_id, "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    /**
     * @given command
     * @when trying to add account asset that overflows
     * @then account asset fails to added
     */
    TEST_F(AddAccountAssetTest, Uint256Overflow) {
      addAsset();
      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, uint256_halfmax)
                                   .creatorAccountId(account->accountId())),
                  true)));
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, uint256_halfmax)
                                   .creatorAccountId(account->accountId())),
                  true);

      std::vector<std::string> query_args{
          account->accountId(), uint256_halfmax, asset_id, "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 4, query_args);
    }

    class AddPeer : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        peer = clone(TestPeerBuilder().build());
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
      }
      std::unique_ptr<shared_model::interface::Peer> peer;
    };

    /**
     * @given command
     * @when trying to add peer
     * @then peer is successfully added
     */
    TEST_F(AddPeer, Valid) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().addPeer(peer->address(), peer->pubkey())))));
    }

    /**
     * @given command
     * @when trying to add peer without perms
     * @then peer is not added
     */
    TEST_F(AddPeer, NoPerms) {
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().addPeer(peer->address(), peer->pubkey())));

      std::vector<std::string> query_args{peer->toString()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);
    }

    class AddSignatory : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id",
                            domain->domainId(),
                            shared_model::interface::types::PubkeyType(
                                std::string('5', 32)))),
                        true)));
      }
    };

    /**
     * @given command
     * @when trying to add signatory with role permission
     * @then signatory is successfully added
     */
    TEST_F(AddSignatory, Valid) {
      addAllPerms();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
              account->accountId(), *pubkey)))));
      auto signatories = query->getSignatories(account->accountId());
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  != signatories->end());
    }

    /**
     * @given command
     * @when trying to add signatory with grantable permission
     * @then signatory is successfully added
     */
    TEST_F(AddSignatory, ValidGrantablePerms) {
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createAccount(
                          "id2",
                          domain->domainId(),
                          shared_model::interface::types::PubkeyType(
                              std::string('2', 32)))),
                      true)));
      auto perm =
          shared_model::interface::permissions::Grantable::kAddMySignatory;
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().grantPermission(
                          account->accountId(), perm)),
                      true,
                      "id2@domain")));
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().addSignatory("id2@domain", *pubkey)))));
      auto signatories = query->getSignatories("id2@domain");
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  != signatories->end());
    }

    /**
     * @given command
     * @when trying to add signatory without permissions
     * @then signatory is not added
     */
    TEST_F(AddSignatory, NoPerms) {
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().addSignatory(
              account->accountId(), *pubkey)));

      std::vector<std::string> query_args{account->accountId(), pubkey->hex()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);

      auto signatories = query->getSignatories(account->accountId());
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  == signatories->end());
    }

    /**
     * @given command
     * @when successfully adding signatory to the account @and trying to add the
     * same signatory again
     * @then signatory is not added
     */
    TEST_F(AddSignatory, ExistingPubKey) {
      addAllPerms();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
              account->accountId(), *pubkey)))));

      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().addSignatory(
              account->accountId(), *pubkey)));

      std::vector<std::string> query_args{account->accountId(), pubkey->hex()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 4, query_args);
    }

    class AppendRole : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
      }
      shared_model::interface::RolePermissionSet role_permissions2;
    };

    /**
     * @given command
     * @when trying to append role
     * @then role is appended
     */
    TEST_F(AppendRole, Valid) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
                                  another_role, role_permissions)),
                              true)));
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().appendRole(
          account->accountId(), another_role)))));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), another_role)
                  != roles->end());
    }

    /**
     * @given command
     * @when trying append role, which does not have any permissions
     * @then role is appended
     */
    TEST_F(AppendRole, ValidEmptyPerms) {
      addAllPerms();
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder().createRole(another_role, {})),
          true)));
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().appendRole(
          account->accountId(), another_role)))));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), another_role)
                  != roles->end());
    }

    /**
     * @given command
     * @when trying to append role with perms that creator does not have but in
     * genesis block
     * @then role is appended
     */
    TEST_F(AppendRole, AccountDoesNotHavePermsGenesis) {
      role_permissions2.set(
          shared_model::interface::permissions::Role::kRemoveMySignatory);
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
                                  another_role, role_permissions2)),
                              true)));
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().appendRole(
                                  account->accountId(), another_role)),
                              true)));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), another_role)
                  != roles->end());
    }

    /**
     * @given command
     * @when trying to append role having no permission to do so
     * @then role is not appended
     */
    TEST_F(AppendRole, NoPerms) {
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
                                  another_role, role_permissions)),
                              true)));
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().appendRole(
              account->accountId(), another_role)));

      std::vector<std::string> query_args{account->accountId(), another_role};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);

      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), another_role)
                  == roles->end());
    }

    /**
     * @given command
     * @when trying to append role with perms that creator does not have
     * @then role is not appended
     */
    TEST_F(AppendRole, NoRolePermsInAccount) {
      role_permissions2.set(
          shared_model::interface::permissions::Role::kRemoveMySignatory);
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
                                  another_role, role_permissions2)),
                              true)));
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().appendRole(
              account->accountId(), another_role)));

      std::vector<std::string> query_args{account->accountId(), another_role};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);
    }

    /**
     * @given command
     * @when trying to append role to non-existing account
     * @then role is not appended
     */
    TEST_F(AppendRole, NoAccount) {
      addAllPerms();
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder().createRole(another_role, {})),
          true)));
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().appendRole("doge@noaccount", another_role)));

      std::vector<std::string> query_args{"doge@noaccount", another_role};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    /**
     * @given command
     * @when trying to append non-existing role
     * @then role is not appended
     */
    TEST_F(AppendRole, NoRole) {
      addAllPerms();
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().appendRole(
              account->accountId(), another_role)));

      std::vector<std::string> query_args{account->accountId(), another_role};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 4, query_args);
    }

    class CreateAccount : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        account = clone(TestAccountBuilder()
                            .domainId(domain->domainId())
                            .accountId("id@" + domain->domainId())
                            .quorum(1)
                            .jsonData("{}")
                            .build());
        account2 = clone(TestAccountBuilder()
                             .domainId(domain->domainId())
                             .accountId("id2@" + domain->domainId())
                             .quorum(1)
                             .jsonData("{}")
                             .build());
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
      }

      std::unique_ptr<shared_model::interface::Account> account2;
    };

    /**
     * @given command
     * @when trying to create account
     * @then account is created
     */
    TEST_F(CreateAccount, Valid) {
      addAllPerms();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createAccount(
              "id2", domain->domainId(), *pubkey)))));
      auto acc = query->getAccount(account2->accountId());
      ASSERT_TRUE(acc);
      ASSERT_EQ(*account2.get(), *acc.get());
    }

    /**
     * @given command
     * @when trying to create account without permission to do so
     * @then account is not created
     */
    TEST_F(CreateAccount, NoPerms) {
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().createAccount(
              account2->accountId(), domain->domainId(), *pubkey)));
      auto acc = query->getAccount(account2->accountId());
      ASSERT_FALSE(acc);

      std::vector<std::string> query_args{
          account2->accountId(), domain->domainId(), pubkey->hex()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);
    }

    /**
     * @given command @and no target domain in ledger
     * @when trying to create account
     * @then account is not created
     */
    TEST_F(CreateAccount, NoDomain) {
      addAllPerms();
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().createAccount("doge", "domain6", *pubkey)));

      std::vector<std::string> query_args{"doge", "domain6", pubkey->hex()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    /**
     * @given command
     * @when trying to create with an occupied name
     * @then account is not created
     */
    TEST_F(CreateAccount, NameExists) {
      addAllPerms();
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().createAccount(
              "id", domain->domainId(), *pubkey)));

      std::vector<std::string> query_args{
          "id", domain->domainId(), pubkey->hex()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 4, query_args);
    }

    class CreateAsset : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
      }
      shared_model::interface::types::AssetIdType asset_name = "coin";
      shared_model::interface::types::AssetIdType asset_id =
          "coin#" + domain->domainId();
    };

    /**
     * @given command
     * @when trying to create asset
     * @then asset is created
     */
    TEST_F(CreateAsset, Valid) {
      role_permissions.set(
          shared_model::interface::permissions::Role::kCreateAsset);
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
                                  role, role_permissions)),
                              true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createDomain(
                          domain->domainId(), role)),
                      true)));
      auto asset = clone(TestAccountAssetBuilder()
                             .domainId(domain->domainId())
                             .assetId(asset_id)
                             .precision(1)
                             .build());
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createAccount(
                          "id", domain->domainId(), *pubkey)),
                      true)));
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createAsset(
          "coin", domain->domainId(), 1)))));
      auto ass = query->getAsset(asset->assetId());
      ASSERT_TRUE(ass);
      ASSERT_EQ(*asset.get(), *ass.get());
    }

    /**
     * @given command
     * @when trying to create asset without permission
     * @then asset is not created
     */
    TEST_F(CreateAsset, NoPerms) {
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
                                  role, role_permissions)),
                              true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createDomain(
                          domain->domainId(), role)),
                      true)));
      auto asset = clone(TestAccountAssetBuilder()
                             .domainId(domain->domainId())
                             .assetId(asset_id)
                             .precision(1)
                             .build());
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createAccount(
                          "id", domain->domainId(), *pubkey)),
                      true)));
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().createAsset("coin", domain->domainId(), 1)));
      auto ass = query->getAsset(asset->assetId());
      ASSERT_FALSE(ass);

      std::vector<std::string> query_args{domain->domainId(), "coin", "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);
    }

    /**
     * @given command and no target domain in ledger
     * @when trying to create asset
     * @then asset is not created
     */
    TEST_F(CreateAsset, NoDomain) {
      role_permissions.set(
          shared_model::interface::permissions::Role::kCreateAsset);
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
                                  role, role_permissions)),
                              true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createDomain(
                          domain->domainId(), role)),
                      true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createAccount(
                          "id", domain->domainId(), *pubkey)),
                      true)));
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().createAsset(asset_name, "no_domain", 1)));

      std::vector<std::string> query_args{asset_name, "no_domain", "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    /**
     * @given command
     * @when trying to create asset with an occupied name
     * @then asset is not created
     */
    TEST_F(CreateAsset, NameNotUnique) {
      role_permissions.set(
          shared_model::interface::permissions::Role::kCreateAsset);
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
                                  role, role_permissions)),
                              true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createDomain(
                          domain->domainId(), role)),
                      true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createAccount(
                          "id", domain->domainId(), *pubkey)),
                      true)));
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createAsset(
          "coin", domain->domainId(), 1)))));
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().createAsset("coin", domain->domainId(), 1)));

      std::vector<std::string> query_args{"coin", domain->domainId(), "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 4, query_args);
    }

    class CreateDomain : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        domain2 = clone(
            TestDomainBuilder().domainId("domain2").defaultRole(role).build());
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
      }

      std::unique_ptr<shared_model::interface::Domain> domain2;
    };

    /**
     * @given command
     * @when trying to create domain
     * @then domain is created
     */
    TEST_F(CreateDomain, Valid) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().createDomain(domain2->domainId(), role)))));
      auto dom = query->getDomain(domain2->domainId());
      ASSERT_TRUE(dom);
      ASSERT_EQ(*dom.get(), *domain2.get());
    }

    /**
     * @given command when there is no perms
     * @when trying to create domain
     * @then domain is not created
     */
    TEST_F(CreateDomain, NoPerms) {
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().createDomain(domain2->domainId(), role)));
      auto dom = query->getDomain(domain2->domainId());
      ASSERT_FALSE(dom);

      std::vector<std::string> query_args{domain2->domainId(), role};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);
    }

    /**
     * @given command
     * @when trying to create domain with an occupied name
     * @then domain is not created
     */
    TEST_F(CreateDomain, NameNotUnique) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().createDomain(domain2->domainId(), role)))));
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().createDomain(domain2->domainId(), role)));

      std::vector<std::string> query_args{domain2->domainId(), role};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    /**
     * @given command when there is no default role
     * @when trying to create domain
     * @then domain is not created
     */
    TEST_F(CreateDomain, NoDefaultRole) {
      addAllPerms();
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().createDomain(
              domain2->domainId(), another_role)));

      std::vector<std::string> query_args{domain2->domainId(), another_role};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 4, query_args);
    }

    class CreateRole : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
      }
      shared_model::interface::RolePermissionSet role_permissions2;
    };

    /**
     * @given command
     * @when trying to create role
     * @then role is created
     */
    TEST_F(CreateRole, Valid) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
          another_role, role_permissions)))));
      auto rl = query->getRolePermissions(role);
      ASSERT_TRUE(rl);
      ASSERT_EQ(rl.get(), role_permissions);
    }

    /**
     * @given command
     * @when trying to create role when creator doesn't have all permissions
     * @then role is not created
     */
    TEST_F(CreateRole, NoPerms) {
      role_permissions2.set(
          shared_model::interface::permissions::Role::kRemoveMySignatory);
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().createRole(
              another_role, role_permissions2)));
      auto rl = query->getRolePermissions(another_role);
      ASSERT_TRUE(rl);
      ASSERT_TRUE(rl->none());

      std::vector<std::string> query_args{another_role,
                                          role_permissions2.toBitstring()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);
    }

    /**
     * @given command
     * @when trying to create role with an occupied name
     * @then role is not created
     */
    TEST_F(CreateRole, NameNotUnique) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
          another_role, role_permissions)))));
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().createRole(another_role, role_permissions)));

      std::vector<std::string> query_args{another_role,
                                          role_permissions.toBitstring()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    class DetachRole : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            another_role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().appendRole(
                            account->accountId(), another_role)),
                        true)));
      }
    };

    /**
     * @given command
     * @when trying to detach role
     * @then role is detached
     */
    TEST_F(DetachRole, Valid) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().detachRole(
          account->accountId(), another_role)))));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), another_role)
                  == roles->end());
    }

    /**
     * @given command
     * @when trying to detach role without permission
     * @then role is detached
     */
    TEST_F(DetachRole, NoPerms) {
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().detachRole(
              account->accountId(), another_role)));

      std::vector<std::string> query_args{account->accountId(), another_role};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);

      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), another_role)
                  != roles->end());
    }

    /**
     * @given command
     * @when trying to detach role from non-existing account
     * @then correspondent error code is returned
     */
    TEST_F(DetachRole, NoAccount) {
      addAllPerms();
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().detachRole("doge@noaccount", another_role)));

      std::vector<std::string> query_args{"doge@noaccount", another_role};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    /**
     * @given command
     * @when trying to detach role, which the account does not have
     * @then correspondent error code is returned
     */
    TEST_F(DetachRole, NoSuchRoleInAccount) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().detachRole(
          account->accountId(), another_role)))));
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().detachRole(
              account->accountId(), another_role)));

      std::vector<std::string> query_args{account->accountId(), another_role};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 4, query_args);
    }

    /**
     * @given command
     * @when trying to detach a non-existing role
     * @then correspondent error code is returned
     */
    TEST_F(DetachRole, NoRole) {
      addAllPerms();
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().detachRole(
              account->accountId(), "not_existing_role")));

      std::vector<std::string> query_args{account->accountId(),
                                          "not_existing_role"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 5, query_args);
    }

    class GrantPermission : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            another_role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
      }
    };

    /**
     * @given command
     * @when trying to grant permission
     * @then permission is granted
     */
    TEST_F(GrantPermission, Valid) {
      addAllPerms();
      auto perm = shared_model::interface::permissions::Grantable::kSetMyQuorum;
      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .grantPermission(account->accountId(), perm)
                                   .creatorAccountId(account->accountId())))));
      auto has_perm = query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), perm);
      ASSERT_TRUE(has_perm);
    }

    /**
     * @given command
     * @when trying to grant permission without permission
     * @then permission is not granted
     */
    TEST_F(GrantPermission, NoPerms) {
      auto perm = shared_model::interface::permissions::Grantable::kSetMyQuorum;
      auto cmd_result =

          execute(buildCommand(TestTransactionBuilder()
                                   .grantPermission(account->accountId(), perm)
                                   .creatorAccountId(account->accountId())));
      auto has_perm = query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), perm);
      ASSERT_FALSE(has_perm);

      std::vector<std::string> query_args{account->accountId(),
                                          perm_converter->toString(perm)};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);
    }

    /**
     * @given command
     * @when trying to grant permission to non-existent account
     * @then corresponding error code is returned
     */
    TEST_F(GrantPermission, NoAccount) {
      addAllPerms();
      auto perm = shared_model::interface::permissions::Grantable::kSetMyQuorum;
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder()
                                   .grantPermission("doge@noaccount", perm)
                                   .creatorAccountId(account->accountId())));

      std::vector<std::string> query_args{"doge@noaccount",
                                          perm_converter->toString(perm)};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    class RemoveSignatory : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        pubkey = std::make_unique<shared_model::interface::types::PubkeyType>(
            std::string('1', 32));
        another_pubkey =
            std::make_unique<shared_model::interface::types::PubkeyType>(
                std::string('7', 32));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
      }
      std::unique_ptr<shared_model::interface::types::PubkeyType> pubkey;
      std::unique_ptr<shared_model::interface::types::PubkeyType>
          another_pubkey;
    };

    /**
     * @given command
     * @when trying to remove signatory
     * @then signatory is successfully removed
     */
    TEST_F(RemoveSignatory, Valid) {
      addAllPerms();
      shared_model::interface::types::PubkeyType pk(std::string('5', 32));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          account->accountId(), pk)),
                      true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().removeSignatory(
              account->accountId(), *pubkey)))));
      auto signatories = query->getSignatories(account->accountId());
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  == signatories->end());
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), pk)
                  != signatories->end());
    }

    /**
     * @given command
     * @when trying to remove signatory
     * @then signatory is successfully removed
     */
    TEST_F(RemoveSignatory, ValidGrantablePerm) {
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createAccount(
                          "id2", domain->domainId(), *pubkey)),
                      true)));
      auto perm =
          shared_model::interface::permissions::Grantable::kRemoveMySignatory;
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().grantPermission(
                          account->accountId(), perm)),
                      true,
                      "id2@domain")));
      shared_model::interface::types::PubkeyType pk(std::string('5', 32));
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder().addSignatory("id2@domain", pk)),
          true)));
      auto signatories = query->getSignatories("id2@domain");
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), pk)
                  != signatories->end());
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().removeSignatory("id2@domain", pk)))));
      signatories = query->getSignatories("id2@domain");
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  != signatories->end());
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), pk)
                  == signatories->end());
    }

    /**
     * @given command
     * @when trying to remove signatory without permission
     * @then signatory is not removed
     */
    TEST_F(RemoveSignatory, NoPerms) {
      shared_model::interface::types::PubkeyType pk(std::string('5', 32));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          account->accountId(), pk)),
                      true)));
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().removeSignatory(
              account->accountId(), *pubkey)));

      std::vector<std::string> query_args{account->accountId(), pubkey->hex()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);

      auto signatories = query->getSignatories(account->accountId());
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  != signatories->end());
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), pk)
                  != signatories->end());
    }

    /**
     * @given command
     * @when trying to remove signatory from a non existing account
     * @then corresponding error code is returned
     */
    TEST_F(RemoveSignatory, NoAccount) {
      addAllPerms();
      shared_model::interface::types::PubkeyType pk(std::string('5', 32));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          account->accountId(), pk)),
                      true)));

      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().removeSignatory("hello", *pubkey)));

      std::vector<std::string> query_args{"hello", pubkey->hex()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    /**
     * @given command
     * @when trying to remove signatory, which is not attached to this account
     * @then corresponding error code is returned
     */
    TEST_F(RemoveSignatory, NoSuchSignatory) {
      addAllPerms();
      shared_model::interface::types::PubkeyType pk(std::string('5', 32));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          account->accountId(), pk)),
                      true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          account->accountId(), *another_pubkey)),
                      true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().removeSignatory(
                          account->accountId(), *another_pubkey)),
                      true)));

      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().removeSignatory(
              account->accountId(), *another_pubkey)));

      std::vector<std::string> query_args{account->accountId(),
                                          another_pubkey->hex()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 4, query_args);
    }

    /**
     * @given command
     * @when trying to remove signatory from an account, after which it will
     * have signatories less, than its quorum
     * @then signatory is not removed
     */
    TEST_F(RemoveSignatory, SignatoriesLessThanQuorum) {
      addAllPerms();
      shared_model::interface::types::PubkeyType pk(std::string('5', 32));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          account->accountId(), pk)),
                      true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().removeSignatory(
              account->accountId(), *pubkey)))));
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().removeSignatory(account->accountId(), pk)));

      std::vector<std::string> query_args{account->accountId(), pk.hex()};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 5, query_args);
    }

    class RevokePermission : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
        ASSERT_TRUE(val(
            execute(buildCommand(TestTransactionBuilder()
                                     .grantPermission(account->accountId(),
                                                      grantable_permission)
                                     .creatorAccountId(account->accountId())),
                    true)));
      }
    };

    /**
     * @given command
     * @when trying to revoke permission
     * @then permission is revoked
     */
    TEST_F(RevokePermission, Valid) {
      auto perm =
          shared_model::interface::permissions::Grantable::kRemoveMySignatory;
      ASSERT_TRUE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), grantable_permission));

      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .grantPermission(account->accountId(), perm)
                                   .creatorAccountId(account->accountId())),
                  true)));
      ASSERT_TRUE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), grantable_permission));
      ASSERT_TRUE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), perm));

      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder()
              .revokePermission(account->accountId(), grantable_permission)
              .creatorAccountId(account->accountId())))));
      ASSERT_TRUE(err(execute(buildCommand(
          TestTransactionBuilder()
              .revokePermission(account->accountId(), grantable_permission)
              .creatorAccountId(account->accountId())))));
      ASSERT_FALSE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), grantable_permission));
      ASSERT_TRUE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), perm));
    }

    /**
     * @given command
     * @when trying to revoke permission without permission
     * @then permission is revoked
     */
    TEST_F(RevokePermission, NoPerms) {
      auto perm =
          shared_model::interface::permissions::Grantable::kRemoveMySignatory;
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder()
                                   .revokePermission(account->accountId(), perm)
                                   .creatorAccountId(account->accountId())));

      std::vector<std::string> query_args{account->accountId(),
                                          perm_converter->toString(perm)};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);
    }

    class SetAccountDetail : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
        account2 = clone(TestAccountBuilder()
                             .domainId(domain->domainId())
                             .accountId("id2@" + domain->domainId())
                             .quorum(1)
                             .jsonData("")
                             .build());
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id2",
                            domain->domainId(),
                            shared_model::interface::types::PubkeyType(
                                std::string('2', 32)))),
                        true)));
      }
      std::unique_ptr<shared_model::interface::Account> account2;
    };

    /**
     * @given command
     * @when trying to set kv
     * @then kv is set
     */
    TEST_F(SetAccountDetail, Valid) {
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().setAccountDetail(
              account->accountId(), "key", "value")))));
      auto kv = query->getAccountDetail(account->accountId());
      ASSERT_TRUE(kv);
      ASSERT_EQ(kv.get(), "{\"id@domain\": {\"key\": \"value\"}}");
    }

    /**
     * @given command
     * @when trying to set kv when has grantable permission
     * @then kv is set
     */
    TEST_F(SetAccountDetail, ValidGrantablePerm) {
      auto perm =
          shared_model::interface::permissions::Grantable::kSetMyAccountDetail;
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().grantPermission(
                          account->accountId(), perm)),
                      true,
                      "id2@domain")));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().setAccountDetail(
                          account2->accountId(), "key", "value")),
                      false,
                      account->accountId())));
      auto kv = query->getAccountDetail(account2->accountId());
      ASSERT_TRUE(kv);
      ASSERT_EQ(kv.get(), "{\"id@domain\": {\"key\": \"value\"}}");
    }

    /**
     * @given command
     * @when trying to set kv when has role permission
     * @then kv is set
     */
    TEST_F(SetAccountDetail, ValidRolePerm) {
      addAllPerms();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().setAccountDetail(
                          account2->accountId(), "key", "value")),
                      false,
                      account->accountId())));
      auto kv = query->getAccountDetail(account2->accountId());
      ASSERT_TRUE(kv);
      ASSERT_EQ(kv.get(), "{\"id@domain\": {\"key\": \"value\"}}");
    }

    /**
     * @given command
     * @when trying to set kv while having no permissions
     * @then corresponding error code is returned
     */
    TEST_F(SetAccountDetail, NoPerms) {
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().setAccountDetail(
                      account2->accountId(), "key", "value")),
                  false,
                  account->accountId());

      std::vector<std::string> query_args{
          account2->accountId(), "key", "value"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);

      auto kv = query->getAccountDetail(account2->accountId());
      ASSERT_TRUE(kv);
      ASSERT_EQ(kv.get(), "{}");
    }

    /**
     * @given command
     * @when trying to set kv to non-existing account
     * @then corresponding error code is returned
     */
    TEST_F(SetAccountDetail, NoAccount) {
      addAllPerms();
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().setAccountDetail(
                      "doge@noaccount", "key", "value")),
                  false,
                  account->accountId());

      std::vector<std::string> query_args{"doge@noaccount", "key", "value"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    class SetQuorum : public CommandExecutorTest {
     public:
      SetQuorum()
          : additional_key_(shared_model::crypto::DefaultCryptoAlgorithmType::
                                generateKeypair()) {}

      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                            account->accountId(), additional_key_.publicKey())),
                        true)));
      }

      shared_model::crypto::Keypair additional_key_;
    };

    /**
     * @given command
     * @when trying to set quorum
     * @then quorum is set
     */
    TEST_F(SetQuorum, Valid) {
      addAllPerms();

      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().setAccountQuorum(
              account->accountId(), 2)))));
    }

    /**
     * @given command
     * @when trying to set quorum
     * @then quorum is set
     */
    TEST_F(SetQuorum, ValidGrantablePerms) {
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createAccount(
                          "id2", domain->domainId(), *pubkey)),
                      true)));
      auto perm = shared_model::interface::permissions::Grantable::kSetMyQuorum;
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().grantPermission(
                          account->accountId(), perm)),
                      true,
                      "id2@domain")));

      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          "id2@domain", additional_key_.publicKey())),
                      true,
                      "id2@domain")));

      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().setAccountQuorum("id2@domain", 2)))));
    }

    /**
     * @given command
     * @when trying to set quorum without perms
     * @then quorum is not set
     */
    TEST_F(SetQuorum, NoPerms) {
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().setAccountQuorum(account->accountId(), 3)));

      std::vector<std::string> query_args{account->accountId(), "3"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);
    }

    /**
     * @given command
     * @when trying to set quorum more than amount of signatories
     * @then quorum is not set
     */
    TEST_F(SetQuorum, LessSignatoriesThanNewQuorum) {
      addAllPerms();
      shared_model::interface::types::PubkeyType pk(std::string('5', 32));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          account->accountId(), pk)),
                      true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().setAccountQuorum(
              account->accountId(), 3)))));

      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().setAccountQuorum(account->accountId(), 5)));

      std::vector<std::string> query_args{account->accountId(), "5"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 5, query_args);
    }

    class SubtractAccountAssetTest : public CommandExecutorTest {
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
      }

     public:
      /**
       * Add default asset and check that it is done
       */
      void addAsset() {
        auto asset = clone(TestAccountAssetBuilder()
                               .domainId(domain->domainId())
                               .assetId(asset_id)
                               .precision(1)
                               .build());

        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAsset(
                            "coin", domain->domainId(), 1)),
                        true)));
      }

      shared_model::interface::types::AssetIdType asset_id =
          "coin#" + domain->domainId();
    };

    /**
     * @given command
     * @when trying to subtract account asset
     * @then account asset is successfully subtracted
     */
    TEST_F(SubtractAccountAssetTest, Valid) {
      addAllPerms();
      addAsset();
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())),
          true)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ(asset_amount_one_zero,
                account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())),
          true)));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("2.0", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder()
              .subtractAssetQuantity(asset_id, asset_amount_one_zero)
              .creatorAccountId(account->accountId())))));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ(asset_amount_one_zero,
                account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given command
     * @when trying to subtract account asset without permissions
     * @then corresponding error code is returned
     */
    TEST_F(SubtractAccountAssetTest, NoPerms) {
      addAsset();
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())),
          true)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ(asset_amount_one_zero,
                account_asset.get()->balance().toStringRepr());

      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder()
              .subtractAssetQuantity(asset_id, asset_amount_one_zero)
              .creatorAccountId(account->accountId())));

      std::vector<std::string> query_args{
          account->accountId(), asset_id, asset_amount_one_zero, "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);

      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ(asset_amount_one_zero,
                account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given command
     * @when trying to subtract account asset with non-existing asset
     * @then account asset fails to be subtracted
     */
    TEST_F(SubtractAccountAssetTest, NoAsset) {
      addAllPerms();
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder()
              .subtractAssetQuantity(asset_id, asset_amount_one_zero)
              .creatorAccountId(account->accountId())));

      std::vector<std::string> query_args{
          account->accountId(), asset_id, asset_amount_one_zero, "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    /**
     * @given command
     * @when trying to add account asset with wrong precision
     * @then account asset fails to be added
     */
    TEST_F(SubtractAccountAssetTest, InvalidPrecision) {
      addAllPerms();
      addAsset();
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder()
                                   .subtractAssetQuantity(asset_id, "1.0000")
                                   .creatorAccountId(account->accountId())));

      std::vector<std::string> query_args{
          account->accountId(), asset_id, "1.0000", "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
    }

    /**
     * @given command
     * @when trying to subtract more account asset than account has
     * @then account asset fails to be subtracted
     */
    TEST_F(SubtractAccountAssetTest, NotEnoughAsset) {
      addAllPerms();
      addAsset();
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())),
          true)));
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder()
                                   .subtractAssetQuantity(asset_id, "2.0")
                                   .creatorAccountId(account->accountId())));

      std::vector<std::string> query_args{
          account->accountId(), asset_id, "2.0", "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 4, query_args);
    }

    class TransferAccountAssetTest : public CommandExecutorTest {
      void SetUp() override {
        CommandExecutorTest::SetUp();

        account2 = clone(TestAccountBuilder()
                             .domainId(domain->domainId())
                             .accountId("id2@" + domain->domainId())
                             .quorum(1)
                             .jsonData("{}")
                             .build());

        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createRole(
                            role, role_permissions)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain->domainId(), role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain->domainId(), *pubkey)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id2", domain->domainId(), *pubkey)),
                        true)));
      }

     public:
      /**
       * Add default asset and check that it is done
       */
      void addAsset() {
        auto asset = clone(TestAccountAssetBuilder()
                               .domainId(domain->domainId())
                               .assetId(asset_id)
                               .precision(1)
                               .build());

        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAsset(
                            "coin", domain->domainId(), 1)),
                        true)));
      }

      shared_model::interface::types::AssetIdType asset_id =
          "coin#" + domain->domainId();
      std::unique_ptr<shared_model::interface::Account> account2;
    };

    /**
     * @given command
     * @when trying to add transfer asset
     * @then account asset is successfully transferred
     */
    TEST_F(TransferAccountAssetTest, Valid) {
      addAllPerms();
      addAllPerms(account2->accountId(), "all2");
      addAsset();
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())),
          true)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ(asset_amount_one_zero,
                account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())),
          true)));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("2.0", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().transferAsset(account->accountId(),
                                                 account2->accountId(),
                                                 asset_id,
                                                 "desc",
                                                 asset_amount_one_zero)))));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ(asset_amount_one_zero,
                account_asset.get()->balance().toStringRepr());
      account_asset = query->getAccountAsset(account2->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ(asset_amount_one_zero,
                account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given command
     * @when trying to add transfer asset
     * @then account asset is successfully transferred
     */
    TEST_F(TransferAccountAssetTest, ValidGrantablePerms) {
      addAllPerms(account2->accountId(), "all2");
      addAsset();
      auto perm =
          shared_model::interface::permissions::Grantable::kTransferMyAssets;
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().grantPermission(
                          account2->accountId(), perm)),
                      true,
                      account->accountId())));

      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "2.0")
                                       .creatorAccountId(account->accountId())),
                      true)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("2.0", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().transferAsset(
                          account->accountId(),
                          account2->accountId(),
                          asset_id,
                          "desc",
                          asset_amount_one_zero)),
                      false,
                      account2->accountId())));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ(asset_amount_one_zero,
                account_asset.get()->balance().toStringRepr());
      account_asset = query->getAccountAsset(account2->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ(asset_amount_one_zero,
                account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given command
     * @when trying to transfer account asset with no permissions
     * @then account asset fails to be transferred
     */
    TEST_F(TransferAccountAssetTest, NoPerms) {
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().transferAsset(account->accountId(),
                                                 account2->accountId(),
                                                 asset_id,
                                                 "desc",
                                                 asset_amount_one_zero)));

      std::vector<std::string> query_args{account->accountId(),
                                          account2->accountId(),
                                          asset_id,
                                          asset_amount_one_zero,
                                          "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 2, query_args);
    }

    /**
     * @given command
     * @when trying to transfer asset back and forth with non-existing account
     * @then account asset fails to be transferred
     */
    TEST_F(TransferAccountAssetTest, NoAccount) {
      addAllPerms();
      addAllPerms(account2->accountId(), "all2");
      addAsset();
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())),
          true)));
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().transferAsset(
                      "some@domain",
                      account2->accountId(),
                      asset_id,
                      "desc",
                      asset_amount_one_zero)),
                  true);

      {
        std::vector<std::string> query_args{"some@domain",
                                            account2->accountId(),
                                            asset_id,
                                            asset_amount_one_zero,
                                            "1"};
        CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 3, query_args);
      }

      cmd_result = execute(buildCommand(TestTransactionBuilder().transferAsset(
                               account->accountId(),
                               "some@domain",
                               asset_id,
                               "desc",
                               asset_amount_one_zero)),
                           true);

      {
        std::vector<std::string> query_args{account->accountId(),
                                            "some@domain",
                                            asset_id,
                                            asset_amount_one_zero,
                                            "1"};
        CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 4, query_args);
      }
    }

    /**
     * @given command
     * @when trying to transfer account asset with non-existing asset
     * @then account asset fails to be transferred
     */
    TEST_F(TransferAccountAssetTest, NoAsset) {
      addAllPerms();
      addAllPerms(account2->accountId(), "all2");
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().transferAsset(account->accountId(),
                                                 account2->accountId(),
                                                 asset_id,
                                                 "desc",
                                                 asset_amount_one_zero)));

      std::vector<std::string> query_args{account->accountId(),
                                          account2->accountId(),
                                          asset_id,
                                          asset_amount_one_zero,
                                          "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 5, query_args);
    }

    /**
     * @given command
     * @when trying to transfer account asset, but has insufficient amount of it
     * @then account asset fails to be transferred
     */
    TEST_F(TransferAccountAssetTest, Overdraft) {
      addAllPerms();
      addAllPerms(account2->accountId(), "all2");
      addAsset();
      ASSERT_TRUE(val(execute(
          buildCommand(TestTransactionBuilder()
                           .addAssetQuantity(asset_id, asset_amount_one_zero)
                           .creatorAccountId(account->accountId())),
          true)));
      auto cmd_result = execute(buildCommand(
          TestTransactionBuilder().transferAsset(account->accountId(),
                                                 account2->accountId(),
                                                 asset_id,
                                                 "desc",
                                                 "2.0")));

      std::vector<std::string> query_args{
          account->accountId(), account2->accountId(), asset_id, "2.0", "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 6, query_args);
    }

    /**
     * @given command
     * @when trying to transfer account asset, but final value overflows the
     * destination's asset value
     * @then account asset fails to be transferred
     */
    TEST_F(TransferAccountAssetTest, OverflowDestination) {
      addAllPerms();
      addAllPerms(account2->accountId(), "all2");
      addAsset();
      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, uint256_halfmax)
                                   .creatorAccountId(account->accountId())),
                  true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addAssetQuantity(
                          asset_id, uint256_halfmax)),
                      false,
                      account2->accountId())));
      auto cmd_result =
          execute(buildCommand(TestTransactionBuilder().transferAsset(
                      account->accountId(),
                      account2->accountId(),
                      asset_id,
                      "desc",
                      uint256_halfmax)),
                  true);

      std::vector<std::string> query_args{account->accountId(),
                                          account2->accountId(),
                                          asset_id,
                                          uint256_halfmax,
                                          "1"};
      CHECK_ERROR_CODE_AND_MESSAGE(cmd_result, 7, query_args);
    }

  }  // namespace ametsuchi
}  // namespace iroha
