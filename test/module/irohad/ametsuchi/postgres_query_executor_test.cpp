/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_query_executor.hpp"

#include <chrono>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <type_traits>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/size.hpp>
#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include "ametsuchi/impl/postgres_command_executor.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "backend/protobuf/proto_query_response_factory.hpp"
#include "datetime/time.hpp"
#include "framework/result_fixture.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/permissions.hpp"
#include "interfaces/query_responses/account_asset_response.hpp"
#include "interfaces/query_responses/account_detail_response.hpp"
#include "interfaces/query_responses/account_response.hpp"
#include "interfaces/query_responses/asset_response.hpp"
#include "interfaces/query_responses/role_permissions.hpp"
#include "interfaces/query_responses/roles_response.hpp"
#include "interfaces/query_responses/signatories_response.hpp"
#include "interfaces/query_responses/transactions_page_response.hpp"
#include "interfaces/query_responses/transactions_response.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/pending_txs_storage/pending_txs_storage_mock.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace framework::expected;
using namespace shared_model::interface;

namespace shared_model {
  namespace crypto {
    void PrintTo(const shared_model::crypto::Hash &hash, std::ostream *os) {
      *os << hash.toString();
    }
  }  // namespace crypto
}  // namespace shared_model

namespace {
  constexpr types::TransactionsNumberType kTxPageSize(10);
  constexpr types::PrecisionType kAssetPrecision(1);
  // TODO mboldyrev 05.12.2018 IR-57 unify the common constants.
  constexpr size_t kHashLength = 32;
  const std::string zero_string{kHashLength, '0'};
  const std::string asset_id = "coin#domain";
  const std::string role = "role";
  const shared_model::interface::types::DomainIdType domain_id = "domain";
  const shared_model::interface::types::DomainIdType another_domain_id =
      "andomain";
  const shared_model::interface::types::AccountIdType account_id =
      "id@" + domain_id;
  const shared_model::interface::types::AccountIdType another_account_id =
      "id@" + another_domain_id;
  const shared_model::interface::types::AccountIdType account_id2 = "id2@" + domain_id;
}  // namespace

namespace iroha {
  namespace ametsuchi {

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

