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

class GetAccount : public AcceptanceFixture {
 public:
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
   * Creates valid GetAccount query of current user
   * @param hash of the tx for querying
   * @return built query
   */
  auto makeQuery(const std::string &user) {
    return complete(baseQry().queryCounter(1).getAccount(user));
  }

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

  auto checkValidAccount() {
    return checkValidAccount(kDomain, kUserId, kNewRole);
  }

  auto checkStatefulInvalid() {
    return [](auto &status) {
      ASSERT_NO_THROW(boost::apply_visitor(
          shared_model::interface::QueryErrorResponseChecker<
              shared_model::interface::StatefulFailedErrorResponse>(),
          status.get()));
    };
  }

  auto checkStatelessInvalid() {
    return [](auto &status) {
      ASSERT_NO_THROW(boost::apply_visitor(
          shared_model::interface::QueryErrorResponseChecker<
              shared_model::interface::StatelessFailedErrorResponse>(),
          status.get()));
    };
  }

  const std::string kNewRole = "rl";
  const std::string kRole2 = "roletwo";
  const std::string kUser2 = "usertwo";
  const crypto::Keypair kUser2Keypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

#define check(i) [](auto &block) { ASSERT_EQ(block->transactions().size(), i); }

/**
 * @given a user with GetMyAccount permission
 * @when GetAccount is queried on the user
 * @then there is a valid AccountResponse
 */
TEST_F(GetAccount, Basic) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeUserWithPerms(), check(1))
      .sendQuery(makeQuery(), checkValidAccount())
      .done();
}

/**
 * @given a user with all required permissions
 * @when GetAccount is queried on the empty account name
 * @then query is stateless invalid response
 */
TEST_F(GetAccount, EmptyAccount) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeUserWithPerms(), check(1))
      .sendQuery(makeQuery(), checkStatelessInvalid())
      .done();
}

/**
 * @given a user with all required permissions
 * @when GetAccount is queried on the user
 * @then query is stateful invalid response
 */
TEST_F(GetAccount, NonexistentAccount) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeUserWithPerms(), check(1))
      .sendQuery(complete(baseQry().queryCounter(1).getAccount("inexistent@"
                                                               + kDomain)),
                 checkStatefulInvalid())
      .done();
}

/**
 * @given a user without GetMyAccount permission
 * @when GetAccount is queried on the user
 * @then query is stateful invalid response
 */
TEST_F(GetAccount, NoPermission) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(makeUserWithPerms({}), check(1))
      .sendQuery(makeQuery(), checkStatefulInvalid())
      .done();
}

/**
 * @given a user with only kGetDomainAccounts permission
 * @when GetAccount is queried on the user
 * @then there is a valid AccountResponse
 */
TEST_F(GetAccount, WithGetDomainPermission) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeUserWithPerms({interface::permissions::Role::kGetDomainAccounts}),
          check(1))
      .sendQuery(makeQuery(), checkValidAccount())
      .done();
}

/**
 * @given a user with only kGetAllAccounts permission
 * @when GetAccount is queried on the user
 * @then there is a valid AccountResponse
 */
TEST_F(GetAccount, WithGetAllPermission) {
  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeUserWithPerms({interface::permissions::Role::kGetAllAccounts}),
          check(1))
      .sendQuery(makeQuery(), checkValidAccount())
      .done();
}

/**
 * @given a user with all required permissions and a user in the same domain
 * @when GetAccount is queried on the second user
 * @then there is a valid AccountResponse
 */
TEST_F(GetAccount, WithGetDomainPermissionOtherAccount) {
  const std::string kUser2Id = kUser2 + "@" + kDomain;

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeUserWithPerms({interface::permissions::Role::kGetDomainAccounts}),
          check(1))
      .sendTxAwait(
          createUserWithPerms(kUser2,
                              kUser2Keypair.publicKey(),
                              kRole2,
                              {interface::permissions::Role::kSetQuorum})
              .build()
              .signAndAddSignature(kAdminKeypair)
              .finish(),
          check(1))
      .sendQuery(makeQuery(kUser2Id),
                 checkValidAccount(kDomain, kUser2Id, kRole2))
      .done();
}

/**
 * @given a user with all required permissions and a user in other domain
 * @when GetAccount is queried on the second user
 * @then there is a valid AccountResponse
 */
TEST_F(GetAccount, WithGetDomainPermissionOtherAccountInterdomain) {
  const std::string kNewDomain = "newdom";
  const std::string kUser2Id = kUser2 + "@" + kNewDomain;
  auto make_second_user =
      baseTx()
          .creatorAccountId(IntegrationTestFramework::kAdminId)
          .createRole(kRole2, {interface::permissions::Role::kSetQuorum})
          .createDomain(kNewDomain, kRole2)
          .createAccount(kUser2, kNewDomain, kUser2Keypair.publicKey())
          .build()
          .signAndAddSignature(kAdminKeypair)
          .finish();

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeUserWithPerms({interface::permissions::Role::kGetAllAccounts}),
          check(1))
      .sendTxAwait(make_second_user, check(1))
      .sendQuery(makeQuery(kUser2Id),
                 checkValidAccount(kNewDomain, kUser2Id, kRole2))
      .done();
}
