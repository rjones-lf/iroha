/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/result_fixture.hpp"
#include "ametsuchi/impl/postgres_command_executor.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"


namespace iroha {
  namespace ametsuchi {

    using namespace framework::expected;

    class CommandExecutorTest : public AmetsuchiTest {
     public:
      WsvQueryCommandTest() {
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
      }

      void SetUp() override {
        AmetsuchiTest::SetUp();
        postgres_connection = std::make_unique<pqxx::lazyconnection>(pgopt_);
        try {
          postgres_connection->activate();
        } catch (const pqxx::broken_connection &e) {
          FAIL() << "Connection to PostgreSQL broken: " << e.what();
        }
        wsv_transaction =
            std::make_unique<pqxx::nontransaction>(*postgres_connection);

        query = std::make_unique<PostgresWsvQuery>(*wsv_transaction);
        executor = std::make_unique<PostgresCommandExecutor>(*wsv_transaction);

        wsv_transaction->exec(init_);
      }

      CommandResult execute(
          const std::unique_ptr<shared_model::interface::Command> &command) {
        executor->setCreatorAccountId(creator->accountId());
        return boost::apply_visitor(*executor, command->get());
      }

      std::string role = "role";
      shared_model::interface::RolePermissionSet role_permissions;
      shared_model::interface::permissions::Grantable grantable_permission;
      std::unique_ptr<shared_model::interface::Account> account;
      std::unique_ptr<shared_model::interface::Domain> domain;

      std::unique_ptr<pqxx::lazyconnection> postgres_connection;
      std::unique_ptr<pqxx::nontransaction> wsv_transaction;

      std::unique_ptr<WsvQuery> query;
      std::unique_ptr<CommandExecutor> executor;
    };

    class AddAccountAssetTest : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();

        iroha::protocol::CreateRole create_role;
        create_role.set_role_name(role);

        ASSERT_TRUE(val(command->insertRole(role)));
        ASSERT_TRUE(val(command->insertDomain(*domain)));
        ASSERT_TRUE(val(command->insertAccount(*account)));
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

