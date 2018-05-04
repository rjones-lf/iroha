/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_signature_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

/**
 * @given Two signatures with same pub key but different signed
 * @when  Invoke operator==
 * @then  Expect true
 */
TEST(SecuritySignature, SignatureOperatorEqual) {
  auto first_signature =
      TestSignatureBuilder()
          .publicKey(shared_model::crypto::PublicKey("one"))
          .signedData(shared_model::crypto::Signed("signed_one"))
          .build();

  auto second_signature =
      TestSignatureBuilder()
          .publicKey(shared_model::crypto::PublicKey("one"))
          .signedData(shared_model::crypto::Signed("signed_two"))
          .build();

  ASSERT_TRUE(first_signature == second_signature);
}

/**
 * @given Transaction with given signature
 * @when  Invoke ::addSignature with same public key but different signed
 * @then  Expect that second signature wasn't added
 */
TEST(SecuritySignature, TransactionAddsignature) {
  auto tx = TestTransactionBuilder().build();
  ASSERT_TRUE(tx.addSignature(shared_model::crypto::Signed("sign_one"),
                              shared_model::crypto::PublicKey("key_one")));
  ASSERT_FALSE(tx.addSignature(shared_model::crypto::Signed("sign_two"),
                               shared_model::crypto::PublicKey("key_one")));
}

/**
 * @given Block with given signature
 * @when  Invoke ::addSignature with same public key but different signed
 * @then  Expect that second signature wasn't added
 */
TEST(SecuritySignature, BlockAddSignature) {
  auto block = TestBlockBuilder().build();
  ASSERT_TRUE(block.addSignature(shared_model::crypto::Signed("sign_one"),
                                 shared_model::crypto::PublicKey("key_one")));
  ASSERT_FALSE(block.addSignature(shared_model::crypto::Signed("sign_two"),
                                  shared_model::crypto::PublicKey("key_one")));
}
