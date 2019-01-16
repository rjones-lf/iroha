/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "module/shared_model/builders/protobuf/common_objects/proto_account_builder.hpp"

/**
 * @given fields for Account object
 * @when AccountBuilder is invoked
 * @then Account object is successfully constructed and has the same fields as
 * provided
 */
TEST(ProtoAccountBuilderTest, AllFieldsBuild) {
  shared_model::proto::AccountBuilder builder;

  auto expected_account_id = "Steve Irwin";
  auto expected_domain_id = "australia.com";
  auto expected_quorum = 3;
  auto expected_json_data = "{}";

  auto account = builder.accountId(expected_account_id)
                     .domainId(expected_domain_id)
                     .quorum(expected_quorum)
                     .jsonData(expected_json_data)
                     .build();

  EXPECT_EQ(account.accountId(), expected_account_id);
  EXPECT_EQ(account.domainId(), expected_domain_id);
  EXPECT_EQ(account.quorum(), expected_quorum);
  EXPECT_EQ(account.jsonData(), expected_json_data);
}

/**
 * @given fields for Account object
 * @when AccountBuilder is invoked twice with the same configuration
 * @then Two constructed Account objects are identical
 */
TEST(ProtoAccountBuilderTest, SeveralObjectsFromOneBuilder) {
  shared_model::proto::AccountBuilder builder;

  auto expected_account_id = "Steve Irwin";
  auto expected_domain_id = "australia.com";
  auto expected_quorum = 3;
  auto expected_json_data = "{}";

  auto state = builder.accountId(expected_account_id)
                   .domainId(expected_domain_id)
                   .quorum(expected_quorum)
                   .jsonData(expected_json_data);

  auto account = state.build();

  auto account2 = state.build();

  EXPECT_EQ(account.accountId(), account2.accountId());
  EXPECT_EQ(account.domainId(), account2.domainId());
  EXPECT_EQ(account.quorum(), account2.quorum());
  EXPECT_EQ(account.jsonData(), account2.jsonData());
}
