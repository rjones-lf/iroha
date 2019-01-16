/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "builders_test_fixture.hpp"
#include "module/shared_model/builders/common_objects/account_builder.hpp"
#include "module/shared_model/builders/protobuf/common_objects/proto_account_builder.hpp"
#include "validators/field_validator.hpp"

// TODO: 14.02.2018 nickaleks mock builder implementation IR-970
// TODO: 14.02.2018 nickaleks mock field validator IR-971

/**
 * @given field values which pass stateless validation
 * @when AccountBuilder is invoked
 * @then Account object is successfully constructed and has valid fields
 */
TEST(AccountBuilderTest, StatelessValidAllFields) {
  shared_model::builder::AccountBuilder<
      shared_model::proto::AccountBuilder,
      shared_model::validation::FieldValidator>
      builder;

  auto valid_account_id = "name@domain";
  auto valid_domain_id = "america";
  auto valid_quorum = 3;
  auto valid_json_data = "{}";

  auto account = builder.accountId(valid_account_id)
                     .domainId(valid_domain_id)
                     .quorum(valid_quorum)
                     .jsonData(valid_json_data)
                     .build();

  account.match(
      [&](shared_model::builder::BuilderResult<
          shared_model::interface::Account>::ValueType &v) {
        EXPECT_EQ(v.value->accountId(), valid_account_id);
        EXPECT_EQ(v.value->domainId(), valid_domain_id);
        EXPECT_EQ(v.value->quorum(), valid_quorum);
        EXPECT_EQ(v.value->jsonData(), valid_json_data);
      },
      [](shared_model::builder::BuilderResult<
          shared_model::interface::Account>::ErrorType &e) {
        FAIL() << *e.error;
      });
}

/**
 * @given field values which pass stateless validation
 * @when AccountBuilder is invoked twice
 * @then Two identical (==) Account objects are constructed
 */
TEST(AccountBuilderTest, SeveralObjectsFromOneBuilder) {
  shared_model::builder::AccountBuilder<
      shared_model::proto::AccountBuilder,
      shared_model::validation::FieldValidator>
      builder;

  auto valid_account_id = "name@domain";
  auto valid_domain_id = "america";
  auto valid_quorum = 3;
  auto valid_json_data = "{}";

  auto state = builder.accountId(valid_account_id)
                   .domainId(valid_domain_id)
                   .quorum(valid_quorum)
                   .jsonData(valid_json_data);

  auto account = state.build();
  auto account2 = state.build();

  testResultObjects(account, account2, [](auto &a, auto &b) {
    // pointer points to different objects
    ASSERT_TRUE(a != b);

    EXPECT_EQ(a->accountId(), b->accountId());
    EXPECT_EQ(a->domainId(), b->domainId());
    EXPECT_EQ(a->quorum(), b->quorum());
    EXPECT_EQ(a->jsonData(), b->jsonData());
  });
}
