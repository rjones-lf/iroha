/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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
#include <memory>
#include "cryptography/crypto_provider/crypto_signer.hpp"
#include "cryptography/crypto_provider/crypto_verifier.hpp"
#include "cryptography/ed25519_sha3_impl/crypto_provider.hpp"

using namespace shared_model::crypto;

class CryptoInitialization : public ::testing::Test {
 public:
  void SetUp() override {
    keypair =
        std::make_shared<Keypair>(CryptoProviderEd25519Sha3::generateKeypair());
    data = std::make_shared<Blob>("raw data for signing");
  }
  std::shared_ptr<Blob> data;
  std::shared_ptr<Keypair> keypair;
};

/**
 * @given Initialized keypiar with _concrete_ algorithm
 * @when sign date without knowledge of cryptography algorithm
 * @then check that siganture valid without clarification of algorithm
 */
TEST_F(CryptoInitialization, RawSignAndVerifyTest) {
  auto signed_blob = CryptoSigner<>::sign(*data, *keypair);
  auto verified =
      CryptoVerifier<>::verify(signed_blob, *data, keypair->publicKey());
  ASSERT_TRUE(verified);
}
