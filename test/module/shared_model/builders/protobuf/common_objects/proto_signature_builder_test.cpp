/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "module/shared_model/builders/protobuf/common_objects/proto_signature_builder.hpp"

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
