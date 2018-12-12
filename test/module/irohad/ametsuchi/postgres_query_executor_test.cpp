/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_query_executor.hpp"

#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include "ametsuchi/impl/postgres_command_executor.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "backend/protobuf/proto_query_response_factory.hpp"
#include "framework/result_fixture.hpp"
#include "interfaces/query_responses/account_asset_response.hpp"
#include "interfaces/query_responses/account_detail_response.hpp"
#include "interfaces/query_responses/account_response.hpp"
#include "interfaces/query_responses/asset_response.hpp"
#include "interfaces/query_responses/role_permissions.hpp"
#include "interfaces/query_responses/roles_response.hpp"
#include "interfaces/query_responses/signatories_response.hpp"
#include "interfaces/query_responses/transactions_response.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/pending_txs_storage/pending_txs_storage_mock.hpp"
#include "module/shared_model/builders/protobuf/test_account_builder.hpp"
#include "module/shared_model/builders/protobuf/test_asset_builder.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_domain_builder.hpp"
#include "module/shared_model/builders/protobuf/test_peer_builder.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "module/shared_model/mock_objects_factories/mock_command_factory.hpp"

namespace iroha {
  namespace ametsuchi {

    using namespace framework::expected;

    class QueryExecutorTest : public AmetsuchiTest {
     public:
      QueryExecutorTest() {
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
        pubkey2 = std::make_unique<shared_model::interface::types::PubkeyType>(
            std::string('2', 32));

        another_domain = clone(
            TestDomainBuilder().domainId("andomain").defaultRole(role).build());
        another_account =
            clone(TestAccountBuilder()
                      .domainId(another_domain->domainId())
                      .accountId("id@" + another_domain->domainId())
                      .quorum(1)
                      .jsonData(R"({"id@andomain": {"key": "value"}})")
                      .build());
        query_response_factory =
            std::make_shared<shared_model::proto::ProtoQueryResponseFactory>();
      }

      void SetUp() override {
        AmetsuchiTest::SetUp();
        sql = std::make_unique<soci::session>(soci::postgresql, pgopt_);

        auto factory =
            std::make_shared<shared_model::proto::ProtoCommonObjectsFactory<
                shared_model::validation::FieldValidator>>();
        query_executor = storage;
        PostgresCommandExecutor::prepareStatements(*sql);
        executor =
            std::make_unique<PostgresCommandExecutor>(*sql, perm_converter);
        pending_txs_storage = std::make_shared<MockPendingTransactionStorage>();

        execute(
            *mock_command_factory->constructCreateRole(role, role_permissions),
            true);
        execute(*mock_command_factory->constructCreateDomain(domain->domainId(),
                                                             role),
                true);
        execute(*mock_command_factory->constructCreateAccount(
                    "id", domain->domainId(), *pubkey),
                true);

        execute(*mock_command_factory->constructCreateDomain(
                    another_domain->domainId(), role),
                true);
        execute(*mock_command_factory->constructCreateAccount(
                    "id", another_domain->domainId(), *pubkey),
                true);
      }

      void TearDown() override {
        sql->close();
        AmetsuchiTest::TearDown();
      }

      auto executeQuery(shared_model::interface::Query &query) {
        return query_executor->createQueryExecutor(pending_txs_storage,
                                                   query_response_factory)
            | [&query](const auto &executor) {
                return executor->validateAndExecute(query);
              };
      }

      template <typename CommandType>
      void execute(CommandType &&command,
                   bool do_validation = false,
                   const shared_model::interface::types::AccountIdType
                       &creator = "id@domain") {
        executor->doValidation(not do_validation);
        executor->setCreatorAccountId(creator);
        ASSERT_TRUE(
            val(executor->operator()(std::forward<CommandType>(command))));
      }

      void addPerms(
          shared_model::interface::RolePermissionSet set,
          const shared_model::interface::types::AccountIdType account_id =
              "id@domain",
          const shared_model::interface::types::RoleIdType role_id = "perms") {
        execute(*mock_command_factory->constructCreateRole(role_id, set), true);
        execute(*mock_command_factory->constructAppendRole(account_id, role_id),
                true);
      }

      void addAllPerms(
          const shared_model::interface::types::AccountIdType account_id =
              "id@domain",
          const shared_model::interface::types::RoleIdType role_id = "all") {
        shared_model::interface::RolePermissionSet permissions;
        permissions.set();
        execute(
            *mock_command_factory->constructCreateRole(role_id, permissions),
            true);
        execute(*mock_command_factory->constructAppendRole(account_id, role_id),
                true);
      }

