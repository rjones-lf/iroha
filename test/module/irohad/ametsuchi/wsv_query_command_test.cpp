/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "ametsuchi/impl/postgres_command_executor.hpp"
#include "ametsuchi/impl/postgres_wsv_command.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "framework/result_fixture.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "module/shared_model/builders/protobuf/test_account_builder.hpp"
#include "module/shared_model/builders/protobuf/test_asset_builder.hpp"
#include "module/shared_model/builders/protobuf/test_domain_builder.hpp"
#include "module/shared_model/builders/protobuf/test_peer_builder.hpp"

namespace iroha {
  namespace ametsuchi {

    using namespace framework::expected;

    class WsvQueryCommandTest : public AmetsuchiTest {
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

        command = std::make_unique<PostgresWsvCommand>(*wsv_transaction);
        query = std::make_unique<PostgresWsvQuery>(*wsv_transaction);
        executor = std::make_unique<PostgresCommandExecutor>(*wsv_transaction);

        wsv_transaction->exec(init_);
      }

      std::string role = "role";
      shared_model::interface::RolePermissionSet role_permissions;
      shared_model::interface::permissions::Grantable grantable_permission;
      std::unique_ptr<shared_model::interface::Account> account;
      std::unique_ptr<shared_model::interface::Domain> domain;

      std::unique_ptr<pqxx::lazyconnection> postgres_connection;
      std::unique_ptr<pqxx::nontransaction> wsv_transaction;

      std::unique_ptr<WsvCommand> command;
      std::unique_ptr<WsvQuery> query;
      std::unique_ptr<CommandExecutor> executor;
    };

    class RoleTest : public WsvQueryCommandTest {};

    /**
     * @given WSV command and valid role name
     * @when trying to insert new role
     * @then role is successfully inserted
     */
    TEST_F(RoleTest, InsertRoleWhenValidName) {
      ASSERT_TRUE(val(command->insertRole(role)));
      auto roles = query->getRoles();
      ASSERT_TRUE(roles);
      ASSERT_EQ(1, roles->size());
      ASSERT_EQ(role, roles->front());
    }

    /**
     * @given WSV command and invalid role name
     * @when trying to insert new role
     * @then role is failed
     */
    TEST_F(RoleTest, InsertRoleWhenInvalidName) {
      ASSERT_TRUE(err(command->insertRole(std::string(46, 'a'))));

      auto roles = query->getRoles();
      ASSERT_TRUE(roles);
      ASSERT_EQ(0, roles->size());
    }

