/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "builders_test_fixture.hpp"
#include "module/shared_model/builders/common_objects/signature_builder.hpp"
#include "module/shared_model/builders/protobuf/common_objects/proto_signature_builder.hpp"
#include "validators/field_validator.hpp"

// TODO: 14.02.2018 nickaleks mock builder implementation IR-970
// TODO: 14.02.2018 nickaleks mock field validator IR-971

/**
 * @given field values which pass stateless validation
 * @when PeerBuilder is invoked
 * @then Peer object is successfully constructed and has valid fields
 */
TEST(PeerBuilderTest, StatelessValidAddressCreation) {
  shared_model::builder::SignatureBuilder<
      shared_model::proto::SignatureBuilder,
      shared_model::validation::FieldValidator>
      builder;

  shared_model::interface::types::PubkeyType expected_key(std::string(32, '1'));
  shared_model::interface::Signature::SignedType expected_signed(
      "signed object");

  auto signature =
      builder.publicKey(expected_key).signedData(expected_signed).build();

  signature.match(
      [&](shared_model::builder::BuilderResult<
          shared_model::interface::Signature>::ValueType &v) {
        EXPECT_EQ(v.value->publicKey(), expected_key);
        EXPECT_EQ(v.value->signedData(), expected_signed);
      },
      [](shared_model::builder::BuilderResult<
          shared_model::interface::Signature>::ErrorType &e) {
        FAIL() << *e.error;
      });
}

/**
 * @given field values which pass stateless validation
 * @when SignatureBuilder is invoked twice
 * @then Two identical (==) Signature objects are constructed
 */
TEST(SignatureBuilderTest, SeveralObjectsFromOneBuilder) {
  shared_model::builder::SignatureBuilder<
      shared_model::proto::SignatureBuilder,
      shared_model::validation::FieldValidator>
      builder;

  shared_model::interface::types::PubkeyType expected_key(std::string(32, '1'));
  shared_model::interface::Signature::SignedType expected_signed(
      "signed object");

  auto state = builder.publicKey(expected_key).signedData(expected_signed);
  auto signature = state.build();
  auto signature2 = state.build();

  testResultObjects(signature, signature2, [](auto &a, auto &b) {
    // pointer points to different objects
    ASSERT_TRUE(a != b);

    EXPECT_EQ(a->publicKey(), b->publicKey());
    EXPECT_EQ(a->signedData(), b->signedData());
  });
}
