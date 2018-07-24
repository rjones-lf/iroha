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
        sql = std::make_unique<soci::session>(soci::postgresql, pgopt_);

        command = std::make_unique<PostgresWsvCommand>(*sql);
        query = std::make_unique<PostgresWsvQuery>(*sql, factory);

        *sql << init_;
      }

      std::string role = "role";
      shared_model::interface::RolePermissionSet role_permissions;
      shared_model::interface::permissions::Grantable grantable_permission;
      std::unique_ptr<shared_model::interface::Account> account;
      std::unique_ptr<shared_model::interface::Domain> domain;

      std::unique_ptr<soci::session> sql;

      std::unique_ptr<WsvCommand> command;
      std::unique_ptr<WsvQuery> query;
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

    TEST_F(RoleTest, InsertTwoRole) {
      ASSERT_TRUE(val(command->insertRole("role")));
      ASSERT_TRUE(err(command->insertRole("role")));
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
      ASSERT_EQ(role_permissions, permissions.get());
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
     * @when update json data in account
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
      EXPECT_FALSE(query->getAccountDetail("invalid account id", "", ""));
    }

    /**
     * @given details, inserted for one account
     * @when performing query to retrieve all account's details
     * @then getAccountDetail will return all details of this account
     */
    TEST_F(AccountTest, GetAccountDetailWithAccount) {
      ASSERT_TRUE(val(command->insertAccount(*account)));
      ASSERT_TRUE(val(command->setAccountKV(
          account->accountId(), account->accountId(), "some_key", "some_val")));

      auto acc_details = query->getAccountDetail(account->accountId(), "", "");
      ASSERT_TRUE(acc_details);
      ASSERT_EQ(R"({"id@domain": {"key": "value", "some_key": "some_val"}})",
                *acc_details);
    }

    /**
     * @given details, inserted into one account by two writers, with one of the
     * keys repeated
     * @when performing query to retrieve details under this key
     * @then getAccountDetail will return details from both writers under the
     * specified key
     */
    TEST_F(AccountTest, GetAccountDetailWithKey) {
      ASSERT_TRUE(val(command->insertAccount(*account)));
      ASSERT_TRUE(val(command->setAccountKV(
          account->accountId(), account->accountId(), "some_key", "some_val")));
      ASSERT_TRUE(val(command->setAccountKV(account->accountId(),
                                            account->accountId(),
                                            "another_key",
                                            "another_val")));
      ASSERT_TRUE(val(command->setAccountKV(
          account->accountId(), "admin", "some_key", "even_third_val")));

      auto acc_details =
          query->getAccountDetail(account->accountId(), "some_key", "");
      ASSERT_TRUE(acc_details);
      ASSERT_EQ(
          "{ \"admin\" : {\"some_key\" : \"even_third_val\"}, "
            "\"id@domain\" : {\"some_key\" : \"some_val\"} }",
          *acc_details);
    }

    /**
     * @given details, inserted into one account by two writers
     * @when performing query to retrieve details, added by one of the writers
     * @then getAccountDetail will return only details, added by the specified
     * writer
     */
    TEST_F(AccountTest, GetAccountDetailWithWriter) {
      ASSERT_TRUE(val(command->insertAccount(*account)));
      ASSERT_TRUE(val(command->setAccountKV(
          account->accountId(), account->accountId(), "some_key", "some_val")));
      ASSERT_TRUE(val(command->setAccountKV(
          account->accountId(), "admin", "another_key", "another_val")));

      auto acc_details =
          query->getAccountDetail(account->accountId(), "", "admin");
      ASSERT_TRUE(acc_details);
      ASSERT_EQ(R"({"admin" : {"another_key": "another_val"}})",
                *acc_details);
    }

    /**
     * @given details, inserted into one account by two writers, with one of the
     * keys repeated
     * @when performing query to retrieve details under this key and added by
     * one of the writers
     * @then getAccountDetail will return only details, which are under the
     * specified key and added by the specified writer
     */
    TEST_F(AccountTest, GetAccountDetailWithKeyAndWriter) {
      ASSERT_TRUE(val(command->insertAccount(*account)));
      ASSERT_TRUE(val(command->setAccountKV(
          account->accountId(), account->accountId(), "some_key", "some_val")));
      ASSERT_TRUE(val(command->setAccountKV(account->accountId(),
                                            account->accountId(),
                                            "another_key",
                                            "another_val")));
      ASSERT_TRUE(val(command->setAccountKV(
          account->accountId(), "admin", "some_key", "even_third_val")));

      auto acc_details = query->getAccountDetail(
          account->accountId(), "some_key", account->accountId());
      ASSERT_TRUE(acc_details);
      ASSERT_EQ(R"({"id@domain" : {"some_key" : "some_val"}})",
                *acc_details);
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
        sql = std::make_unique<soci::session>(soci::postgresql, pgopt_);

        command = std::make_unique<PostgresWsvCommand>(*sql);
        query = std::make_unique<PostgresWsvQuery>(*sql, factory);
      }
    };

    std::unique_ptr<soci::session> sql;

    /**
     * @given not set up database
     * @when performing query to retrieve information from nonexisting tables
     * @then query will return nullopt
     */
    TEST_F(DatabaseInvalidTest, QueryInvalidWhenDatabaseInvalid) {
      EXPECT_FALSE(query->getAccount("some account"));
    }
  }  // namespace ametsuchi
}  // namespace iroha
