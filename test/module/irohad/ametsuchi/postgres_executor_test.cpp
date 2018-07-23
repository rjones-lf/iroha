/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_command_executor.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "framework/result_fixture.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
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
        executor = std::make_unique<PostgresCommandExecutor>(*sql);

        *sql << init_;
      }

      CommandResult execute(
          const std::unique_ptr<shared_model::interface::Command> &command,
          const shared_model::interface::types::AccountIdType &creator =
              "id@domain") {
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

      std::string role = "role";
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

        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole(role, role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createDomain(domain->domainId(), role)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id", domain->domainId(), *pubkey)))));
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
                "coin", domain->domainId(), 1)))));
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
     * @when trying to add account asset with non-existing asset
     * @then account asset fails to be added
     */
    TEST_F(AddAccountAssetTest, AddAccountAssetTestInvalidAsset) {
      ASSERT_TRUE(err(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, "1.0")
                                   .creatorAccountId(account->accountId())))));
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
                                   .creatorAccountId(account->accountId())))));
      ASSERT_TRUE(err(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, uint256_halfmax)
                                   .creatorAccountId(account->accountId())))));
    }

    class AddPeer : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        peer = clone(TestPeerBuilder().build());
      }
      std::unique_ptr<shared_model::interface::Peer> peer;
    };

    /**
     * @given  command
     * @when trying to add peer
     * @then peer is successfully added
     */
    TEST_F(AddPeer, ValidAddPeerTest) {
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().addPeer(peer->address(), peer->pubkey())))));
    }

    class AddSignatory : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole(role, role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createDomain(domain->domainId(), role)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id",
                domain->domainId(),
                shared_model::interface::types::PubkeyType(
                    std::string('5', 32)))))));
      }
    };

    /**
     * @given  command
     * @when trying to add signatory
     * @then signatory is successfully added
     */
    TEST_F(AddSignatory, ValidAddSignatoryTest) {
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().addSignatory(
              account->accountId(), *pubkey)))));
      auto signatories = query->getSignatories(account->accountId());
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  != signatories->end());
    }

    class AppendRole : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole(role, role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole("role2", role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createDomain(domain->domainId(), role)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id", domain->domainId(), *pubkey)))));
      }
    };

    /**
     * @given  command
     * @when trying to append role
     * @then role is successfully appended
     */
    TEST_F(AppendRole, ValidAppendRoleTest) {
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().appendRole(
          account->accountId(), "role2")))));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), "role2")
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
      }
    };

    /**
     * @given  command and no target domain in ledger
     * @when trying to create account
     * @then account is not created
     */
    TEST_F(CreateAccount, InvalidCreateAccountNoDomainTest) {
      ASSERT_TRUE(
          err(execute(buildCommand(TestTransactionBuilder().createAccount(
              "id", domain->domainId(), *pubkey)))));
    }

    /**
     * @given  command ]
     * @when trying to create account
     * @then account is created
     */
    TEST_F(CreateAccount, ValidCreateAccountWithDomainTest) {
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().createRole(role, role_permissions)))));
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().createDomain(domain->domainId(), role)))));
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().createAccount(
              "id", domain->domainId(), *pubkey)))));
      auto acc = query->getAccount(account->accountId());
      ASSERT_TRUE(acc);
      ASSERT_EQ(*account.get(), *acc.get());
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
     * @given  command ]
     * @when trying to create asset
     * @then asset is created
     */
    TEST_F(CreateAsset, ValidCreateAssetWithDomainTest) {
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().createRole(role, role_permissions)))));
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().createDomain(domain->domainId(), role)))));
      auto asset = clone(TestAccountAssetBuilder()
                             .domainId(domain->domainId())
                             .assetId(asset_id)
                             .precision(1)
                             .build());
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().createAsset(
          "coin", domain->domainId(), 1)))));
      auto ass = query->getAsset(asset->assetId());
      ASSERT_TRUE(ass);
      ASSERT_EQ(*asset.get(), *ass.get());
    }

    class CreateDomain : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
      }
    };

    /**
     * @given  command when there is no role
     * @when trying to create domain
     * @then domain is not created
     */
    TEST_F(CreateDomain, InvalidCreateDomainWhenNoRoleTest) {
      ASSERT_TRUE(err(execute(buildCommand(
          TestTransactionBuilder().createDomain(domain->domainId(), role)))));
    }

    /**
     * @given  command when there is no role
     * @when trying to create domain
     * @then domain is not created
     */
    TEST_F(CreateDomain, ValidCreateDomainTest) {
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().createRole(role, role_permissions)))));
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().createDomain(domain->domainId(), role)))));
      auto dom = query->getDomain(domain->domainId());
      ASSERT_TRUE(dom);
      ASSERT_EQ(*dom.get(), *domain.get());
    }

    class CreateRole : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
      }
    };

    /**
     * @given  command
     * @when trying to create role
     * @then role is created
     */
    TEST_F(CreateRole, ValidCreateRoleTest) {
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder().createRole(role, role_permissions)))));
      auto rl = query->getRolePermissions(role);
      ASSERT_TRUE(rl);
      ASSERT_EQ(rl.get(), role_permissions);
    }

    class DetachRole : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole(role, role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole("role2", role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createDomain(domain->domainId(), role)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id", domain->domainId(), *pubkey)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().appendRole(
                account->accountId(), "role2")))));
      }
    };

    /**
     * @given  command
     * @when trying to detach role
     * @then role is detached
     */
    TEST_F(DetachRole, ValidDetachRoleTest) {
      ASSERT_TRUE(val(execute(buildCommand(TestTransactionBuilder().detachRole(
          account->accountId(), "role2")))));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), "role2")
                  == roles->end());
    }

    class GrantPermission : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole(role, role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole("role2", role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createDomain(domain->domainId(), role)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id", domain->domainId(), *pubkey)))));
      }
    };

    /**
     * @given  command
     * @when trying to grant permission
     * @then permission is granted
     */
    TEST_F(GrantPermission, ValidGrantPermissionTest) {
      auto perm = shared_model::interface::permissions::Grantable::kSetMyQuorum;
      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .grantPermission(account->accountId(), perm)
                                   .creatorAccountId(account->accountId())))));
      auto has_perm = query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), perm);
      ASSERT_TRUE(has_perm);
    }

    class RemoveSignatory : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        pubkey = std::make_unique<shared_model::interface::types::PubkeyType>(
            std::string('1', 32));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole(role, role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createDomain(domain->domainId(), role)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id", domain->domainId(), *pubkey)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().addSignatory(
                account->accountId(),
                shared_model::interface::types::PubkeyType(
                    std::string('5', 32)))))));
      }
      std::unique_ptr<shared_model::interface::types::PubkeyType> pubkey;
    };

    /**
     * @given  command
     * @when trying to remove signatory
     * @then signatory is successfully removed
     */
    TEST_F(RemoveSignatory, ValidRemoveSignatoryTest) {
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().removeSignatory(
              account->accountId(), *pubkey)))));
      auto signatories = query->getSignatories(account->accountId());
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  == signatories->end());
    }

    class RevokePermission : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole(role, role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createDomain(domain->domainId(), role)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id", domain->domainId(), *pubkey)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder()
                .grantPermission(account->accountId(), grantable_permission)
                .creatorAccountId(account->accountId())))));
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
                                   .creatorAccountId(account->accountId())))));
      ASSERT_TRUE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), grantable_permission));
      ASSERT_TRUE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), perm));

      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder()
              .revokePermission(account->accountId(), grantable_permission)
              .creatorAccountId(account->accountId())))));
      ASSERT_FALSE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), grantable_permission));
      ASSERT_TRUE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), perm));
    }

    class SetAccountDetail : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole(role, role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createDomain(domain->domainId(), role)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id", domain->domainId(), *pubkey)))));
      }
    };

    /**
     * @given  command
     * @when trying to set kv
     * @then kv is set
     */
    TEST_F(SetAccountDetail, ValidSetAccountDetailTest) {
      ASSERT_TRUE(val(execute(buildCommand(
          TestTransactionBuilder()
              .setAccountDetail(account->accountId(), "key", "value")
              .creatorAccountId(account->accountId())))));
      auto kv = query->getAccountDetail(account->accountId());
      ASSERT_TRUE(kv);
      ASSERT_EQ(kv.get(), "{\"id@domain\": {\"key\": \"value\"}}");
    }

    class SetQuorum : public CommandExecutorTest {
     public:
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole(role, role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createDomain(domain->domainId(), role)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id", domain->domainId(), *pubkey)))));
      }
    };

    /**
     * @given  command
     * @when trying to set kv
     * @then kv is set
     */
    TEST_F(SetQuorum, ValidSetQuorumTest) {
      ASSERT_TRUE(
          val(execute(buildCommand(TestTransactionBuilder().setAccountQuorum(
              account->accountId(), 3)))));
    }

    class SubtractAccountAssetTest : public CommandExecutorTest {
      void SetUp() override {
        CommandExecutorTest::SetUp();
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole(role, role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createDomain(domain->domainId(), role)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id", domain->domainId(), *pubkey)))));
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
                "coin", domain->domainId(), 1)))));
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
      addAsset();
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
     * @when trying to subtract account asset with non-existing asset
     * @then account asset fails to be subtracted
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestInvalidAsset) {
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
      addAsset();
      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, "1.0")
                                   .creatorAccountId(account->accountId())))));
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

        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createRole(role, role_permissions)))));
        ASSERT_TRUE(val(execute(buildCommand(
            TestTransactionBuilder().createDomain(domain->domainId(), role)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id", domain->domainId(), *pubkey)))));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                "id2", domain->domainId(), *pubkey)))));
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
                "coin", domain->domainId(), 1)))));
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
    TEST_F(TransferAccountAssetTest, ValidTransferAccountAssetTest) {
      addAsset();
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
     * @when trying to transfer account asset with non-existing asset
     * @then account asset fails to be transfered
     */
    TEST_F(TransferAccountAssetTest, TransferAccountAssetTestInvalidAsset) {
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
      addAsset();
      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, "1.0")
                                   .creatorAccountId(account->accountId())))));
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
      addAsset();
      ASSERT_TRUE(val(
          execute(buildCommand(TestTransactionBuilder()
                                   .addAssetQuantity(asset_id, "1.0")
                                   .creatorAccountId(account->accountId())))));
      ASSERT_TRUE(err(execute(buildCommand(
          TestTransactionBuilder().transferAsset(account->accountId(),
                                                 account2->accountId(),
                                                 asset_id,
                                                 "desc",
                                                 "2.0")))));
    }

  }  // namespace ametsuchi
}  // namespace iroha
