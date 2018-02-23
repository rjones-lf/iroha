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

#include "builders/protobuf/common_objects/proto_signature_builder.hpp"

/**
 * @given fields for Signature object
 * @when SignatureBuilder is invoked
 * @then Signature object is successfully constructed and has the same fields as
 * provided
 */
TEST(ProtoSignatureBuilderTest, AllFieldsBuild) {
  shared_model::proto::SignatureBuilder builder;

  shared_model::interface::types::PubkeyType expected_key(std::string(32, '1'));
  shared_model::interface::Signature::SignedType expected_signed(
      "signed object");

  auto signature =
      builder.publicKey(expected_key).signedData(expected_signed).build();

  EXPECT_EQ(signature.publicKey(), expected_key);
  EXPECT_EQ(signature.signedData(), expected_signed);
}

/**
 * @given fields for Signature object
 * @when SignatureBuilder is invoked twice with the same configuration
 * @then Two constructed Signature objects are identical
 */
TEST(ProtoSignatureBuilderTest, SeveralObjectsFromOneBuilder) {
  shared_model::proto::SignatureBuilder builder;

  shared_model::interface::types::PubkeyType expected_key(std::string(32, '1'));
  shared_model::interface::Signature::SignedType expected_signed(
      "signed object");

  auto state = builder.publicKey(expected_key).signedData(expected_signed);

  auto signature = state.build();
  auto signature2 = state.build();

  EXPECT_EQ(signature.publicKey(), signature2.publicKey());
  EXPECT_EQ(signature.signedData(), signature2.signedData());
}
