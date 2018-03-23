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
      std::string(64, 'g'));, std::runtime_error);
  ASSERT_THROW(shared_model::bindings::ModelCrypto().fromPrivateKey(
      std::string(63, 'a'));, std::runtime_error);
  ASSERT_THROW(shared_model::bindings::ModelCrypto().fromPrivateKey(
      std::string(65, 'a'));, std::runtime_error);
  ASSERT_THROW(shared_model::bindings::ModelCrypto().fromPrivateKey(
      std::string(32, 'a'));, std::invalid_argument);
}