        ASSERT_TRUE(val(command->insertAsset(*asset)));
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
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 1)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 1)));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("2", account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given  command
     * @when trying to add account asset with non-existing asset
     * @then account asset fails to be added
     */
    TEST_F(AddAccountAssetTest, AddAccountAssetTestInvalidAsset) {
      ASSERT_FALSE(val(
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 1)));
    }

    /**
     * @given  command
     * @when trying to add account asset with non-existing account
     * @then account asset fails to added
     */
    TEST_F(AddAccountAssetTest, AddAccountAssetTestInvalidAccount) {
      addAsset();
      ASSERT_FALSE(
          val(executor->addAssetQuantity("some@domain", asset_id, "1", 1)));
    }

    /**
     * @given  command
     * @when trying to add account asset with wrong precision
     * @then account asset fails to added
     */
    TEST_F(AddAccountAssetTest, AddAccountAssetTestInvalidPrecision) {
      addAsset();
      ASSERT_FALSE(val(
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 5)));
    }

    /**
     * @given  command
     * @when trying to add account asset that overflows
     * @then account asset fails to added
     */
    TEST_F(AddAccountAssetTest, AddAccountAssetTestUint256Overflow) {
      std::string uint256_halfmax =
          "72370055773322622139731865630429942408293740416025352524660990004945"
          "7060"
          "2495.0";  // 2**252 - 1
      addAsset();
      ASSERT_TRUE(val(executor->addAssetQuantity(
          account->accountId(), asset_id, uint256_halfmax, 1)));
      ASSERT_FALSE(val(executor->addAssetQuantity(
          account->accountId(), asset_id, uint256_halfmax, 1)));
    }

    class AddPeer : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
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
      ASSERT_TRUE(val(executor->addPeer(*peer)));
    }

    class AddSignatory : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
        pubkey = std::make_unique<shared_model::interface::types::PubkeyType>(
            std::string('1', 32));
        ASSERT_TRUE(val(command->insertRole(role)));
        ASSERT_TRUE(val(command->insertDomain(*domain)));
        ASSERT_TRUE(val(command->insertAccount(*account)));
      }
      std::unique_ptr<shared_model::interface::types::PubkeyType> pubkey;
    };

    /**
     * @given  command
     * @when trying to add signatory
     * @then signatory is successfully added
     */
    TEST_F(AddSignatory, ValidAddSignatoryTest) {
      ASSERT_TRUE(val(executor->addSignatory(account->accountId(), *pubkey)));
      auto signatories = query->getSignatories(account->accountId());
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  != signatories->end());
    }

    class AppendRole : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
        ASSERT_TRUE(val(command->insertRole(role)));
        ASSERT_TRUE(val(command->insertDomain(*domain)));
        ASSERT_TRUE(val(command->insertAccount(*account)));
      }
    };

    /**
     * @given  command
     * @when trying to append role
     * @then role is successfully appended
     */
    TEST_F(AppendRole, ValidAppendRoleTest) {
      ASSERT_TRUE(val(executor->appendRole(account->accountId(), role)));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), role)
                  != roles->end());
    }

    class CreateAccount : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
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
      ASSERT_TRUE(err(executor->createAccount(
          "id",
          domain->domainId(),
          shared_model::interface::types::PubkeyType(std::string('1', 32)))));
    }

    /**
     * @given  command ]
     * @when trying to create account
     * @then account is created
     */
    TEST_F(CreateAccount, ValidCreateAccountWithDomainTest) {
      ASSERT_TRUE(val(command->insertRole(role)));
      ASSERT_TRUE(val(command->insertDomain(*domain)));
      ASSERT_TRUE(val(executor->createAccount(
          "id",
          domain->domainId(),
          shared_model::interface::types::PubkeyType(std::string('1', 32)))));
      auto acc = query->getAccount(account->accountId());
      ASSERT_TRUE(acc);
      ASSERT_EQ(*account.get(), *acc.get());
    }

    class CreateAsset : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
      }
      shared_model::interface::types::AssetIdType asset_id =
          "coin#" + domain->domainId();
    };

    /**
     * @given  command and no target domain in ledger
     * @when trying to create asset
     * @then asset is not created
     */
    TEST_F(CreateAsset, InvalidCreateAssetNoDomainTest) {
      ASSERT_TRUE(err(executor->createAsset("coin", domain->domainId(), 1)));
    }

    /**
     * @given  command ]
     * @when trying to create asset
     * @then asset is created
     */
    TEST_F(CreateAsset, ValidCreateAssetWithDomainTest) {
      ASSERT_TRUE(val(command->insertRole(role)));
      ASSERT_TRUE(val(command->insertDomain(*domain)));
      auto asset = clone(TestAccountAssetBuilder()
                             .domainId(domain->domainId())
                             .assetId(asset_id)
                             .precision(1)
                             .build());
      ASSERT_TRUE(val(executor->createAsset("coin", domain->domainId(), 1)));
      auto ass = query->getAsset(asset->assetId());
      ASSERT_TRUE(ass);
      ASSERT_EQ(*asset.get(), *ass.get());
    }

    class CreateDomain : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
      }
    };

    /**
     * @given  command when there is no role
     * @when trying to create domain
     * @then domain is not created
     */
    TEST_F(CreateDomain, InvalidCreateDomainWhenNoRoleTest) {
      ASSERT_TRUE(err(executor->createDomain(account->accountId(), role)));
    }

    /**
     * @given  command when there is no role
     * @when trying to create domain
     * @then domain is not created
     */
    TEST_F(CreateDomain, ValidCreateDomainTest) {
      ASSERT_TRUE(val(command->insertRole(role)));
      ASSERT_TRUE(val(executor->createDomain(domain->domainId(), role)));
      auto dom = query->getDomain(domain->domainId());
      ASSERT_TRUE(dom);
      ASSERT_EQ(*dom.get(), *domain.get());
    }

    class CreateRole : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
      }
    };

    /**
     * @given  command
     * @when trying to create role
     * @then role is created
     */
    TEST_F(CreateRole, ValidCreateRoleTest) {
      ASSERT_TRUE(val(executor->createRole(
          role, shared_model::interface::RolePermissionSet())));
      auto rl = query->getRolePermissions(role);
      ASSERT_TRUE(rl);
      ASSERT_EQ(rl.get(), shared_model::interface::RolePermissionSet());
    }

    class DetachRole : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
        ASSERT_TRUE(val(executor->createRole(
            role, shared_model::interface::RolePermissionSet())));
        ASSERT_TRUE(val(executor->createRole(
            "role2", shared_model::interface::RolePermissionSet())));
        ASSERT_TRUE(val(executor->createDomain(domain->domainId(), "role2")));
        ASSERT_TRUE(val(executor->createAccount(
            "id",
            domain->domainId(),
            shared_model::interface::types::PubkeyType(std::string('1', 32)))));
        ASSERT_TRUE(val(executor->appendRole(account->accountId(), role)));
      }
    };

    /**
     * @given  command
     * @when trying to detach role
     * @then role is detached
     */
    TEST_F(DetachRole, ValidDetachRoleTest) {
      ASSERT_TRUE(val(executor->detachRole(account->accountId(), role)));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_TRUE(std::find(roles->begin(), roles->end(), role)
                  == roles->end());
    }

    class GrantPermission : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
        ASSERT_TRUE(val(executor->createRole(
            role, shared_model::interface::RolePermissionSet())));
        ASSERT_TRUE(val(executor->createRole(
            "role2", shared_model::interface::RolePermissionSet())));
        ASSERT_TRUE(val(executor->createDomain(domain->domainId(), role)));
        ASSERT_TRUE(val(executor->createAccount(
            "id",
            domain->domainId(),
            shared_model::interface::types::PubkeyType(std::string('1', 32)))));
      }
    };

    /**
     * @given  command
     * @when trying to grant permission
     * @then permission is granted
     */
    TEST_F(GrantPermission, ValidGrantPermissionTest) {
      auto perm = shared_model::interface::permissions::Grantable::kSetMyQuorum;
      ASSERT_TRUE(val(executor->grantPermission(
          account->accountId(), account->accountId(), perm)));
      auto has_perm = query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), perm);
      ASSERT_TRUE(has_perm);
    }

    class RemoveSignatory : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
        pubkey = std::make_unique<shared_model::interface::types::PubkeyType>(
            std::string('1', 32));
        ASSERT_TRUE(val(executor->createRole(
            role, shared_model::interface::RolePermissionSet())));
        ASSERT_TRUE(val(executor->createDomain(domain->domainId(), role)));
        ASSERT_TRUE(val(executor->createAccount(
            "id",
            domain->domainId(),
            shared_model::interface::types::PubkeyType(std::string('2', 32)))));
        ASSERT_TRUE(val(executor->addSignatory(account->accountId(), *pubkey)));
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
          val(executor->removeSignatory(account->accountId(), *pubkey)));
      auto signatories = query->getSignatories(account->accountId());
      ASSERT_TRUE(signatories);
      ASSERT_TRUE(std::find(signatories->begin(), signatories->end(), *pubkey)
                  == signatories->end());
    }

    class RevokePermission : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
        ASSERT_TRUE(val(executor->createRole(
            role, shared_model::interface::RolePermissionSet())));
        ASSERT_TRUE(val(executor->createDomain(domain->domainId(), role)));
        ASSERT_TRUE(val(executor->createAccount(
            "id",
            domain->domainId(),
            shared_model::interface::types::PubkeyType(std::string('1', 32)))));
        ASSERT_TRUE(val(executor->grantPermission(
            account->accountId(), account->accountId(), grantable_permission)));
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

      ASSERT_TRUE(val(executor->grantPermission(
          account->accountId(), account->accountId(), perm)));
      ASSERT_TRUE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), grantable_permission));
      ASSERT_TRUE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), perm));

      ASSERT_TRUE(val(executor->revokePermission(
          account->accountId(), account->accountId(), grantable_permission)));
      ASSERT_FALSE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), grantable_permission));
      ASSERT_TRUE(query->hasAccountGrantablePermission(
          account->accountId(), account->accountId(), perm));
    }

    class SetAccountDetail : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
        ASSERT_TRUE(val(executor->createRole(
            role, shared_model::interface::RolePermissionSet())));
        ASSERT_TRUE(val(executor->createDomain(domain->domainId(), role)));
        ASSERT_TRUE(val(executor->createAccount(
            "id",
            domain->domainId(),
            shared_model::interface::types::PubkeyType(std::string('1', 32)))));
      }
    };

    /**
     * @given  command
     * @when trying to set kv
     * @then kv is set
     */
    TEST_F(SetAccountDetail, ValidSetAccountDetailTest) {
      ASSERT_TRUE(val(executor->setAccountDetail(
          account->accountId(), account->accountId(), "key", "value")));
      auto kv = query->getAccountDetail(account->accountId());
      ASSERT_TRUE(kv);
      ASSERT_EQ(kv.get(), "{\"id@domain\": {\"key\": \"value\"}}");
    }

    class SetQuorum : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
        ASSERT_TRUE(val(executor->createRole(
            role, shared_model::interface::RolePermissionSet())));
        ASSERT_TRUE(val(executor->createDomain(domain->domainId(), role)));
        ASSERT_TRUE(val(executor->createAccount(
            "id",
            domain->domainId(),
            shared_model::interface::types::PubkeyType(std::string('1', 32)))));
      }
    };

    /**
     * @given  command
     * @when trying to set kv
     * @then kv is set
     */
    TEST_F(SetQuorum, ValidSetQuorumTest) {
      ASSERT_TRUE(val(executor->setQuorum(account->accountId(), 3)));
    }

    class SubtractAccountAssetTest : public WsvQueryCommandTest {
      void SetUp() override {
        WsvQueryCommandTest::SetUp();

        ASSERT_TRUE(val(command->insertRole(role)));
        ASSERT_TRUE(val(command->insertDomain(*domain)));
        ASSERT_TRUE(val(command->insertAccount(*account)));
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

        ASSERT_TRUE(val(command->insertAsset(*asset)));
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
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 1)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 1)));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("2", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(executor->subtractAssetQuantity(
          account->accountId(), asset_id, "1", 1)));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1", account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given  command
     * @when trying to subtract account asset with non-existing asset
     * @then account asset fails to be subtracted
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestInvalidAsset) {
      ASSERT_FALSE(val(executor->subtractAssetQuantity(
          account->accountId(), asset_id, "1", 1)));
    }

    /**
     * @given  command
     * @when trying to add account subtract with non-existing account
     * @then account asset fails to subtracted
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestInvalidAccount) {
      addAsset();
      ASSERT_FALSE(val(
          executor->subtractAssetQuantity("some@domain", asset_id, "1", 1)));
    }

    /**
     * @given  command
     * @when trying to add account asset with wrong precision
     * @then account asset fails to added
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestInvalidPrecision) {
      addAsset();
      ASSERT_FALSE(val(executor->subtractAssetQuantity(
          account->accountId(), asset_id, "1", 5)));
    }

    /**
     * @given  command
     * @when trying to add account asset that overflows
     * @then account asset fails to added
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestUint256Overflow) {
      addAsset();
      ASSERT_TRUE(val(
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 1)));
      ASSERT_FALSE(val(executor->subtractAssetQuantity(
          account->accountId(), asset_id, "2", 1)));
    }

    class TransferAccountAssetTest : public WsvQueryCommandTest {
      void SetUp() override {
        WsvQueryCommandTest::SetUp();

        account2 = clone(TestAccountBuilder()
                             .domainId(domain->domainId())
                             .accountId("id2@" + domain->domainId())
                             .quorum(1)
                             .jsonData("{}")
                             .build());

        ASSERT_TRUE(val(command->insertRole(role)));
        ASSERT_TRUE(val(command->insertDomain(*domain)));
        ASSERT_TRUE(val(command->insertAccount(*account)));
        ASSERT_TRUE(val(command->insertAccount(*account2)));
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

        ASSERT_TRUE(val(command->insertAsset(*asset)));
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
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 1)));
      auto account_asset =
          query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 1)));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("2", account_asset.get()->balance().toStringRepr());
      ASSERT_TRUE(val(executor->transferAsset(
          account->accountId(), account2->accountId(), asset_id, "1", 1)));
      account_asset = query->getAccountAsset(account->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1", account_asset.get()->balance().toStringRepr());
      account_asset = query->getAccountAsset(account2->accountId(), asset_id);
      ASSERT_TRUE(account_asset);
      ASSERT_EQ("1", account_asset.get()->balance().toStringRepr());
    }

    /**
     * @given  command
     * @when trying to transfer account asset with non-existing asset
     * @then account asset fails to be transfered
     */
    TEST_F(TransferAccountAssetTest, TransferAccountAssetTestInvalidAsset) {
      ASSERT_FALSE(val(executor->transferAsset(
          account->accountId(), account2->accountId(), asset_id, "1", 1)));
    }

    /**
     * @given  command
     * @when trying to transfer account asset with non-existing account
     * @then account asset fails to transfered
     */
    TEST_F(TransferAccountAssetTest, TransferAccountAssetTestInvalidAccount) {
      addAsset();
      ASSERT_TRUE(val(
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 1)));
      ASSERT_FALSE(val(executor->transferAsset(
          account->accountId(), "some@domain", asset_id, "1", 1)));
      ASSERT_FALSE(val(executor->transferAsset(
          "some@domain", account->accountId(), asset_id, "1", 1)));
    }

    /**
     * @given  command
     * @when trying to transfer account asset that overflows
     * @then account asset fails to transfered
     */
    TEST_F(TransferAccountAssetTest, TransferAccountAssetOwerdraftTest) {
      addAsset();
      ASSERT_TRUE(val(
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 1)));
      ASSERT_FALSE(val(executor->transferAsset(
          account->accountId(), account2->accountId(), asset_id, "2", 1)));
    }

  }  // namespace ametsuchi
}  // namespace iroha
