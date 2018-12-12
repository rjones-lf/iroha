/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "validators/protobuf/proto_transaction_validator.hpp"

#include <gtest/gtest.h>
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "module/shared_model/validators/validators_fixture.hpp"

class ProtoTxValidatorTest : public ValidatorsTest {
 protected:
  iroha::protocol::Transaction generateEmptyTransaction() {
    std::string creator_account_id = "admin@test";

    TestTransactionBuilder builder;
    auto tx = builder.creatorAccountId(creator_account_id)
                  .createdTime(created_time)
                  .quorum(1)
                  .build()
                  .getTransport();
    return tx;
  }

  iroha::protocol::Transaction generateCreateRoleTransaction(
      const std::string &role_name,
      iroha::protocol::RolePermission permission) {
    auto tx = generateEmptyTransaction();

    auto cr = tx.mutable_payload()
                  ->mutable_reduced_payload()
                  ->add_commands()
                  ->mutable_create_role();
    cr->set_role_name(role_name);
    cr->add_permissions(permission);
    return tx;
  }

  iroha::protocol::Transaction generateGrantPermissionTransaction(
      const std::string &account_id,
      iroha::protocol::GrantablePermission permission) {
    auto tx = generateEmptyTransaction();

    auto gp = tx.mutable_payload()
                  ->mutable_reduced_payload()
                  ->add_commands()
                  ->mutable_grant_permission();
    gp->set_account_id(account_id);
    gp->set_permission(permission);
    return tx;
  }

  iroha::protocol::Transaction generateRevokePermissionTransaction(
      const std::string &account_id,
      iroha::protocol::GrantablePermission permission) {
    auto tx = generateEmptyTransaction();

    auto gp = tx.mutable_payload()
                  ->mutable_reduced_payload()
                  ->add_commands()
                  ->mutable_revoke_permission();
    gp->set_account_id(account_id);
    gp->set_permission(permission);
    return tx;
  }

  shared_model::validation::ProtoTransactionValidator validator;
};

/**
 * @given iroha::protocol::Transaction with defined command
 * @when it is validated
 * @then answer with no errors is returned
 */
TEST_F(ProtoTxValidatorTest, CommandIsSet) {
  auto tx = generateEmptyTransaction();

  iroha::protocol::CreateDomain cd;
  cd.set_domain_id(domain_id);
  cd.set_default_role(role_name);

  tx.mutable_payload()
      ->mutable_reduced_payload()
      ->add_commands()
      ->mutable_create_domain()
      ->CopyFrom(cd);

  auto answer = validator.validate(tx);
  ASSERT_FALSE(answer.hasErrors()) << answer.reason();
}

/**
 * @given iroha::protocol::Transaction with undefined command
 * @when it is validated
 * @then answer with errors is returned
 */
TEST_F(ProtoTxValidatorTest, CommandNotSet) {
  auto tx = generateEmptyTransaction();
  // add not set command
  tx.mutable_payload()->mutable_reduced_payload()->add_commands();

  auto answer = validator.validate(tx);
  ASSERT_TRUE(answer.hasErrors());
}

/**
 * @given iroha::protocol::Transaction containing create role transaction with
 * valid role permission
 * @when it is validated
 * @then answer with no errors is returned
 */
TEST_F(ProtoTxValidatorTest, CreateRoleValid) {
  auto tx = generateCreateRoleTransaction(
      role_name, iroha::protocol::RolePermission::can_read_assets);

  auto answer = validator.validate(tx);
  ASSERT_FALSE(answer.hasErrors());
}

/**
 * @given iroha::protocol::Transaction containing create role transaction with
 * undefined role permission
 * @when it is validated
 * @then answer with errors is returned
 */
TEST_F(ProtoTxValidatorTest, CreateRoleInvalid) {
  auto tx = generateCreateRoleTransaction(
      role_name, static_cast<iroha::protocol::RolePermission>(-1));

  auto answer = validator.validate(tx);
  ASSERT_TRUE(answer.hasErrors());
}

/**
 * @given iroha::protocol::Transaction containing grant permission transaction
 * with valid grantable permission
 * @when it is validated
 * @then answer with no errors is returned
 */
TEST_F(ProtoTxValidatorTest, GrantPermissionValid) {
  auto tx = generateGrantPermissionTransaction(
      account_id, iroha::protocol::GrantablePermission::can_add_my_signatory);

  auto answer = validator.validate(tx);
  ASSERT_FALSE(answer.hasErrors()) << answer.reason();
}

/**
 * @given iroha::protocol::Transaction containing grant permission transaction
 * with invalid grantable permission
 * @when it is validated
 * @then answer with errors is returned
 */
TEST_F(ProtoTxValidatorTest, GrantPermissionInvalid) {
  auto tx = generateGrantPermissionTransaction(
      account_id, static_cast<iroha::protocol::GrantablePermission>(-1));

  auto answer = validator.validate(tx);
  ASSERT_TRUE(answer.hasErrors());
}

/**
 * @given iroha::protocol::Transaction containing revoke permission transaction
 * with valid grantable permission
 * @when it is validated
 * @then answer with no errors is returned
 */
TEST_F(ProtoTxValidatorTest, RevokePermissionValid) {
  auto tx = generateRevokePermissionTransaction(
      account_id, iroha::protocol::GrantablePermission::can_add_my_signatory);

  auto answer = validator.validate(tx);
  ASSERT_FALSE(answer.hasErrors()) << answer.reason();
}

/**
 * @given iroha::protocol::Transaction containing revoke permission transaction
 * with invalid grantable permission
 * @when it is validated
 * @then answer with errors is returned
 */
TEST_F(ProtoTxValidatorTest, RevokePermissionInvalid) {
  auto tx = generateRevokePermissionTransaction(
      account_id, static_cast<iroha::protocol::GrantablePermission>(-1));

  auto answer = validator.validate(tx);
  ASSERT_TRUE(answer.hasErrors());
}