    class RolePermissionsTest : public WsvQueryCommandTest {
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
        ASSERT_TRUE(val(command->insertRole(role)));
      }
    };

    /**
     * @given WSV command and role exists and valid permissions
     * @when trying to insert role permissions
     * @then RolePermissions are inserted
     */
    TEST_F(RolePermissionsTest, InsertRolePermissionsWhenRoleExists) {
      ASSERT_TRUE(val(command->insertRolePermissions(role, role_permissions)));

      auto permissions = query->getRolePermissions(role);
      ASSERT_TRUE(permissions);
      ASSERT_EQ(role_permissions, permissions);
    }

    /**
     * @given WSV command and role doesn't exist and valid permissions
     * @when trying to insert role permissions
     * @then RolePermissions are not inserted
     */
    TEST_F(RolePermissionsTest, InsertRolePermissionsWhenNoRole) {
      auto new_role = role + " ";
      ASSERT_TRUE(
          err(command->insertRolePermissions(new_role, role_permissions)));

      auto permissions = query->getRolePermissions(new_role);
      ASSERT_TRUE(permissions);
      ASSERT_FALSE(role_permissions.isSubsetOf(*permissions));
    }

    class AccountTest : public WsvQueryCommandTest {
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
        ASSERT_TRUE(val(command->insertRole(role)));
        ASSERT_TRUE(val(command->insertDomain(*domain)));
      }
    };

    /**
     * @given inserted role, domain
     * @when insert account with filled json data
     * @then get account and check json data is the same
     */
    TEST_F(AccountTest, InsertAccountWithJSONData) {
      ASSERT_TRUE(val(command->insertAccount(*account)));
      auto acc = query->getAccount(account->accountId());
      ASSERT_TRUE(acc);
      ASSERT_EQ(account->jsonData(), acc.value()->jsonData());
    }

    /**
     * @given inserted role, domain, account
     * @when insert to account new json data
     * @then get account and check json data is the same
     */
    TEST_F(AccountTest, InsertNewJSONDataAccount) {
      ASSERT_TRUE(val(command->insertAccount(*account)));
      ASSERT_TRUE(val(command->setAccountKV(
          account->accountId(), account->accountId(), "id", "val")));
      auto acc = query->getAccount(account->accountId());
      ASSERT_TRUE(acc);
      ASSERT_EQ(R"({"id@domain": {"id": "val", "key": "value"}})",
                acc.value()->jsonData());
    }

    /**
     * @given inserted role, domain, account
     * @when insert to account new json data
     * @then get account and check json data is the same
     */
    TEST_F(AccountTest, InsertNewJSONDataToOtherAccount) {
      ASSERT_TRUE(val(command->insertAccount(*account)));
      ASSERT_TRUE(val(
          command->setAccountKV(account->accountId(), "admin", "id", "val")));
      auto acc = query->getAccount(account->accountId());
      ASSERT_TRUE(acc);
      ASSERT_EQ(R"({"admin": {"id": "val"}, "id@domain": {"key": "value"}})",
                acc.value()->jsonData());
    }

    /**
     * @given inserted role, domain, account
     * @when insert to account new complex json data
     * @then get account and check json data is the same
     */
    TEST_F(AccountTest, InsertNewComplexJSONDataAccount) {
      ASSERT_TRUE(val(command->insertAccount(*account)));
      ASSERT_TRUE(val(command->setAccountKV(
          account->accountId(), account->accountId(), "id", "[val1, val2]")));
      auto acc = query->getAccount(account->accountId());
      ASSERT_TRUE(acc);
      ASSERT_EQ(R"({"id@domain": {"id": "[val1, val2]", "key": "value"}})",
                acc.value()->jsonData());
    }

    /**
     * @given inserted role, domain, account
     * @when update  json data in account
     * @then get account and check json data is the same
     */
    TEST_F(AccountTest, UpdateAccountJSONData) {
      ASSERT_TRUE(val(command->insertAccount(*account)));
      ASSERT_TRUE(val(command->setAccountKV(
          account->accountId(), account->accountId(), "key", "val2")));
      auto acc = query->getAccount(account->accountId());
      ASSERT_TRUE(acc);
      ASSERT_EQ(R"({"id@domain": {"key": "val2"}})", acc.value()->jsonData());
    }

    /**
     * @given database without needed account
     * @when performing query to retrieve non-existent account
     * @then getAccount will return nullopt
     */
    TEST_F(AccountTest, GetAccountInvalidWhenNotFound) {
      EXPECT_FALSE(query->getAccount("invalid account id"));
    }

    /**
     * @given database without needed account
     * @when performing query to retrieve non-existent account's details
     * @then getAccountDetail will return nullopt
     */
    TEST_F(AccountTest, GetAccountDetailInvalidWhenNotFound) {
      EXPECT_FALSE(query->getAccountDetail("invalid account id"));
    }

    class AccountRoleTest : public WsvQueryCommandTest {
      void SetUp() override {
        WsvQueryCommandTest::SetUp();
        ASSERT_TRUE(val(command->insertRole(role)));
        ASSERT_TRUE(val(command->insertDomain(*domain)));
        ASSERT_TRUE(val(command->insertAccount(*account)));
      }
    };

    /**
     * @given WSV command and account exists and valid account role
     * @when trying to insert account
     * @then account role is inserted
     */
    TEST_F(AccountRoleTest, InsertAccountRoleWhenAccountRoleExist) {
      ASSERT_TRUE(val(command->insertAccountRole(account->accountId(), role)));

      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_EQ(1, roles->size());
      ASSERT_EQ(role, roles->front());
    }

    /**
     * @given WSV command and account does not exist and valid account role
     * @when trying to insert account
     * @then account role is not inserted
     */
    TEST_F(AccountRoleTest, InsertAccountRoleWhenNoAccount) {
      auto account_id = account->accountId() + " ";
      ASSERT_TRUE(err(command->insertAccountRole(account_id, role)));

      auto roles = query->getAccountRoles(account_id);
      ASSERT_TRUE(roles);
      ASSERT_EQ(0, roles->size());
    }

    /**
     * @given WSV command and account exists and invalid account role
     * @when trying to insert account
     * @then account role is not inserted
     */
    TEST_F(AccountRoleTest, InsertAccountRoleWhenNoRole) {
      auto new_role = role + " ";
      ASSERT_TRUE(
          err(command->insertAccountRole(account->accountId(), new_role)));

      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_EQ(0, roles->size());
    }

    /**
     * @given inserted role, domain
     * @when insert and delete account role
     * @then role is detached
     */
    TEST_F(AccountRoleTest, DeleteAccountRoleWhenExist) {
      ASSERT_TRUE(val(command->insertAccountRole(account->accountId(), role)));
      ASSERT_TRUE(val(command->deleteAccountRole(account->accountId(), role)));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_EQ(0, roles->size());
    }

    /**
     * @given inserted role, domain
     * @when no account exist
     * @then nothing is deleted
     */
    TEST_F(AccountRoleTest, DeleteAccountRoleWhenNoAccount) {
      ASSERT_TRUE(val(command->insertAccountRole(account->accountId(), role)));
      ASSERT_TRUE(val(command->deleteAccountRole("no", role)));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_EQ(1, roles->size());
    }

    /**
     * @given inserted role, domain
     * @when no role exist
     * @then nothing is deleted
     */
    TEST_F(AccountRoleTest, DeleteAccountRoleWhenNoRole) {
      ASSERT_TRUE(val(command->insertAccountRole(account->accountId(), role)));
      ASSERT_TRUE(val(command->deleteAccountRole(account->accountId(), "no")));
      auto roles = query->getAccountRoles(account->accountId());
      ASSERT_TRUE(roles);
      ASSERT_EQ(1, roles->size());
    }

    class AccountGrantablePermissionTest : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();

        permittee_account =
            clone(TestAccountBuilder()
                      .domainId(domain->domainId())
                      .accountId("id2@" + domain->domainId())
                      .quorum(1)
                      .jsonData(R"({"id@domain": {"key": "value"}})")
                      .build());

        ASSERT_TRUE(val(command->insertRole(role)));
        ASSERT_TRUE(val(command->insertDomain(*domain)));
        ASSERT_TRUE(val(command->insertAccount(*account)));
        ASSERT_TRUE(val(command->insertAccount(*permittee_account)));
      }

      std::shared_ptr<shared_model::interface::Account> permittee_account;
    };

    /**
     * @given WSV command and account exists and valid grantable permissions
     * @when trying to insert grantable permissions
     * @then grantable permissions are inserted
     */
    TEST_F(AccountGrantablePermissionTest,
           InsertAccountGrantablePermissionWhenAccountsExist) {
      ASSERT_TRUE(val(command->insertAccountGrantablePermission(
          permittee_account->accountId(),
          account->accountId(),
          grantable_permission)));

      ASSERT_TRUE(
          query->hasAccountGrantablePermission(permittee_account->accountId(),
                                               account->accountId(),
                                               grantable_permission));
    }

    /**
     * @given WSV command and invalid permittee and valid grantable permissions
     * @when trying to insert grantable permissions
     * @then grantable permissions are not inserted
     */
    TEST_F(AccountGrantablePermissionTest,
           InsertAccountGrantablePermissionWhenNoPermitteeAccount) {
      auto permittee_account_id = permittee_account->accountId() + " ";
      ASSERT_TRUE(err(command->insertAccountGrantablePermission(
          permittee_account_id, account->accountId(), grantable_permission)));

      ASSERT_FALSE(query->hasAccountGrantablePermission(
          permittee_account_id, account->accountId(), grantable_permission));
    }

    TEST_F(AccountGrantablePermissionTest,
           InsertAccountGrantablePermissionWhenNoAccount) {
      auto account_id = account->accountId() + " ";
      ASSERT_TRUE(err(command->insertAccountGrantablePermission(
          permittee_account->accountId(), account_id, grantable_permission)));

      ASSERT_FALSE(query->hasAccountGrantablePermission(
          permittee_account->accountId(), account_id, grantable_permission));
    }

    /**
     * @given WSV command to delete grantable permission with valid parameters
     * @when trying to delete grantable permissions
     * @then grantable permissions are deleted
     */
    TEST_F(AccountGrantablePermissionTest,
           DeleteAccountGrantablePermissionWhenAccountsPermissionExist) {
      ASSERT_TRUE(val(command->deleteAccountGrantablePermission(
          permittee_account->accountId(),
          account->accountId(),
          grantable_permission)));

      ASSERT_FALSE(
          query->hasAccountGrantablePermission(permittee_account->accountId(),
                                               account->accountId(),
                                               grantable_permission));
    }

    class DeletePeerTest : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();

        peer = clone(TestPeerBuilder().build());
      }
      std::unique_ptr<shared_model::interface::Peer> peer;
    };

    /**
     * @given storage with peer
     * @when trying to delete existing peer
     * @then peer is successfully deleted
     */
    TEST_F(DeletePeerTest, DeletePeerValidWhenPeerExists) {
      ASSERT_TRUE(val(command->insertPeer(*peer)));

      ASSERT_TRUE(val(command->deletePeer(*peer)));
    }

    class GetAssetTest : public WsvQueryCommandTest {};

    /**
     * @given database without needed asset
     * @when performing query to retrieve non-existent asset
     * @then getAsset will return nullopt
     */
    TEST_F(GetAssetTest, GetAssetInvalidWhenAssetDoesNotExist) {
      EXPECT_FALSE(query->getAsset("invalid asset"));
    }

    class GetDomainTest : public WsvQueryCommandTest {};

    /**
     * @given database without needed domain
     * @when performing query to retrieve non-existent asset
     * @then getAsset will return nullopt
     */
    TEST_F(GetDomainTest, GetDomainInvalidWhenDomainDoesNotExist) {
      EXPECT_FALSE(query->getDomain("invalid domain"));
    }

    // Since mocking database is not currently possible, use SetUp to create
    // invalid database
    class DatabaseInvalidTest : public WsvQueryCommandTest {
      // skip database setup
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

        command = std::make_unique<PostgresWsvCommand>(*wsv_transaction);
        query = std::make_unique<PostgresWsvQuery>(*wsv_transaction);
      }
    };

    /**
     * @given not set up database
     * @when performing query to retrieve information from nonexisting tables
     * @then query will return nullopt
     */
    TEST_F(DatabaseInvalidTest, QueryInvalidWhenDatabaseInvalid) {
      EXPECT_FALSE(query->getAccount("some account"));
    }

    class AddAccountAssetTest : public WsvQueryCommandTest {
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
     * @given WSV command
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
     * @given WSV command
     * @when trying to add account asset with non-existing asset
     * @then account asset fails to be added
     */
    TEST_F(AddAccountAssetTest, AddAccountAssetTestInvalidAsset) {
      ASSERT_FALSE(val(
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 1)));
    }

    /**
     * @given WSV command
     * @when trying to add account asset with non-existing account
     * @then account asset fails to added
     */
    TEST_F(AddAccountAssetTest, AddAccountAssetTestInvalidAccount) {
      addAsset();
      ASSERT_FALSE(
          val(executor->addAssetQuantity("some@domain", asset_id, "1", 1)));
    }

    /**
     * @given WSV command
     * @when trying to add account asset with wrong precision
     * @then account asset fails to added
     */
    TEST_F(AddAccountAssetTest, AddAccountAssetTestInvalidPrecision) {
      addAsset();
      ASSERT_FALSE(val(
          executor->addAssetQuantity(account->accountId(), asset_id, "1", 5)));
    }

    /**
     * @given WSV command
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
     * @given WSV command
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
     * @given WSV command
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
     * @given WSV command
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
     * @given WSV command and no target domain in ledger
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
     * @given WSV command ]
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
     * @given WSV command and no target domain in ledger
     * @when trying to create asset
     * @then asset is not created
     */
    TEST_F(CreateAsset, InvalidCreateAssetNoDomainTest) {
      ASSERT_TRUE(err(executor->createAsset("coin", domain->domainId(), 1)));
    }

    /**
     * @given WSV command ]
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
     * @given WSV command when there is no role
     * @when trying to create domain
     * @then domain is not created
     */
    TEST_F(CreateDomain, InvalidCreateDomainWhenNoRoleTest) {
      ASSERT_TRUE(err(executor->createDomain(account->accountId(), role)));
    }

    /**
     * @given WSV command when there is no role
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
     * @given WSV command
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
     * @given WSV command
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
     * @given WSV command
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
     * @given WSV command
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
     * @given WSV command
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
     * @given WSV command
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
     * @given WSV command
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
     * @given WSV command
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
     * @given WSV command
     * @when trying to subtract account asset with non-existing asset
     * @then account asset fails to be subtracted
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestInvalidAsset) {
      ASSERT_FALSE(val(executor->subtractAssetQuantity(
          account->accountId(), asset_id, "1", 1)));
    }

    /**
     * @given WSV command
     * @when trying to add account subtract with non-existing account
     * @then account asset fails to subtracted
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestInvalidAccount) {
      addAsset();
      ASSERT_FALSE(val(
          executor->subtractAssetQuantity("some@domain", asset_id, "1", 1)));
    }

    /**
     * @given WSV command
     * @when trying to add account asset with wrong precision
     * @then account asset fails to added
     */
    TEST_F(SubtractAccountAssetTest, SubtractAccountAssetTestInvalidPrecision) {
      addAsset();
      ASSERT_FALSE(val(executor->subtractAssetQuantity(
          account->accountId(), asset_id, "1", 5)));
    }

    /**
     * @given WSV command
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
     * @given WSV command
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
     * @given WSV command
     * @when trying to transfer account asset with non-existing asset
     * @then account asset fails to be transfered
     */
    TEST_F(TransferAccountAssetTest, TransferAccountAssetTestInvalidAsset) {
      ASSERT_FALSE(val(executor->transferAsset(
          account->accountId(), account2->accountId(), asset_id, "1", 1)));
    }

    /**
     * @given WSV command
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
     * @given WSV command
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