      // TODO [IR-1816] Akvinikym 06.12.18: remove these constants after
      // introducing a uniform way to use them in code
      static constexpr shared_model::interface::ErrorQueryResponse::
          ErrorCodeType kNoStatefulError = 0;
      static constexpr shared_model::interface::ErrorQueryResponse::
          ErrorCodeType kNoPermissions = 2;

      /**
       * Check that query response meets defined requirements
       * @tparam ExpectedQueryResponseType - expected type of that query
       * response
       * @tparam QueryResultCheckCallable - type of callable, which checks query
       * response
       * @param exec_result to be checked
       * @param check_callable - that check callable
       */
      template <typename ExpectedQueryResponseType,
                typename QueryResultCheckCallable>
      void checkSuccessfulResult(QueryExecutorResult exec_result,
                                 QueryResultCheckCallable check_callable) {
        ASSERT_NO_THROW({
          const auto &cast_resp =
              boost::get<const ExpectedQueryResponseType &>(exec_result->get());
          check_callable(cast_resp);
        }) << exec_result->toString();
      }

      /**
       * Check that stateful error in query response is the one expected
       * @tparam ExpectedQueryErrorType - expected sub-type of that query
       * response
       * @param exec_result to be checked
       * @param expected_code, which is to be in the query response
       */
      template <typename ExpectedQueryErrorType>
      void checkStatefulError(
          QueryExecutorResult exec_result,
          shared_model::interface::ErrorQueryResponse::ErrorCodeType
              expected_code) {
        ASSERT_NO_THROW({
          const auto &error_qry_rsp =
              boost::get<const shared_model::interface::ErrorQueryResponse &>(
                  exec_result->get());
          ASSERT_EQ(error_qry_rsp.errorCode(), expected_code);
          boost::get<const ExpectedQueryErrorType &>(error_qry_rsp.get());
        }) << exec_result->toString();
      }

      void createDefaultAccount() {
        execute(*mock_command_factory->constructCreateAccount(
                    "id2", domain->domainId(), *pubkey2),
                true);
      }

      void createDefaultAsset() {
        execute(*mock_command_factory->constructCreateAsset(
                    "coin", domain->domainId(), 1),
                true);
      }

      std::string role = "role";
      shared_model::interface::RolePermissionSet role_permissions;
      shared_model::interface::permissions::Grantable grantable_permission;
      std::unique_ptr<shared_model::interface::Account> account,
          another_account;
      std::unique_ptr<shared_model::interface::Domain> domain, another_domain;
      std::unique_ptr<shared_model::interface::types::PubkeyType> pubkey;
      std::unique_ptr<shared_model::interface::types::PubkeyType> pubkey2;

      std::unique_ptr<soci::session> sql;

      std::unique_ptr<shared_model::interface::Command> command;

      std::shared_ptr<QueryExecutorFactory> query_executor;
      std::unique_ptr<CommandExecutor> executor;
      std::shared_ptr<MockPendingTransactionStorage> pending_txs_storage;

      std::unique_ptr<KeyValueStorage> block_store;

      std::shared_ptr<shared_model::interface::QueryResponseFactory>
          query_response_factory;

      std::shared_ptr<shared_model::interface::PermissionToString>
          perm_converter =
              std::make_shared<shared_model::proto::ProtoPermissionToString>();

      std::unique_ptr<shared_model::interface::MockCommandFactory>
          mock_command_factory =
              std::make_unique<shared_model::interface::MockCommandFactory>();
    };

    class BlocksQueryExecutorTest : public QueryExecutorTest {};

    /**
     * @given permissions to get blocks
     * @when get blocks query is validated
     * @then result is successful
     */
    TEST_F(BlocksQueryExecutorTest, BlocksQueryExecutorTestValid) {
      addAllPerms();
      auto blocks_query = TestBlocksQueryBuilder()
                              .creatorAccountId(account->accountId())
                              .build();
      ASSERT_TRUE(query_executor->createQueryExecutor(pending_txs_storage,
                                                      query_response_factory)
                  | [&blocks_query](const auto &executor) {
                      return executor->validate(blocks_query);
                    });
    }

    /**
     * @given no permissions to get blocks given
     * @when get blocks query is validated
     * @then result is error
     */
    TEST_F(BlocksQueryExecutorTest, BlocksQueryExecutorTestInvalid) {
      auto blocks_query = TestBlocksQueryBuilder()
                              .creatorAccountId(account->accountId())
                              .build();
      ASSERT_FALSE(query_executor->createQueryExecutor(pending_txs_storage,
                                                       query_response_factory)
                   | [&blocks_query](const auto &executor) {
                       return executor->validate(blocks_query);
                     });
    }

