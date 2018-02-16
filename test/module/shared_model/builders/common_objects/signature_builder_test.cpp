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

#include "builders/common_objects/signature_builder.hpp"
#include "builders/protobuf/common_objects/proto_signature_builder.hpp"
#include "validators/field_validator.hpp"

// TODO: 14.02.2018 nickaleks mock builder implementation IR-970
// TODO: 14.02.2018 nickaleks mock field validator IR-971

TEST(PeerBuilderTest, StatelessValidAddressCreation) {

  shared_model::builder::SignatureBuilder<shared_model::proto::SignatureBuilder,
                                     shared_model::validation::FieldValidator>
      builder;

  shared_model::interface::types::PubkeyType expected_key(std::string(32, '1'));
  shared_model::interface::Signature::SignedType expected_signed("signed object");

  auto signature = builder.publicKey(expected_key).signedData(expected_signed)
      .build();

  signature.match(
      [&](shared_model::builder::BuilderResult<shared_model::interface::Signature>::ValueType &v) {
        EXPECT_EQ(v.value->publicKey(), expected_key);
        EXPECT_EQ(v.value->signedData(), expected_signed);
      },
      [](shared_model::builder::BuilderResult<shared_model::interface::Signature>::ErrorType &e) { FAIL() << *e.error; });
}
