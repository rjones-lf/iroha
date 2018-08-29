/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/queries.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "utils/query_error_response_visitor.hpp"

using namespace integration_framework;
using namespace shared_model;

#define CHECK_BLOCK(i) \
  [](auto &block) { ASSERT_EQ(block->transactions().size(), i); }

class GetAccount : public AcceptanceFixture {
 public:
  GetAccount() : itf(1) {}

  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx
   */
  auto makeUserWithPerms(const interface::RolePermissionSet &perms = {
                             interface::permissions::Role::kGetMyAccount}) {
    auto new_perms = perms;
    new_perms.set(interface::permissions::Role::kSetQuorum);
    return AcceptanceFixture::makeUserWithPerms(kNewRole, new_perms);
  }

  /**
   * Prepares base state that contains a default user
   * @param perms are the permissions of the user
   * @return itf with the base state
   */
  IntegrationTestFramework &prepareState(
      const interface::RolePermissionSet &perms = {
          interface::permissions::Role::kGetMyAccount}) {
    itf.setInitialState(kAdminKeypair)
        .sendTxAwait(makeUserWithPerms(perms), CHECK_BLOCK(1));
    return itf;
  }

  /**
   * Creates valid GetAccount query of the selected user
   * @param user is account to query
   * @return built query
   */
  auto makeQuery(const std::string &user) {
    return complete(baseQry().getAccount(user));
  }

  /**
   * Creates valid GetAccount query of the current user
   * @return built query
   */
  auto makeQuery() {
    return makeQuery(kUserId);
  }

  /**
   * @return a lambda that verifies that query response says the query has
   * no account at response
   */
  auto checkNoAccountResponse() {
    return [](auto &response) {
      ASSERT_TRUE(boost::apply_visitor(
          shared_model::interface::QueryErrorResponseChecker<
              shared_model::interface::NoAccountErrorResponse>(),
          response.get()))
          << "Actual response: " << response.toString();
    };
  }

  /**
   * @param domain is domain for checking
   * @param user is account id for checking
   * @param role is role for checking
   * @return a lambda that verifies that query response contains created account
   */
  auto checkValidAccount(const std::string &domain,
                         const std::string &user,
                         const std::string &role) {
    return [&](const proto::QueryResponse &response) {
      ASSERT_NO_THROW({
        const auto &resp = boost::apply_visitor(
            framework::SpecifiedVisitor<interface::AccountResponse>(),
            response.get());
        ASSERT_EQ(resp.account().accountId(), user);
        ASSERT_EQ(resp.account().domainId(), domain);
        ASSERT_EQ(resp.roles().size(), 1);
        ASSERT_EQ(resp.roles()[0], role);
      }) << "Actual response: "
         << response.toString();
    };
  }

  /**
   * @return a lambda that verifies that query response contains created account
   */
  auto checkValidAccount() {
    return checkValidAccount(kDomain, kUserId, kNewRole);
  }

  /**
   * @return a command for creating second user in the default domain
   */
  auto makeSecondUser() {
    return complete(
        createUserWithPerms(kUser2,
                            kUser2Keypair.publicKey(),
                            kRole2,
                            {interface::permissions::Role::kSetQuorum}),
        kAdminKeypair);
  }

  /**
   * @return a command for creating second user in the new domain
   */
  auto makeSecondInterdomainUser() {
    return complete(
        baseTx()
            .creatorAccountId(IntegrationTestFramework::kAdminId)
            .createRole(kRole2, {interface::permissions::Role::kSetQuorum})
            .createDomain(kNewDomain, kRole2)
            .createAccount(kUser2, kNewDomain, kUser2Keypair.publicKey()),
        kAdminKeypair);
  }

  const std::string kNewDomain = "newdom";
  const std::string kNewRole = "rl";
  const std::string kRole2 = "roletwo";
  const std::string kUser2 = "usertwo";
  const crypto::Keypair kUser2Keypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  IntegrationTestFramework itf;
};

/**
 * C321 Pass an empty account id
 * @given a user with all required permissions
 * @when GetAccount is queried on the empty account name
 * @then query is stateless invalid response
 */
TEST_F(GetAccount, EmptyAccount) {
  prepareState().sendQuery(
      makeQuery(""),
      checkQueryErrorResponse<
          shared_model::interface::StatelessFailedErrorResponse>());
}

/**
 * C320 Get an non-existing account
 * @given a user with all required permissions
 * @when GetAccount is queried on the user
 * @then query is stateful invalid response
 */
TEST_F(GetAccount, NonexistentAccount) {
  prepareState().sendQuery(
      complete(baseQry().queryCounter(1).getAccount("inexistent@" + kDomain)),

      checkQueryErrorResponse<
          shared_model::interface::StatefulFailedErrorResponse>());
}

/**
 * C315 Get my account without a CanGetMyAccount permission
 * @given a user without any query-related permission
 * @when GetAccount is queried on the user
 * @then query is stateful invalid response
 */
TEST_F(GetAccount, NoPermission) {
  prepareState({}).sendQuery(
      makeQuery(),
      checkQueryErrorResponse<
          shared_model::interface::StatefulFailedErrorResponse>());
}

/**
 * C322 Get my account with a CanGetMyAccount permission
 * @given a user with GetMyAccount permission
 * @when GetAccount is queried on the user
 * @then there is a valid AccountResponse
 */
TEST_F(GetAccount, WithGetMyPermission) {
  prepareState().sendQuery(makeQuery(), checkValidAccount());
}