    class GetAccountExecutorTest : public QueryExecutorTest {
     public:
      void SetUp() override {
        QueryExecutorTest::SetUp();
        account2 = clone(TestAccountBuilder()
                             .domainId(domain->domainId())
                             .accountId("id2@" + domain->domainId())
                             .quorum(1)
                             .jsonData(R"({"id@domain": {"key": "value"}})")
                             .build());
        createDefaultAccount();
      }

      std::unique_ptr<shared_model::interface::Account> account2;
    };

    /**
     * @given initialized storage, permission to his/her account
     * @when get account information
     * @then Return account
     */
    TEST_F(GetAccountExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMyAccount});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccount(account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.account().accountId(), account->accountId());
          });
    }

    /**
     * @given initialized storage, global permission
     * @when get account information about other user
     * @then Return account
     */
    TEST_F(GetAccountExecutorTest, ValidAllAccounts) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccounts});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccount(account2->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.account().accountId(), account2->accountId());
          });
    }

    /**
     * @given initialized storage, domain permission
     * @when get account information about other user in the same domain
     * @then Return account
     */
    TEST_F(GetAccountExecutorTest, ValidDomainAccount) {
      addPerms(
          {shared_model::interface::permissions::Role::kGetDomainAccounts});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccount(account2->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.account().accountId(), account2->accountId());
          });
    }

    /**
     * @given initialized storage, domain permission
     * @when get account information about other user in the other domain
     * @then Return error
     */
    TEST_F(GetAccountExecutorTest, InvalidDifferentDomain) {
      addPerms(
          {shared_model::interface::permissions::Role::kGetDomainAccounts});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccount(another_account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::StatefulFailedErrorResponse>(
          std::move(result), kNoPermissions);
    }

    /**
     * @given initialized storage, permission
     * @when get account information about non existing account
     * @then Return error
     */
    TEST_F(GetAccountExecutorTest, InvalidNoAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccounts});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccount("some@domain")
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::NoAccountErrorResponse>(
          std::move(result), kNoStatefulError);
    }

    class GetSignatoriesExecutorTest : public QueryExecutorTest {
     public:
      void SetUp() override {
        QueryExecutorTest::SetUp();
        account2 = clone(TestAccountBuilder()
                             .domainId(domain->domainId())
                             .accountId("id2@" + domain->domainId())
                             .quorum(1)
                             .jsonData(R"({"id@domain": {"key": "value"}})")
                             .build());
        createDefaultAccount();
      }

      std::unique_ptr<shared_model::interface::Account> account2;
    };

    /**
     * @given initialized storage, permission to his/her account
     * @when get signatories
     * @then Return signatories of user
     */
    TEST_F(GetSignatoriesExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMySignatories});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getSignatories(account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::SignatoriesResponse>(
          std::move(result),
          [](const auto &cast_resp) { ASSERT_EQ(cast_resp.keys().size(), 1); });
    }

    /**
     * @given initialized storage, global permission
     * @when get signatories of other user
     * @then Return signatories
     */
    TEST_F(GetSignatoriesExecutorTest, ValidAllAccounts) {
      addPerms(
          {shared_model::interface::permissions::Role::kGetAllSignatories});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getSignatories(account2->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::SignatoriesResponse>(
          std::move(result),
          [](const auto &cast_resp) { ASSERT_EQ(cast_resp.keys().size(), 1); });
    }

    /**
     * @given initialized storage, domain permission
     * @when get signatories of other user in the same domain
     * @then Return signatories
     */
    TEST_F(GetSignatoriesExecutorTest, ValidDomainAccount) {
      addPerms(
          {shared_model::interface::permissions::Role::kGetDomainSignatories});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getSignatories(account2->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::SignatoriesResponse>(
          std::move(result),
          [](const auto &cast_resp) { ASSERT_EQ(cast_resp.keys().size(), 1); });
    }

    /**
     * @given initialized storage, domain permission
     * @when get signatories of other user in the other domain
     * @then Return error
     */
    TEST_F(GetSignatoriesExecutorTest, InvalidDifferentDomain) {
      addPerms(
          {shared_model::interface::permissions::Role::kGetDomainAccounts});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getSignatories(another_account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::StatefulFailedErrorResponse>(
          std::move(result), kNoPermissions);
    }

    /**
     * @given initialized storage, permission
     * @when get signatories of non existing account
     * @then Return error
     */
    TEST_F(GetSignatoriesExecutorTest, InvalidNoAccount) {
      addPerms(
          {shared_model::interface::permissions::Role::kGetAllSignatories});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getSignatories("some@domain")
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::NoSignatoriesErrorResponse>(
          std::move(result), kNoStatefulError);
    }

    class GetAccountAssetExecutorTest : public QueryExecutorTest {
     public:
      void SetUp() override {
        QueryExecutorTest::SetUp();

        auto asset = clone(TestAccountAssetBuilder()
                               .domainId(domain->domainId())
                               .assetId(asset_id)
                               .precision(1)
                               .build());
        account2 = clone(TestAccountBuilder()
                             .domainId(domain->domainId())
                             .accountId("id2@" + domain->domainId())
                             .quorum(1)
                             .jsonData(R"({"id@domain": {"key": "value"}})")
                             .build());
        createDefaultAccount();
        createDefaultAsset();

        execute(*mock_command_factory->constructAddAssetQuantity(
                    asset_id, shared_model::interface::Amount{"1.0"}),
                true);
        execute(*mock_command_factory->constructAddAssetQuantity(
                    asset_id, shared_model::interface::Amount{"1.0"}),
                true,
                account2->accountId());
      }

      std::unique_ptr<shared_model::interface::Account> account2;
      shared_model::interface::types::AssetIdType asset_id =
          "coin#" + domain->domainId();
    };

    /**
     * @given initialized storage, permission to his/her account
     * @when get account assets
     * @then Return account asset of user
     */
    TEST_F(GetAccountAssetExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMyAccAst});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountAssets(account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountAssetResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.accountAssets()[0].accountId(),
                      account->accountId());
            ASSERT_EQ(cast_resp.accountAssets()[0].assetId(), asset_id);
          });
    }

    /**
     * @given initialized storage, global permission
     * @when get account assets of other user
     * @then Return account asset
     */
    TEST_F(GetAccountAssetExecutorTest, ValidAllAccounts) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccAst});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountAssets(account2->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountAssetResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.accountAssets()[0].accountId(),
                      account2->accountId());
            ASSERT_EQ(cast_resp.accountAssets()[0].assetId(), asset_id);
          });
    }

    /**
     * @given initialized storage, domain permission
     * @when get account assets of other user in the same domain
     * @then Return account asset
     */
    TEST_F(GetAccountAssetExecutorTest, ValidDomainAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetDomainAccAst});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountAssets(account2->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountAssetResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.accountAssets()[0].accountId(),
                      account2->accountId());
            ASSERT_EQ(cast_resp.accountAssets()[0].assetId(), asset_id);
          });
    }

    /**
     * @given initialized storage, domain permission
     * @when get account assets of other user in the other domain
     * @then Return error
     */
    TEST_F(GetAccountAssetExecutorTest, InvalidDifferentDomain) {
      addPerms({shared_model::interface::permissions::Role::kGetDomainAccAst});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountAssets(another_account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::StatefulFailedErrorResponse>(
          std::move(result), kNoPermissions);
    }

    /**
     * @given initialized storage, permission
     * @when get account assets of non existing account
     * @then Return error
     */
    TEST_F(GetAccountAssetExecutorTest, DISABLED_InvalidNoAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccAst});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountAssets("some@domain")
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::NoAccountAssetsErrorResponse>(
          std::move(result), kNoStatefulError);
    }

    class GetAccountDetailExecutorTest : public QueryExecutorTest {
     public:
      void SetUp() override {
        QueryExecutorTest::SetUp();

        account2 = clone(TestAccountBuilder()
                             .domainId(domain->domainId())
                             .accountId("id2@" + domain->domainId())
                             .quorum(1)
                             .jsonData("{\"id@domain\": {\"key\": \"value\", "
                                       "\"key2\": \"value2\"},"
                                       " \"id2@domain\": {\"key\": \"value\", "
                                       "\"key2\": \"value2\"}}")
                             .build());
        createDefaultAccount();
        createDefaultAsset();

        execute(*mock_command_factory->constructSetAccountDetail(
                    account2->accountId(), "key", "value"),
                true,
                account->accountId());
        execute(*mock_command_factory->constructSetAccountDetail(
                    account2->accountId(), "key2", "value2"),
                true,
                account->accountId());
        execute(*mock_command_factory->constructSetAccountDetail(
                    account2->accountId(), "key", "value"),
                true,
                account2->accountId());
        execute(*mock_command_factory->constructSetAccountDetail(
                    account2->accountId(), "key2", "value2"),
                true,
                account2->accountId());
      }

      std::unique_ptr<shared_model::interface::Account> account2;
    };

    /**
     * @given initialized storage, permission to his/her account
     * @when get account detail
     * @then Return account detail
     */
    TEST_F(GetAccountDetailExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMyAccDetail});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountDetail(account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountDetailResponse>(
          std::move(result),
          [](const auto &cast_resp) { ASSERT_EQ(cast_resp.detail(), "{}"); });
    }

    /**
     * @given initialized storage, global permission
     * @when get account detail of other user
     * @then Return account detail
     */
    TEST_F(GetAccountDetailExecutorTest, ValidAllAccounts) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccDetail});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountDetail(account2->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountDetailResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.detail(), account2->jsonData());
          });
    }

    /**
     * @given initialized storage, domain permission
     * @when get account detail of other user in the same domain
     * @then Return account detail
     */
    TEST_F(GetAccountDetailExecutorTest, ValidDomainAccount) {
      addPerms(
          {shared_model::interface::permissions::Role::kGetDomainAccDetail});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountDetail(account2->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountDetailResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.detail(), account2->jsonData());
          });
    }

    /**
     * @given initialized storage, domain permission
     * @when get account detail of other user in the other domain
     * @then Return error
     */
    TEST_F(GetAccountDetailExecutorTest, InvalidDifferentDomain) {
      addPerms(
          {shared_model::interface::permissions::Role::kGetDomainAccDetail});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountDetail(another_account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::StatefulFailedErrorResponse>(
          std::move(result), kNoPermissions);
    }

    /**
     * @given initialized storage, permission
     * @when get account detail of non existing account
     * @then Return error
     */
    TEST_F(GetAccountDetailExecutorTest, InvalidNoAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccDetail});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountDetail("some@domain")
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::NoAccountDetailErrorResponse>(
          std::move(result), kNoStatefulError);
    }

    /**
     * @given details, inserted into one account by two writers, with one of the
     * keys repeated
     * @when performing query to retrieve details under this key
     * @then getAccountDetail will return details from both writers under the
     * specified key
     */
    TEST_F(GetAccountDetailExecutorTest, ValidKey) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccDetail});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountDetail(account2->accountId(), "key")
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountDetailResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.detail(),
                      R"({ "id@domain" : {"key" : "value"}, )"
                      R"("id2@domain" : {"key" : "value"} })");
          });
    }

    /**
     * @given details, inserted into one account by two writers
     * @when performing query to retrieve details, added by one of the writers
     * @then getAccountDetail will return only details, added by the specified
     * writer
     */
    TEST_F(GetAccountDetailExecutorTest, ValidWriter) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccDetail});
      auto query =
          TestQueryBuilder()
              .creatorAccountId(account->accountId())
              .getAccountDetail(account2->accountId(), "", account->accountId())
              .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountDetailResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.detail(),
                      R"({"id@domain" : {"key": "value", "key2": "value2"}})");
          });
    }

    /**
     * @given details, inserted into one account by two writers, with one of the
     * keys repeated
     * @when performing query to retrieve details under this key and added by
     * one of the writers
     * @then getAccountDetail will return only details, which are under the
     * specified key and added by the specified writer
     */
    TEST_F(GetAccountDetailExecutorTest, ValidKeyWriter) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccDetail});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountDetail(
                           account2->accountId(), "key", account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountDetailResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.detail(),
                      R"({"id@domain" : {"key" : "value"}})");
          });
    }

    class GetRolesExecutorTest : public QueryExecutorTest {
     public:
      void SetUp() override {
        QueryExecutorTest::SetUp();
      }
    };

    /**
     * @given initialized storage, permission to read all roles
     * @when get system roles
     * @then Return roles
     */
    TEST_F(GetRolesExecutorTest, Valid) {
      addPerms({shared_model::interface::permissions::Role::kGetRoles});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getRoles()
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::RolesResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.roles().size(), 2);
            ASSERT_EQ(cast_resp.roles()[0], "role");
            ASSERT_EQ(cast_resp.roles()[1], "perms");
          });
    }

    /**
     * @given initialized storage, no permission to read all roles
     * @when get system roles
     * @then Return Error
     */
    TEST_F(GetRolesExecutorTest, Invalid) {
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getRoles()
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::StatefulFailedErrorResponse>(
          std::move(result), kNoPermissions);
    }

    class GetRolePermsExecutorTest : public QueryExecutorTest {
     public:
      void SetUp() override {
        QueryExecutorTest::SetUp();
      }
    };

    /**
     * @given initialized storage, permission to read all roles
     * @when get role permissions
     * @then Return role permissions
     */
    TEST_F(GetRolePermsExecutorTest, Valid) {
      addPerms({shared_model::interface::permissions::Role::kGetRoles});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getRolePermissions("perms")
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::RolePermissionsResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_TRUE(cast_resp.rolePermissions().test(
                shared_model::interface::permissions::Role::kGetRoles));
          });
    }

    /**
     * @given initialized storage, permission to read all roles, role does not
     * exist
     * @when get role permissions
     * @then Return error
     */
    TEST_F(GetRolePermsExecutorTest, InvalidNoRole) {
      addPerms({shared_model::interface::permissions::Role::kGetRoles});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getRolePermissions("some")
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::NoRolesErrorResponse>(
          std::move(result), kNoStatefulError);
    }

    /**
     * @given initialized storage, no permission to read all roles
     * @when get role permissions
     * @then Return error
     */
    TEST_F(GetRolePermsExecutorTest, Invalid) {
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getRolePermissions("role")
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::StatefulFailedErrorResponse>(
          std::move(result), kNoPermissions);
    }

    class GetAssetInfoExecutorTest : public QueryExecutorTest {
     public:
      void SetUp() override {
        QueryExecutorTest::SetUp();
      }

      void createAsset() {
        execute(*mock_command_factory->constructCreateAsset(
                    "coin", domain->domainId(), 1),
                true);
      }
      const std::string asset_id = "coin#domain";
    };

    /**
     * @given initialized storage, permission to read all system assets
     * @when get asset info
     * @then Return asset
     */
    TEST_F(GetAssetInfoExecutorTest, Valid) {
      addPerms({shared_model::interface::permissions::Role::kReadAssets});
      createAsset();
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAssetInfo(asset_id)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AssetResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.asset().assetId(), asset_id);
            ASSERT_EQ(cast_resp.asset().domainId(), domain->domainId());
            ASSERT_EQ(cast_resp.asset().precision(), 1);
          });
    }

    /**
     * @given initialized storage, all permissions
     * @when get asset info of non existing asset
     * @then Error
     */
    TEST_F(GetAssetInfoExecutorTest, InvalidNoAsset) {
      addPerms({shared_model::interface::permissions::Role::kReadAssets});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAssetInfo("some#domain")
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::NoAssetErrorResponse>(
          std::move(result), kNoStatefulError);
    }

    /**
     * @given initialized storage, no permissions
     * @when get asset info
     * @then Error
     */
    TEST_F(GetAssetInfoExecutorTest, Invalid) {
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAssetInfo(asset_id)
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::StatefulFailedErrorResponse>(
          std::move(result), kNoPermissions);
    }

    class GetTransactionsExecutorTest : public QueryExecutorTest {
     public:
      void SetUp() override {
        QueryExecutorTest::SetUp();
        std::string block_store_dir = "/tmp/block_store";
        auto block_converter =
            std::make_shared<shared_model::proto::ProtoBlockJsonConverter>();
        auto factory =
            std::make_shared<shared_model::proto::ProtoCommonObjectsFactory<
                shared_model::validation::FieldValidator>>();
        auto block_store = FlatFile::create(block_store_dir);
        ASSERT_TRUE(block_store);
        this->block_store = std::move(block_store.get());

        account2 = clone(TestAccountBuilder()
                             .domainId(domain->domainId())
                             .accountId("id2@" + domain->domainId())
                             .quorum(1)
                             .jsonData(R"({"id@domain": {"key": "value"}})")
                             .build());
        createDefaultAccount();
        createDefaultAsset();
      }

      /**
       * Apply block to given storage
       * @tparam S storage type
       * @param storage storage object
       * @param block to apply
       */
      template <typename S>
      void apply(S &&storage, const shared_model::interface::Block &block) {
        std::unique_ptr<MutableStorage> ms;
        auto storageResult = storage->createMutableStorage();
        storageResult.match(
            [&](iroha::expected::Value<std::unique_ptr<MutableStorage>>
                    &_storage) { ms = std::move(_storage.value); },
            [](iroha::expected::Error<std::string> &error) {
              FAIL() << "MutableStorage: " << error.error;
            });
        ms->apply(block);
        storage->commit(std::move(ms));
      }

      void commitBlocks() {
        auto zero_string = std::string(32, '0');
        auto fake_hash = shared_model::crypto::Hash(zero_string);
        auto fake_pubkey = shared_model::crypto::PublicKey(zero_string);

        std::vector<shared_model::proto::Transaction> txs1;
        txs1.push_back(TestTransactionBuilder()
                           .creatorAccountId(account->accountId())
                           .createRole("user", {})
                           .build());
        txs1.push_back(TestTransactionBuilder()
                           .creatorAccountId(account->accountId())
                           .addAssetQuantity(asset_id, "2.0")
                           .transferAsset(account->accountId(),
                                          account2->accountId(),
                                          asset_id,
                                          "",
                                          "1.0")
                           .build());
        txs1.push_back(TestTransactionBuilder()
                           .creatorAccountId(account2->accountId())
                           .createRole("user2", {})
                           .build());

        auto block1 = TestBlockBuilder()
                          .transactions(txs1)
                          .height(1)
                          .prevHash(fake_hash)
                          .build();

        apply(storage, block1);

        std::vector<shared_model::proto::Transaction> txs2;
        txs2.push_back(TestTransactionBuilder()
                           .creatorAccountId(account2->accountId())
                           .transferAsset(account->accountId(),
                                          account2->accountId(),
                                          asset_id,
                                          "",
                                          "1.0")
                           .build());
        txs2.push_back(TestTransactionBuilder()
                           .creatorAccountId(account->accountId())
                           .createRole("user3", {})
                           .build());

        auto block2 = TestBlockBuilder()
                          .transactions(txs2)
                          .height(2)
                          .prevHash(block1.hash())
                          .build();

        apply(storage, block2);

        hash1 = txs1.at(0).hash();
        hash2 = txs1.at(1).hash();
        hash3 = txs2.at(0).hash();
      }

      const std::string asset_id = "coin#domain";
      std::unique_ptr<shared_model::interface::Account> account2;
      shared_model::crypto::Hash hash1;
      shared_model::crypto::Hash hash2;
      shared_model::crypto::Hash hash3;
    };

    class GetAccountTransactionsExecutorTest
        : public GetTransactionsExecutorTest {};

    /**
     * @given initialized storage, permission to his/her account
     * @when get account transactions
     * @then Return account transactions of user
     */
    TEST_F(GetAccountTransactionsExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMyAccTxs});

      commitBlocks();

      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountTransactions(account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.transactions().size(), 3);
            for (const auto &tx : cast_resp.transactions()) {
              static size_t i = 0;
              EXPECT_EQ(account->accountId(), tx.creatorAccountId())
                  << tx.toString() << " ~~ " << i;
              ++i;
            }
          });
    }

    /**
     * @given initialized storage, global permission
     * @when get account transactions of other user
     * @then Return account transactions
     */
    TEST_F(GetAccountTransactionsExecutorTest, ValidAllAccounts) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccTxs});

      commitBlocks();

      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountTransactions(account2->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.transactions().size(), 2);
            for (const auto &tx : cast_resp.transactions()) {
              EXPECT_EQ(account2->accountId(), tx.creatorAccountId())
                  << tx.toString();
            }
          });
    }

    /**
     * @given initialized storage, domain permission
     * @when get account transactions of other user in the same domain
     * @then Return account transactions
     */
    TEST_F(GetAccountTransactionsExecutorTest, ValidDomainAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetDomainAccTxs});

      commitBlocks();

      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountTransactions(account2->accountId())
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.transactions().size(), 2);
            for (const auto &tx : cast_resp.transactions()) {
              EXPECT_EQ(account2->accountId(), tx.creatorAccountId())
                  << tx.toString();
            }
          });
    }

    /**
     * @given initialized storage, domain permission
     * @when get account transactions of other user in the other domain
     * @then Return error
     */
    TEST_F(GetAccountTransactionsExecutorTest, InvalidDifferentDomain) {
      addPerms({shared_model::interface::permissions::Role::kGetDomainAccTxs});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountTransactions(another_account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::StatefulFailedErrorResponse>(
          std::move(result), kNoPermissions);
    }

    /**
     * @given initialized storage, permission
     * @when get account transactions of non existing account
     * @then Return empty account transactions
     */
    TEST_F(GetAccountTransactionsExecutorTest, DISABLED_InvalidNoAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccTxs});

      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountTransactions("some@domain")
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::StatefulFailedErrorResponse>(
          std::move(result), kNoStatefulError);
    }

    class GetTransactionsHashExecutorTest : public GetTransactionsExecutorTest {
    };

    /**
     * @given initialized storage, global permission
     * @when get transactions of other user
     * @then Return transactions
     */
    TEST_F(GetTransactionsHashExecutorTest, ValidAllAccounts) {
      addPerms({shared_model::interface::permissions::Role::kGetAllTxs});

      commitBlocks();

      std::vector<decltype(hash3)> hashes;
      hashes.push_back(hash3);

      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getTransactions(hashes)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.transactions().size(), 1);
            ASSERT_EQ(cast_resp.transactions()[0].hash(), hash3);
          });
    }

    /**
     * @given initialized storage, permission to his/her account
     * @when get transactions
     * @then Return transactions of user
     */
    TEST_F(GetTransactionsHashExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMyTxs});

      commitBlocks();

      std::vector<decltype(hash1)> hashes;
      hashes.push_back(hash1);
      hashes.push_back(hash2);
      hashes.push_back(hash3);

      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getTransactions(hashes)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.transactions().size(), 2);
            ASSERT_EQ(cast_resp.transactions()[0].hash(), hash1);
            ASSERT_EQ(cast_resp.transactions()[1].hash(), hash2);
          });
    }

    class GetAccountAssetTransactionsExecutorTest
        : public GetTransactionsExecutorTest {};

    /**
     * @given initialized storage, permission to his/her account
     * @when get account asset transactions
     * @then Return account asset transactions of user
     */
    TEST_F(GetAccountAssetTransactionsExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMyAccAstTxs});

      commitBlocks();

      auto query =
          TestQueryBuilder()
              .creatorAccountId(account->accountId())
              .getAccountAssetTransactions(account->accountId(), asset_id)
              .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.transactions().size(), 2);
            ASSERT_EQ(cast_resp.transactions()[0].hash(), hash2);
            ASSERT_EQ(cast_resp.transactions()[1].hash(), hash3);
          });
    }

    /**
     * @given initialized storage, global permission
     * @when get account asset transactions of other user
     * @then Return account asset transactions
     */
    TEST_F(GetAccountAssetTransactionsExecutorTest, ValidAllAccounts) {
      addPerms({shared_model::interface::permissions::Role::kGetAllAccAstTxs});

      commitBlocks();

      auto query =
          TestQueryBuilder()
              .creatorAccountId(account->accountId())
              .getAccountAssetTransactions(account2->accountId(), asset_id)
              .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.transactions().size(), 2);
            ASSERT_EQ(cast_resp.transactions()[0].hash(), hash2);
            ASSERT_EQ(cast_resp.transactions()[1].hash(), hash3);
          });
    }

    /**
     * @given initialized storage, domain permission
     * @when get account asset transactions of other user in the same domain
     * @then Return account asset transactions
     */
    TEST_F(GetAccountAssetTransactionsExecutorTest, ValidDomainAccount) {
      addPerms(
          {shared_model::interface::permissions::Role::kGetDomainAccAstTxs});

      commitBlocks();

      auto query =
          TestQueryBuilder()
              .creatorAccountId(account->accountId())
              .getAccountAssetTransactions(account2->accountId(), asset_id)
              .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.transactions().size(), 2);
            ASSERT_EQ(cast_resp.transactions()[0].hash(), hash2);
            ASSERT_EQ(cast_resp.transactions()[1].hash(), hash3);
          });
    }

    /**
     * @given initialized storage, domain permission
     * @when get account asset transactions of other user in the other domain
     * @then Return error
     */
    TEST_F(GetAccountAssetTransactionsExecutorTest, InvalidDifferentDomain) {
      addPerms(
          {shared_model::interface::permissions::Role::kGetDomainAccAstTxs});

      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getAccountTransactions(another_account->accountId())
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::StatefulFailedErrorResponse>(
          std::move(result), kNoPermissions);
    }

    /**
     * @given initialized storage
     * @when get pending transactions
     * @then pending txs storage will be requested for query creator account
     */
    TEST_F(QueryExecutorTest, TransactionsStorageIsAccessed) {
      auto query = TestQueryBuilder()
                       .creatorAccountId(account->accountId())
                       .getPendingTransactions()
                       .build();

      EXPECT_CALL(*pending_txs_storage,
                  getPendingTransactions(account->accountId()))
          .Times(1);

      executeQuery(query);
    }

  }  // namespace ametsuchi
}  // namespace iroha
