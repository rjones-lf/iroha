/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "backend/protobuf/transaction.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "interfaces/permissions.hpp"
#include "interfaces/query_responses/error_query_response.hpp"
#include "interfaces/query_responses/error_responses/stateful_failed_error_response.hpp"

using namespace integration_framework;
using namespace shared_model;
using namespace common_constants;

/**
 * @given a user with CanGetRoles permission
 * @when execute query with getRoles command
 * @then the query returns list of roles
 */
TEST_F(AcceptanceFixture, CanGetRoles) {
  auto checkQuery = [](auto &queryResponse) {
    ASSERT_NO_THROW(boost::apply_visitor(
        framework::SpecifiedVisitor<shared_model::interface::RolesResponse>(),
        queryResponse.get()));
  };

  auto query = TestUnsignedQueryBuilder()
                   .createdTime(iroha::time::now())
                   .creatorAccountId(kUserId)
                   .queryCounter(1)
                   .getRoles()
                   .build()
                   .signAndAddSignature(kUserKeypair)
                   .finish();

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms(
          {shared_model::interface::permissions::Role::kGetRoles}))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(boost::size(block->transactions()), 1); })
      .sendQuery(query, checkQuery);
}

/**
 * @given a user without CanGetRoles permission
 * @when execute query with getRoles command
 * @then there is no way to to get roles due to user hasn't permissions enough
 */
TEST_F(AcceptanceFixture, CanNotGetRoles) {
  auto checkQuery = [](auto &queryResponse) {
    ASSERT_NO_THROW({
      boost::apply_visitor(
          framework::SpecifiedVisitor<
              shared_model::interface::StatefulFailedErrorResponse>(),
          boost::apply_visitor(
              framework::SpecifiedVisitor<
                  shared_model::interface::ErrorQueryResponse>(),
              queryResponse.get())
              .get());
    });
  };

  auto query = TestUnsignedQueryBuilder()
                   .createdTime(iroha::time::now())
                   .creatorAccountId(kUserId)
                   .queryCounter(1)
                   .getRoles()
                   .build()
                   .signAndAddSignature(kUserKeypair)
                   .finish();

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms({}))
      .skipProposal()
      .checkVerifiedProposal(
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 1); })
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(query, checkQuery);
}
