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
      .sendTx(makeUserWithPerms(
          {shared_model::interface::permissions::Role::kGetRoles}))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(boost::size(block->transactions()), 1); })
      .sendQuery(query, checkQuery);
}

TEST_F(AcceptanceFixture, CanNotGetRolePermissions) {
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

  auto query = complete(baseQry().getRolePermissions(kRole));

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
