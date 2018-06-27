/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "module/shared_model/validators/validators_fixture.hpp"

#include "builders/protobuf/queries.hpp"

class QueryValidatorTest : public ValidatorsTest {
 public:
  shared_model::validation::DefaultQueryValidator query_validator;
};

using namespace shared_model;

/**
 * @given Protobuf query object
 * @when Each query type created with valid fields
 * @then Answer has no errors
 */
TEST_F(QueryValidatorTest, StatelessValidTest) {
  iroha::protocol::Query qry;
  auto *meta = new iroha::protocol::QueryPayloadMeta();
  meta->set_created_time(created_time);
  meta->set_creator_account_id(account_id);
  meta->set_query_counter(counter);
  qry.mutable_payload()->set_allocated_meta(meta);
  auto payload = qry.mutable_payload();

  // Iterate through all query types, filling query fields with valid values
  iterateContainer(
      [] {
        return iroha::protocol::Query::Payload::descriptor()->FindOneofByName(
            "query");
      },
      [&](auto field) {
        // Set concrete type for new query
        return payload->GetReflection()->MutableMessage(payload, field);
      },
      [this](auto field, auto query) {
        // Will throw key exception in case new field is added
        try {
          field_setters.at(field->name())(query->GetReflection(), query, field);
        } catch (const std::out_of_range &e) {
          FAIL() << "Missing field setter: " << field->name();
        }
      },
      [&] {
        auto result = proto::Query(iroha::protocol::Query(qry));
        auto answer = query_validator.validate(result);

        ASSERT_FALSE(answer.hasErrors()) << answer.reason();
      });
}

/**
 * @given Protobuf query object
 * @when Query has no fields set, and each query type has no fields set
 * @then Answer contains error
 */
TEST_F(QueryValidatorTest, StatelessInvalidTest) {
  iroha::protocol::Query qry;
  auto payload = qry.mutable_payload();

  // create queries from default constructors, which will have empty, therefore
  // invalid values
  iterateContainer(
      [] {
        return iroha::protocol::Query::Payload::descriptor()->FindOneofByName(
            "query");
      },
      [&](auto field) {
        // Set concrete type for new query
        return payload->GetReflection()->MutableMessage(payload, field);
      },
      [](auto, auto) {
        // Note that no fields are set
      },
      [&] {
        auto result = proto::Query(iroha::protocol::Query(qry));
        auto answer = query_validator.validate(result);

        ASSERT_TRUE(answer.hasErrors());
      });
}
