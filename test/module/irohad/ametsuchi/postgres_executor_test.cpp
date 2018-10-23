/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_command_executor.hpp"
#include "ametsuchi/impl/postgres_query_executor.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
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

    using namespace framework::expected;

    class CommandExecutorTest : public AmetsuchiTest {
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
        executor = std::make_unique<PostgresCommandExecutor>(*sql);

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
     * @given  command
     * @when trying to add account asset
     * @then account asset is successfully added
     */
    TEST_F(AddAccountAssetTest, ValidAddAccountAssetTest) {
      addAsset();
      addAllPerms();
      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, "1.0")
                                   .creatorAccountId(account->accountId())))));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1.0", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, "1.0")
                                   .creatorAccountId(account->accountId())))));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("2.0", account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given  command
     * @when trying to add account asset without permission
     * @then account asset not added
     */
    TEST_F(AddAccountAssetTest, InvalidAddAccountAssetTestNoPerms) {
      addAsset();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "1.0")
                                       .creatorAccountId(account->accountId())),
                      true)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1.0", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(err(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, "1.0")
                                   .creatorAccountId(account->accountId())))));
    }

    /**
     * @given  command
     * @when trying to add account asset with non-existing asset
     * @then account asset fails to be added
     */
    TEST_F(AddAccountAssetTest, AddAccountAssetTestInvalidAsset) {
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "1.0")
                                       .creatorAccountId(account->accountId())),
                      true)));
    }

    /**
     * @given  command
     * @when trying to add account asset with non-existing account
     * @then account asset fails to added
     */
    TEST_F(AddAccountAssetTest, AddAccountAssetTestInvalidAccount) {
      addAsset();
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "1.0")
                                       .creatorAccountId("some@domain")),
                      true,
                      "some@domain")));
    }

    /**
     * @given  command
     * @when trying to add account asset that overflows
     * @then account asset fails to added
     */
    TEST_F(AddAccountAssetTest, AddAccountAssetTestUint256Overflow) {
      std::string uint256_halfmax =
          "57896044618658097711785492504343953926634992332820282019728792003956"
          "5648"
          "19966.0";  // 2**255 - 2tra
      addAsset();
      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, uint256_halfmax)
                                   .creatorAccountId(account->accountId())),
                  true)));
      ASSERT_TRUE(err(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, uint256_halfmax)
                                   .creatorAccountId(account->accountId())),
                  true)));
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
     * @given  command
     * @when trying to add peer
     * @then peer is successfully added
     */
    TEST_F(AddPeer, ValidAddPeerTest) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().addPeer(peer->address(), peer->pubkey())))));
    }

    /**
     * @given  command
     * @when trying to add peer without perms
     * @then peer is not added
     */
    TEST_F(AddPeer, InvalidAddPeerTestWhenNoPerms) {
      ASSERT_TRUE(err(execute(buildCommand(
          TestTransactionBuilder().addPeer(peer->address(), peer->pubkey())))));
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
     * @given  command
     * @when trying to add signatory with role permission
     * @then signatory is successfully added
     */
    TEST_F(AddSignatory, ValidAddSignatoryTestRolePerms) {
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
     * @given  command
     * @when trying to add signatory with grantable permission
     * @then signatory is successfully added
     */
    TEST_F(AddSignatory, ValidAddSignatoryTestGrantablePerms) {
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
     * @given  command
     * @when trying to add signatory without permissions
     * @then signatory is not added
     */
    TEST_F(AddSignatory, InvalidAddSignatoryTestWhenNoPerms) {
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder().addSignatory(
              account->accountId(), *pubkey)))));
      auto signatories = query->getSignatories(account->accountId());
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  == signatories->end());
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
     * @given  command
     * @when trying to append role with perms that creator does not have
     * @then role is not appended
     */
    TEST_F(AppendRole, AppendRoleTestInvalidWhenAccountDoesNotHavePerms) {
      role_permissions2.set(
          shared_model::interface::permissions::Role::kRemoveMySignatory);
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
                                  another_role, role_permissions2)),
                              true)));
      ASSERT_TRUE(err(execute(buildCommand(TestTransactionBuilder().appendRole(
          account->accountId(), another_role)))));
    }

    /**
     * @given  command
     * @when trying to append role with perms that creator does not have
     *      but in genesis block
     * @then role is appended
     */
    TEST_F(AppendRole, AppendRoleTestValidWhenAccountDoesNotHavePermsGenesis) {
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

    TEST_F(AppendRole, InvalidAppendRoleTestWhenNoPerms) {
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
                                  another_role, role_permissions)),
                              true)));
      ASSERT_TRUE(err(execute(buildCommand(TestTransactionBuilder().appendRole(
          account->accountId(), another_role)))));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), another_role)
                  == roles->end());
    }

    TEST_F(AppendRole, ValidAppendRoleTest) {
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

    TEST_F(AppendRole, ValidAppendRoleTestWhenEmptyPerms) {
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
     * @given  command and no target domain in ledger
     * @when trying to create account
     * @then account is not created
     */
    TEST_F(CreateAccount, InvalidCreateAccountNoDomainTest) {
      addAllPerms();
      ASSERT_TRUE(err(execute(buildCommand(
          TestTransactionBuilder().createAccount("id2", "domain2", *pubkey)))));
    }

    /**
     * @given  command
     * @when trying to create account
     * @then account is created
     */
    TEST_F(CreateAccount, ValidCreateAccountWithDomainTest) {
      addAllPerms();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createAccount(
              "id2", domain->domainId(), *pubkey)))));
      auto acc = query->getAccount(account2->accountId());
      ASSERT_TRUE(acc);
      ASSERT_EQ(*account2.get(), *acc.get());
    }

    /**
     * @given  command
     * @when trying to create account
     * @then account is created
     */
    TEST_F(CreateAccount, InvalidCreateAccountWithoutPermsTest) {
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder().createAccount(
              "id2", domain->domainId(), *pubkey)))));
      auto acc = query->getAccount(account2->accountId());
      ASSERT_FALSE(acc);
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
     * @given  command and no target domain in ledger
     * @when trying to create asset
     * @then asset is not created
     */
    TEST_F(CreateAsset, InvalidCreateAssetNoDomainTest) {
      ASSERT_TRUE(err(execute(buildCommand(TestTransactionBuilder().createAsset(
          asset_name, domain->domainId(), 1)))));
    }

    /**
     * @given  command
     * @when trying to create asset
     * @then asset is created
     */
    TEST_F(CreateAsset, ValidCreateAssetWithDomainTest) {
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
     * @given  command
     * @when trying to create asset without permission
     * @then asset is not created
     */
    TEST_F(CreateAsset, InvalidCreateAssetWithDomainTest) {
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
      ASSERT_TRUE(err(execute(buildCommand(TestTransactionBuilder().createAsset(
          "coin", domain->domainId(), 1)))));
      auto ass = query->getAsset(asset->assetId());
      ASSERT_FALSE(ass);
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
     * @given  command when there is no role
     * @when trying to create domain
     * @then domain is not created
     */
    TEST_F(CreateDomain, InvalidCreateDomainWhenNoRoleTest) {
      addAllPerms();
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder().createDomain(
              domain2->domainId(), another_role)))));
    }

    /**
     * @given  command
     * @when trying to create domain
     * @then domain is created
     */
    TEST_F(CreateDomain, ValidCreateDomainTest) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().createDomain(domain2->domainId(), role)))));
      auto dom = query->getDomain(domain2->domainId());
      ASSERT_TRUE(dom);
      ASSERT_EQ(*dom.get(), *domain2.get());
    }

    /**
     * @given  command when there is no perms
     * @when trying to create domain
     * @then domain is not created
     */
    TEST_F(CreateDomain, InvalidCreateDomainTestWhenNoPerms) {
      ASSERT_TRUE(err(execute(buildCommand(
          TestTransactionBuilder().createDomain(domain2->domainId(), role)))));
      auto dom = query->getDomain(domain2->domainId());
      ASSERT_FALSE(dom);
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
     * @given  command
     * @when trying to create role
     * @then role is created
     */
    TEST_F(CreateRole, ValidCreateRoleTest) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createRole(
          another_role, role_permissions)))));
      auto rl = query->getRolePermissions(role);
      ASSERT_TRUE(rl);
      ASSERT_EQ(rl.get(), role_permissions);
    }

    /**
     * @given  command
     * @when trying to create role when creator doesn't have all permissions
     * @then role is not created
     */
    TEST_F(CreateRole, CreateRoleTestInvalidWhenHasNoPerms) {
      role_permissions2.set(
          shared_model::interface::permissions::Role::kRemoveMySignatory);
      ASSERT_TRUE(err(execute(buildCommand(TestTransactionBuilder().createRole(
          another_role, role_permissions2)))));
      auto rl = query->getRolePermissions(another_role);
      ASSERT_TRUE(rl);
      ASSERT_TRUE(rl->none());
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
     * @given  command
     * @when trying to detach role
     * @then role is detached
     */
    TEST_F(DetachRole, ValidDetachRoleTest) {
      addAllPerms();
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().detachRole(
          account->accountId(), another_role)))));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), another_role)
                  == roles->end());
    }

    /**
     * @given  command
     * @when trying to detach role without permission
     * @then role is detached
     */
    TEST_F(DetachRole, InvalidDetachRoleTestWhenNoPerms) {
      ASSERT_TRUE(err(execute(buildCommand(TestTransactionBuilder().detachRole(
          account->accountId(), another_role)))));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), another_role)
                  != roles->end());
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
     * @given  command
     * @when trying to grant permission
     * @then permission is granted
     */
    TEST_F(GrantPermission, ValidGrantPermissionTest) {
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
     * @given  command
     * @when trying to grant permission without permission
     * @then permission is not granted
     */
    TEST_F(GrantPermission, InvalidGrantPermissionTestWhenNoPerm) {
      auto perm = shared_model::interface::permissions::Grantable::kSetMyQuorum;
      ASSERT_TRUE(err(
          execute(buildCommand(TestTransactionBuilder()
                                   .grantPermission(account->accountId(), perm)
                                   .creatorAccountId(account->accountId())))));
      auto has_perm = query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), perm);
      ASSERT_FALSE(has_perm);
    }

    class RemoveSignatory : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        pubkey = std::make_unique<shared_model::interface::types::PubkeyType>(
            std::string('1', 32));
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
    };

    /**
     * @given  command
     * @when trying to remove signatory
     * @then signatory is successfully removed
     */
    TEST_F(RemoveSignatory, ValidRemoveSignatoryTest) {
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
     * @given  command
     * @when trying to remove signatory
     * @then signatory is successfully removed
     */
    TEST_F(RemoveSignatory, ValidRemoveSignatoryTestWhenHasGrantablePerm) {
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
     * @given  command
     * @when trying to remove signatory without permission
     * @then signatory is not removed
     */
    TEST_F(RemoveSignatory, InvalidRemoveSignatoryTestWhenNoPerms) {
      shared_model::interface::types::PubkeyType pk(std::string('5', 32));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          account->accountId(), pk)),
                      true)));
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder().removeSignatory(
              account->accountId(), *pubkey)))));
      auto signatories = query->getSignatories(account->accountId());
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  != signatories->end());
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), pk)
                  != signatories->end());
    }

    /**
     * @given  command
     * @when trying to remove signatory from account so it has less than quorum
     * @then signatory is not removed
     */
    TEST_F(RemoveSignatory, RemoveSignatoryTestInvalidWhenQuorum) {
      addAllPerms();
      shared_model::interface::types::PubkeyType pk(std::string('5', 32));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          account->accountId(), pk)),
                      true)));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().removeSignatory(
              account->accountId(), *pubkey)))));
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder().removeSignatory(
              account->accountId(), pk)))));
    }

    /**
     * @given  command
     * @when trying to remove signatory from a non existing account
     * @then signatory is not removed
     */
    TEST_F(RemoveSignatory, RemoveSignatoryTestNonExistingAccount) {
      addAllPerms();
      shared_model::interface::types::PubkeyType pk(std::string('5', 32));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          account->accountId(), pk)),
                      true)));

      ASSERT_TRUE(err(execute(buildCommand(
          TestTransactionBuilder().removeSignatory("hello", *pubkey)))));
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
     * @given  command
     * @when trying to revoke permission
     * @then permission is revoked
     */
    TEST_F(RevokePermission, ValidRevokePermissionTest) {
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
     * @given  command
     * @when trying to revoke permission without permission
     * @then permission is revoked
     */
    TEST_F(RevokePermission, InvalidRevokePermissionTestWithNoPermission) {
      auto perm =
          shared_model::interface::permissions::Grantable::kRemoveMySignatory;
      ASSERT_TRUE(err(
          execute(buildCommand(TestTransactionBuilder()
                                   .revokePermission(account->accountId(), perm)
                                   .creatorAccountId(account->accountId())))));
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
     * @given  command
     * @when trying to set kv
     * @then kv is set
     */
    TEST_F(SetAccountDetail, ValidSetAccountDetailTest) {
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().setAccountDetail(
              account->accountId(), "key", "value")))));
      auto kv = query->getAccountDetail(account->accountId());
      ASSERT_TRUE(kv);
      ASSERT_EQ(kv.get(), "{\"id@domain\": {\"key\": \"value\"}}");
    }

    /**
     * @given  command
     * @when trying to set kv when has grantable permission
     * @then kv is set
     */
    TEST_F(SetAccountDetail, ValidSetAccountDetailTestWhenGrantablePerm) {
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
     * @given  command
     * @when trying to set kv when has role permission
     * @then kv is set
     */
    TEST_F(SetAccountDetail, ValidSetAccountDetailTestWhenPerm) {
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
     * @given  command
     * @when trying to set kv
     * @then kv is set
     */
    TEST_F(SetAccountDetail, InvalidSetAccountDetailTest) {
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder().setAccountDetail(
                          account2->accountId(), "key", "value")),
                      false,
                      account->accountId())));
      auto kv = query->getAccountDetail(account2->accountId());
      ASSERT_TRUE(kv);
      ASSERT_EQ(kv.get(), "{}");
    }

    class SetQuorum : public CommandExecutorTest {
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
    };

    /**
     * @given  command
     * @when trying to set quorum more than amount of signatories
     * @then quorum is not set
     */
    TEST_F(SetQuorum, SetQuorumTestInvalidSignatories) {
      addAllPerms();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().setAccountQuorum(
              account->accountId(), 3)))));
      shared_model::interface::types::PubkeyType pk(std::string('5', 32));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                          account->accountId(), pk)),
                      true)));
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder().setAccountQuorum(
              account->accountId(), 1)))));
    }

    /**
     * @given  command
     * @when trying to set quorum
     * @then quorum is set
     */
    TEST_F(SetQuorum, ValidSetQuorumTestWithRolePerms) {
      addAllPerms();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().setAccountQuorum(
              account->accountId(), 3)))));
    }

    /**
     * @given  command
     * @when trying to set quorum
     * @then quorum is set
     */
    TEST_F(SetQuorum, ValidSetQuorumTestWithGrantablePerms) {
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

      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().setAccountQuorum("id2@domain", 3)))));
    }

    /**
     * @given  command
     * @when trying to set quorum without perms
     * @then quorum is not set
     */
    TEST_F(SetQuorum, InvalidSetQuorumTestWithNoPerms) {
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder().setAccountQuorum(
              account->accountId(), 3)))));
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
     * @given  command
     * @when trying to subtract account asset
     * @then account asset is successfully subtracted
     */
    TEST_F(SubtractAccountAssetTest, ValidSubtractAccountAssetTest) {
      addAllPerms();
      addAsset();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "1.0")
                                       .creatorAccountId(account->accountId())),
                      true)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1.0", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "1.0")
                                       .creatorAccountId(account->accountId())),
                      true)));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("2.0", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .subtractAssetQuantity(asset_id, "1.0")
                                   .creatorAccountId(account->accountId())))));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1.0", account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given  command
     * @when trying to subtract account asset
     * @then account asset is successfully subtracted
     */
    TEST_F(SubtractAccountAssetTest,
           InvalidSubtractAccountAssetTestWhenNoPerms) {
      addAsset();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "1.0")
                                       .creatorAccountId(account->accountId())),
                      true)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1.0", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(err(
          execute(buildCommand(TestTransactionBuilder()
                                   .subtractAssetQuantity(asset_id, "1.0")
                                   .creatorAccountId(account->accountId())))));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1.0", account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given  command
     * @when trying to subtract account asset with non-existing asset
     * @then account asset fails to be subtracted
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestInvalidAsset) {
      addAllPerms();
      ASSERT_TRUE(err(
          execute(buildCommand(TestTransactionBuilder()
                                   .subtractAssetQuantity(asset_id, "1.0")
                                   .creatorAccountId(account->accountId())))));
    }

    /**
     * @given  command
     * @when trying to add account subtract with non-existing account
     * @then account asset fails to subtracted
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestInvalidAccount) {
      addAllPerms();
      addAsset();
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder()
                                       .subtractAssetQuantity(asset_id, "1.0")
                                       .creatorAccountId("some@domain")))));
    }

    /**
     * @given  command
     * @when trying to add account asset with wrong precision
     * @then account asset fails to added
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestInvalidPrecision) {
      addAllPerms();
      addAsset();
      ASSERT_TRUE(err(
          execute(buildCommand(TestTransactionBuilder()
                                   .subtractAssetQuantity(asset_id, "1.0000")
                                   .creatorAccountId(account->accountId())))));
    }

    /**
     * @given  command
     * @when trying to add account asset that overflows
     * @then account asset fails to added
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestUint256Overflow) {
      addAllPerms();
      addAsset();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "1.0")
                                       .creatorAccountId(account->accountId())),
                      true)));
      ASSERT_TRUE(err(
          execute(buildCommand(TestTransactionBuilder()
                                   .subtractAssetQuantity(asset_id, "2.0")
                                   .creatorAccountId(account->accountId())))));
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
     * @given  command
     * @when trying to add transfer asset
     * @then account asset is successfully transfered
     */
    TEST_F(TransferAccountAssetTest,
           ValidTransferAccountAssetTestWhenRolePerms) {
      addAllPerms();
      addAllPerms(account2->accountId(), "all2");
      addAsset();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "1.0")
                                       .creatorAccountId(account->accountId())),
                      true)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1.0", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "1.0")
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
                                                 "1.0")))));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1.0", account_asset.get()->balance().toStringRepr());
      account_asset = query->getAccountAsset(account2->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1.0", account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given  command
     * @when trying to add transfer asset
     * @then account asset is successfully transfered
     */
    TEST_F(TransferAccountAssetTest,
           ValidTransferAccountAssetTestWhenGrantablePerms) {
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
                          "1.0")),
                      false,
                      account2->accountId())));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1.0", account_asset.get()->balance().toStringRepr());
      account_asset = query->getAccountAsset(account2->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1.0", account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given  command
     * @when trying to transfer account asset with non-existing asset
     * @then account asset fails to be transfered
     */
    TEST_F(TransferAccountAssetTest, TransferAccountAssetTestInvalidAsset) {
      addAllPerms();
      addAllPerms(account2->accountId(), "all2");
      ASSERT_TRUE(err(execute(buildCommand(
          TestTransactionBuilder().transferAsset(account->accountId(),
                                                 account2->accountId(),
                                                 asset_id,
                                                 "desc",
                                                 "1.0")))));
    }

    /**
     * @given  command
     * @when trying to transfer account asset with non-existing account
     * @then account asset fails to transfered
     */
    TEST_F(TransferAccountAssetTest, TransferAccountAssetTestInvalidAccount) {
      addAllPerms();
      addAllPerms(account2->accountId(), "all2");
      addAsset();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "1.0")
                                       .creatorAccountId(account->accountId())),
                      true)));
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder().transferAsset(
              account->accountId(), "some@domain", asset_id, "desc", "1.0")))));
      ASSERT_TRUE(err(execute(buildCommand(
          TestTransactionBuilder().transferAsset("some@domain",
                                                 account2->accountId(),
                                                 asset_id,
                                                 "desc",
                                                 "1.0")))));
    }

    /**
     * @given  command
     * @when trying to transfer account asset that overflows
     * @then account asset fails to transfered
     */
    TEST_F(TransferAccountAssetTest, TransferAccountAssetOwerdraftTest) {
      addAllPerms();
      addAllPerms(account2->accountId(), "all2");
      addAsset();
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder()
                                       .addAssetQuantity(asset_id, "1.0")
                                       .creatorAccountId(account->accountId())),
                      true)));
      ASSERT_TRUE(err(execute(buildCommand(
          TestTransactionBuilder().transferAsset(account->accountId(),
                                                 account2->accountId(),
                                                 asset_id,
                                                 "desc",
                                                 "2.0")))));
    }
  }  // namespace ametsuchi
}  // namespace iroha
