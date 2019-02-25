/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bindings/model_crypto.hpp"

#include <gtest/gtest.h>

/**
 * @given ModelCrypto module
 * @when Receive 32 byte hex string
 * @then assertion is not thrown on keypair generation
 */
TEST(ModelCryptoTest, GenerateKeypair) {
  ASSERT_NO_THROW(shared_model::bindings::ModelCrypto().fromPrivateKey(
      std::string(64, 'a')););
  ASSERT_NO_THROW(shared_model::bindings::ModelCrypto().fromPrivateKey(
      std::string(64, 'A')););
}
/**
 * @given ModelCrypto module
 * @when Receive invalid hex byte string
 * @then assertion is thrown
 */
TEST(ModelCryptoTest, GenerateKeypairInvalidSeed) {
  ASSERT_THROW(shared_model::bindings::ModelCrypto().fromPrivateKey(
      std::string(64, 'g'));, std::invalid_argument);
  ASSERT_THROW(shared_model::bindings::ModelCrypto().fromPrivateKey(
      std::string(63, 'a'));, std::invalid_argument);
  ASSERT_THROW(shared_model::bindings::ModelCrypto().fromPrivateKey(
      std::string(65, 'a'));, std::invalid_argument);
  ASSERT_THROW(shared_model::bindings::ModelCrypto().fromPrivateKey(
      std::string(32, 'a'));, std::invalid_argument);
}
