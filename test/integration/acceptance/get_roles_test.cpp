/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "integration/acceptance/acceptance_fixture.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "backend/protobuf/transaction.hpp"
#include "framework/specified_visitor.hpp"
#include "interfaces/permissions.hpp"


using namespace integration_framework;
using namespace shared_model;

class GetRoles : public AcceptanceFixture {
public:
//    auto makeUserWithPerms(const interface::RolePermissionSet &perms = {
//            interface::permissions::Role::kGetRoles}) {
//        return AcceptanceFixture::makeUserWithPerms(perms);
//    }

};

/**
 * @given a user with CanGetRoles permission
 * @when execute query with getRoles command
 * @then there is should be no exception
 */
TEST_F(GetRoles, CanGetRoles) {
    auto checkQuery = [](auto &queryResponse) {
        ASSERT_NO_THROW(boost::apply_visitor(
                framework::SpecifiedVisitor<shared_model::interface::RolesResponse>(), queryResponse.get()));
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
            .sendTx(makeUserWithPerms({shared_model::interface::permissions::Role::kGetRoles}))
            .skipProposal()
            .checkBlock([](auto &block) {
                ASSERT_EQ(boost::size(block->transactions()), 1);
            })
            .sendQuery(query, checkQuery);
}

/**
 * @given a user without CanGetRoles permission
 * @when execute query with getRoles command
 * @then there is should be an exception
 */
TEST_F(GetRoles, CanNotGetRoles) {
    auto checkQuery = [](auto &queryResponse) {
        ASSERT_ANY_THROW(boost::apply_visitor(
                framework::SpecifiedVisitor<shared_model::interface::RolesResponse>(), queryResponse.get()));
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
            .checkBlock([](auto &block) {
                ASSERT_EQ(boost::size(block->transactions()), 1);
            })
            .sendQuery(query, checkQuery);
}


