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

#include "backend/protobuf/queries/proto_query.hpp"

#include <gtest/gtest.h>

#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/irange.hpp>

/**
 * For each protobuf query type
 * @given protobuf query object
 * @when create shared model query object
 * @then corresponding shared model object is created
 */

TEST(ProtoQuery, QueryLoad) {
  iroha::protocol::Query query;
  auto refl = query.GetReflection();
  auto desc = query.GetDescriptor();
  boost::for_each(
      // TODO 03/12/17 dumitru replace 1 with desc->field_count()
      boost::irange(0, 2),
      [&](auto i) {
        auto field = desc->field(i);
        refl->SetAllocatedMessage(
            &query, refl->GetMessage(query, field).New(), field);
        ASSERT_EQ(i, shared_model::proto::Query(query).get().which());
      });
}
