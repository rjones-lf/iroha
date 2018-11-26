/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "validators/protobuf/proto_query_validator.hpp"
#include "module/shared_model/validators/validators_fixture.hpp"
#include "queries.pb.h"

class ProtoQueryValidatorTest : public ValidatorsTest {
 public:
  shared_model::validation::ProtoQueryValidator validator;
};

/**
 * @given Protobuf query object with unset query
 * @when validate is called
 * @then there is a error returned
 */
TEST_F(ProtoQueryValidatorTest, UnsetQuery) {
  iroha::protocol::Query qry;
  qry.mutable_payload()->mutable_meta()->set_created_time(created_time);
  qry.mutable_payload()->mutable_meta()->set_creator_account_id(account_id);
  qry.mutable_payload()->mutable_meta()->set_query_counter(counter);

  auto answer = validator.validate(qry);
  ASSERT_TRUE(answer.hasErrors());
}
