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

#include "builders/common_objects/account_builder.hpp"
#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "validators/field_validator.hpp"

// TODO: 14.02.2018 nickaleks mock builder implementation IR-970
// TODO: 14.02.2018 nickaleks mock field validator IR-971

TEST(AccountBuilderTest, StatelessValidAllFields) {

  shared_model::builder::AccountBuilder<shared_model::proto::AccountBuilder,
                                     shared_model::validation::FieldValidator>
      builder;

  auto valid_account_id = "name@domain";
  auto valid_domain_id = "america";
  auto valid_quorum = 3;
  auto valid_json_data = "{}";

  auto account = builder.accountId(valid_account_id).domainId(valid_domain_id).quorum(valid_quorum).jsonData(valid_json_data).build();

  account.match(
      [&](shared_model::builder::BuilderResult<shared_model::interface::Account>::ValueType &v) {
        EXPECT_EQ(v.value->accountId(), valid_account_id);
        EXPECT_EQ(v.value->domainId(), valid_domain_id);
        EXPECT_EQ(v.value->quorum(), valid_quorum);
        EXPECT_EQ(v.value->jsonData(), valid_json_data);
      },
      [](shared_model::builder::BuilderResult<shared_model::interface::Account>::ErrorType &e) { FAIL() << *e.error; });
}
