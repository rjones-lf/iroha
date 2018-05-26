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

#include "backend/protobuf/query_responses/proto_query_response.hpp"

#include <gtest/gtest.h>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/irange.hpp>
#include "cryptography/hash.hpp"
#include "interfaces/utils/specified_visitor.hpp"

/**
 * @given protobuf's QueryResponse with different responses and some hash
 * @when converting to shared model
 * @then ensure that status and hash remain the same
 */
TEST(QueryResponse, QueryResponseLoad) {
  iroha::protocol::QueryResponse response;
  const std::string hash = "123";
  response.set_query_hash(iroha::bytestringToHexstring(hash));
  auto refl = response.GetReflection();
  auto desc = response.GetDescriptor();
  auto resp_status = desc->FindOneofByName("response");
  ASSERT_NE(nullptr, resp_status);

  boost::for_each(boost::irange(0, resp_status->field_count()), [&](auto i) {
    auto field = desc->field(i);
    refl->SetAllocatedMessage(
        &response, refl->GetMessage(response, field).New(), field);
    auto shared_response = shared_model::proto::QueryResponse(response);
    ASSERT_EQ(i, shared_response.get().which());
    ASSERT_EQ(shared_response.queryHash(), shared_model::crypto::Hash(hash));
  });
}

/**
 * @given protobuf's ErrorResponse with different reasons and some hash
 * @when converting to shared model
 * @then ensure that reason and hash remain the same
 */
TEST(QueryResponse, ErrorResponseLoad) {
  iroha::protocol::QueryResponse response;
  const std::string hash = "123";
  response.set_query_hash(iroha::bytestringToHexstring(hash));
  auto error_resp = response.mutable_error_response();
  auto refl = error_resp->GetReflection();
  auto desc = error_resp->GetDescriptor();
  auto resp_reason = desc->FindFieldByName("reason");
  ASSERT_NE(nullptr, resp_reason);
  auto resp_reason_enum = resp_reason->enum_type();
  ASSERT_NE(nullptr, resp_reason_enum);

  boost::for_each(
      boost::irange(0, resp_reason_enum->value_count()), [&](auto i) {
        refl->SetEnumValue(
            error_resp, resp_reason, resp_reason_enum->value(i)->number());
        auto shared_response = shared_model::proto::QueryResponse(response);
        ASSERT_EQ(i,
                  boost::apply_visitor(
                      shared_model::interface::SpecifiedVisitor<
                          shared_model::interface::ErrorQueryResponse>(),
                      shared_response.get())
                      ->get()
                      .which());
        ASSERT_EQ(shared_response.queryHash(),
                  shared_model::crypto::Hash(hash));
      });
}
