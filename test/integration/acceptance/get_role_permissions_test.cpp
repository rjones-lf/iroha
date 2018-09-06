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

using namespace integration_framework;
using namespace shared_model;

/**
 * Get role permissions by user with allowed GetRoles permission
 * @given user with kGetRoles permission
 * @when user send query with getRolePermissions request
 * @then there is a valid RolePermissionsResponse
 */
TEST_F(AcceptanceFixture, CanGetRolePermissions) {
  auto checkQuery = [](auto &queryResponse) {
    ASSERT_NO_THROW(boost::apply_visitor(
        framework::SpecifiedVisitor<
            shared_model::interface::RolePermissionsResponse>(),
        queryResponse.get()));
  };

  auto query = complete(baseQry().getRolePermissions(kRole));

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeUserWithPerms(
              {shared_model::interface::permissions::Role::kGetRoles}),
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 1); })
      .sendQuery(query, checkQuery);
}

/**
 * Get role permissions without allowed GetRoles permission
 * @given user without kGetRoles permission
 * @when user send query with getRolePermissions request
 * @then query should be recognized as stateful invalid
 */
TEST_F(AcceptanceFixture, CanNotGetRolePermissions) {
  auto query = complete(baseQry().getRolePermissions(kRole));

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTxAwait(
          makeUserWithPerms({}),
          [](auto &proposal) { ASSERT_EQ(proposal->transactions().size(), 1); })
      .sendQuery(query,
                 checkQueryErrorResponse<
                     shared_model::interface::StatefulFailedErrorResponse>());
}