/**
 * C316 Get my account with only CanGetDomainAccounts permission
 * @given a user with GetDomainAccounts permission
 * @when GetAccount is queried on the user
 * @then there is a valid AccountResponse
 */
TEST_F(GetAccount, WithGetDomainPermission) {
  prepareState({interface::permissions::Role::kGetDomainAccounts})
      .sendQuery(makeQuery(), checkValidAccount());
}

/**
 * C317 Get my account with only CanGetAllAccounts permission
 * @given a user with GetAllAccounts permission
 * @when GetAccount is queried on the user
 * @then there is a valid AccountResponse
 */
TEST_F(GetAccount, WithGetAllPermission) {
  prepareState({interface::permissions::Role::kGetAllAccounts})
      .sendQuery(makeQuery(), checkValidAccount());
}

/**
 * @given a user without any permission and a user in the same domain
 * @when GetAccount is queried on the second user
 * @then query is stateful invalid response
 */
TEST_F(GetAccount, NoPermissionOtherAccount) {
  const std::string kUser2Id = kUser2 + "@" + kDomain;
  prepareState({})
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendQuery(makeQuery(kUser2Id),
                 checkQueryErrorResponse<
                     shared_model::interface::StatefulFailedErrorResponse>());
}

/**
 * @given a user with GetMyAccount permission and a user in the same domain
 * @when GetAccount is queried on the second user
 * @then query is stateful invalid response
 */
TEST_F(GetAccount, WithGetMyPermissionOtherAccount) {
  const std::string kUser2Id = kUser2 + "@" + kDomain;
  prepareState({interface::permissions::Role::kGetMyAccount})
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendQuery(makeQuery(kUser2Id),
                 checkQueryErrorResponse<
                     shared_model::interface::StatefulFailedErrorResponse>());
}

/**
 * C318 Get an account from the domain having CanGetDomainAccounts
 * @given a user with GetDomainAccounts permission and a user in the same domain
 * @when GetAccount is queried on the second user
 * @then there is a valid AccountResponse
 */
TEST_F(GetAccount, WithGetDomainPermissionOtherAccount) {
  const std::string kUser2Id = kUser2 + "@" + kDomain;
  prepareState({interface::permissions::Role::kGetDomainAccounts})
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendQuery(makeQuery(kUser2Id),
                 checkValidAccount(kDomain, kUser2Id, kRole2));
}

/**
 * @given a user with GetAllAccounts permission and a user in the same domain
 * @when GetAccount is queried on the second user
 * @then there is a valid AccountResponse
 */
TEST_F(GetAccount, WithGetAllPermissionOtherAccount) {
  const std::string kUser2Id = kUser2 + "@" + kDomain;
  prepareState({interface::permissions::Role::kGetAllAccounts})
      .sendTxAwait(makeSecondUser(), CHECK_BLOCK(1))
      .sendQuery(makeQuery(kUser2Id),
                 checkValidAccount(kDomain, kUser2Id, kRole2));
}

/**
 * @given a user with all required permissions and a user in other domain
 * @when GetAccount is queried on the second user
 * @then query is stateful invalid response
 */
TEST_F(GetAccount, NoPermissionOtherAccountInterdomain) {
  const std::string kUser2Id = kUser2 + "@" + kNewDomain;
  prepareState({})
      .sendTxAwait(makeSecondInterdomainUser(), CHECK_BLOCK(1))
      .sendQuery(makeQuery(kUser2Id),
                 checkQueryErrorResponse<
                     shared_model::interface::StatefulFailedErrorResponse>());
}

/**
 * @given a user with all required permissions and a user in other domain
 * @when GetAccount is queried on the second user
 * @then query is stateful invalid response
 */
TEST_F(GetAccount, WithGetMyPermissionOtherAccountInterdomain) {
  const std::string kUser2Id = kUser2 + "@" + kNewDomain;
  prepareState({interface::permissions::Role::kGetMyAccount})
      .sendTxAwait(makeSecondInterdomainUser(), CHECK_BLOCK(1))
      .sendQuery(makeQuery(kUser2Id),
                 checkQueryErrorResponse<
                     shared_model::interface::StatefulFailedErrorResponse>());
}

/**
 * @given a user with all required permissions and a user in other domain
 * @when GetAccount is queried on the second user
 * @then query is stateful invalid response
 */
TEST_F(GetAccount, WithGetDomainPermissionOtherAccountInterdomain) {
  const std::string kUser2Id = kUser2 + "@" + kNewDomain;
  prepareState({interface::permissions::Role::kGetDomainAccounts})
      .sendTxAwait(makeSecondInterdomainUser(), CHECK_BLOCK(1))
      .sendQuery(makeQuery(kUser2Id),
                 checkQueryErrorResponse<
                     shared_model::interface::StatefulFailedErrorResponse>());
}

/**
 * C319 Get an account from another domain in the system having
 * CanGetAllAccounts
 * @given a user with all required permissions and a user in other domain
 * @when GetAccount is queried on the second user
 * @then there is a valid AccountResponse
 */
TEST_F(GetAccount, WithGetAllPermissionOtherAccountInterdomain) {
  const std::string kUser2Id = kUser2 + "@" + kNewDomain;
  prepareState({interface::permissions::Role::kGetAllAccounts})
      .sendTxAwait(makeSecondInterdomainUser(), CHECK_BLOCK(1))
      .sendQuery(makeQuery(kUser2Id),
                 checkValidAccount(kNewDomain, kUser2Id, kRole2));
}
