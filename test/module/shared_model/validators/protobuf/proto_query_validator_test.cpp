/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "validators/protobuf/proto_query_validator.hpp"

#include <gmock/gmock-matchers.h>
#include "module/shared_model/validators/validators_fixture.hpp"
#include "queries.pb.h"

using testing::HasSubstr;

class ProtoQueryValidatorTest : public ValidatorsTest {
 public:
  shared_model::validation::ProtoQueryValidator validator;
};

/**
 * @given Protobuf query object with unset query
 * @when validate is called
 * @then there is an error returned
 */
TEST_F(ProtoQueryValidatorTest, UnsetQuery) {
  iroha::protocol::Query qry;
  qry.mutable_payload()->mutable_meta()->set_created_time(created_time);
  qry.mutable_payload()->mutable_meta()->set_creator_account_id(account_id);
  qry.mutable_payload()->mutable_meta()->set_query_counter(counter);

  auto answer = validator.validate(qry);
  ASSERT_TRUE(answer.hasErrors());
  ASSERT_THAT(answer.reason(), HasSubstr("undefined"));
}

/**
 * @given well-formed protobuf query object
 * @when validated is called
 * @then validation is passed
 */
TEST_F(ProtoQueryValidatorTest, SetQuery) {
  iroha::protocol::Query qry;
  qry.mutable_payload()->mutable_get_account()->set_account_id(account_id);

  auto answer = validator.validate(qry);
  ASSERT_FALSE(answer.hasErrors());
}
