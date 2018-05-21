/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "builders/protobuf/common_objects/proto_signature_builder.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "validation/utils.hpp"

using namespace iroha::validation;
using namespace shared_model::crypto;

class SignaturesSubset : public testing::Test {
 public:
  auto makeSignature(PublicKey key, std::string sign) {
    return shared_model::proto::SignatureBuilder()
        .publicKey(key)
        .signedData(Signed(sign))
        .build();
  }
};

/**
 * @given three different keys and three signatures with the same keys
 * @when signaturesSubset is executed
 * @then returned true
 */
TEST_F(SignaturesSubset, Equal) {
  std::vector<PublicKey> keys{PublicKey("a"), PublicKey("b"), PublicKey("c")};
  std::vector<shared_model::proto::Signature> signatures;
  for (const auto &k : keys) {
    signatures.push_back(makeSignature(k, ""));
  }
  ASSERT_TRUE(signaturesSubset(signatures, keys));
}

/**
 * @given two different keys and two signatures with the same keys plus
 * additional one
 * @when signaturesSubset is executed
 * @then returned false
 */
TEST_F(SignaturesSubset, Lesser) {
  std::vector<PublicKey> keys{PublicKey("a"), PublicKey("b")};
  std::vector<shared_model::proto::Signature> signatures;
  for (const auto &k : keys) {
    signatures.push_back(makeSignature(k, ""));
  }
  signatures.push_back(makeSignature(PublicKey("c"), ""));
  ASSERT_FALSE(signaturesSubset(signatures, keys));
}

/**
 * @given three different keys and two signatures with the first pair of keys
 * @when signaturesSubset is executed
 * @then returned true
 */
TEST_F(SignaturesSubset, StrictSubset) {
  std::vector<PublicKey> keys{PublicKey("a"), PublicKey("b")};
  std::vector<shared_model::proto::Signature> signatures;
  for (const auto &k : keys) {
    signatures.push_back(makeSignature(k, ""));
  }
  keys.push_back(PublicKey("c"));
  ASSERT_TRUE(signaturesSubset(signatures, keys));
}

/**
 * @given two same keys and two signatures with different keys
 * @when signaturesSubset is executed
 * @then returned false
 */
TEST_F(SignaturesSubset, PublickeyUniqueness) {
  std::vector<PublicKey> keys{PublicKey("a"), PublicKey("a")};
  std::vector<shared_model::proto::Signature> signatures;
  signatures.push_back(makeSignature(PublicKey("a"), ""));
  signatures.push_back(makeSignature(PublicKey("c"), ""));
  ASSERT_FALSE(signaturesSubset(signatures, keys));
}
