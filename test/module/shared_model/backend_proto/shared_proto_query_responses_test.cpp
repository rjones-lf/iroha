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

/**
 * @given protobuf's QueryResponse with different tx_statuses and some hash
 * @when converting to shared model
 * @then ensure that status and hash remain the same
 */
TEST(QueryResponse, QueryResponseLoad) {
  iroha::protocol::QueryResponse response;
  const std::string hash = "123";
  response.set_query_hash(hash);
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