    class QueryExecutorTest : public AmetsuchiTest {
     public:
      QueryExecutorTest() {
        role_permissions.set(
            shared_model::interface::permissions::Role::kAddMySignatory);
        grantable_permission =
            shared_model::interface::permissions::Grantable::kAddMySignatory;
        pubkey = std::make_unique<shared_model::interface::types::PubkeyType>(
            std::string('1', 32));

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

        auto result = execute(buildCommand(TestTransactionBuilder().createRole(
                                  role, role_permissions)),
                              true);
        ASSERT_TRUE(val(result)) << err(result)->error.toString();
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            domain_id, role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", domain_id, *pubkey)),
                        true)));

        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createDomain(
                            another_domain_id, role)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id", another_domain_id, *pubkey)),
                        true)));
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
       * Helper function to build command and wrap it into
       * std::unique_ptr<>
       * @param builder command builder
       * @return command
       */
      std::unique_ptr<shared_model::interface::Command> buildCommand(
          const TestTransactionBuilder &builder) {
        return clone(builder.build().commands().front());
      }

      void addPerms(
          shared_model::interface::RolePermissionSet set,
          const shared_model::interface::types::AccountIdType account_id =
              "id@domain",
          const shared_model::interface::types::RoleIdType role_id = "perms") {
        ASSERT_TRUE(val(execute(
            buildCommand(TestTransactionBuilder().createRole(role_id, set)),
            true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().appendRole(
                            account_id, role_id)),
                        true)));
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

      // TODO [IR-1816] Akvinikym 06.12.18: remove these constants after
      // introducing a uniform way to use them in code
      static constexpr shared_model::interface::ErrorQueryResponse::
          ErrorCodeType kNoStatefulError = 0;
      static constexpr shared_model::interface::ErrorQueryResponse::
          ErrorCodeType kNoPermissions = 2;
      static constexpr shared_model::interface::ErrorQueryResponse::
          ErrorCodeType kInvalidPagination = 4;

      std::string role = "role";
      shared_model::interface::RolePermissionSet role_permissions;
      shared_model::interface::permissions::Grantable grantable_permission;

      std::unique_ptr<shared_model::interface::types::PubkeyType> pubkey;

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
    };

    class BlocksQueryExecutorTest : public QueryExecutorTest {};

    /**
     * @given permissions to get blocks
     * @when get blocks query is validated
     * @then result is successful
     */
    TEST_F(BlocksQueryExecutorTest, BlocksQueryExecutorTestValid) {
      addAllPerms();
      auto blocks_query =
          TestBlocksQueryBuilder().creatorAccountId(account_id).build();
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
      auto blocks_query =
          TestBlocksQueryBuilder().creatorAccountId(account_id).build();
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
        auto pubkey2 =
            std::make_unique<shared_model::interface::types::PubkeyType>(
                std::string('2', 32));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id2", domain_id, *pubkey2)),
                        true)));
      }
    };

    /**
     * @given initialized storage, permission to his/her account
     * @when get account information
     * @then Return account
     */
    TEST_F(GetAccountExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMyAccount});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account_id)
                       .getAccount(account_id)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.account().accountId(), account_id);
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
                       .creatorAccountId(account_id)
                       .getAccount(another_account_id)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.account().accountId(), another_account_id);
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
                       .creatorAccountId(account_id)
                       .getAccount(account_id2)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.account().accountId(), account_id2);
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
                       .creatorAccountId(account_id)
                       .getAccount(another_account_id)
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
                       .creatorAccountId(account_id)
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
        auto pubkey2 =
            std::make_unique<shared_model::interface::types::PubkeyType>(
                std::string('2', 32));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id2", domain_id, *pubkey2)),
                        true)));
      }
    };

    /**
     * @given initialized storage, permission to his/her account
     * @when get signatories
     * @then Return signatories of user
     */
    TEST_F(GetSignatoriesExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMySignatories});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account_id)
                       .getSignatories(account_id)
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
                       .creatorAccountId(account_id)
                       .getSignatories(another_account_id)
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
                       .creatorAccountId(account_id)
                       .getSignatories(account_id2)
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
                       .creatorAccountId(account_id)
                       .getSignatories(another_account_id)
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
                       .creatorAccountId(account_id)
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

        auto pubkey2 =
            std::make_unique<shared_model::interface::types::PubkeyType>(
                std::string('2', 32));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id2", domain_id, *pubkey2)),
                        true)));

        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAsset(
                            "coin", domain_id, 1)),
                        true)));

        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder()
                                         .addAssetQuantity(asset_id, "1.0")
                                         .creatorAccountId(account_id)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder()
                                         .addAssetQuantity(asset_id, "1.0")
                                         .creatorAccountId(account_id2)),
                        true,
                        account_id2)));
      }
    };

    /**
     * @given initialized storage, permission to his/her account
     * @when get account assets
     * @then Return account asset of user
     */
    TEST_F(GetAccountAssetExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMyAccAst});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account_id)
                       .getAccountAssets(account_id)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountAssetResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.accountAssets()[0].accountId(), account_id);
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
                       .creatorAccountId(account_id)
                       .getAccountAssets(account_id2)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountAssetResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.accountAssets()[0].accountId(),
                      account_id2);
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
                       .creatorAccountId(account_id)
                       .getAccountAssets(account_id2)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountAssetResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.accountAssets()[0].accountId(),
                      account_id2);
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
                       .creatorAccountId(account_id)
                       .getAccountAssets(another_account_id)
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
                       .creatorAccountId(account_id)
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
        detail =
            "{\"id@domain\": {\"key\": \"value\", "
            "\"key2\": \"value2\"},"
            " \"id2@domain\": {\"key\": \"value\", "
            "\"key2\": \"value2\"}}";
        auto pubkey2 =
            std::make_unique<shared_model::interface::types::PubkeyType>(
                std::string('2', 32));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id2", domain_id, *pubkey2)),
                        true)));

        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAsset(
                            "coin", domain_id, 1)),
                        true)));

        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().setAccountDetail(
                            account_id2, "key", "value")),
                        true,
                        account_id)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().setAccountDetail(
                            account_id2, "key2", "value2")),
                        true,
                        account_id)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().setAccountDetail(
                            account_id2, "key", "value")),
                        true,
                        account_id2)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().setAccountDetail(
                            account_id2, "key2", "value2")),
                        true,
                        account_id2)));
      }

      shared_model::interface::types::DetailType detail;
    };

    /**
     * @given initialized storage, permission to his/her account
     * @when get account detail
     * @then Return account detail
     */
    TEST_F(GetAccountDetailExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMyAccDetail});
      auto query = TestQueryBuilder()
                       .creatorAccountId(account_id)
                       .getAccountDetail(account_id)
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
                       .creatorAccountId(account_id)
                       .getAccountDetail(account_id2)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountDetailResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.detail(), detail);
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
                       .creatorAccountId(account_id)
                       .getAccountDetail(account_id2)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AccountDetailResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.detail(), detail);
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
                       .creatorAccountId(account_id)
                       .getAccountDetail(another_account_id)
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
                       .creatorAccountId(account_id)
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
                       .creatorAccountId(account_id)
                       .getAccountDetail(account_id2, "key")
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
      auto query = TestQueryBuilder()
                       .creatorAccountId(account_id)
                       .getAccountDetail(account_id2, "", account_id)
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
                       .creatorAccountId(account_id)
                       .getAccountDetail(account_id2, "key", account_id)
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
      auto query =
          TestQueryBuilder().creatorAccountId(account_id).getRoles().build();
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
      auto query =
          TestQueryBuilder().creatorAccountId(account_id).getRoles().build();
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
                       .creatorAccountId(account_id)
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
                       .creatorAccountId(account_id)
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
                       .creatorAccountId(account_id)
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
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAsset(
                            "coin", domain_id, 1)),
                        true)));
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
                       .creatorAccountId(account_id)
                       .getAssetInfo(asset_id)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::AssetResponse>(
          std::move(result), [this](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.asset().assetId(), asset_id);
            ASSERT_EQ(cast_resp.asset().domainId(), domain_id);
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
                       .creatorAccountId(account_id)
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
                       .creatorAccountId(account_id)
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
        auto pubkey2 =
            std::make_unique<shared_model::interface::types::PubkeyType>(
                std::string('2', 32));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAccount(
                            "id2", domain_id, *pubkey2)),
                        true)));
        ASSERT_TRUE(
            val(execute(buildCommand(TestTransactionBuilder().createAsset(
                            "coin", domain_id, 1)),
                        true)));
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
        auto fake_hash = shared_model::crypto::Hash(zero_string);
        auto fake_pubkey = shared_model::crypto::PublicKey(zero_string);

        std::vector<shared_model::proto::Transaction> txs1;
        txs1.push_back(TestTransactionBuilder()
                           .creatorAccountId(account_id)
                           .createRole("user", {})
                           .build());
        txs1.push_back(
            TestTransactionBuilder()
                .creatorAccountId(account_id)
                .addAssetQuantity(asset_id, "2.0")
                .transferAsset(account_id, account_id2, asset_id, "", "1.0")
                .build());
        txs1.push_back(TestTransactionBuilder()
                           .creatorAccountId(account_id2)
                           .createRole("user2", {})
                           .build());

        auto block1 = TestBlockBuilder()
                          .transactions(txs1)
                          .height(1)
                          .prevHash(fake_hash)
                          .build();

        apply(storage, block1);

        std::vector<shared_model::proto::Transaction> txs2;
        txs2.push_back(
            TestTransactionBuilder()
                .creatorAccountId(account_id2)
                .transferAsset(account_id, account_id2, asset_id, "", "1.0")
                .build());
        txs2.push_back(TestTransactionBuilder()
                           .creatorAccountId(account_id)
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
      shared_model::crypto::Hash fake_hash{zero_string};
      shared_model::crypto::PublicKey fake_pubkey{zero_string};
      shared_model::crypto::Hash hash1;
      shared_model::crypto::Hash hash2;
      shared_model::crypto::Hash hash3;
    };

    template <typename QueryTxPaginationTest>
    class GetPagedTransactionsExecutorTest
        : public GetTransactionsExecutorTest {
     protected:
      using Impl = QueryTxPaginationTest;

      // create valid transactions and commit them
      void createTransactionsAndCommit(size_t transactions_amount) {
        addPerms(Impl::getUserPermissions());

        auto initial_txs = Impl::makeInitialTransactions(transactions_amount);
        auto target_txs = Impl::makeTargetTransactions(transactions_amount);

        tx_hashes_.reserve(target_txs.size());
        initial_txs.reserve(initial_txs.size() + target_txs.size());
        for (auto &tx : target_txs) {
          tx_hashes_.emplace_back(tx.hash());
          initial_txs.emplace_back(std::move(tx));
        }

        auto block = TestBlockBuilder()
                         .transactions(initial_txs)
                         .height(1)
                         .prevHash(fake_hash)
                         .build();

        apply(storage, block);
      }

      auto queryPage(
          types::TransactionsNumberType page_size,
          const boost::optional<types::HashType> &first_hash = boost::none) {
        auto query = Impl::makeQuery(page_size, first_hash);
        return executeQuery(query);
      }

      /**
       * Check the transactions pagination response compliance to general rules:
       * - total transactions number is equal to the number of target
       * transactions
       * - the number of transactions in response is equal to the requested
       * amount if there are enough, otherwie equal to the available amount
       * - the returned transactions' and the target transactions' hashes match
       * - next transaction hash in response is unset if the last transaction is
       * in the response, otherwise it matches the next target transaction hash
       */
      void generalTransactionsPageResponseCheck(
          const TransactionsPageResponse &tx_page_response,
          types::TransactionsNumberType page_size,
          const boost::optional<types::HashType> &first_hash =
              boost::none) const {
        EXPECT_EQ(tx_page_response.allTransactionsSize(), tx_hashes_.size())
            << "Wrong `total transactions' number.";
        auto resp_tx_hashes = tx_page_response.transactions()
            | boost::adaptors::transformed(
                                  [](const auto &tx) { return tx.hash(); });
        const auto page_start = first_hash
            ? std::find(tx_hashes_.cbegin(), tx_hashes_.cend(), *first_hash)
            : tx_hashes_.cbegin();
        if (first_hash and page_start == tx_hashes_.cend()) {
          // Should never reach here as a non-existing first_hash in the
          // pagination metadata must cause an error query response instead of
          // transaction page response. If we get here, it is a problem of wrong
          // test logic.
          BOOST_THROW_EXCEPTION(
              std::runtime_error("Checking response that does not match "
                                 "the provided query pagination data."));
          return;
        }
        const auto expected_txs_amount =
            std::min<size_t>(page_size, tx_hashes_.cend() - page_start);
        const auto response_txs_amount = boost::size(resp_tx_hashes);
        EXPECT_EQ(response_txs_amount, expected_txs_amount)
            << "Wrong number of transactions returned.";
        auto expected_hash = page_start;
        auto response_hash = resp_tx_hashes.begin();
        const auto page_end =
            page_start + std::min(response_txs_amount, expected_txs_amount);
        while (expected_hash != page_end) {
          EXPECT_EQ(*expected_hash++, *response_hash++)
              << "Wrong transaction returned.";
        }
        if (page_end == tx_hashes_.cend()) {
          EXPECT_EQ(tx_page_response.nextTxHash(), boost::none)
              << "Next transaction hash value must be unset.";
        } else {
          EXPECT_TRUE(tx_page_response.nextTxHash());
          if (tx_page_response.nextTxHash()) {
            EXPECT_EQ(*tx_page_response.nextTxHash(), *page_end)
                << "Wrong next transaction hash value.";
          }
        }
      }

      std::vector<types::HashType> tx_hashes_;
    };

    struct GetAccountTxPaginationImpl {
      static std::initializer_list<permissions::Role> getUserPermissions() {
        return {permissions::Role::kSetDetail, permissions::Role::kGetMyAccTxs};
      }

      static std::vector<shared_model::proto::Transaction>
      makeInitialTransactions(size_t transactions_amount) {
        return {};
      }

      static auto makeTargetTransactions(size_t transactions_amount) {
        std::vector<shared_model::proto::Transaction> transactions;
        transactions.reserve(transactions_amount);
        for (size_t i = 0; i < transactions_amount; ++i) {
          transactions.emplace_back(
              TestTransactionBuilder()
                  .creatorAccountId(account_id)
                  .createdTime(iroha::time::now(std::chrono::milliseconds(i)))
                  .setAccountDetail(account_id,
                                    "key_" + std::to_string(i),
                                    "val_" + std::to_string(i))
                  .build());
        }
        return transactions;
      }

      static shared_model::proto::Query makeQuery(
          types::TransactionsNumberType page_size,
          const boost::optional<types::HashType> &first_hash = boost::none) {
        return TestQueryBuilder()
            .creatorAccountId(account_id)
            .createdTime(iroha::time::now())
            .getAccountTransactions(account_id, page_size, first_hash)
            .build();
      }
    };

    template <typename T>
    static std::string assetAmount(T mantissa, types::PrecisionType precision) {
      std::stringstream ss;
      ss << std::setprecision(precision) << mantissa;
      return ss.str();
    }

    struct GetAccountAssetTxPaginationImpl {
      static std::initializer_list<permissions::Role> getUserPermissions() {
        return {permissions::Role::kReceive,
                permissions::Role::kGetMyAccAstTxs};
      }

      static std::vector<shared_model::proto::Transaction>
      makeInitialTransactions(size_t transactions_amount) {
        return {
            TestTransactionBuilder()
                .creatorAccountId(account_id)
                .createdTime(iroha::time::now())
                .addAssetQuantity(
                    asset_id, assetAmount(transactions_amount, kAssetPrecision))
                .build()};
      }

      static auto makeTargetTransactions(size_t transactions_amount) {
        std::vector<shared_model::proto::Transaction> transactions;
        transactions.reserve(transactions_amount);
        for (size_t i = 0; i < transactions_amount; ++i) {
          transactions.emplace_back(
              TestTransactionBuilder()
                  .creatorAccountId(account_id)
                  .createdTime(iroha::time::now(std::chrono::milliseconds(i)))
                  .transferAsset(account_id,
                                 another_account_id,
                                 asset_id,
                                 "tx #" + std::to_string(i),
                                 assetAmount(1, kAssetPrecision))
                  .build());
        }
        return transactions;
      }

      static shared_model::proto::Query makeQuery(
          types::TransactionsNumberType page_size,
          const boost::optional<types::HashType> &first_hash = boost::none) {
        return TestQueryBuilder()
            .creatorAccountId(account_id)
            .createdTime(iroha::time::now())
            .getAccountAssetTransactions(
                account_id, asset_id, page_size, first_hash)
            .build();
      }
    };

    using GetAccountTransactionsExecutorTest =
        GetPagedTransactionsExecutorTest<GetAccountTxPaginationImpl>;

    /**
     * @given initialized storage, permission to his/her account
     * @when get account transactions
     * @then Return account transactions of user
     */
    TEST_F(GetAccountTransactionsExecutorTest, ValidMyAccount) {
      addPerms({shared_model::interface::permissions::Role::kGetMyAccTxs});

      commitBlocks();

      auto query = TestQueryBuilder()
                       .creatorAccountId(account_id)
                       .getAccountTransactions(account_id, kTxPageSize)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsPageResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.transactions().size(), 3);
            for (const auto &tx : cast_resp.transactions()) {
              static size_t i = 0;
              EXPECT_EQ(account_id, tx.creatorAccountId())
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
                       .creatorAccountId(account_id)
                       .getAccountTransactions(account_id2, kTxPageSize)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsPageResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.transactions().size(), 2);
            for (const auto &tx : cast_resp.transactions()) {
              EXPECT_EQ(account_id2, tx.creatorAccountId())
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
                       .creatorAccountId(account_id)
                       .getAccountTransactions(account_id2, kTxPageSize)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsPageResponse>(
          std::move(result), [](const auto &cast_resp) {
            ASSERT_EQ(cast_resp.transactions().size(), 2);
            for (const auto &tx : cast_resp.transactions()) {
              EXPECT_EQ(account_id2, tx.creatorAccountId())
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
                       .creatorAccountId(account_id)
                       .getAccountTransactions(another_account_id, kTxPageSize)
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
                       .creatorAccountId(account_id)
                       .getAccountTransactions("some@domain", kTxPageSize)
                       .build();
      auto result = executeQuery(query);
      checkStatefulError<shared_model::interface::StatefulFailedErrorResponse>(
          std::move(result), kNoStatefulError);
    }

    // ------------------------/ tx pagination tests \----------------------- //

    using QueryTxPaginationTestingTypes =
        ::testing::Types<GetAccountTxPaginationImpl,
                         GetAccountAssetTxPaginationImpl>;
    TYPED_TEST_CASE(GetPagedTransactionsExecutorTest,
                    QueryTxPaginationTestingTypes);

    /**
     * @given initialized storage, user has 3 transactions committed
     * @when query contains second transaction as a starting
     * hash @and 2 transactions page size
     * @then response contains exactly 2 transaction
     * @and list of transactions starts from second transaction
     * @and next transaction hash is not present
     */
    TYPED_TEST(GetPagedTransactionsExecutorTest, ValidPagination) {
      this->createTransactionsAndCommit(3);
      auto &hash = this->tx_hashes_.at(1);
      auto size = 2;
      auto query_response = this->queryPage(size, hash);
      checkSuccessfulResult<TransactionsPageResponse>(
          std::move(query_response),
          [this, &hash, size](const auto &tx_page_response) {
            EXPECT_EQ(tx_page_response.transactions().begin()->hash(), hash);
            EXPECT_FALSE(tx_page_response.nextTxHash());
            this->generalTransactionsPageResponseCheck(
                tx_page_response, size, hash);
          });
    }

    /**
     * @given initialized storage, user has 3 transactions committed
     * @when query contains 2 transactions page size without starting hash
     * @then response contains exactly 2 transactions
     * @and starts from the first one
     * @and next transaction hash is equal to last committed transaction
     * @and total number of transactions equal to 3
     */
    TYPED_TEST(GetPagedTransactionsExecutorTest, ValidPaginationNoHash) {
      this->createTransactionsAndCommit(3);
      auto size = 2;
      auto query_response = this->queryPage(size);
      checkSuccessfulResult<TransactionsPageResponse>(
          std::move(query_response),
          [this, size](const auto &tx_page_response) {
            EXPECT_EQ(tx_page_response.transactions().begin()->hash(),
                      this->tx_hashes_.at(0));
            ASSERT_TRUE(tx_page_response.nextTxHash());
            this->generalTransactionsPageResponseCheck(tx_page_response, size);
          });
    }

    /**
     * @given initialized storage, user has 3 transactions committed
     * @when query contains 10 page size
     * @then response contains only 3 committed transactions
     */
    TYPED_TEST(GetPagedTransactionsExecutorTest,
               PaginationPageBiggerThanTotal) {
      this->createTransactionsAndCommit(3);
      auto size = 10;
      auto query_response = this->queryPage(size);

      checkSuccessfulResult<TransactionsPageResponse>(
          std::move(query_response),
          [this, size](const auto &tx_page_response) {
            this->generalTransactionsPageResponseCheck(tx_page_response, size);
          });
    }

    /**
     * @given initialized storage, user has 3 transactions committed
     * @when query contains non-existent starting hash
     * @then error response is returned
     */
    TYPED_TEST(GetPagedTransactionsExecutorTest, InvalidHashInPagination) {
      this->createTransactionsAndCommit(3);
      auto size = 2;
      char unknown_hash_string[kHashLength];
      zero_string.copy(unknown_hash_string, kHashLength);
      std::strcpy(unknown_hash_string, "no such hash!");
      auto query_response =
          this->queryPage(size, types::HashType(unknown_hash_string));

      checkStatefulError<StatefulFailedErrorResponse>(
          std::move(query_response),
          BlocksQueryExecutorTest::kInvalidPagination);
    }

    /**
     * @given initialized storage, user has no committed transactions
     * @when query contains 2 transactions page size
     * @then response does not contain any transactions
     * @and total size is 0
     * @and next hash is not present
     */
    TYPED_TEST(GetPagedTransactionsExecutorTest, PaginationNoTransactions) {
      this->createTransactionsAndCommit(0);
      auto size = 2;
      auto query_response = this->queryPage(size);

      checkSuccessfulResult<TransactionsPageResponse>(
          std::move(query_response),
          [this, size](const auto &tx_page_response) {
            this->generalTransactionsPageResponseCheck(tx_page_response, size);
          });
    }

    // --------------------\ end of tx pagination tests /-------------------- //

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
                       .creatorAccountId(account_id)
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
                       .creatorAccountId(account_id)
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

    using GetAccountAssetTransactionsExecutorTest =
        GetPagedTransactionsExecutorTest<GetAccountAssetTxPaginationImpl>;

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
              .creatorAccountId(account_id)
              .getAccountAssetTransactions(account_id, asset_id, kTxPageSize)
              .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsPageResponse>(
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

      auto query = TestQueryBuilder()
                       .creatorAccountId(account_id)
                       .getAccountAssetTransactions(
                               account_id2, asset_id, kTxPageSize)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsPageResponse>(
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

      auto query = TestQueryBuilder()
                       .creatorAccountId(account_id)
                       .getAccountAssetTransactions(
                               account_id2, asset_id, kTxPageSize)
                       .build();
      auto result = executeQuery(query);
      checkSuccessfulResult<shared_model::interface::TransactionsPageResponse>(
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
                       .creatorAccountId(account_id)
                       .getAccountTransactions(another_account_id, kTxPageSize)
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
                       .creatorAccountId(account_id)
                       .getPendingTransactions()
                       .build();

      EXPECT_CALL(*pending_txs_storage, getPendingTransactions(account_id))
          .Times(1);

      executeQuery(query);
    }

  }  // namespace ametsuchi
}  // namespace iroha
