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
#include "interfaces/query_responses/error_responses/stateful_failed_error_response.hpp"

using namespace integration_framework;
using namespace shared_model;
using namespace common_constants;

/**
 * C369 Get role permissions by user with allowed GetRoles permission
 * @given a user with kGetRoles permission
 * @when the user send query with getRolePermissions request
 * @then there is a valid RolePermissionsResponse
 */
TEST_F(AcceptanceFixture, CanGetRolePermissions) {
  auto check_query = [](auto &query_response) {
    ASSERT_NO_THROW(boost::apply_visitor(
        framework::SpecifiedVisitor<
            shared_model::interface::RolePermissionsResponse>(),
        query_response.get()));
  };

  auto query = complete(baseQry().getRolePermissions(kRole));

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeUserWithPerms(
              {shared_model::interface::permissions::Role::kGetRoles}),
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(query, check_query);
}

/**
 * C370 Get role permissions without allowed GetRoles permission
 * @given a user without kGetRoles permission
 * @when the user send query with getRolePermissions request
 * @then query should be recognized as stateful invalid
 */
TEST_F(AcceptanceFixture, CanNotGetRolePermissions) {
  auto query = complete(baseQry().getRolePermissions(kRole));

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeUserWithPerms({}),
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(query,
                 checkQueryErrorResponse<
                     shared_model::interface::StatefulFailedErrorResponse>());
}
