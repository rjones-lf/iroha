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

#include "backend/protobuf/commands/proto_command.hpp"

#include <gtest/gtest.h>

#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/irange.hpp>
#include <boost/variant.hpp>

/**
 * For each protobuf command type
 * @given protobuf command object
 * @when create shared model command object
 * @then corresponding shared model object is created
 */
TEST(ProtoCommand, CommandLoad) {
  iroha::protocol::Command command;
  auto refl = command.GetReflection();
  auto desc = command.GetDescriptor();
  boost::for_each(boost::irange(0, desc->field_count()), [&](auto i) {
    auto field = desc->field(i);
    refl->SetAllocatedMessage(
        &command, refl->GetMessage(command, field).New(), field);
    ASSERT_EQ(i, shared_model::proto::Command(command).get().which());
  });
}
