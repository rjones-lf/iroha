/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include <gtest/gtest.h>

#include "builders/protobuf/common_objects/proto_account_builder.hpp"

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
